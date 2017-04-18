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

#include "ftp/socketfifo.h"


#include "esp_task.h"
	
#include "py/mpstate.h"
#include "py/runtime.h"


#include "mb_wifi_test.h"
#include "serverstask.h"


#define MB_WIFI_SSID "ESP32_MB"
#define MB_WIFI_PASSWORD "12345678"
#define MB_WIFI_LENTHOFPASS 8

#define MB_EAP_METHOD 1  //be useed in the mode of STA
#define MB_EAP_ID "ESP"
#define MB_EAP_USERNAME "fftustff"
#define MB_EAP_PASSWORD "19920112"

/******************************************************************************
 DEFINE PRIVATE CONSTANTS
 ******************************************************************************/
#define MB_FTP_CMD_PORT                        21
#define MB_FTP_ACTIVE_DATA__PORT                20
#define MB_FTP_PASIVE_DATA_PORT                2024


#define MB_FTP_BUFFER_SIZE                     512
#define MB_FTP_TX_RETRIES_MAX                  25
#define MB_FTP_CMD_SIZE_MAX                    6
#define MB_FTP_CMD_CLIENTS_MAX                 1
#define MB_FTP_DATA_CLIENTS_MAX                1
#define MB_FTP_MAX_PARAM_SIZE                  (MICROPY_ALLOC_PATH_MAX + 1)
#define MB_FTP_UNIX_TIME_20000101              946684800
#define MB_FTP_UNIX_TIME_20150101              1420070400
#define MB_FTP_UNIX_SECONDS_180_DAYS           15552000
#define MB_FTP_DATA_TIMEOUT_MS                 5000            // 5 seconds
#define MB_FTP_SOCKETFIFO_ELEMENTS_MAX         4
#define MB_FTP_CYCLE_TIME_MS                   (SERVERS_CYCLE_TIME_MS * 2)


volatile bool mb_wifi_connected=false;  //fftus:the flag of whether the wlan is connected

typedef enum {
    E_FTP_STE_DISABLED = 0, 
    E_FTP_STE_START,
    E_FTP_STE_READY,
    E_FTP_STE_END_TRANSFER,
    E_FTP_STE_CONTINUE_LISTING,   
    E_FTP_STE_CONTINUE_FILE_TX,
    E_FTP_STE_CONTINUE_FILE_RX
} ftp_state_t;     

typedef struct {
    bool            uservalid : 1;
    bool            passvalid : 1;
} ftp_loggin_t;

typedef enum {
    E_FTP_STE_SUB_DISCONNECTED = 0,
    E_FTP_STE_SUB_LISTEN_FOR_DATA,
    E_FTP_STE_SUB_DATA_CONNECTED
} ftp_substate_t;

typedef enum {
    E_FTP_RESULT_OK = 0,
    E_FTP_RESULT_CONTINUE,
    E_FTP_RESULT_FAILED
} ftp_result_t;

typedef struct {
    char * cmd;
} mb_ftp_cmd_t;

typedef struct {
    char * month;
} mb_ftp_month_t;


typedef struct {
    uint8_t             *dBuffer;  
    uint32_t            ctimeout;
    //union {
    //    DIR             dp;
    //    FIL             fp;
    //};
    int32_t             lc_sd;  //server control id
    int32_t             ld_sd;  //server data id
    int32_t             c_sd;   //client control id
    int32_t             d_sd;   //server data id
    int32_t             dtimeout;
    uint32_t            volcount;
    uint32_t            ip_addr; //client
    uint8_t             state;   //control sta
    uint8_t             substate; //data tran sta
    uint8_t             txRetries; 
    uint8_t             logginRetries;
    ftp_loggin_t        loggin;
    uint8_t             e_open;
    bool                closechild;
    bool                enabled;
    bool                special_file;
    bool                listroot;
} mb_ftp_data_t;

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/
static mb_ftp_data_t mb_ftp_data;
static char *mb_ftp_path;
static char *mb_ftp_scratch_buffer;
static char *mb_ftp_cmd_buffer;
static const mb_ftp_cmd_t mb_ftp_cmd_table[] = { { "FEAT" }, { "SYST" }, { "CDUP" }, { "CWD"  },
                                           { "PWD"  }, { "XPWD" }, { "SIZE" }, { "MDTM" },
                                           { "TYPE" }, { "USER" }, { "PASS" }, { "PASV" },
                                           { "LIST" }, { "RETR" }, { "STOR" }, { "DELE" },
                                           { "RMD"  }, { "MKD"  }, { "RNFR" }, { "RNTO" },
                                           { "NOOP" }, { "QUIT" } };

static const mb_ftp_month_t mb_ftp_month[] = { { "Jan" }, { "Feb" }, { "Mar" }, { "Apr" },
                                         { "May" }, { "Jun" }, { "Jul" }, { "Ago" },
                                         { "Sep" }, { "Oct" }, { "Nov" }, { "Dec" } };

static SocketFifoElement_t mb_ftp_fifoelements[MB_FTP_SOCKETFIFO_ELEMENTS_MAX];
static FIFO_t mb_ftp_socketfifo;





/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t mb_wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;
#define TAG "mb_wifiexample"

static esp_err_t mb_event_handler(void *ctx, system_event_t *event)
{
    
	switch(event->event_id) {
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

static void mb_initialise_wifi(void)
{
  tcpip_adapter_init();  //ffftust:tcpip config
  
  mb_wifi_event_group = xEventGroupCreate(); 
  ESP_ERROR_CHECK( esp_event_loop_init(mb_event_handler, NULL) );
  
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
  //ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.ap.ssid);
  ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_AP) );
  ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config) );
  ESP_ERROR_CHECK( esp_wifi_start() );
	
}



static void mb_ftp_wait_for_enabled (void) 
{
  // Check if the ftp service has been enabled
  if (mb_ftp_data.enabled) 
  {
    mb_ftp_data.state = E_FTP_STE_START;
  }
}

static bool mb_ftp_create_listening_socket (int32_t *sd, uint32_t port, uint8_t backlog) 
{
  struct sockaddr_in sServerAddress;
  int32_t _sd;
  int32_t result;
  
  // open a socket for ftp data listen
  *sd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  _sd = *sd;
  
  if (_sd > 0) {
      // add the new socket to the network administration
      //modusocket_socket_add(_sd, false);
  
      // enable non-blocking mode
      uint32_t option = fcntl(_sd, F_GETFL, 0);
      option |= O_NONBLOCK;
      fcntl(_sd, F_SETFL, option);
  
      // enable address reusing
      option = 1;
      result = setsockopt(_sd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  
      // bind the socket to a port number
      sServerAddress.sin_family = AF_INET;
      sServerAddress.sin_addr.s_addr = INADDR_ANY;
      sServerAddress.sin_len = sizeof(sServerAddress);
      sServerAddress.sin_port = htons(port);
  
      result |= bind(_sd, (const struct sockaddr *)&sServerAddress, sizeof(sServerAddress));
  
      // start listening
      result |= listen (_sd, backlog);
  
      if (!result) {
  		printf("fftust: listen connect\n");
          return true;
      }
      closesocket(_sd);
  	*sd=-1;
  }
  return false;
}

STATIC void mb_close_socket(int32_t *sd)
{
 if (*sd > 0)
 {
  closesocket(*sd);
  *sd = -1;
 }
}

STATIC  void ftp_close_cmd_data (void) 
{
  mb_close_socket(&mb_ftp_data.c_sd);
  mb_close_socket(&mb_ftp_data.d_sd);
  //ftp_close_filesystem_on_error ();
}

STATIC void mb_ftp_reset (void) 
{
  // close all connections and start all over again
  mb_close_socket(&mb_ftp_data.lc_sd);
  mb_close_socket(&mb_ftp_data.ld_sd);
  ftp_close_cmd_data();
  mb_ftp_data.state = E_FTP_STE_START;
  mb_ftp_data.substate = E_FTP_STE_SUB_DISCONNECTED;
  mb_ftp_data.volcount = 0;
  //SOCKETFIFO_Flush();
}



static ftp_result_t mb_ftp_wait_for_connection (int32_t l_sd, int32_t *n_sd, uint32_t *ip_addr)
{
  struct sockaddr_in  sClientAddress;
  socklen_t  in_addrSize;
  
  // accepts a connection from a TCP client, if there is any, otherwise returns EAGAIN
  *n_sd = accept(l_sd, (struct sockaddr *)&sClientAddress, (socklen_t *)&in_addrSize); //fftust:get the id of the connected client
  int32_t _sd = *n_sd;
  if (_sd < 0) 
  {
    if (errno == EAGAIN) 
    {
      return E_FTP_RESULT_CONTINUE;
    }
    // error
    mb_ftp_reset();
    return E_FTP_RESULT_FAILED;
  }
  
  if (ip_addr) 
  {
    tcpip_adapter_ip_info_t ip_info;
    wifi_mode_t wifi_mode;
    esp_wifi_get_mode(&wifi_mode);
    if (wifi_mode != WIFI_MODE_APSTA)
	{
      // easy way
      tcpip_adapter_if_t if_type;
      if (wifi_mode == WIFI_MODE_AP)
	  {
        if_type = TCPIP_ADAPTER_IF_AP;
      } 
	  else 
	  {
        if_type = TCPIP_ADAPTER_IF_STA;
      }
      tcpip_adapter_get_ip_info(if_type, &ip_info);
    }
	else 
	{
      // see on which subnet is the client ip address
      tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ip_info);
      if ((ip_info.ip.addr & ip_info.netmask.addr) != (ip_info.netmask.addr & sClientAddress.sin_addr.s_addr)) 
	  {
        tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info);
      }
    }
    *ip_addr = ip_info.ip.addr;
  }

// add the new socket to the network administration
//modusocket_socket_add(_sd, false);

// enable non-blocking mode
uint32_t option = fcntl(_sd, F_GETFL, 0);
option |= O_NONBLOCK;
fcntl(_sd, F_SETFL, option);

// client connected, so go on
return E_FTP_RESULT_OK;
}

static void mb_ftp_send_reply (uint32_t status, char *message) 
{
  SocketFifoElement_t fifoelement;
  if (!message) {
      message = "";
  }
  snprintf((char *)ftp_cmd_buffer, 4, "%u", status);
  strcat ((char *)ftp_cmd_buffer, " ");
  strcat ((char *)ftp_cmd_buffer, message);
  strcat ((char *)ftp_cmd_buffer, "\r\n");
  fifoelement.sd = &ftp_data.c_sd;
  fifoelement.datasize = strlen((char *)ftp_cmd_buffer);
  fifoelement.data = pvPortMalloc(fifoelement.datasize);
  if (status == 221) {
      fifoelement.closesockets = E_FTP_CLOSE_CMD_AND_DATA;
  } else if (status == 426 || status == 451 || status == 550) {
      fifoelement.closesockets = E_FTP_CLOSE_DATA;
  } else {
      fifoelement.closesockets = E_FTP_CLOSE_NONE;
  }
  fifoelement.freedata = true;
  if (fifoelement.data) {
      memcpy (fifoelement.data, ftp_cmd_buffer, fifoelement.datasize);
      if (!SOCKETFIFO_Push (&fifoelement)) {
          vPortFree(fifoelement.data);
      }
  }
}



void mb_ftp_init (void)
{
  // Allocate memory for the data buffer, and the file system structs (from the RTOS heap)
  mb_ftp_data.dBuffer = pvPortMalloc(512);
  mb_ftp_path = pvPortMalloc(129);
  mb_ftp_scratch_buffer = pvPortMalloc(129);
    
  SOCKETFIFO_Init (&mb_ftp_socketfifo, (void *)mb_ftp_fifoelements, MB_FTP_SOCKETFIFO_ELEMENTS_MAX);

  mb_ftp_data.c_sd  = -1;
  mb_ftp_data.d_sd  = -1;
  mb_ftp_data.lc_sd = -1;
  mb_ftp_data.ld_sd = -1;
  mb_ftp_data.e_open = 0;
  mb_ftp_data.state = E_FTP_STE_DISABLED;
  mb_ftp_data.substate = E_FTP_STE_SUB_DISCONNECTED;
  mb_ftp_data.enabled=true;
  mb_ftp_data.special_file = false;
  mb_ftp_data.volcount = 0;
}

char fftust_char[]="fftust";
STATIC void test_task()
{
  for(;;)
  {
    vTaskDelay(20 / portTICK_RATE_MS);
	switch(mb_ftp_data.state)
  	{
   
	  case E_FTP_STE_DISABLED:
	  	   printf("fftust disabled\n");
           mb_ftp_wait_for_enabled();
		  
           break;
      case E_FTP_STE_START:
	  	   printf("fftust start\n");
           if((mb_wifi_connected) &&  mb_ftp_create_listening_socket(&mb_ftp_data.lc_sd, MB_FTP_CMD_PORT, 0)) //listen at then port of 21
		   {
             printf("fftust connected\n");
			 mb_ftp_data.state = E_FTP_STE_READY;
           }
          break;
      case E_FTP_STE_READY:
		   if(mb_ftp_data.c_sd < 0 && mb_ftp_data.substate == E_FTP_STE_SUB_DISCONNECTED) 
		   {
             if(E_FTP_RESULT_OK == mb_ftp_wait_for_connection(mb_ftp_data.lc_sd, &mb_ftp_data.c_sd, &mb_ftp_data.ip_addr)) 
   		     {
   			   printf("fftust server connect in");
   			   mb_ftp_data.txRetries = 0;
               mb_ftp_data.logginRetries = 0;
               mb_ftp_data.ctimeout = 0;
               mb_ftp_data.loggin.uservalid = false;
               mb_ftp_data.loggin.passvalid = false;
   			  
               strcpy (mb_ftp_path, "/");
   			   send(mb_ftp_data.c_sd,fftust_char,6,0);
               //fftust_ftp_send_reply (220, "Micropython FTP Server");
               break;
             }
           }


          if(SOCKETFIFO_IsEmpty()) 
		  {
            if(mb_ftp_data.c_sd > 0 && mb_ftp_data.substate != E_FTP_STE_SUB_LISTEN_FOR_DATA)
			{
              //ftp_process_cmd(); //fftust:ftp command process
              if (mb_ftp_data.state != E_FTP_STE_READY) 
			  {
                break;
              }
            }
          }
		  
          break;
      case E_FTP_STE_END_TRANSFER:
           break;
     
      default:
          break;
      }
	
      /*switch (mb_ftp_data.substate) {
        case E_FTP_STE_SUB_DISCONNECTED:
            break;
        case E_FTP_STE_SUB_LISTEN_FOR_DATA:
            if (E_FTP_RESULT_OK == fftust_ftp_wait_for_connection(mb_ftp_data.ld_sd, &mb_ftp_data.d_sd, NULL)) {
                mb_ftp_data.dtimeout = 0;
                mb_ftp_data.substate = E_FTP_STE_SUB_DATA_CONNECTED;
            } else if (mb_ftp_data.dtimeout++ > 5000 / 4) {
                mb_ftp_data.dtimeout = 0;
                // close the listening socket
                //servers_close_socket(&ftp_data.ld_sd);
                closesocket(mb_ftp_data.ld_sd);
				mb_ftp_data.ld_sd=-1;
                mb_ftp_data.substate = E_FTP_STE_SUB_DISCONNECTED;
            }
            break;
        case E_FTP_STE_SUB_DATA_CONNECTED:
            if (mb_ftp_data.state == E_FTP_STE_READY && mb_ftp_data.dtimeout++ > 5000 / 4) {
                // close the listening and the data socket
                //servers_close_socket(&ftp_data.ld_sd);
                //servers_close_socket(&ftp_data.d_sd);
                //ftp_close_filesystem_on_error ();
                closesocket(mb_ftp_data.ld_sd);
				mb_ftp_data.ld_sd=-1;
				closesocket(mb_ftp_data.d_sd);
				mb_ftp_data.d_sd=-1;
                mb_ftp_data.substate = E_FTP_STE_SUB_DISCONNECTED;
            }
            break;
        default:
            break;

	  
      	}*/
 
  }

}

void ftp_test()
{
  //mb_initialise_wifi();
  //mb_ftp_init();
  xTaskCreate(test_task, "task_test", 1024, NULL, 1, NULL);
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
  mb_initialise_wifi();
  mb_ftp_init();
}


	
STATIC mp_obj_t mb_wifi_test_run(mp_uint_t n_args, const mp_obj_t *args)
{
  mb_wifi_test_obj_t *self=args[0];
  ftp_test();
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




