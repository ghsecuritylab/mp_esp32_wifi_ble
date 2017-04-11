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

#include "ff.h"
#include "diskio.h"
#include "ffconf.h"

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

#include "modmachine.h"
#include "mptask.h"
#include "makeblock/mb_makeblock.h"
#include "makeblock/mb_sys.h"

extern void process_serial_data(void *pvParameters); 



// MicroPython runs as a task under FreeRTOS
#define MP_TASK2_PRIORITY       (ESP_TASK_PRIO_MIN + 2)
#define MP_TASK_PRIORITY        (ESP_TASK_PRIO_MIN + 1)
#define MP_TASK_STACK_SIZE      (16 * 1024)
#define MP_TASK_STACK_LEN       (MP_TASK_STACK_SIZE / sizeof(StackType_t))
#define MP_TASK_HEAP_SIZE       (96 * 1024)

//STATIC StaticTask_t mp_task_tcb;
//STATIC StackType_t mp_task_stack[MP_TASK_STACK_LEN] __attribute__((aligned (8)));
STATIC uint8_t mp_task_heap[MP_TASK_HEAP_SIZE];

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/

static char fresh_main_py[] = "# main.py -- put your code here!\r\n";
static char fresh_boot_py[] = "# boot.py -- run on boot-up\r\n"
                              "import os\r\n"
                              "from machine import UART\r\n"
                              "uart = UART(0, 115200)\r\n"
                              "os.dupterm(uart)\r\n";


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
  }
}


STATIC void mptask_create_main_py (void) {
    // create empty main.py
    FIL fp;
    f_open(&sflash_fatfs,&fp, "/flash/main.py", FA_WRITE | FA_CREATE_ALWAYS);
    UINT n;
    f_write(&fp, fresh_main_py, sizeof(fresh_main_py) - 1 /* don't count null terminator */, &n);
    f_close(&fp);
}

STATIC void mptask_init_sflash_filesystem (void) {
    FILINFO fno;

    // Initialise the local flash filesystem.
    // Create it if needed, and mount in on /flash.
    FRESULT res = f_mount(&sflash_fatfs);
    if (res == FR_NO_FILESYSTEM) {
        // no filesystem, so create a fresh one
        res = f_mkfs(&sflash_fatfs, FM_SFD | FM_FAT, 0, NULL, 0);
        if (res != FR_OK) {
            printf("no file sys,failed to create flash\n");
        }
        // create empty main.py
        mptask_create_main_py();
    }
    else if (res == FR_OK) {
        // mount sucessful
        if (FR_OK != f_stat(&sflash_fatfs,"/flash/main.py", &fno)) {
            // create empty main.py
            mptask_create_main_py();
        }
    } else {
        printf("failed to create flash,res:%d\n",res);
    }

    // create /flash/sys, /flash/lib and /flash/cert if they don't exist
    if (FR_OK != f_chdir (&sflash_fatfs,"/flash/sys")) {
        f_mkdir(&sflash_fatfs,"/flash/sys");
    }
    if (FR_OK != f_chdir (&sflash_fatfs,"/flash/lib")) {
        f_mkdir(&sflash_fatfs,"/flash/lib");
    }
    if (FR_OK != f_chdir (&sflash_fatfs,"/flash/cert")) {
        f_mkdir(&sflash_fatfs,"flash/cert");
    }

    f_chdir (&sflash_fatfs,"/flash");

    // make sure we have a /flash/boot.py. Create it if needed.
    res = f_stat(&sflash_fatfs,"/flash/boot.py", &fno);
    if (res == FR_OK) {
        if (fno.fattrib & AM_DIR) {
            // exists as a directory
            // TODO handle this case
            // see http://elm-chan.org/fsw/ff/img/app2.c for a "rm -rf" implementation
        } else {
            // exists as a file, good!
        }
    } else {
        // doesn't exist, create fresh file
        FIL fp;
        f_open(&sflash_fatfs,&fp, "/flash/boot.py", FA_WRITE | FA_CREATE_ALWAYS);
        UINT n;
        f_write(&fp, fresh_boot_py, sizeof(fresh_boot_py) - 1 /* don't count null terminator */, &n);
        // TODO check we could write n bytes
        f_close(&fp);
    }
}


void mp_task(void *pvParameter) {
    uart_init();
soft_reset:
    mp_stack_set_top((void*)&pvParameter);
    mp_stack_set_limit(MP_TASK_STACK_SIZE - 512);
    gc_init(mp_task_heap, mp_task_heap + sizeof(mp_task_heap));
    mp_init();
    mp_obj_list_init(mp_sys_path, 0);
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR_));
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_flash_slash_lib));
    mp_obj_list_init(mp_sys_argv, 0);
    readline_init0();
    // initialise peripherals
    machine_pins_init();
    //mptask_init_sflash_filesystem();
    communication_channel_init();

    // run boot-up scripts
    pyexec_frozen_module("_boot.py");
    pyexec_file("boot.py");
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

    mp_hal_stdout_tx_str("PYB: soft reboot\r\n");

    // deinitialise peripherals
    machine_pins_deinit();

    mp_deinit();
    fflush(stdout);
    goto soft_reset;
}

void app_main(void) {
    nvs_flash_init();
    // TODO use xTaskCreateStatic (needs custom FreeRTOSConfig.h)
    xTaskCreatePinnedToCore(mp_task, "mp_task", MP_TASK_STACK_LEN, NULL, MP_TASK_PRIORITY, NULL, 0);
    xTaskCreatePinnedToCore(process_serial_data, "process_serial_data", MP_TASK_STACK_LEN, NULL, MP_TASK_PRIORITY, NULL, 0);
	xTaskCreatePinnedToCore(sensor_updata, "sensor_updata", MP_TASK_STACK_LEN, NULL, MP_TASK_PRIORITY, NULL, 0);
}

void nlr_jump_fail(void *val) {
    printf("NLR jump failed, val=%p\n", val);
    for (;;) {
    }
}

// modussl_mbedtls uses this function but it's not enabled in ESP IDF
void mbedtls_debug_set_threshold(int threshold) {
    (void)threshold;
}
