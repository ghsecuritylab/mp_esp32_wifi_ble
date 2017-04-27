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

#include "tcpip_adapter.h"
#include "lwip/sockets.h"

#include "esp_task.h"
	
#include "py/mpstate.h"
#include "py/runtime.h"


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

#include "mb_wifi_test.h"

#define MB_WIFI_SSID "ESP32_MB"
#define MB_WIFI_PASSWORD "12345678"
#define MB_WIFI_LENTHOFPASS 8
#define MB_EAP_USERNAME "Maker-office"
#define MB_EAP_PASSWORD "hulurobot423"
#define TEST_TAG "mb_wifiexample"


/******************************************************************************
 DEFINE PRIVATE CONSTANTS
 ******************************************************************************/



volatile bool mb_test_wifi_connected=false;  //fftus:the flag of whether the wlan is connected

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t mb_test_wifi_event_group;

const int TEST_CONNECTED_BIT = BIT0;


static esp_err_t mb_test_event_handler(void *ctx, system_event_t *event)
{
    
	switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
	    mb_test_wifi_connected=true;
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(mb_test_wifi_event_group, TEST_CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(mb_test_wifi_event_group, TEST_CONNECTED_BIT);
		mb_test_wifi_connected=false;
        break;


	case SYSTEM_EVENT_AP_START:	
		 break;
	case SYSTEM_EVENT_AP_STACONNECTED:
         printf("fftust: a sta connect this ap\n");
		 break;
	case SYSTEM_EVENT_AP_STADISCONNECTED:
         printf("fftust: a sta disconnect this ap\n");
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

void mb_initialise_wifi(void)
{
  tcpip_adapter_init();  //ffftust:tcpip config
  
  mb_test_wifi_event_group = xEventGroupCreate(); 
  ESP_ERROR_CHECK(esp_event_loop_init(mb_test_event_handler, NULL) );
  
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
  ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) ); //fftust: store the config in the RAM 
  wifi_config_t wifi_config_ap = {
  .ap={
	   .ssid=MB_WIFI_SSID,
	   .password=MB_WIFI_PASSWORD,
	   .ssid_len=strlen(MB_WIFI_PASSWORD), //MB_WIFI_LENTHOFPASS
	   .authmode=WIFI_AUTH_WPA2_PSK,
	   .channel=6,
	   .max_connection=2,
	   .beacon_interval=100,

	  },	  
  };
  ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_APSTA) ); 
  ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config_ap) );
  wifi_config_t wifi_config_sta = {
  .sta={
       .ssid=MB_EAP_USERNAME,
       .password=MB_EAP_PASSWORD,
  	  },	  
  };
  ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config_sta) );
	
  ESP_ERROR_CHECK( esp_wifi_start() );
}



/******************************************************************************
 DEFINE TYPES
 ******************************************************************************/
typedef struct
{
  mp_obj_base_t base;
} mb_wifi_test_obj_t;

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/
STATIC mb_wifi_test_obj_t mb_wifi_test_obj = {};

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/


/******************************************************************************/

STATIC mp_obj_t mb_wifi_test_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
  // parse args
  mp_map_t kw_args;
  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);

  mb_wifi_test_config();   //fftust:GPIO config
  // setup the object
  mb_wifi_test_obj_t *self = &mb_wifi_test_obj;
  self->base.type = &mb_wifi_test_type;
  return self;
}

STATIC void mb_wifi_test_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{

}

void mb_wifi_test_config()  
{  
  //mb_initialise_wifi();
}


	
STATIC mp_obj_t mb_wifi_test_run(mp_uint_t n_args, const mp_obj_t *args)
{
  tcpip_adapter_ip_info_t ip;

  //mb_wifi_test_obj_t *self=args[0];
  memset(&ip, 0, sizeof(tcpip_adapter_ip_info_t));
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  if (tcpip_adapter_get_ip_info(ESP_IF_WIFI_STA, &ip) == 0)
  {
    ESP_LOGI(TEST_TAG, "~~~~~~~~~~~");
    ESP_LOGI(TEST_TAG, "IP:"IPSTR, IP2STR(&ip.ip));
    ESP_LOGI(TEST_TAG, "MASK:"IPSTR, IP2STR(&ip.netmask));
    ESP_LOGI(TEST_TAG, "GW:"IPSTR, IP2STR(&ip.gw));
    ESP_LOGI(TEST_TAG, "~~~~~~~~~~~");
  }
  //char *strff="{"
  //	          "\"name\":\"fftust\",\"age\":10"
  //	          "}";
  //printf("makeblock:%s\n",strff);
  //cJSON * root = cJSON_Parse(strff);
  //cJSON_Print(root);
  //cJSON_Delete(root);
  
  vTaskDelay(100/portTICK_PERIOD_MS);
  pyexec_frozen_module("makeblocktest.py");
  return mp_const_none;

}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mb_wifi_test_run_obj,1, 1, mb_wifi_test_run);
//MP_DEFINE_CONST_FUN_OBJ_3(mb_potention_meter_board_value_obj, mb_potention_meter_board_value);


STATIC mp_obj_t mb_wifi_test_call(mp_obj_t self_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args)
{
  mp_arg_check_num(n_args, n_kw, 0, 0, false);
  return mb_wifi_test_run(n_args,args);
}

STATIC const mp_map_elem_t mb_wifi_test_locals_dict_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_run), 			  (mp_obj_t)&mb_wifi_test_run_obj },
};

STATIC MP_DEFINE_CONST_DICT(mb_wifi_test_locals_dict, mb_wifi_test_locals_dict_table);

const mp_obj_type_t mb_wifi_test_type =
{
  { &mp_type_type },
  .name = MP_QSTR_wifi_test,
  .print = mb_wifi_test_print,
  .call = mb_wifi_test_call,
  .make_new = mb_wifi_test_make_new,
  .locals_dict = (mp_obj_t)&mb_wifi_test_locals_dict,
};




