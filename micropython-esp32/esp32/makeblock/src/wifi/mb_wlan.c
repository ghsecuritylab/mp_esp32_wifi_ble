/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for makeblock wlan module
 * @file    mb_wlan.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/04/024
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
 * This file is a drive wlan module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 *  fftust            2017/04/24        1.0.0            build the new.
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
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp32_mphal.h"

#include "nvs_flash.h"
		
#include "py/mpstate.h"
#include "py/runtime.h"
#include "py/nlr.h"
#include "py/objexcept.h"
#include "py/obj.h"

#include "tcpip_adapter.h"	
#include "mb_ftp/socketfifo.h"	
#include "mb_ftp/mb_ftp.h"
#include "mb_ftp_task.h"

	
/******************************************************************************
 MACRO DEFINITION
 ******************************************************************************/

/******************************************************************************
 DECLARE CONSTANTS
 ******************************************************************************/
const char *mb_wifi_ap_ssid="Makeblock_ESP32";
const char *mb_wifi_ap_password="12345678";

const char *mb_wifi_sta_ssid="Maker-office";
const char *mb_wifi_sta_password="hulurobot423";
/******************************************************************************
 DEFINE TYPES
 ******************************************************************************/
typedef struct
{
  uint8_t ssid[32];
  uint8_t password[32];
}mb_wifi_ap_config_t;

typedef struct
{
  uint8_t ssid[32];
  uint8_t password[32];

}mb_wifi_ap_config_t;

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/
 
STATIC mb_wifi_ap_config_t mb_wifi_ap_config = {.ssid=mb_wifi_ap_ssid,.password=mb_wifi_ap_password};
STATIC mb_wifi_ap_config_t mb_wifi_ap_config = {.ssid=mb_wifi_ap_ssid,.password=mb_wifi_ap_password};

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/


/******************************************************************************/

