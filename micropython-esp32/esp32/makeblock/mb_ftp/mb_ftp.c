/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for makeblock ftp module
 * @file    mb_ftp.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/04/10
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
 * This file is a drive flame_sensor module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 *  fftust            2017/04/10       1.0.0            build the new.
 * </pre>
 *
 */

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_task.h"
#include "nvs_flash.h"

#include "py/mpstate.h"
#include "py/runtime.h"

#include "tcpip_adapter.h"
#include "lwip/sockets.h"

#include "ff.h"
#include "diskio.h"
#include "ffconf.h"


#include "mb_ftp/mb_ftp.h"
#include "mb_ftp/socketfifo.h"
#include "mb_ftp/fifo.h"




/******************************************************************************
 MACRO DEFINITION
 ******************************************************************************/
#define MB_FTP_CMD_PORT                        21
#define MB_FTP_ACTIVE_DATA_PORT                20
#define MB_FTP_PASIVE_DATA_PORT                2024


#define MB_FTP_BUFFER_SIZE                     512
#define MB_FTP_TX_RETRIES_MAX                  25
#define MB_FTP_CMD_SIZE_MAX                    6
#define MB_FTP_CMD_CLIENTS_MAX                 1
#define MB_FTP_DATA_CLIENTS_MAX                1

#define MB_FTP_MAX_PARAM_SIZE                  129  //(MICROPY_ALLOC_PATH_MAX + 1)
//#define MB_FTP_UNIX_TIME_20000101              946684800
//#define MB_FTP_UNIX_TIME_20150101              1420070400
//#define MB_FTP_UNIX_SECONDS_180_DAYS           15552000
//#define MB_FTP_DATA_TIMEOUT_MS                 5000            // 5 seconds
#define MB_FTP_SOCKETFIFO_ELEMENTS_MAX         4
//#define MB_FTP_CYCLE_TIME_MS                  (SERVERS_CYCLE_TIME_MS * 2)

/******************************************************************************
 DECLARE CONSTANTS
 ******************************************************************************/

/******************************************************************************
 DEFINE TYPES
 ******************************************************************************/
typedef enum 
{
  MB_FTP_RESULT_OK = 0,
  MB_FTP_RESULT_CONTINUE,
  MB_FTP_RESULT_FAILED
} mb_ftp_result_t;

typedef enum 
{
  MB_FTP_NOTHING_OPEN = 0,
  MB_FTP_FILE_OPEN,
  MB_FTP_DIR_OPEN
} mb_ftp_open_t;

typedef struct 
{
  bool            uservalid;
  bool            passvalid;
  bool            userloggined;
} mb_ftp_loggin_t;

typedef enum 
{
  MB_FTP_DISABLED = 0, 
  MB_FTP_CONTROL_START,
  MB_FTP_CONTROL_LISTEN,
  MB_FTP_CONTROL_CONNECTED,
  MB_FTP_STE_END_TRANSFER,
  MB_FTP_STE_CONTINUE_LISTING,   
  MB_FTP_STE_CONTINUE_FILE_TX,
  MB_FTP_STE_CONTINUE_FILE_RX
} mb_ftp_state_t;     

typedef enum 
{
  MB_FTP_DATATRANS_DISCONNECTED = 0,
  MB_FTP_DATATRANS_LISTEN_FOR_DATA,
  MB_FTP_DATATRANS_CONNECTED
} mb_ftp_datatrans_state_t;

typedef enum {
    MB_FTP_CLOSE_NONE = 0,
    MB_FTP_CLOSE_DATA,
    MB_FTP_CLOSE_CMD_AND_DATA,
} mb_ftp_closesocket_t;

typedef struct 
{
  uint8_t             *ftpBuffer;  
  uint32_t            ctimeout;
  union 
  {
    FF_DIR            dp;
    FIL               fp;
  };
  int32_t             sc_sd;  //server control id
  int32_t             sd_sd;  //server data id
  int32_t             cc_sd;   //client control id
  int32_t             cd_sd;   //client data id
  int32_t             dtimeout;
  uint32_t            volcount;
  uint32_t            ip_addr; //client
  uint8_t             state;   //control sta
  mb_ftp_datatrans_state_t      data_trans_state; //data tran sta
  uint8_t             txRetries; 
  uint8_t             logginRetries;
  mb_ftp_loggin_t     loggin;
  uint8_t             e_open;
  bool                closechild;
  bool                enabled;
  bool                special_file;
  bool                listroot;
} mb_ftp_manager_t;


typedef enum {
    MB_FTP_CMD_NOT_SUPPORTED = -1,
    MB_FTP_CMD_FEAT = 0,
    MB_FTP_CMD_SYST,
    MB_FTP_CMD_CDUP,
    MB_FTP_CMD_CWD,
    MB_FTP_CMD_PWD,
    MB_FTP_CMD_XPWD,
    MB_FTP_CMD_SIZE,
    MB_FTP_CMD_MDTM,
    MB_FTP_CMD_TYPE,
    MB_FTP_CMD_USER,
    MB_FTP_CMD_PASS,
    MB_FTP_CMD_PASV,
    MB_FTP_CMD_LIST,
    MB_FTP_CMD_RETR,
    MB_FTP_CMD_STOR,
    MB_FTP_CMD_DELE,
    MB_FTP_CMD_RMD,
    MB_FTP_CMD_MKD,
    MB_FTP_CMD_RNFR,
    MB_FTP_CMD_RNTO,
    MB_FTP_CMD_NOOP,
    MB_FTP_CMD_QUIT,
    MB_FTP_NUM_FTP_CMDS
}mb_ftp_cmd_index_t;



typedef char* mb_ftp_cmdchars_t;
static mb_ftp_manager_t mb_ftp_manager;
static char *mb_ftp_path;
static char *mb_ftp_scratch_buffer;
static char *mb_ftp_cmd_buffer;
static const mb_ftp_cmdchars_t mb_ftp_cmd_table[] = {"FEAT" ,  "SYST" ,  "CDUP" ,  "CWD" ,
                                                     "PWD"  ,  "XPWD" ,  "SIZE" ,  "MDTM" ,
                                                     "TYPE" ,  "USER" ,  "PASS" ,  "PASV" ,
                                                     "LIST" ,  "RETR" ,  "STOR" ,  "DELE" ,
                                                     "RMD"  ,  "MKD"  ,  "RNFR" ,  "RNTO" ,
                                                     "NOOP" ,  "QUIT"  };

static const char mb_ftp_user[]={"makeblock"};   
static const char mb_ftp_pass[]={"12345678"};

static SocketFifoElement_t mb_ftp_fifoelements[MB_FTP_SOCKETFIFO_ELEMENTS_MAX];
static FIFO_t mb_ftp_socketfifo;
static FATFS mb_fatfs;


/******************************************************************************/



STATIC void mb_ftp_close_socket(int32_t *sd);
STATIC void mb_ftp_enabled_check(void);
STATIC mb_ftp_result_t mb_ftp_send_data_non_blocking (int32_t sd, void *data, int32_t Len);
STATIC void mb_ftp_send_reply(uint32_t status, char *message);
STATIC bool mb_ftp_create_controlanddata_socket(int32_t *sd, uint32_t port);
STATIC mb_ftp_result_t mb_ftp_wait_for_connection (int32_t l_sd, int32_t *n_sd, uint32_t *ip_addr);
STATIC mb_ftp_result_t mb_ftp_receive_data(int32_t sd, void *buff, int32_t Maxlen, int32_t *rxLen);
STATIC void mb_ftp_cmd_process();
STATIC void mb_ftp_close_socket(int32_t *sd);
STATIC void mb_ftp_close_client_sockets (void);




/**********************************************************************************/


void mb_ftp_enable(void)
{
   mb_ftp_manager.enabled = true;
}

void mb_ftp_disable (void) 
{
  mb_ftp_reset();
  mb_ftp_manager.enabled = false;
  mb_ftp_manager.state = MB_FTP_DISABLED;
}



void  mb_ftp_reset(void)
{
  mb_ftp_close_socket(&mb_ftp_manager.sc_sd);
  mb_ftp_close_socket(&mb_ftp_manager.sd_sd);
  mb_ftp_close_client_sockets();
  mb_ftp_manager.state = MB_FTP_CONTROL_START;
  mb_ftp_manager.data_trans_state = MB_FTP_DATATRANS_DISCONNECTED;
  mb_ftp_manager.volcount = 0;
  //SOCKETFIFO_Flush();
}

STATIC void mb_ftp_enabled_check(void)
{
  if (mb_ftp_manager.enabled) 
  {
    mb_ftp_manager.state = MB_FTP_CONTROL_START;
  }
}

STATIC void mb_ftp_close_socket(int32_t *sd)
{
  if (*sd > 0)
  {
	closesocket(*sd);
	*sd = -1;
  }
}
STATIC mb_ftp_result_t mb_ftp_receive_data(int32_t sd, void *buff, int32_t Maxlen, int32_t *rxLen)
{
  *rxLen = recv(sd, buff, Maxlen, 0);

  if(*rxLen > 0)
  {
    return MB_FTP_RESULT_OK;
  } 
  else if (errno != EAGAIN)
  {
    // error
    return MB_FTP_RESULT_FAILED;
  }
  return MB_FTP_RESULT_CONTINUE;
}



STATIC mb_ftp_result_t mb_ftp_send_data_non_blocking(int32_t sd, void *data, int32_t Len) 
{
  int32_t result = send(sd, data, Len, 0);
  if (result > 0)
  {
    mb_ftp_manager.txRetries = 0;
    return MB_FTP_RESULT_OK;
  } 
  else if ((MB_FTP_TX_RETRIES_MAX >= ++mb_ftp_manager.txRetries) && (errno == EAGAIN)) 
  {
    return MB_FTP_RESULT_CONTINUE;
  }
  else 
  {
    // error
    mb_ftp_reset();
    return MB_FTP_RESULT_FAILED;
	printf("makeblock:data send eroor,ftp reset ");//fftust:debug
  }
}

STATIC void mb_ftp_send_data_to_fifo (uint32_t datasize) 
{
  SocketFifoElement_t fifoelement;

  fifoelement.data = mb_ftp_manager.ftpBuffer;
  fifoelement.datasize = datasize;
  fifoelement.sd = &mb_ftp_manager.cd_sd;
  fifoelement.closesockets = MB_FTP_CLOSE_NONE;
  fifoelement.freedata = false;
  SOCKETFIFO_Push (&fifoelement);
}

STATIC void mb_ftp_send_from_fifo (void)
{
  SocketFifoElement_t fifoelement;
  if (SOCKETFIFO_Peek (&fifoelement))
  {
    int32_t _sd = *fifoelement.sd;
    if (_sd > 0)
    {
      if (MB_FTP_RESULT_OK == mb_ftp_send_data_non_blocking (_sd, fifoelement.data, fifoelement.datasize)) 
	  {
        SOCKETFIFO_Pop (&fifoelement);
        if (fifoelement.closesockets != MB_FTP_CLOSE_NONE)
		{
          mb_ftp_close_socket(&mb_ftp_manager.cd_sd); //fftust:close the client data socket
          if (fifoelement.closesockets == MB_FTP_CLOSE_CMD_AND_DATA)
		  {
            mb_ftp_close_socket(&mb_ftp_manager.sd_sd);//fftust:close the serve data socket
            // this one is the command socket
            mb_ftp_close_socket(fifoelement.sd); //fftust:????????????
            mb_ftp_manager.data_trans_state = MB_FTP_DATATRANS_DISCONNECTED;
          }
          //ftp_close_filesystem_on_error();//fftust:need to define this function:
        }
        if (fifoelement.freedata)
		{
          vPortFree(fifoelement.data);
        }
      }
    } 
    else 
	{
	  // socket closed, remove it from the queue
      SOCKETFIFO_Pop (&fifoelement);
      if (fifoelement.freedata) 
	  {
        vPortFree(fifoelement.data);
      }
    }
  } 
  else if (mb_ftp_manager.state == MB_FTP_STE_END_TRANSFER && (mb_ftp_manager.cd_sd > 0))
  {
    // close the listening and the data sockets
    mb_ftp_close_socket(&mb_ftp_manager.sd_sd);
    mb_ftp_close_socket(&mb_ftp_manager.cd_sd);
    if (mb_ftp_manager.special_file) 
    {
      mb_ftp_manager.special_file = false;
    }
  }
}

STATIC void mb_ftp_send_reply(uint32_t status, char *message)
{
  //char *data;
  //uint32_t datasize;
  //int32_t _sd;
  SocketFifoElement_t fifoelement;
  //data=pvPortMalloc(64);
  if (!message) 
  {
  	message = "";
  }
  snprintf((char *)mb_ftp_cmd_buffer, 4, "%u", status);
  //strcat ((char *)mb_ftp_cmd_buffer, "makeblock");
  strcat ((char *)mb_ftp_cmd_buffer, " ");  
  strcat ((char *)mb_ftp_cmd_buffer, message);
  strcat ((char *)mb_ftp_cmd_buffer, "\r\n");
  //_sd=mb_ftp_manager.cc_sd;
  //datasize=strlen((char *)data);
  
  fifoelement.sd = &mb_ftp_manager.cc_sd;
  fifoelement.datasize = strlen((char *)mb_ftp_cmd_buffer);
  fifoelement.data = pvPortMalloc(fifoelement.datasize);
  if (status == 221) 
  {
  	fifoelement.closesockets = MB_FTP_CLOSE_CMD_AND_DATA;
  }
  else if (status == 426 || status == 451 || status == 550) 
  {
  	fifoelement.closesockets = MB_FTP_CLOSE_DATA;
  }
  else
  {
  	fifoelement.closesockets = MB_FTP_CLOSE_NONE;
  }

  
  fifoelement.freedata = true;
  if (fifoelement.data) 
  {
  	memcpy (fifoelement.data, mb_ftp_cmd_buffer, fifoelement.datasize);
  	if (!SOCKETFIFO_Push (&fifoelement)) 
	{
  		vPortFree(fifoelement.data);
  	}
    //if(data)
    //{
     //memcpy(data, mb_ftp_cmd_buffer,datasize);
    //  mb_ftp_send_data_non_blocking(_sd,data,datasize);
  }

}


STATIC bool mb_ftp_create_controlanddata_socket(int32_t *sd, uint32_t port)
{
  struct sockaddr_in sServeAddress;
  int32_t result;
  int32_t _sd;
  // open a socket for ftp control
  *sd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  _sd=*sd;
  if (_sd > 0) 
  {
    // enable non-blocking mode
    uint32_t option = fcntl(_sd, F_GETFL, 0);
    option |= O_NONBLOCK;
    fcntl(_sd, F_SETFL, option);

    // enable address reusing
    option = 1;
    result = setsockopt(_sd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    // bind the socket to a port number
    sServeAddress.sin_family = AF_INET;
    sServeAddress.sin_addr.s_addr = INADDR_ANY;
    sServeAddress.sin_len = sizeof(sServeAddress);
    sServeAddress.sin_port = htons(port);

    result = bind(_sd, (const struct sockaddr *)&sServeAddress, sizeof(sServeAddress));

    // start listening
    result |= listen (_sd, 0);

    if (!result) 
	{
      return true;
    }
    mb_ftp_close_socket(sd);
  }

  return false;
  
}


STATIC mb_ftp_result_t mb_ftp_wait_for_connection (int32_t l_sd, int32_t *n_sd, uint32_t *ip_addr)
{
  struct sockaddr_in  sClientAddress;
  socklen_t  in_addrSize;
  
  // accepts a connection from a TCP client, if there is any, otherwise returns EAGAIN
  *n_sd = accept(l_sd, (struct sockaddr *)&sClientAddress, (socklen_t *)&in_addrSize);
  int32_t _sd = *n_sd;
  if (_sd < 0)
  {
   if (errno == EAGAIN) 
   {
     return MB_FTP_RESULT_CONTINUE;
   }
   // error
   mb_ftp_reset();
   return MB_FTP_RESULT_FAILED;
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
  
  
  // enable non-blocking mode
  uint32_t option = fcntl(_sd, F_GETFL, 0);
  option |= O_NONBLOCK;
  fcntl(_sd, F_SETFL, option);
  
  // client connected, so go on
  return MB_FTP_RESULT_OK;
}

STATIC void mb_ftp_close_client_sockets (void)
{
  mb_ftp_close_socket(&mb_ftp_manager.cc_sd);
  mb_ftp_close_socket(&mb_ftp_manager.cd_sd);
  //ftp_close_filesystem_on_error ();
}

STATIC void mb_ftp_pop_param (char **str, char *param) 
{
  while (**str != ' ' && **str != '\r' && **str != '\n' && **str != '\0')
  {
    *param++ = **str;
    (*str)++;
  }
  *param = '\0';
}

void mb_stoupper (char *str)  //fftust:case-insensitive
{
  while (str && *str != '\0') 
  {
    *str = (char)toupper((int)(*str));
    str++;
  }
}

STATIC mb_ftp_cmd_index_t mb_ftp_pop_command (char **str) 
{
  char _cmd[MB_FTP_CMD_SIZE_MAX];
  mb_ftp_pop_param (str, _cmd);
  mb_stoupper(_cmd);
  for (mb_ftp_cmd_index_t i = 0; i < MB_FTP_NUM_FTP_CMDS; i++)
  {
    if (!strcmp (_cmd, mb_ftp_cmd_table[i])) 
	{
      (*str)++;
      return i;
    }
  }
    return MB_FTP_CMD_NOT_SUPPORTED;
}

STATIC void mb_ftp_close_files (void)
{
  if (mb_ftp_manager.e_open == MB_FTP_FILE_OPEN)
  {
    f_close(&mb_ftp_manager.fp);
  } 
  else if (mb_ftp_manager.e_open == MB_FTP_DIR_OPEN)
  {
    f_closedir(&mb_ftp_manager.dp);
  }
  mb_ftp_manager.e_open = MB_FTP_NOTHING_OPEN;
}

STATIC void mb_ftp_open_child (char *pwd, char *dir)
{
  if (dir[0] == '/') 
  {
    strcpy (pwd, dir);
  }
  else
  {
    if (strlen(pwd) > 1) 
	{
      strcat (pwd, "/");
    }
    strcat (pwd, dir);
  }

  uint len = strlen(pwd);
  if ((len > 1) && (pwd[len - 1] == '/'))
  {
    pwd[len - 1] = '\0';
  }
}

STATIC void mb_ftp_close_child (char *pwd)
{
  uint len = strlen(pwd);
  while (len && (pwd[len] != '/'))
  {
    len--;
  }
  if (len == 0)
  {
    strcpy (pwd, "/");
  }
  else 
  {
    pwd[len] = '\0';
  }
}

STATIC void mb_ftp_get_param_and_open_child (char **bufptr)
{
    mb_ftp_pop_param (bufptr, mb_ftp_scratch_buffer);
    mb_ftp_open_child (mb_ftp_path, mb_ftp_scratch_buffer);
    mb_ftp_manager.closechild = true;
}

STATIC int mb_ftp_print_eplf_item (char *dest, uint32_t destsize, FILINFO *fno)
{
  char *type = (fno->fattrib & AM_DIR) ? "d" : "-";
   return snprintf(dest, destsize, "%srw-rw-r--   1 root  root %9u %s %2u %5u %s\r\n", type, (uint32_t)fno->fsize, "Jan",1,1980 + ((fno->fdate >> 9) & 0x7f), fno->fname);
#if 0
    char *type = (fno->fattrib & AM_DIR) ? "d" : "-";
    uint32_t tseconds;
    uint mindex = (((fno->fdate >> 5) & 0x0f) > 0) ? (((fno->fdate >> 5) & 0x0f) - 1) : 0;
    uint day = ((fno->fdate & 0x1f) > 0) ? (fno->fdate & 0x1f) : 1;
    uint fseconds = timeutils_seconds_since_epoch(1980 + ((fno->fdate >> 9) & 0x7f),
                                                        (fno->fdate >> 5) & 0x0f,
                                                        fno->fdate & 0x1f,
                                                        (fno->ftime >> 11) & 0x1f,
                                                        (fno->ftime >> 5) & 0x3f,
                                                        2 * (fno->ftime & 0x1f));
    tseconds = 3600; // pyb_rtc_get_seconds(); // FIXME
    if (FTP_UNIX_SECONDS_180_DAYS < tseconds - fseconds) {
        return snprintf(dest, destsize, "%srw-rw-r--   1 root  root %9u %s %2u %5u %s\r\n",
                        type, (uint32_t)fno->fsize, ftp_month[mindex].month, day,
                        1980 + ((fno->fdate >> 9) & 0x7f), fno->fname);
    } else {
        return snprintf(dest, destsize, "%srw-rw-r--   1 root  root %9u %s %2u %02u:%02u %s\r\n",
                        type, (uint32_t)fno->fsize, ftp_month[mindex].month, day,
                        (fno->ftime >> 11) & 0x1f, (fno->ftime >> 5) & 0x3f, fno->fname);
    }
  #endif
  
  
}

STATIC int mb_ftp_print_eplf_drive (char *dest, uint32_t destsize, char *name)
{
  #if 0
	timeutils_struct_time_t tm;
    uint32_t tseconds;
    char *type = "d";

    timeutils_seconds_since_epoch_to_struct_time((FTP_UNIX_TIME_20150101 - FTP_UNIX_TIME_20000101), &tm);

    tseconds = 3600; // pyb_rtc_get_seconds(); // FIXME
    if (FTP_UNIX_SECONDS_180_DAYS < tseconds - (FTP_UNIX_TIME_20150101 - FTP_UNIX_TIME_20000101)) {
        return snprintf(dest, destsize, "%srw-rw-r--   1 root  root %9u %s %2u %5u %s\r\n",
                        type, 0, ftp_month[(tm.tm_mon - 1)].month, tm.tm_mday, tm.tm_year, name);
    } else {
        return snprintf(dest, destsize, "%srw-rw-r--   1 root  root %9u %s %2u %02u:%02u %s\r\n",
                        type, 0, ftp_month[(tm.tm_mon - 1)].month, tm.tm_mday, tm.tm_hour, tm.tm_min, name);
    }
  #endif
  return snprintf(dest, destsize, "%srw-rw-r--   1 root  root %9u %s %2u %5u %s\r\n",
                        "d", 0, "Jan", 1, 2017, name);
}

STATIC mb_ftp_result_t mb_ftp_list_dir (char *list, uint32_t maxlistsize, uint32_t *listsize) 
{
  uint next = 0;
  uint listcount = 0;
  FRESULT res;
  mb_ftp_result_t result = MB_FTP_RESULT_CONTINUE;
  FILINFO fno;
  
  // read up to 8 directory items
  while (listcount < 8)
  {
    if(mb_ftp_manager.listroot) //root path
	{
      // root directory "hack"
      if (0 == mb_ftp_manager.volcount) 
   	  {
          next += mb_ftp_print_eplf_drive((list + next), (maxlistsize - next), "flash");
      } 
   	  else if (mb_ftp_manager.volcount <= 0/*MP_STATE_PORT(mount_obj_list).len*/) 
   	  {
        //os_fs_mount_t *mount_obj = ((os_fs_mount_t *)(MP_STATE_PORT(mount_obj_list).items[( mb_ftp_manager.volcount - 1)]));
        next += mb_ftp_print_eplf_drive((list + next), (maxlistsize - next), "/flash"/*(char *)&mount_obj->path[1]*/);
      } 
   	  else 
   	  {
        if (!next) 
   		{
          // no volume found this time, we are done
          mb_ftp_manager.volcount = 0;
        }
        break;
      }
      mb_ftp_manager.volcount++;
    }
	else
	{
      // a "normal" directory
      res = f_readdir(&mb_ftp_manager.dp, &fno);                                                       /* Read a directory item */
      if (res != FR_OK || fno.fname[0] == 0) 
	  {
        result = MB_FTP_RESULT_OK;
        break;                                                                                 /* Break on error or end of dp */
      }
      if (fno.fname[0] == '.' && fno.fname[1] == 0) continue;                                    /* Ignore . entry */
      if (fno.fname[0] == '.' && fno.fname[1] == '.' && fno.fname[2] == 0) continue;             /* Ignore .. entry */

      // add the entry to the list
      next += mb_ftp_print_eplf_item((list + next), (maxlistsize - next), &fno);
      }
      listcount++;
  }
  if (result == MB_FTP_RESULT_OK) 
  {
    mb_ftp_close_files();
  }
  *listsize = next;
  
  return result;
}

  
STATIC mb_ftp_result_t mb_ftp_open_dir_for_listing (const char *path) 
{
  // "hack" to detect the root directory
  if (path[0] == '/' && path[1] == '\0') 
  {
    mb_ftp_manager.listroot = true;
  } 
  else
  {
  FRESULT res=FR_OK;
  res = f_opendir(&mb_fatfs,&mb_ftp_manager.dp, path);                       /* Open the directory */
  if (res != FR_OK)
  {
    return MB_FTP_RESULT_FAILED;
  }
  mb_ftp_manager.e_open = MB_FTP_DIR_OPEN;
  mb_ftp_manager.listroot = false;
  }
  return MB_FTP_RESULT_CONTINUE;
}


void mb_ftp_init()
{
  // Allocate memory for the data buffer, and the file system structs (from the RTOS heap)
  mb_ftp_manager.ftpBuffer = pvPortMalloc(MB_FTP_BUFFER_SIZE);
  mb_ftp_path = pvPortMalloc(MB_FTP_MAX_PARAM_SIZE);
  mb_ftp_scratch_buffer = pvPortMalloc(MB_FTP_MAX_PARAM_SIZE);
  mb_ftp_cmd_buffer = pvPortMalloc(MB_FTP_MAX_PARAM_SIZE + MB_FTP_CMD_SIZE_MAX);
  SOCKETFIFO_Init (&mb_ftp_socketfifo, (void *)mb_ftp_fifoelements, MB_FTP_SOCKETFIFO_ELEMENTS_MAX);
  mb_ftp_manager.cc_sd  = -1;
  mb_ftp_manager.cd_sd  = -1;
  mb_ftp_manager.sc_sd = -1;
  mb_ftp_manager.sd_sd = -1;
  mb_ftp_manager.e_open = MB_FTP_NOTHING_OPEN;
  mb_ftp_manager.state = MB_FTP_DISABLED;
  mb_ftp_manager.data_trans_state = MB_FTP_DATATRANS_DISCONNECTED;
  mb_ftp_manager.enabled=false;
  mb_ftp_manager.special_file = false;
  mb_ftp_manager.listroot=false;
  mb_ftp_manager.volcount = 0;
  mb_ftp_manager.loggin.passvalid=false;
  mb_ftp_manager.loggin.uservalid=false;
  mb_ftp_manager.loggin.userloggined=false;

}


void mb_ftp_run()
{
  switch(mb_ftp_manager.state)
  {
    case MB_FTP_DISABLED:
		 mb_ftp_enabled_check();  
		 break;
    case MB_FTP_CONTROL_START:
	     if(mb_ftp_create_controlanddata_socket(&mb_ftp_manager.sc_sd,MB_FTP_CMD_PORT))
	     {
           mb_ftp_manager.state=MB_FTP_CONTROL_CONNECTED;
		 }
		 break;

    case MB_FTP_CONTROL_CONNECTED:
  	     if (mb_ftp_manager.cc_sd < 0 &&  mb_ftp_manager.data_trans_state == MB_FTP_DATATRANS_DISCONNECTED) 
		 {
           if (MB_FTP_RESULT_OK == mb_ftp_wait_for_connection(mb_ftp_manager.sc_sd, &mb_ftp_manager.cc_sd, &mb_ftp_manager.ip_addr)) 
 		  {
             mb_ftp_manager.txRetries = 0;
             mb_ftp_manager.logginRetries = 0;
             mb_ftp_manager.ctimeout = 0;
             mb_ftp_manager.loggin.uservalid = false;
             mb_ftp_manager.loggin.passvalid = false;
			 mb_ftp_manager.loggin.userloggined=false;
             strcpy (mb_ftp_path, "/");
             mb_ftp_send_reply (220, "Makeblock:Micropython  FTP Server");
           }
         }
		 else
		 {
		   mb_ftp_cmd_process();
		 }
		 break;	 
    case MB_FTP_STE_CONTINUE_LISTING:
         // go on with listing only if the transmit buffer is empty
         if (SOCKETFIFO_IsEmpty())
		 {
		   uint32_t listsize=0;
           mb_ftp_list_dir((char *)mb_ftp_manager.ftpBuffer, MB_FTP_BUFFER_SIZE, &listsize);
           if (listsize > 0)
		   {
             mb_ftp_send_reply(100, NULL);
			 mb_ftp_send_data_to_fifo(listsize);
           } 
		   else
		   {
             mb_ftp_send_reply(226, NULL);
             mb_ftp_manager.state = MB_FTP_STE_END_TRANSFER;
           }
           mb_ftp_manager.ctimeout = 0;
         }
	 
		 break;
    case MB_FTP_STE_CONTINUE_FILE_TX:
		 break;
    case MB_FTP_STE_CONTINUE_FILE_RX:
		 break;
	case MB_FTP_STE_END_TRANSFER:
		 break;	 
	default:
		 break;

  }
  /*process the data transmit channel*/
  if(mb_ftp_manager.loggin.userloggined)//fftust:only do it after the client has loggined
  {
    switch(mb_ftp_manager.data_trans_state)
    {
      case MB_FTP_DATATRANS_DISCONNECTED:
	  	   break;
	  case MB_FTP_DATATRANS_LISTEN_FOR_DATA:
           if (MB_FTP_RESULT_OK == mb_ftp_wait_for_connection(mb_ftp_manager.sd_sd, &mb_ftp_manager.cd_sd, NULL))
		   {
             mb_ftp_manager.dtimeout = 0;
             mb_ftp_manager.data_trans_state = MB_FTP_DATATRANS_CONNECTED;
			 mb_ftp_send_reply(1000,"makeblock:the data channel have connected");//fftust:debug
           } 
		   else if (mb_ftp_manager.dtimeout++ > 100000000/*FTP_DATA_TIMEOUT_MS / FTP_CYCLE_TIME_MS*/) 
		   {
             mb_ftp_manager.dtimeout = 0;
             /* close the listening socket*/
             mb_ftp_close_socket(&mb_ftp_manager.sd_sd);
             mb_ftp_manager.data_trans_state= MB_FTP_DATATRANS_DISCONNECTED;
           }
		   break;
      case MB_FTP_DATATRANS_CONNECTED:
           if (mb_ftp_manager.state == MB_FTP_CONTROL_CONNECTED && mb_ftp_manager.dtimeout++ > 100000000/*FTP_DATA_TIMEOUT_MS / FTP_CYCLE_TIME_MS*/)
		   {
             mb_ftp_manager.dtimeout=0;
 			 // close the listening and the data socket
             mb_ftp_close_socket(&mb_ftp_manager.sd_sd);
             mb_ftp_close_socket(&mb_ftp_manager.cd_sd);
             //ftp_close_filesystem_on_error ();
             mb_ftp_manager.data_trans_state = MB_FTP_DATATRANS_DISCONNECTED;
           }
		   break;

	}


  }

  
  mb_ftp_send_from_fifo();//fftust:send the data  that haved writed to the fifo 
  
  if (mb_ftp_manager.cd_sd < 0 && (mb_ftp_manager.state > MB_FTP_CONTROL_CONNECTED)) //fftust:if data disconnected, stop the data transmit 
  {
    mb_ftp_manager.data_trans_state =MB_FTP_DATATRANS_DISCONNECTED;
    mb_ftp_manager.state = MB_FTP_CONTROL_CONNECTED;
  }
}

STATIC void mb_ftp_cmd_process()
{
 
  int32_t receivedata_len;
  char *bufptr = (char *)mb_ftp_cmd_buffer;
  mb_ftp_result_t result;
  FRESULT fres;
  FILINFO fno;
  
  mb_ftp_manager.closechild = false;
  if (MB_FTP_RESULT_OK == (result = mb_ftp_receive_data(mb_ftp_manager.cc_sd, mb_ftp_cmd_buffer, MB_FTP_MAX_PARAM_SIZE + MB_FTP_CMD_SIZE_MAX, &receivedata_len)))
  {
    
	mb_ftp_cmd_index_t cmd = mb_ftp_pop_command(&bufptr);
	//mb_ftp_send_data_non_blocking(mb_ftp_manager.cc_sd,&receivedata_len,1); //fftust: debug
	switch(cmd)
	{
      case MB_FTP_CMD_PASV:
      {
        mb_ftp_close_socket(&mb_ftp_manager.cd_sd);
        mb_ftp_manager.data_trans_state = MB_FTP_DATATRANS_DISCONNECTED;
        bool socketcreated = true;
        if (mb_ftp_manager.sd_sd < 0) 
		{
          socketcreated = mb_ftp_create_controlanddata_socket(&mb_ftp_manager.sd_sd, MB_FTP_PASIVE_DATA_PORT);
        }
        if (socketcreated)
		{
          uint8_t *pip = (uint8_t *)&mb_ftp_manager.ip_addr;
          mb_ftp_manager.dtimeout = 0;
          snprintf((char *)mb_ftp_manager.ftpBuffer, MB_FTP_BUFFER_SIZE, "(%u,%u,%u,%u,%u,%u)", pip[0], pip[1], pip[2], pip[3], (MB_FTP_PASIVE_DATA_PORT >> 8), (MB_FTP_PASIVE_DATA_PORT & 0xFF));
          mb_ftp_manager.data_trans_state = MB_FTP_DATATRANS_LISTEN_FOR_DATA; //fftust:mb_ftp_run() will deal the later process 
          mb_ftp_send_reply(227, (char *)mb_ftp_manager.ftpBuffer);//fftust:access the pasive mode
        } 
		else
		{
          mb_ftp_send_reply(425, NULL);//fftust:cann't open the data connector
        }
      }
      break;
	  case MB_FTP_CMD_USER:   
           mb_ftp_pop_param (&bufptr, mb_ftp_scratch_buffer);
		   //mb_ftp_send_data_non_blocking(mb_ftp_manager.cc_sd,mb_ftp_scratch_buffer,strlen(mb_ftp_scratch_buffer));  //fftust: debug
           if (!memcmp(mb_ftp_scratch_buffer, mb_ftp_user, MAX(strlen(mb_ftp_scratch_buffer), strlen(mb_ftp_user))))
		   {
             mb_ftp_manager.loggin.uservalid = true && (strlen(mb_ftp_user) == strlen(mb_ftp_scratch_buffer));
			 if(mb_ftp_manager.loggin.uservalid)
			 {
               mb_ftp_send_reply(331, NULL);
			 }  
           }
           
		   break;
	  case MB_FTP_CMD_PASS:
           mb_ftp_pop_param (&bufptr, mb_ftp_scratch_buffer);
           if (!memcmp(mb_ftp_scratch_buffer, mb_ftp_pass, MAX(strlen(mb_ftp_scratch_buffer), strlen(mb_ftp_pass))) && mb_ftp_manager.loggin.uservalid)
		   {
             mb_ftp_manager.loggin.passvalid = true && (strlen(mb_ftp_pass) == strlen(mb_ftp_scratch_buffer));
             if (mb_ftp_manager.loggin.passvalid)
			 {
               mb_ftp_manager.loggin.userloggined=true; //fftust:client loggined,client can open a data connector to read/write data
			   mb_ftp_send_reply(230, NULL);
               break;
             }
           }
           mb_ftp_send_reply(530, NULL);
		   break;
     case MB_FTP_CMD_CWD:
            {
              fres = FR_NO_PATH;
              mb_ftp_pop_param (&bufptr, mb_ftp_scratch_buffer);
              mb_ftp_open_child (mb_ftp_path, mb_ftp_scratch_buffer);
              if ((mb_ftp_path[0] == '/' && mb_ftp_path[1] == '\0') || ((fres = f_opendir (&mb_fatfs,&mb_ftp_manager.dp, mb_ftp_path)) == FR_OK))
			  {
                if (fres == FR_OK)
				{
                  f_closedir(&mb_ftp_manager.dp);
                }
                mb_ftp_send_reply(250, NULL);
                } 
			    else
			  	{
                  mb_ftp_close_child (mb_ftp_path);
                  mb_ftp_send_reply(550, NULL);
                }
            }
            break;
     case MB_FTP_CMD_LIST:
	 	  if (mb_ftp_open_dir_for_listing(mb_ftp_path) == MB_FTP_RESULT_CONTINUE)
		  {
			mb_ftp_manager.state = MB_FTP_STE_CONTINUE_LISTING;
            mb_ftp_send_reply(150, NULL);
          } 
		  else
		  {
            mb_ftp_send_reply(550, NULL);
          }
          break;  

     case MB_FTP_CMD_DELE:
     case MB_FTP_CMD_RMD:
          mb_ftp_get_param_and_open_child (&bufptr);
          if (FR_OK == f_unlink(&mb_fatfs,mb_ftp_path))
 		  {
            mb_ftp_send_reply(250, NULL);
          } 
 		  else
 		  {
            mb_ftp_send_reply(550, NULL);
          }
          break;
     case MB_FTP_CMD_MKD:
          mb_ftp_get_param_and_open_child (&bufptr);
          if (FR_OK == f_mkdir(&mb_fatfs,mb_ftp_path))
   		  {
            mb_ftp_send_reply(250, NULL);
          } 
   		  else 
   		  {
            mb_ftp_send_reply(550, NULL);
          }
          break;
     case MB_FTP_CMD_RNFR:
          mb_ftp_get_param_and_open_child (&bufptr);
          if (FR_OK == f_stat (&mb_fatfs,mb_ftp_path, &fno)) 
		  {
            mb_ftp_send_reply(350, NULL);
            // save the current path
            strcpy ((char *)mb_ftp_manager.ftpBuffer, mb_ftp_path);
          } 
		  else
		  {
            mb_ftp_send_reply(550, NULL);
          }
          break;
     case MB_FTP_CMD_RNTO:
          mb_ftp_get_param_and_open_child (&bufptr);
          // old path was saved in the data buffer
          if (FR_OK == (fres = f_rename (&mb_fatfs,(char *)mb_ftp_manager.ftpBuffer, mb_ftp_path)))
		  {
            mb_ftp_send_reply(250, NULL);
          } 
		  else
		  {
            mb_ftp_send_reply(550, NULL);
          }
          break;
      case MB_FTP_CMD_PWD:
	  	   mb_ftp_send_reply(257, mb_ftp_path);
           break;
      case MB_FTP_CMD_XPWD:
           mb_ftp_send_reply(257, mb_ftp_path);
           break;
	  case MB_FTP_CMD_TYPE:
           mb_ftp_send_reply(200, NULL);
           break;   
	  case MB_FTP_CMD_FEAT:
           mb_ftp_send_reply(211, "no-features");
           break;
      case MB_FTP_CMD_SYST:
           mb_ftp_send_reply(215, "UNIX Type: L8");
           break;
	  case MB_FTP_CMD_NOOP:
           mb_ftp_send_reply(200, NULL);	   
      case MB_FTP_CMD_QUIT:
	 	   mb_ftp_send_reply(221, NULL);
           break;
      default:
	  	   mb_ftp_send_reply(502,"the command is unknow");
	  	   break;






    }
    
  }
  else if (result == MB_FTP_RESULT_CONTINUE) 
  {
  	if (mb_ftp_manager.ctimeout++ > 100000000/*(servers_get_timeout() / FTP_CYCLE_TIME_MS)*/) 
	{
	  mb_ftp_send_reply(221, NULL); //fftust:debug
	  mb_ftp_manager.ctimeout=0;
  	}
  } 
  else
  {
    //mb_ftp_send_reply (1000, "Makeblock:socket closed");
	mb_ftp_close_client_sockets();
	//printf("makeblock:socketclosed");
    //ESP_LOGI("makeblock:","socketclosed\n");
  }
  #if 0
   if (E_FTP_RESULT_OK == (result = ftp_recv_non_blocking(ftp_data.c_sd, ftp_cmd_buffer, FTP_MAX_PARAM_SIZE + FTP_CMD_SIZE_MAX, &len))) {
  	// bufptr is moved as commands are being popped
  	ftp_cmd_index_t cmd = ftp_pop_command(&bufptr);
  	if (!ftp_data.loggin.passvalid && (cmd != E_FTP_CMD_USER && cmd != E_FTP_CMD_PASS && cmd != E_FTP_CMD_QUIT)) {
  		ftp_send_reply(332, NULL);
  		return;
  	}
  	switch (cmd) {
  	case E_FTP_CMD_FEAT:
  		ftp_send_reply(211, "no-features");
  		break;
  	case E_FTP_CMD_SYST:
  		ftp_send_reply(215, "UNIX Type: L8");
  		break;
  	case E_FTP_CMD_CDUP:
  		ftp_close_child(ftp_path);
  		ftp_send_reply(250, NULL);
  		break;
  	case E_FTP_CMD_CWD:
  		{
  			fres = FR_NO_PATH;
  			ftp_pop_param (&bufptr, ftp_scratch_buffer);
  			ftp_open_child (ftp_path, ftp_scratch_buffer);
  			if ((ftp_path[0] == '/' && ftp_path[1] == '\0') || ((fres = f_opendir (&ftp_data.dp, ftp_path)) == FR_OK)) {
  				if (fres == FR_OK) {
  					f_closedir(&ftp_data.dp);
  				}
  				ftp_send_reply(250, NULL);
  			} else {
  				ftp_close_child (ftp_path);
  				ftp_send_reply(550, NULL);
  			}
  		}
  		break;
  	case E_FTP_CMD_PWD:
  	case E_FTP_CMD_XPWD:
  		ftp_send_reply(257, ftp_path);
  		break;
  	case E_FTP_CMD_SIZE:
  		{
  			ftp_get_param_and_open_child (&bufptr);
  			if (FR_OK == f_stat (ftp_path, &fno)) {
  				// send the size
  				snprintf((char *)ftp_data.dBuffer, FTP_BUFFER_SIZE, "%u", (uint32_t)fno.fsize);
  				ftp_send_reply(213, (char *)ftp_data.dBuffer);
  			} else {
  				ftp_send_reply(550, NULL);
  			}
  		}
  		break;
  	case E_FTP_CMD_MDTM:
  		ftp_get_param_and_open_child (&bufptr);
  		if (FR_OK == f_stat (ftp_path, &fno)) {
  			// send the last modified time
  			snprintf((char *)ftp_data.dBuffer, FTP_BUFFER_SIZE, "%u%02u%02u%02u%02u%02u",
  					 1980 + ((fno.fdate >> 9) & 0x7f), (fno.fdate >> 5) & 0x0f,
  					 fno.fdate & 0x1f, (fno.ftime >> 11) & 0x1f,
  					 (fno.ftime >> 5) & 0x3f, 2 * (fno.ftime & 0x1f));
  			ftp_send_reply(213, (char *)ftp_data.dBuffer);
  		} else {
  			ftp_send_reply(550, NULL);
  		}
  		break;
  	case E_FTP_CMD_TYPE:
  		ftp_send_reply(200, NULL);
  		break;
  	case E_FTP_CMD_USER:
  		ftp_pop_param (&bufptr, ftp_scratch_buffer);
  		if (!memcmp(ftp_scratch_buffer, servers_user, MAX(strlen(ftp_scratch_buffer), strlen(servers_user)))) {
  			ftp_data.loggin.uservalid = true && (strlen(servers_user) == strlen(ftp_scratch_buffer));
  		}
  		ftp_send_reply(331, NULL);
  		break;
  	case E_FTP_CMD_PASS:
  		ftp_pop_param (&bufptr, ftp_scratch_buffer);
  		if (!memcmp(ftp_scratch_buffer, servers_pass, MAX(strlen(ftp_scratch_buffer), strlen(servers_pass))) &&
  				ftp_data.loggin.uservalid) {
  			ftp_data.loggin.passvalid = true && (strlen(servers_pass) == strlen(ftp_scratch_buffer));
  			if (ftp_data.loggin.passvalid) {
  				ftp_send_reply(230, NULL);
  				break;
  			}
  		}
  		ftp_send_reply(530, NULL);
  		break;
  	case E_FTP_CMD_PASV:
  		{
  			// some servers (e.g. google chrome) send PASV several times very quickly
  			servers_close_socket(&ftp_data.d_sd);
  			ftp_data.substate = E_FTP_STE_SUB_DISCONNECTED;
  			bool socketcreated = true;
  			if (ftp_data.ld_sd < 0) {
  				socketcreated = ftp_create_listening_socket(&ftp_data.ld_sd, FTP_PASIVE_DATA_PORT, FTP_DATA_CLIENTS_MAX - 1);
  			}
  			if (socketcreated) {
  				uint8_t *pip = (uint8_t *)&ftp_data.ip_addr;
  				ftp_data.dtimeout = 0;
  				snprintf((char *)ftp_data.dBuffer, FTP_BUFFER_SIZE, "(%u,%u,%u,%u,%u,%u)",
  						 pip[0], pip[1], pip[2], pip[3], (FTP_PASIVE_DATA_PORT >> 8), (FTP_PASIVE_DATA_PORT & 0xFF));
  				ftp_data.substate = E_FTP_STE_SUB_LISTEN_FOR_DATA;
  				ftp_send_reply(227, (char *)ftp_data.dBuffer);
  			} else {
  				ftp_send_reply(425, NULL);
  			}
  		}
  		break;
  	case E_FTP_CMD_LIST:
  		if (ftp_open_dir_for_listing(ftp_path) == E_FTP_RESULT_CONTINUE) {
  			ftp_data.state = E_FTP_STE_CONTINUE_LISTING;
  			ftp_send_reply(150, NULL);
  		} else {
  			ftp_send_reply(550, NULL);
  		}
  		break;
  	case E_FTP_CMD_RETR:
  		ftp_get_param_and_open_child (&bufptr);
  		if (ftp_open_file (ftp_path, FA_READ)) {
  			ftp_data.state = E_FTP_STE_CONTINUE_FILE_TX;
  			ftp_send_reply(150, NULL);
  		} else {
  			ftp_data.state = E_FTP_STE_END_TRANSFER;
  			ftp_send_reply(550, NULL);
  		}
  		break;
  	case E_FTP_CMD_STOR:
  		ftp_get_param_and_open_child (&bufptr);
  		// first check if a software update is being requested
  		if (updater_check_path (ftp_path)) {
  			if (updater_start()) {
  				ftp_data.special_file = true;
  				ftp_data.state = E_FTP_STE_CONTINUE_FILE_RX;
  				ftp_send_reply(150, NULL);
  			} else {
  				// to unlock the updater
  				updater_finish();
  				ftp_data.state = E_FTP_STE_END_TRANSFER;
  				ftp_send_reply(550, NULL);
  			}
  		} else {
  			if (ftp_open_file (ftp_path, FA_WRITE | FA_CREATE_ALWAYS)) {
  				ftp_data.state = E_FTP_STE_CONTINUE_FILE_RX;
  				ftp_send_reply(150, NULL);
  			} else {
  				ftp_data.state = E_FTP_STE_END_TRANSFER;
  				ftp_send_reply(550, NULL);
  			}
  		}
  		break;
  	case E_FTP_CMD_DELE:
  	case E_FTP_CMD_RMD:
  		ftp_get_param_and_open_child (&bufptr);
  		if (FR_OK == f_unlink(ftp_path)) {
  			ftp_send_reply(250, NULL);
  		} else {
  			ftp_send_reply(550, NULL);
  		}
  		break;
  	case E_FTP_CMD_MKD:
  		ftp_get_param_and_open_child (&bufptr);
  		if (FR_OK == f_mkdir(ftp_path)) {
  			ftp_send_reply(250, NULL);
  		} else {
  			ftp_send_reply(550, NULL);
  		}
  		break;
  	case E_FTP_CMD_RNFR:
  		ftp_get_param_and_open_child (&bufptr);
  		if (FR_OK == f_stat (ftp_path, &fno)) {
  			ftp_send_reply(350, NULL);
  			// save the current path
  			strcpy ((char *)ftp_data.dBuffer, ftp_path);
  		} else {
  			ftp_send_reply(550, NULL);
  		}
  		break;
  	case E_FTP_CMD_RNTO:
  		ftp_get_param_and_open_child (&bufptr);
  		// old path was saved in the data buffer
  		if (FR_OK == (fres = f_rename ((char *)ftp_data.dBuffer, ftp_path))) {
  			ftp_send_reply(250, NULL);
  		} else {
  			ftp_send_reply(550, NULL);
  		}
  		break;
  	case E_FTP_CMD_NOOP:
  		ftp_send_reply(200, NULL);
  		break;
  	case E_FTP_CMD_QUIT:
  		ftp_send_reply(221, NULL);
  		break;
  	default:
  		// command not implemented
  		ftp_send_reply(502, NULL);
  		break;
  	}
  
  	if (ftp_data.closechild) {
  		ftp_return_to_previous_path(ftp_path, ftp_scratch_buffer);
  	}
  } else if (result == E_FTP_RESULT_CONTINUE) {
  	if (ftp_data.ctimeout++ > (servers_get_timeout() / FTP_CYCLE_TIME_MS)) {
  		ftp_send_reply(221, NULL);
  	}
  } else {
  	ftp_close_cmd_data();
  }
  #endif
}


