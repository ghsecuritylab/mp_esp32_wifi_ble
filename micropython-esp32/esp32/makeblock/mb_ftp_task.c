/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for makeblock mb_ftp_task module
 * @file    mb_flame_sensor.c
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
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp32_mphal.h"

	
#include "tcpip_adapter.h"
#include "lwip/sockets.h"
	
#include "mb_ftp/socketfifo.h"

	
#include "esp_task.h"
		
#include "py/mpstate.h"
#include "py/runtime.h"
#include "py/nlr.h"
#include "py/objexcept.h"
#include "py/obj.h"
	
	
#include "mb_ftp/mb_ftp.h"
#include "mb_ftp_task.h"



/******************************************************************************
 MACRO DEFINITION
 ******************************************************************************/
#define MB_WIFI_SSID "ESP32_MB"
#define MB_WIFI_PASSWORD "12345678"
#define MB_WIFI_LENTHOFPASS 8

#define MB_EAP_METHOD 1  //be useed in the mode of STA
#define MB_EAP_ID "ESP"
#define MB_EAP_USERNAME "fftustff"
#define MB_EAP_PASSWORD "19920112"
/******************************************************************************
 DECLARE CONSTANTS
 ******************************************************************************/
 const int CONNECTED_BIT = BIT0;

/******************************************************************************
 DEFINE TYPES
 ******************************************************************************/
 typedef struct {
    uint32_t timeout;
    bool enabled;
    bool do_disable;
    bool do_enable;
    bool do_reset;
    bool do_wlan_cycle_power;
} mb_ftp_data_t;

 mb_ftp_data_t mb_ftp_data;

 static EventGroupHandle_t mb_wifi_event_group;
 
 static volatile bool mb_sleep_sockets = false;
 
 volatile bool mb_wifi_connected=false;  //fftus:the flag of whether the wlan is connected

/******************************************************************************/
static esp_err_t mb_event_handler(void *ctx, system_event_t *event)
{  
  switch(event->event_id) 
  {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
    	printf("fftust:start"); //fftust add 
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(mb_wifi_event_group, CONNECTED_BIT);
    	printf("fftust:got ip"); //fftust add 
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(mb_wifi_event_group, CONNECTED_BIT);
    	printf("fftust:disc"); //fftust add 
        break;
    
    
    case SYSTEM_EVENT_AP_START:	
    	 break;
    case SYSTEM_EVENT_AP_STACONNECTED:
         printf("fftust: a sta connect this ap\n");
    	 mb_wifi_connected=true;
    	 break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
         printf("fftust: a sta disconnect this ap\n");
    	 mb_wifi_connected=false;
    	 break;		
    case SYSTEM_EVENT_AP_PROBEREQRECVED:
    	 printf("fftust: Receive probe request packet\n");
    	 break;	 
    default:
    	 printf("fftust: wifistatus default\n");
         break;
  }
  return ESP_OK;
}

STATIC void mb_wifi_init(void)
{
  tcpip_adapter_init();  //ffftust:tcpip config
  
  mb_wifi_event_group = xEventGroupCreate(); 
  ESP_ERROR_CHECK(esp_event_loop_init(mb_event_handler, NULL) );
  
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
  ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) ); //fftust: store the config in the RAM 
  wifi_config_t wifi_config = {
  .ap={
	   .ssid=MB_WIFI_SSID,
	   .password=MB_WIFI_PASSWORD,
	   .ssid_len=MB_WIFI_LENTHOFPASS,
	   .authmode=WIFI_AUTH_WPA2_PSK,
	   .channel=6,
	   .max_connection=2,
	   .beacon_interval=100,

	  },
  };
  ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_AP) );
  ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config) );
  ESP_ERROR_CHECK( esp_wifi_start() );
	
}


void mb_ftp_task(void *pvParameters) 
{  
  mb_wifi_init(); 
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


#define TASK_Servers (void *pvParameters)             mb_ftp_task(void *pvParameters)
#define servers_start (void)                          mb_ftp_task_start(void)
#define servers_stop (void)                           mb_ftp_task_stop(void)
#define servers_reset (void)                          mb_ftp_task_reset(void)
#define servers_wlan_cycle_power (void)               mb_ftp_wlan_cycle_power(void)
#define servers_are_enabled (void)                    mb_ftp_task_are_enabled(void)
#define servers_close_socket (int16_t *sd)            mb_ftp_task_close_socket(void)
#define servers_set_login (char *user, char *pass)    mb_ftp_task_set_login(char *user, char *pass)
#define server_sleep_sockets (void)                   mb_ftp_task_sleep_sockets(void)
#define servers_set_timeout (uint32_t timeout)        mb_ftp_task_set_timeout(uint32_t timeout)
#define servers_get_timeout (void)                    mb_ftp_task_get_timeout(void)
#define servers_user                                  mb_ftp_user
#define servers_pass                                  mb_ftp_pass


























