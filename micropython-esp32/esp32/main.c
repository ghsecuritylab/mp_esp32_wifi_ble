/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * Development of the code in this file was sponsored by Microbric Pty Ltd
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_task.h"

#include "soc/cpu.h"


#include "ff.h"
#include "diskio.h"
#include "ffconf.h"
#include "mb_fatfs/pybflash.h"
#include "mb_fatfs/drivers/sflash_diskio.h"
#include "extmod/vfs_fat.h"

#include "py/stackctrl.h"
#include "py/nlr.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/gc.h"
#include "py/mphal.h"
#include "lib/mp-readline/readline.h"
#include "lib/utils/pyexec.h"
#include "uart.h"

#include "mb_ftp/mb_ftp_task.h"
#include "mb_ftp/mb_ftp.h"
#include "modmachine.h"
#include "mptask.h"
#include "mb_makeblock.h"
#include "mb_sys.h"

#include "mpthreadport.h"


extern void process_serial_data(void *pvParameters); 

// MicroPython runs as a task under FreeRTOS
#define MP_TASK_PRIORITY        (ESP_TASK_PRIO_MIN + 1)

#define MP_TASK_STACK_SIZE      (16 * 1024)
#define MP_TASK_STACK_LEN       (MP_TASK_STACK_SIZE / sizeof(StackType_t))
#define MP_TASK_HEAP_SIZE       (96 * 1024)

STATIC StaticTask_t mp_task_tcb;
STATIC StackType_t mp_task_stack[MP_TASK_STACK_LEN] __attribute__((aligned (8)));
STATIC uint8_t mp_task_heap[MP_TASK_HEAP_SIZE];



/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/
static fs_user_mount_t *mb_sflash_vfs_fat;

static char fresh_main_py[] = "# main.py -- put your code here!\r\n"
                              "from makeblock import dcmotor\r\n";
static char fresh_boot_py[] = "# boot.py -- run on boot-up\r\n"
                              "import os\r\n"
                              "from makeblock import dcmotor\r\n";


STATIC void sensor_updata(void *pvParameters)
{
  static unsigned long  lastTime = 0;
  while(1)
  {
    if(gyro_board_enabled() == true)
    {
      if((millis() - lastTime) > 19)
      {
        lastTime = millis();
        gyro_board_update();
      }
    }
	vTaskDelay(100/ portTICK_PERIOD_MS);
  }
}


STATIC void mptask_create_main_py (void)
{
  // create empty main.py
  FIL fp;
  f_chdir (&mb_sflash_vfs_fat->fatfs,"/flash");
  f_open(&mb_sflash_vfs_fat->fatfs, &fp, "main.py", FA_WRITE | FA_CREATE_ALWAYS);
  UINT n;
  f_write(&fp, fresh_main_py, sizeof(fresh_main_py) - 1 /* don't count null terminator */, &n);
  f_close(&fp);
}

STATIC void mptask_init_sflash_filesystem (void)
{
  FILINFO fno;
  mb_sflash_vfs_fat = pvPortMalloc(sizeof(*mb_sflash_vfs_fat));
    
  fs_user_mount_t *vfs_fat = mb_sflash_vfs_fat;
  vfs_fat->flags = 0;
  pyb_flash_init_vfs(vfs_fat);
  FRESULT res = f_mount(&vfs_fat->fatfs);

  if (res == FR_NO_FILESYSTEM)
  {
    uint8_t working_buf[_MAX_SS];
    res=f_mkfs(&vfs_fat->fatfs, FM_FAT | FM_SFD, 0, working_buf, sizeof(working_buf));
	f_mount(&mb_sflash_vfs_fat->fatfs);
	if (FR_OK != f_chdir (&vfs_fat->fatfs,"/flash")) 
	{
      printf("create flash! 2\r\n");
	  f_mkdir(&vfs_fat->fatfs,"/flash");
	}
	else
	{
	  printf("flash find! 2\r\n");
	}

    if (res != FR_OK)
	{
      printf("no file sys,failed to create flash\n");
    }
    mptask_create_main_py();
  }
  else if (res == FR_OK)
  {
    f_chdir(&mb_sflash_vfs_fat->fatfs,"/flash");
    if (FR_OK != f_stat(&mb_sflash_vfs_fat->fatfs,"main.py", &fno))
	{
      mptask_create_main_py();
    }
  }
  else
  {
    printf("failed to create flash,res:%d\n",res);
  }


  
  mp_vfs_mount_t *vfs = m_new_obj_maybe(mp_vfs_mount_t);
  if (vfs == NULL) 
  {
    printf("makeblock:fatal error");
  }


  vfs->str = "/flash";
  vfs->len = 6;
  vfs->obj = MP_OBJ_FROM_PTR(vfs_fat);
  vfs->next = NULL;
  MP_STATE_VM(vfs_mount_table) = vfs;
  MP_STATE_PORT(vfs_cur) = vfs;  // The current directory is used as the boot up directory.  // It is set to the internal flash filesystem by default.
  

  if (FR_OK != f_chdir (&vfs_fat->fatfs,"/sys")) 
  {
    f_mkdir(&vfs_fat->fatfs,"/sys");
  }
  if (FR_OK != f_chdir (&vfs_fat->fatfs,"/lib")) 
  {
    f_mkdir(&vfs_fat->fatfs,"/lib");
  }
  if (FR_OK != f_chdir (&vfs_fat->fatfs,"/cert"))
  {
    f_mkdir(&vfs_fat->fatfs,"/cert");
  }
	
  
	
  char mb_read_buffer[128];
  memset(mb_read_buffer,'X',128);
  
  f_chdir(&mb_sflash_vfs_fat->fatfs,"/flash");
  if (FR_OK != f_stat(&mb_sflash_vfs_fat->fatfs,"boot.py", &fno)) 
  {
    FIL fp1;
    UINT n1;

    f_chdir (&mb_sflash_vfs_fat->fatfs,"/flash");
    f_open(&mb_sflash_vfs_fat->fatfs,&fp1, "boot.py", FA_WRITE | FA_CREATE_ALWAYS);
    f_write(&fp1, fresh_boot_py, sizeof(fresh_boot_py) - 1 /* don't count null terminator */, &n1);
    f_close(&fp1);
	printf("boot.py can't find,created a new one\n");
  }
  else
  {   
    FIL fp2;
    UINT n2;
    f_chdir (&mb_sflash_vfs_fat->fatfs,"/flash");
    f_open(&mb_sflash_vfs_fat->fatfs,&fp2, "boot.py", FA_READ); //fftust: can not add FA_CREATE_ALWAYS
    f_read(&fp2, mb_read_buffer, sizeof(fresh_boot_py)-1 /* don't count null terminator */, &n2);
    f_close(&fp2);
    mb_read_buffer[sizeof(fresh_boot_py)-1] = '\n';
    mb_read_buffer[sizeof(fresh_boot_py)] = '\0';
	printf(mb_read_buffer);
  }

  f_chdir (&mb_sflash_vfs_fat->fatfs,"/flash");
  if (FR_OK != f_stat(&mb_sflash_vfs_fat->fatfs,"main.py", &fno)) 
  {
    mptask_create_main_py();
	printf("main.py can't find,created a new one!\n");
  }
  else
  {   
    FIL fp3;
    UINT n3;
    f_chdir (&mb_sflash_vfs_fat->fatfs,"/flash");
    f_open(&mb_sflash_vfs_fat->fatfs,&fp3, "main.py", FA_READ); //fftust: can not add FA_CREATE_ALWAYS
    f_read(&fp3, mb_read_buffer, sizeof(fresh_main_py)-1 , &n3);
    f_close(&fp3);
    mb_read_buffer[sizeof(fresh_main_py)-1] = '\n';
    mb_read_buffer[sizeof(fresh_main_py)] = '\0';
	printf(mb_read_buffer);
  }
  
}



void mp_task(void *pvParameter) {
    volatile uint32_t sp = (uint32_t)get_sp();
    #if MICROPY_PY_THREAD
    mp_thread_init(&mp_task_stack[0], MP_TASK_STACK_LEN);
    #endif
    uart_init();
soft_reset:
    // initialise the stack pointer for the main thread
    mp_stack_set_top((void *)sp);
    mp_stack_set_limit(MP_TASK_STACK_SIZE - 1024);
    gc_init(mp_task_heap, mp_task_heap + sizeof(mp_task_heap));
    mp_init();
    mp_obj_list_init(mp_sys_path, 0);
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR_));
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_flash_slash_lib));
    mp_obj_list_init(mp_sys_argv, 0);
    readline_init0();
    // initialise peripherals
    machine_pins_init();
    mptask_init_sflash_filesystem();
	////pyexec_frozen_module("_boot.py");
	pyexec_frozen_module("makeblockclass.py");
	pyexec_frozen_module("mb_callback.py");
	pyexec_frozen_module("mb_factory.py");
	pyexec_frozen_module("thread_start1.py");
    communication_channel_init();
    // run boot-up scripts
    //pyexec_frozen_module("mb_cbconfig.py");
    //pyexec_file("boot.py");
	//pyexec_file("main.py");
    if (pyexec_mode_kind == PYEXEC_MODE_FRIENDLY_REPL) {
        pyexec_file("main.py");
    }

    for (;;) {
        if (pyexec_mode_kind == PYEXEC_MODE_RAW_REPL) {
            if (pyexec_raw_repl() != 0) {
				break;
            }
        }else if(pyexec_mode_kind == PYEXEC_MODE_FRIENDLY_REPL){
            if (pyexec_friendly_repl() != 0) {
				break;
             }
        }else{
            pyexec_pure_cmd_repl();
        }
    }

    #if MICROPY_PY_THREAD
    mp_thread_deinit();
    #endif

    mp_hal_stdout_tx_str("PYB: soft reboot\r\n");

    // deinitialise peripherals
    machine_pins_deinit();

    mp_deinit();
    fflush(stdout);
    goto soft_reset;
}

STATIC void pure_cmd_task(void *pvParameter)
{
  while(1)
  {
    pyexec_pure_cmd_repl();
	vTaskDelay(20/portTICK_PERIOD_MS);
  }
}
void app_main(void)
{
  nvs_flash_init();
  // TODO use xTaskCreateStatic (needs custom FreeRTOSConfig.h)
  //    xTaskCreateStaticPinnedToCore(mp_task, "mp_task", MP_TASK_STACK_LEN, NULL, MP_TASK_PRIORITY,
  //                                &mp_task_stack[0], &mp_task_tcb, 0);
  xTaskCreatePinnedToCore(mp_task, "mp_task", MP_TASK_STACK_LEN, NULL, MP_TASK_PRIORITY, NULL, 0);
  xTaskCreatePinnedToCore(process_serial_data, "process_serial_data", MP_TASK_STACK_LEN, NULL, MP_TASK_PRIORITY, NULL, 0);
  xTaskCreatePinnedToCore(sensor_updata, "sensor_updata", MP_TASK_STACK_LEN, NULL, MP_TASK_PRIORITY, NULL, 0); 
  xTaskCreatePinnedToCore(mb_ftp_task, "mb_ftp_task", MP_TASK_STACK_LEN, NULL, MP_TASK_PRIORITY, NULL, 0);
  //xTaskCreatePinnedToCore(pure_cmd_task, "pure_cmd_task", MP_TASK_STACK_LEN, NULL, MP_TASK_PRIORITY, NULL, 0);
}

void nlr_jump_fail(void *val) {
    printf("NLR jump failed, val=%p\n", val);
    esp_restart();
}

// modussl_mbedtls uses this function but it's not enabled in ESP IDF
void mbedtls_debug_set_threshold(int threshold) {
    (void)threshold;
}
