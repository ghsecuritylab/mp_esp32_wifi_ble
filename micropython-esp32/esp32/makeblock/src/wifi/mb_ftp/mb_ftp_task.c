/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for makeblock mb_ftp_task module
 * @file    mb_ftp_task.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/04/07
 *
 * \par Copyright
 * This software is Copyright (C), 2012-2016, MakeBlock. Use is subject to license \n
 * conditions. The main licensing options available are GPL V2 or Commercial: \n
 *
 * \par Open Source Licensing GPL V2
 * This is the appropriate option if you want to share the source code of your \n
 * application with everyone you distribute it to, and you also want to give them \n
 * the right to share who uses it. If you wish to use this software under Open \n
 * Source Licensing, you must contribute all your source code to the open source \n
 * community in accordance with the GPL Version 2 when your application is \n
 * distributed. See http://www.gnu.org/copyleft/gpl.html
 *
 * \par Description
 * This file is a drive ftp_task module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 *  fftust            2017/04/07      1.0.0            build the new.
 * </pre>
 *
 */
	
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp32_mphal.h"

#include "nvs_flash.h"
		
#include "py/mpstate.h"
#include "py/runtime.h"
#include "py/nlr.h"
#include "py/objexcept.h"
#include "py/obj.h"

#include "mb_ftp/mb_ftp.h"
#include "mb_ftp/mb_ftp_task.h"

/******************************************************************************
 MACRO DEFINITION
 ******************************************************************************/

/******************************************************************************
 DECLARE CONSTANTS
 ******************************************************************************/

/******************************************************************************
 DEFINE TYPES
 ******************************************************************************/
typedef struct
{
  uint32_t timeout;
  bool enabled;
  bool do_disable;
  bool do_enable;
  bool do_reset;
  bool do_wlan_cycle_power;
} mb_ftp_data_t;

 mb_ftp_data_t mb_ftp_data;

 static volatile bool mb_sleep_sockets = false;
 
/******************************************************************************/

void mb_ftp_task(void *pvParameters) 
{  
  mb_ftp_init();
  mb_ftp_task_start();
  
  for ( ; ; ) 
  {
    if(mb_ftp_data.do_enable)
	{
      mb_ftp_enable();
      mb_ftp_data.enabled = true;
      mb_ftp_data.do_enable = false;
    }
    else if(mb_ftp_data.do_disable )
	{
      //ftp_disable();
      mb_ftp_data.do_disable = false;
      mb_ftp_data.enabled = false;
    }
    else if (mb_ftp_data.do_reset)
	{
      mb_ftp_data.do_reset = false;
      if (mb_ftp_data.enabled) 
	  {
        //mb_ftp_reset();
      }

    }
  
      mb_ftp_run();
  
  /*
  if (mb_sleep_sockets) 
  {
    mp_hal_delay_ms(FTP_TASK_CYCLE_TIME_MS * 2);
    if (mb_ftp_data.do_wlan_cycle_power)
    {
      mb_ftp_data.do_wlan_cycle_power = false;
    }
    mb_sleep_sockets = false;
  }
  */

  vTaskDelay (FTP_TASK_CYCLE_TIME_MS / portTICK_PERIOD_MS);
  }
}

void mb_ftp_task_start(void) 
{
  mb_ftp_data.do_enable = true;
  mp_hal_delay_ms(FTP_TASK_CYCLE_TIME_MS * 3);
}

void mb_ftp_task_stop (void) 
{
  mb_ftp_data.do_disable = true;
  do {
       mp_hal_delay_ms(FTP_TASK_CYCLE_TIME_MS);
     } while (mb_ftp_task_are_enabled());
  mp_hal_delay_ms(FTP_TASK_CYCLE_TIME_MS * 3);
}

void mb_ftp_task_reset (void) 
{
  mb_ftp_data.do_reset = true;
}

void mb_ftp_wlan_cycle_power (void) 
{
  mb_ftp_data.do_wlan_cycle_power = true;
}

bool mb_ftp_task_are_enabled (void) 
{
  return mb_ftp_data.enabled;
}

void mb_ftp_task_sleep_sockets (void) 
{
  mb_sleep_sockets = true;
  mp_hal_delay_ms(FTP_TASK_CYCLE_TIME_MS + 1);
}



void mb_ftp_task_set_login (char *user, char *pass)
{
  if (strlen(user) > FTP_TASK_USER_PASS_LEN_MAX || strlen(pass) > FTP_TASK_USER_PASS_LEN_MAX)
  {
    //nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, mpexception_value_invalid_arguments));
  }
  memcpy(mb_ftp_user, user, FTP_TASK_USER_PASS_LEN_MAX);
  memcpy(mb_ftp_pass, pass, FTP_TASK_USER_PASS_LEN_MAX);
}

void mb_ftp_task_set_timeout (uint32_t timeout) 
{
  if (timeout < FTP_TASK_MIN_TIMEOUT_MS)
  {
    // timeout is too low
   // nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, mpexception_value_invalid_arguments));
  }
  mb_ftp_data.timeout = timeout;
}

uint32_t mb_ftp_task_get_timeout (void)
{
  return mb_ftp_data.timeout;
}





























