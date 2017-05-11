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
#include "mb_wlan.h"
/******************************************************************************
 MACRO DEFINITION
 ******************************************************************************/

/******************************************************************************
 DECLARE CONSTANTS
 ******************************************************************************/
const char *mb_wifi_ap_ssid_default="Makeblock_ESP32";
const char *mb_wifi_ap_password_default="12345678";

const char *mb_wifi_sta_ssid_default="Maker-office";
const char *mb_wifi_sta_password_default="hulurobot423";

const int MB_CONNECTED_BIT = BIT0;

/******************************************************************************
 DEFINE TYPES
 ******************************************************************************/
typedef struct
{
  mp_obj_base_t base;
  char ssid[32];
  char pass[32]; 
} mb_wlan_obj_t;

typedef struct
{
  volatile bool     wifi_enabled_flag;
  volatile bool     wifi_ap_connected_flag;  
  volatile bool     wifi_sta_connected_flag;
  volatile bool     wifi_sta_auto_connect_flag;
  volatile bool     wifi_loopevent_set_flag;
  wifi_mode_t       wifi_cur_mode;
  wifi_config_t     wifi_ap_cur_config;
  wifi_config_t     wifi_sta_cur_config;
  uint8_t           wifi_ap_mac[6];
  uint8_t           wifi_sta_mac[6];
}mb_wifi_info_t;

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t mb_wifi_event_group;	

wifi_config_t wifi_cfg_ap;
wifi_config_t wifi_cfg_sta;

static mb_wlan_obj_t   mb_wifi_ap_cfgpara  = {};
static mb_wlan_obj_t   mb_wifi_sta_cfgpara = {};
static mb_wifi_info_t  mb_wifi_info={.wifi_enabled_flag=false ,
	                                 .wifi_ap_connected_flag=false ,
	                                 .wifi_sta_connected_flag=false,
	                                 .wifi_sta_auto_connect_flag=true,
	                                 .wifi_loopevent_set_flag=false,
	                                 .wifi_cur_mode=WIFI_MODE_NULL};

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/

void mb_wifi_enable(void);
void mb_wifi_disenable(void);
bool mb_wifi_get_ap_status();
bool mb_wifi_get_sta_status();
void mb_wifi_pre_init(void);

STATIC esp_err_t mb_wifi_event_handler(void *ctx, system_event_t *event);
STATIC void mb_wifi_ap_config();
STATIC void mb_wifi_sta_config();
STATIC void mb_wifi_start_apmode_only();
STATIC void mb_wifi_start_stamode_only();
STATIC void mb_wifi_start_apstamode();
STATIC uint8_t* mb_wifi_get_mac(wifi_interface_t ifx);
STATIC void mb_wifi_scan(uint16_t ap_num, wifi_ap_record_t *ap_record);



void mb_wifi_pre_init(void)
{
  tcpip_adapter_init();
  strcpy (mb_wifi_ap_cfgpara.ssid, mb_wifi_ap_ssid_default);
  strcpy (mb_wifi_ap_cfgpara.pass,mb_wifi_ap_password_default);
  strcpy (mb_wifi_sta_cfgpara.ssid, mb_wifi_sta_ssid_default);
  strcpy (mb_wifi_sta_cfgpara.pass,mb_wifi_sta_password_default);
  
}

void mb_wifi_enable(void)
{
  if(mb_wifi_info.wifi_enabled_flag)
  {
    //printf("makeblock the wifi is enabled")
  }
  else
  {
	mb_wifi_pre_init();
    if(!mb_wifi_info.wifi_loopevent_set_flag)
    {
      mb_wifi_event_group = xEventGroupCreate(); 
      ESP_ERROR_CHECK(esp_event_loop_init(mb_wifi_event_handler, NULL) );
      mb_wifi_info.wifi_loopevent_set_flag=true;
	}
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) ); 
	
	mb_wifi_info.wifi_enabled_flag=true;
  }
  
}

void mb_wifi_disenable(void)
{
  esp_wifi_stop();
  esp_wifi_set_mode(WIFI_MODE_NULL);
  esp_wifi_deinit();

  mb_wifi_info.wifi_enabled_flag=false;
  mb_wifi_info.wifi_enabled_flag=false ,
  mb_wifi_info.wifi_ap_connected_flag=false ,
  mb_wifi_info.wifi_sta_connected_flag=false,
  mb_wifi_info.wifi_sta_auto_connect_flag=false,
  mb_wifi_info.wifi_cur_mode=WIFI_MODE_NULL;
  
}

bool mb_wifi_get_ap_status()
{
  return mb_wifi_info.wifi_ap_connected_flag;  
}

bool mb_wifi_get_sta_status()
{
  return mb_wifi_info.wifi_sta_connected_flag;
}


STATIC esp_err_t mb_wifi_event_handler(void *ctx, system_event_t *event)
{
  switch(event->event_id)
  {
    case SYSTEM_EVENT_STA_START:
		 if(mb_wifi_info.wifi_sta_auto_connect_flag)
         {
           esp_wifi_connect();
           mb_wifi_info.wifi_sta_connected_flag=true;
		 }
		 break;
    case SYSTEM_EVENT_STA_GOT_IP:
         xEventGroupSetBits(mb_wifi_event_group, MB_CONNECTED_BIT);
         break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
		 if(mb_wifi_info.wifi_sta_auto_connect_flag)
         {
           esp_wifi_connect();
  	       mb_wifi_info.wifi_sta_connected_flag=false;
		 }
		 xEventGroupClearBits(mb_wifi_event_group, MB_CONNECTED_BIT);
         break;

    case SYSTEM_EVENT_AP_START:	
  	     break;
    case SYSTEM_EVENT_AP_STACONNECTED:
		 mb_wifi_info.wifi_ap_connected_flag=true;
         printf("fftust: a sta connect this ap\n");
  	     break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
		 mb_wifi_info.wifi_ap_connected_flag=false;
         printf("fftust: a sta disconnect this ap\n");
  	     break;		
    case SYSTEM_EVENT_AP_PROBEREQRECVED:
  	     printf("fftust: Receive probe request packet\n");
  	     break;	 
    default:
  	     //printf("fftust: wifistatus default\n");
         break;
  }
  return ESP_OK;
  
}

STATIC void mb_wifi_ap_config(void)
{
  strcpy((char*)wifi_cfg_ap.ap.ssid,mb_wifi_ap_cfgpara.ssid);
  strcpy((char*)wifi_cfg_ap.ap.password,mb_wifi_ap_cfgpara.pass);
  wifi_cfg_ap.ap.ssid_len=strlen(mb_wifi_ap_cfgpara.ssid); 
  wifi_cfg_ap.ap.authmode=WIFI_AUTH_WPA2_PSK;
  wifi_cfg_ap.ap.channel=6;
  wifi_cfg_ap.ap.max_connection=2;
  wifi_cfg_ap.ap.beacon_interval=100;

}

STATIC void mb_wifi_sta_config(void)
{
  strcpy((char*)wifi_cfg_sta.sta.ssid,mb_wifi_sta_cfgpara.ssid);
  strcpy((char*)wifi_cfg_sta.sta.password,mb_wifi_sta_cfgpara.pass);

}

STATIC void mb_wifi_start_apmode_only()
{
  //ESP_ERROR_CHECK(esp_wifi_deinit());
  
  //wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  //ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
  //ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) ); 
  
  //ESP_ERROR_CHECK( esp_wifi_stop());
  //ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_AP) ); 
  
  ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_cfg_ap) );	
  ESP_ERROR_CHECK( esp_wifi_start() ); 

  ESP_ERROR_CHECK( esp_wifi_clear_fast_connect() );

  mb_wifi_info.wifi_cur_mode=WIFI_MODE_AP;


}

STATIC void mb_wifi_start_stamode_only()
{

  //ESP_ERROR_CHECK(esp_wifi_deinit());
  
  //wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  //ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
  //ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) ); 

  //ESP_ERROR_CHECK( esp_wifi_stop());
  //ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) ); 
  
  ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_cfg_sta) );	
  ESP_ERROR_CHECK( esp_wifi_start() ); 

  mb_wifi_info.wifi_cur_mode=WIFI_MODE_STA;
}

STATIC void mb_wifi_start_apstamode()
{
  //ESP_ERROR_CHECK(esp_wifi_deinit());
  
  //wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  //ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
  //ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) ); //fftust: store the config in the RAM 

  //ESP_ERROR_CHECK(esp_wifi_stop());
  //ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_APSTA) ); 
  
  ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_cfg_ap) );
  ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_cfg_sta));
	
  ESP_ERROR_CHECK( esp_wifi_start() );
  
  mb_wifi_info.wifi_cur_mode=WIFI_MODE_APSTA;


}

STATIC uint8_t* mb_wifi_get_mac(wifi_interface_t ifx)
{
 if(ifx==ESP_IF_WIFI_STA)
 {
   esp_wifi_get_mac(ifx,mb_wifi_info.wifi_sta_mac);
   return mb_wifi_info.wifi_sta_mac;
 }
 else if(ifx==ESP_IF_WIFI_AP)
 {
   esp_wifi_get_mac(ifx,mb_wifi_info.wifi_ap_mac);
   return mb_wifi_info.wifi_ap_mac;
 }
 else
 {
   //printf("makeblock:para wrong\n");
 }
 return NULL;
}

STATIC wifi_config_t* mb_wifi_get_config(wifi_interface_t wifi_interface)
{

  if(wifi_interface==ESP_IF_WIFI_STA)
  {
    esp_wifi_get_config(wifi_interface,&mb_wifi_info.wifi_sta_cur_config);
	return &mb_wifi_info.wifi_sta_cur_config;
  }
  else if(wifi_interface==ESP_IF_WIFI_AP)
  {
    esp_wifi_get_config(wifi_interface,&mb_wifi_info.wifi_sta_cur_config);
	return &mb_wifi_info.wifi_sta_cur_config;
  }
  else
  {
    return NULL;
	//printf("makeblock:para error\n");
  }
  
}


STATIC  void mb_wifi_scan(uint16_t ap_num, wifi_ap_record_t *ap_record) 
{
  if (mb_wifi_info.wifi_cur_mode == WIFI_MODE_AP)
  {
    nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "the requested operation is not possible"));
  }

  esp_wifi_scan_start(NULL, true);

  if (ap_record == NULL)
  {
    return;
  }
  
  if (ESP_OK == esp_wifi_scan_get_ap_records(&ap_num, (wifi_ap_record_t *)ap_record)) 
  {
	//ap_record = ap_record_buffer;
  }

  esp_wifi_scan_stop();
}


/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
 
STATIC mp_obj_t mb_wlan_enable(mp_obj_t self_in)
{
  mb_wifi_enable();
  return mp_const_none; 
}
MP_DEFINE_CONST_FUN_OBJ_1(mb_wlan_enable_obj,mb_wlan_enable);


STATIC mp_obj_t mb_wlan_set_mode(mp_obj_t self_in, mp_obj_t mode)
{
  wifi_mode_t wifimode;
  //mb_wlan_obj_t *self = self_in;
  wifimode=mp_obj_get_int(mode);
  ESP_ERROR_CHECK( esp_wifi_stop());
  if(wifimode<WIFI_MODE_MAX)
  {
    switch(wifimode)
    {
      case WIFI_MODE_NULL:
	       esp_wifi_set_mode(WIFI_MODE_NULL);
           break;
      case WIFI_MODE_STA:
	       esp_wifi_set_mode(WIFI_MODE_STA);
           break;
      case WIFI_MODE_AP:
	       esp_wifi_set_mode(WIFI_MODE_AP);
           break;
      case WIFI_MODE_APSTA:
	       esp_wifi_set_mode(WIFI_MODE_APSTA);
           break;
	  default:
	  	   break;
	}
  }
  return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(mb_wlan_set_mode_obj,mb_wlan_set_mode);


STATIC mp_obj_t mb_wlan_get_mode(mp_obj_t self_in)
{
  wifi_mode_t wifimode;
  //mb_wlan_obj_t *self = self_in;
  esp_wifi_get_mode(&wifimode);
  mb_wifi_info.wifi_cur_mode=wifimode;
  printf("makeblock:%d\n",wifimode);
  return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(mb_wlan_get_mode_obj,mb_wlan_get_mode);


STATIC mp_obj_t mb_wlan_set_ap(mp_uint_t n_args, const mp_obj_t *args)
{
  mb_wlan_obj_t *self = args[0];
  if (n_args == 1) 
  {
    printf("makeblock:the default ssid and password is set\n");
	return mp_obj_new_str((const char *)self->ssid, strlen((const char *)self->ssid), false);
  } 
  else 
  {
    if(n_args>=2)
	{
	  size_t len;
      const char *ssid = mp_obj_str_get_data(args[1], &len);
	  if(len>32)
	  {
	    //printf("makeblock:lenth of ssid is too long");
 	  }
	  else
	  {
        strcpy((char*)mb_wifi_ap_cfgpara.ssid,ssid);
	    printf("you have changed the ap ssid to :%s\n",(char*)mb_wifi_ap_cfgpara.ssid);
	  } 
    }
	else if(n_args>=3)
	{
      size_t len;
	  const char *pword = mp_obj_str_get_data(args[2], &len);
	  if(len>32)
	  {
	    //printf("makeblock:lenth of password is too long");
	  }
	  else
	  {
	    strcpy((char*)mb_wifi_ap_cfgpara.pass,pword);
	    printf("you have changed the ap password to :%s\n",(char*)mb_wifi_ap_cfgpara.pass);
	  } 
	}
  }	
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mb_wlan_set_ap_obj,1, 3, mb_wlan_set_ap);


STATIC mp_obj_t mb_wlan_set_sta(mp_uint_t n_args, const mp_obj_t *args)
{
  mb_wlan_obj_t *self = args[0];
  if (n_args == 1) 
  {
    printf("makeblock:the default ssid and password is set\n");
	return mp_obj_new_str((const char *)self->ssid, strlen((const char *)self->ssid), false);
  } 
  else 
  {
    if(n_args>=2)
	{
	  size_t len;
      const char *ssid = mp_obj_str_get_data(args[1], &len);
	  if(len>32)
	  {
	    //printf("makeblock:lenth of ssid is too long");
 	  }
	  else
	  {
        strcpy((char*)mb_wifi_sta_cfgpara.ssid,ssid);
	    printf("you have changed the ap ssid to :%s\n",(char*)mb_wifi_sta_cfgpara.ssid);
	  } 
    }
	else if(n_args>=3)
	{
      size_t len;
	  const char *pword = mp_obj_str_get_data(args[2], &len);
	  if(len>32)
	  {
	    //printf("makeblock:lenth of password is too long");
	  }
	  else
	  {
	    strcpy((char*)mb_wifi_sta_cfgpara.pass,pword);
	    printf("you have changed the ap password to :%s\n",(char*)mb_wifi_sta_cfgpara.pass);
	  } 
	}
  }	
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mb_wlan_set_sta_obj,1, 3, mb_wlan_set_sta);


STATIC mp_obj_t mb_wlan_scan(mp_obj_t self_in) 
{
  uint16_t apnum=0;
  wifi_ap_record_t* ap_rec_buffer=NULL;
  wifi_ap_record_t* ap_rec=NULL;
  esp_wifi_scan_start(NULL, true);
  esp_wifi_scan_get_ap_num(&apnum);
  esp_wifi_scan_stop(); 
  ap_rec_buffer= pvPortMalloc(apnum * sizeof(wifi_ap_record_t));
  if(ap_rec_buffer==NULL)
  {
	return mp_const_none;
  }
  else
  {
    mb_wifi_scan(apnum,ap_rec_buffer);
	ap_rec=ap_rec_buffer;
    for (int i = 0; i < apnum; i++)
    {
	  printf("%s\n",(char*)ap_rec->ssid);
	  ap_rec++;
	  
    }
    vPortFree(ap_rec_buffer);
  }
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mb_wlan_scan_obj, mb_wlan_scan);


STATIC mp_obj_t mb_wlan_set_auto_connect(mp_obj_t self_in ,mp_obj_t en)
{
  uint8_t autocon=0;
  //mb_wlan_obj_t *self = self_in;
  autocon=mp_obj_get_int(en);
  if(autocon==0)
  {
    mb_wifi_info.wifi_sta_auto_connect_flag=false;
  }
  else
  {
    mb_wifi_info.wifi_sta_auto_connect_flag=true;
  }
  esp_wifi_set_auto_connect((bool)autocon);
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mb_wlan_set_auto_connect_obj, mb_wlan_set_auto_connect);


STATIC mp_obj_t mb_wlan_connect(mp_obj_t self_in) 
{
  esp_wifi_connect();
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mb_wlan_connect_obj, mb_wlan_connect);

STATIC mp_obj_t mb_wlan_disconnect(mp_obj_t self_in) 
{
  esp_wifi_disconnect();
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mb_wlan_disconnect_obj, mb_wlan_disconnect);


STATIC mp_obj_t mb_wlan_start(mp_obj_t self_in, mp_obj_t type )
{
  uint8_t starttype;
  //mb_wlan_obj_t *self = self_in;
  starttype=mp_obj_get_int(type);
  switch(starttype)
  {
    case 0:
	  break;
	case 1:
	  mb_wifi_sta_config();
	  mb_wifi_start_stamode_only();
	  break;
    case 2:
	  mb_wifi_ap_config();
      mb_wifi_start_apmode_only();
	  break;
    case 3:
	  mb_wifi_sta_config();
	  mb_wifi_ap_config();
	  mb_wifi_start_apstamode();
	  break;
	default:
	  break;
  }
  return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(mb_wlan_start_obj,mb_wlan_start);


STATIC mp_obj_t mb_wlan_stop(mp_obj_t self_in )
{
  mb_wifi_disenable();
  return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(mb_wlan_stop_obj,mb_wlan_stop);


STATIC mp_obj_t mb_wlan_deinit(mp_obj_t self_in)
{
  esp_wifi_stop();
  esp_wifi_set_mode(WIFI_MODE_NULL);
  esp_wifi_deinit();

  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mb_wlan_deinit_obj, mb_wlan_deinit);


STATIC mp_obj_t mb_wlan_get_config(mp_obj_t self_in,mp_obj_t interface)
{
  wifi_interface_t wifi_interface;
  //mb_wlan_obj_t *self = self_in;
  wifi_interface=mp_obj_get_int(interface);
  
  if(wifi_interface<ESP_IF_MAX)
  {
    mb_wifi_get_config(wifi_interface);
  }
  return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(mb_wlan_get_config_obj,mb_wlan_get_config);


STATIC mp_obj_t mb_wlan_get_mac(mp_obj_t self_in ,mp_obj_t ifx)
{
 uint8_t* mac=NULL;
 //mb_wlan_obj_t *self = self_in;
 wifi_interface_t wifi_ifx=mp_obj_get_int(ifx);
 mac=mb_wifi_get_mac(wifi_ifx);
 printf("makeblock:mac address:\n");
 for(uint8_t i=0;i<6;i++)
 {
   printf("%x ",mac[i]);
 }
 printf("\n");
 return mp_obj_new_bytes(mac,6);
 //return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mb_wlan_get_mac_obj, mb_wlan_get_mac);




STATIC mp_obj_t mb_wlan_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
  // parse args
  mp_map_t kw_args;
  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);

  // setup the object
  mb_wlan_obj_t *self = &mb_wifi_ap_cfgpara;
  self->base.type = &mb_wlan_type;
  self=&mb_wifi_sta_cfgpara;
  self->base.type = &mb_wlan_type;
  return self;
}


STATIC void mb_wlan_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{

}

STATIC const mp_map_elem_t mb_wlan_locals_dict_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_enable),                (mp_obj_t)&mb_wlan_enable_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_set_ap),				(mp_obj_t)&mb_wlan_set_ap_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_set_sta),				(mp_obj_t)&mb_wlan_set_sta_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_start),				    (mp_obj_t)&mb_wlan_start_obj },	
  { MP_OBJ_NEW_QSTR(MP_QSTR_stop),				    (mp_obj_t)&mb_wlan_stop_obj },	
  { MP_OBJ_NEW_QSTR(MP_QSTR_deinit),				(mp_obj_t)&mb_wlan_deinit_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_scan),				    (mp_obj_t)&mb_wlan_scan_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_get_mac),				(mp_obj_t)&mb_wlan_get_mac_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_set_auto_connect),		(mp_obj_t)&mb_wlan_set_auto_connect_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_connect),		        (mp_obj_t)&mb_wlan_connect_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_set_mode),		        (mp_obj_t)&mb_wlan_set_mode_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_get_mode),		        (mp_obj_t)&mb_wlan_get_mode_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_get_config),		    (mp_obj_t)&mb_wlan_get_config_obj },

};

STATIC MP_DEFINE_CONST_DICT(mb_wlan_locals_dict, mb_wlan_locals_dict_table);

const mp_obj_type_t mb_wlan_type =
{
  { &mp_type_type },
  .name = MP_QSTR_wlan,
  .print = mb_wlan_print,
  .make_new = mb_wlan_make_new,
  .locals_dict = (mp_obj_t)&mb_wlan_locals_dict,
};



