/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Module for setup ble to device and start ble device with a makeblock profile
 * @file    mb_ble_device.c
 * @author  Leo lu
 * @version V1.0.0
 * @date    2017/04/27
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
 * This file setup ble to device and start ble device with a makeblock profile.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 *  Leo lu            2017/04/27       1.0.0            Initial version
 * </pre>
 *
 */
	
#include <stdint.h>
#include <string.h>
#include <stdio.h>
	
	
#include "py/mpstate.h"
#include "py/runtime.h"
	
#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "uart.h"

#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "bt.h"
#include "bta_api.h"

#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "rom/ets_sys.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"

	
/******************************************************************************
 MACRO DEFINITION
 ******************************************************************************/
 
/******************************************************************************
 DECLARE CONSTANTS
 ******************************************************************************/

/******************************************************************************
 DEFINE TYPES
 ******************************************************************************/

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
#define SAMPLE_DEVICE_NAME              "Makeblock_ESP32"
 
#define	MK_UART_PROFILE_NUM				1
#define	MK_UART_PROFILE_APP_IDX			0
#define ESP_MK_UART_APP_ID				0x56
#define MK_UART_SVC_INST_ID	    		0
 
#define GATTS_TABLE_TAG 				("GATTS_TABLE_DEMO")
#define GATTS_DEMO_CHAR_VAL_LEN_MAX		(0x40)
#define	MK_GATT_SERVER_TAG				("MakeBlock")
#define time_after( a, b )				( (int)(b) - (int)(a) < 0  )

// makeblock bluetooth server
enum
{
	// Service decleration
	MK_IDX_SVC,

	// Read notify Charateristic definition
	MK_IDX_RX_CHAR_DECL,
	MK_IDX_RX_CHAR_VAL,
	MK_IDX_RX_CHAR_CCC,

	// Write without respond Charateristic definition
	MK_IDX_TX_CHAR_DECL,
	MK_IDX_TX_CHAR_VAL,

	// Service decleration
	MK_IDX_CTRL_SVC,

	// GPIO read/write characteristic
	MK_IDX_GPIO1_CHAR_DECL,
	MK_IDX_GPIO1_CHAR_VALUE,

	// GPIO read/write characteristic
	MK_IDX_GPIO2_CHAR_DECL,
	MK_IDX_GPIO2_CHAR_VALUE,

	// GPIO read/write characteristic
	MK_IDX_GPIO3_CHAR_DECL,
	MK_IDX_GPIO3_CHAR_VALUE,

	// GPIO read/write characteristic
	MK_IDX_GPIO4_CHAR_DECL,
	MK_IDX_GPIO4_CHAR_VALUE,

	// Uart config characteristic
	MK_IDX_UART_CFG_CHAR_DECL,
	MK_IDX_UART_CFG_CHAR_VALUE,

	// Ble adv config
	MK_IDX_ADV_CFG_CHAR_DECL,
	MK_IDX_ADV_CFG_CHAR_VALUE,

	// Ble sleep control point
	MK_IDX_BLE_SLEEP_CHAR_DECL,
	MK_IDX_BLE_SLEEP_CHAR_VALUE,

	// 
	MK_IDX_NB,
};

uint16_t mk_uart_handle_table[MK_IDX_NB];
// static uint8_t mk_uart_uuid[2] = { 0xFF, 0xE1 };

static esp_ble_adv_data_t heart_rate_adv_config = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x20,
    .max_interval = 0x40,
    .appearance = 0x00,
    .manufacturer_len = 0, //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data =  NULL, //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = 0,
    .p_service_uuid = NULL,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_params_t heart_rate_adv_params = {
    .adv_int_min        = 0x20,
    .adv_int_max        = 0x40,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    //.peer_addr            =
    //.peer_addr_type       =
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

struct gatts_profile_inst {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
};

static void mk_uart_gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

/* One gatt-based profile one app_id and one gatts_if, this array will store the gatts_if returned by ESP_GATTS_REG_EVT */
static struct gatts_profile_inst mk_uart_profile_tab[MK_UART_PROFILE_NUM] = {
    [MK_UART_PROFILE_APP_IDX] = {
        .gatts_cb = mk_uart_gatts_profile_event_handler,
        .gatts_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
        .conn_id = 0xFFFF,
    },
    
};


/*
 * HTPT PROFILE ATTRIBUTES
 ****************************************************************************************
 */


/*
 *  Heart Rate PROFILE ATTRIBUTES
 ****************************************************************************************
 */
#define CHAR_DECLARATION_SIZE   (sizeof(uint8_t))
static const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
static const uint8_t char_prop_notify = ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static const uint8_t char_prop_read = ESP_GATT_CHAR_PROP_BIT_READ;
static const uint8_t char_prop_read_write = ESP_GATT_CHAR_PROP_BIT_WRITE|ESP_GATT_CHAR_PROP_BIT_READ;
static const uint8_t char_prop_write_no_rsp = ESP_GATT_CHAR_PROP_BIT_WRITE_NR;
static const uint8_t char_prop_read_write_write_no_rsp = ESP_GATT_CHAR_PROP_BIT_READ|ESP_GATT_CHAR_PROP_BIT_WRITE|ESP_GATT_CHAR_PROP_BIT_WRITE_NR;


/*
 *  MK uart profile attribute
 ****************************************************************************************
 */
#define	ESP_GATT_UUID_MK_UART_SVC			(0xFFE1)
#define	ESP_GATT_UUID_MK_UART_RX_UUID		(0xFFE2)
#define	ESP_GATT_UUID_MK_UART_TX_UUID		(0xFFE3)
#define	ESP_GATT_UUID_MK_CTRL_SVC			(0xFFE4)
#define	ESP_GATT_UUID_MK_GPIO1				(0xFFE5)
#define	ESP_GATT_UUID_MK_GPIO2				(0xFFE6)
#define	ESP_GATT_UUID_MK_GPIO3				(0xFFE7)
#define	ESP_GATT_UUID_MK_GPIO4				(0xFFE8)
#define ESP_GATT_UUID_MK_UART_CFG			(0xFFE9)
#define	ESP_GATT_UUID_MK_ADV_CFG			(0xFFEA)
#define	ESP_GATT_UUID_MK_BLE_SLEEP_CFG		(0xFFEB)
#define ESP_GATT_MK_UART_RX_VAL_MAX_LEN		(64)
#define ESP_GATT_MK_UART_TX_VAL_MAX_LEN		(64)
#define ESP_GATT_MK_GPIO_VAL_MAX_LEN		(1)


// ----------------------------------------------------------------------
/// mk uart service
static const uint16_t mk_uart_svc = ESP_GATT_UUID_MK_UART_SVC;

/// mk uart RX Characteristic, notify
static const uint16_t mk_uart_rx_uuid = ESP_GATT_UUID_MK_UART_RX_UUID;
static const uint8_t mk_uart_rx_char_ccc[2] ={ 0x00, 0x00};

/// mk uart TX characteristic, write without respond
static const uint16_t mk_uart_tx_uuid = ESP_GATT_UUID_MK_UART_TX_UUID;

//----------------------------------------------------------------------
// mk control service
static const uint16_t mk_control_svc = ESP_GATT_UUID_MK_CTRL_SVC;
static const uint16_t mk_gpio1_uuid = ESP_GATT_UUID_MK_GPIO1;
static const uint16_t mk_gpio2_uuid = ESP_GATT_UUID_MK_GPIO2;
static const uint16_t mk_gpio3_uuid = ESP_GATT_UUID_MK_GPIO3;
static const uint16_t mk_gpio4_uuid = ESP_GATT_UUID_MK_GPIO4;
static const uint16_t mk_uart_cfg_uuid = ESP_GATT_UUID_MK_UART_CFG;
static const uint16_t mk_adv_cfg_uuid = ESP_GATT_UUID_MK_ADV_CFG;
static const uint16_t mk_ble_sleep_uuid = ESP_GATT_UUID_MK_BLE_SLEEP_CFG;

static const esp_gatts_attr_db_t mk_uart_gatt_db[MK_IDX_NB] = 
{
	// 1) service
	[MK_IDX_SVC]						=
	{
		{ESP_GATT_AUTO_RSP}, 
		{
			ESP_UUID_LEN_16, (uint8_t *)(&primary_service_uuid), ESP_GATT_PERM_READ, 
			sizeof(uint16_t), sizeof(mk_uart_svc), (uint8_t *)(&mk_uart_svc)
		}
	},

	// 2) rx char
	[MK_IDX_RX_CHAR_DECL]			  	= 
	{
		{ESP_GATT_AUTO_RSP}, 
		{
			ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
	  		CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)(&char_prop_notify)
	  	}
	},
    [MK_IDX_RX_CHAR_VAL]             	=   
    {
    	{ESP_GATT_AUTO_RSP}, 
		{
			ESP_UUID_LEN_16, (uint8_t *)&mk_uart_rx_uuid, ESP_GATT_PERM_READ,
		    ESP_GATT_MK_UART_RX_VAL_MAX_LEN, 0, NULL
		}
	},
    [MK_IDX_RX_CHAR_CCC]     			=    
    {
    	{ESP_GATT_AUTO_RSP}, 
		{
			ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
      		sizeof(uint16_t), sizeof(mk_uart_rx_char_ccc), (uint8_t *)(mk_uart_rx_char_ccc)
      	}
	},
	  
    // 3) tx char
    [MK_IDX_TX_CHAR_DECL]          		= 
    {
    	{ESP_GATT_AUTO_RSP}, 
		{
			ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      		CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)(&char_prop_write_no_rsp)
      	}
	},
    [MK_IDX_TX_CHAR_VAL] 	            = 
    {	
    	{ESP_GATT_AUTO_RSP}, 
		{
			ESP_UUID_LEN_16, (uint8_t *)&mk_uart_tx_uuid, ESP_GATT_PERM_WRITE,
		    ESP_GATT_MK_UART_TX_VAL_MAX_LEN, 0, NULL
		}
	},

	// -----------------------------------------------------------------------------------
	// 1) service
	[MK_IDX_CTRL_SVC]					=
	{
		{ESP_GATT_AUTO_RSP},
		{
			ESP_UUID_LEN_16, (uint8_t *)(&primary_service_uuid), ESP_GATT_PERM_READ,
		    sizeof(uint16_t), sizeof(mk_control_svc), (uint8_t *)(&mk_control_svc)
		}
	},
	
    // 2) gpio1 char
    [MK_IDX_GPIO1_CHAR_DECL] 	      	= 
    {	
    	{ESP_GATT_AUTO_RSP}, 
		{
			ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
		    CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)(&char_prop_read_write_write_no_rsp)
		}
	},
    [MK_IDX_GPIO1_CHAR_VALUE] 	      	= 
    {	
    	{ESP_GATT_AUTO_RSP}, 
		{
			ESP_UUID_LEN_16, (uint8_t *)&mk_gpio1_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
		    ESP_GATT_MK_GPIO_VAL_MAX_LEN, 0, NULL
		}
	},

    // 3) gpio2 char
    [MK_IDX_GPIO2_CHAR_DECL] 	      	= 
    {	
    	{ESP_GATT_AUTO_RSP}, 
		{
			ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
		    CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)(&char_prop_read_write_write_no_rsp)
		}
	},
    [MK_IDX_GPIO2_CHAR_VALUE] 	      	= 
    {	
    	{ESP_GATT_AUTO_RSP}, 
		{
			ESP_UUID_LEN_16, (uint8_t *)&mk_gpio2_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
		    ESP_GATT_MK_GPIO_VAL_MAX_LEN, 0, NULL
		}
	},

    // 4) gpio3 char
    [MK_IDX_GPIO3_CHAR_DECL] 	      	= 
    {	
    	{ESP_GATT_AUTO_RSP}, 
		{
			ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
		    CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)(&char_prop_read_write_write_no_rsp)
		}
	},
    [MK_IDX_GPIO3_CHAR_VALUE] 	      	= 
    {	
    	{ESP_GATT_AUTO_RSP}, 
		{
			ESP_UUID_LEN_16, (uint8_t *)&mk_gpio3_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
		    ESP_GATT_MK_GPIO_VAL_MAX_LEN, 0, NULL
		}
	},

    // 5) gpio4 char
    [MK_IDX_GPIO4_CHAR_DECL] 	      	= 
    {	
    	{ESP_GATT_AUTO_RSP}, 
		{
			ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
		    CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)(&char_prop_read_write_write_no_rsp)
		}
	},
    [MK_IDX_GPIO4_CHAR_VALUE] 	      	= 
    {	
    	{ESP_GATT_AUTO_RSP}, 
		{
			ESP_UUID_LEN_16, (uint8_t *)&mk_gpio4_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
		    ESP_GATT_MK_GPIO_VAL_MAX_LEN, 0, NULL
		}
	},

	// 6)  uart config char
    [MK_IDX_UART_CFG_CHAR_DECL] 	  	= 
    {	
    	{ESP_GATT_AUTO_RSP}, 
		{
			ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
		    CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)(&char_prop_read_write_write_no_rsp)
		}
	},
    [MK_IDX_UART_CFG_CHAR_VALUE] 	      	= 
    {	
    	{ESP_GATT_AUTO_RSP}, 
		{
			ESP_UUID_LEN_16, (uint8_t *)&mk_uart_cfg_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
		    ESP_GATT_MK_GPIO_VAL_MAX_LEN, 0, NULL
		}
	},

	// 7)  ble advertising config char
    [MK_IDX_ADV_CFG_CHAR_DECL] 	  			= 
    {	
    	{ESP_GATT_AUTO_RSP}, 
		{
			ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
		    CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)(&char_prop_read_write_write_no_rsp)
		}
	},
    [MK_IDX_ADV_CFG_CHAR_VALUE] 	      	= 
    {	
    	{ESP_GATT_AUTO_RSP}, 
		{
			ESP_UUID_LEN_16, (uint8_t *)&mk_adv_cfg_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
		    ESP_GATT_MK_GPIO_VAL_MAX_LEN, 0, NULL
		}
	},	

	// 8)  ble sleep control char
    [MK_IDX_BLE_SLEEP_CHAR_DECL] 	  		= 
    {	
    	{ESP_GATT_AUTO_RSP}, 
		{
			ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
		    CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)(&char_prop_read_write_write_no_rsp)
		}
	},
    [MK_IDX_BLE_SLEEP_CHAR_VALUE] 	      	= 
    {	
    	{ESP_GATT_AUTO_RSP}, 
		{
			ESP_UUID_LEN_16, (uint8_t *)&mk_ble_sleep_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
		    ESP_GATT_MK_GPIO_VAL_MAX_LEN, 0, NULL
		}
	},	
};

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    ESP_LOGE(GATTS_TABLE_TAG, "GAP_EVT, event %d\n", event);

    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
		ESP_LOGI( GATTS_TABLE_TAG, "GAP start adverting...\n" );
        esp_ble_gap_start_advertising(&heart_rate_adv_params);
        break;
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        //advertising start complete event to indicate advertising start successfully or failed
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(GATTS_TABLE_TAG, "Advertising start failed\n");
        }
        break;
    default:
		ESP_LOGE( GATTS_TABLE_TAG, "GAP UNHANDLE EVENT: %d\n", event );
        break;
    }
}

static void mk_uart_gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param )
{
    ESP_LOGE(GATTS_TABLE_TAG, "mk uart gatts profile event = %d\n", event);
    switch (event) {
    	case ESP_GATTS_REG_EVT:
			ESP_LOGI(GATTS_TABLE_TAG, "%s %d\n", __func__, __LINE__);
			esp_ble_gap_set_device_name(SAMPLE_DEVICE_NAME);
        	ESP_LOGI(GATTS_TABLE_TAG, "%s %d\n", __func__, __LINE__);
       		esp_ble_gap_config_adv_data(&heart_rate_adv_config);
        	ESP_LOGI(GATTS_TABLE_TAG, "%s %d\n", __func__, __LINE__);
			esp_ble_gatts_create_attr_tab(mk_uart_gatt_db, gatts_if, MK_IDX_NB, MK_UART_SVC_INST_ID);
       	break;
    	case ESP_GATTS_READ_EVT:
       
       	 break;
    	case ESP_GATTS_WRITE_EVT: 
			ESP_LOGI( GATTS_TABLE_TAG, "handle = %d, len = %d\n", param->write.handle, param->write.len );
			int idx;
			for ( idx = 0; idx < param->write.len; idx++ ) {
				ESP_LOGI( GATTS_TABLE_TAG, "%02x", param->write.value[idx] );
			}
			// Send to make block uart
			ESP_LOGI( GATTS_TABLE_TAG, "write to UART TX\n" )
      	break;
    	case ESP_GATTS_EXEC_WRITE_EVT:
		break;
    	case ESP_GATTS_MTU_EVT:
		break;
   	 	case ESP_GATTS_CONF_EVT:
		break;
    	case ESP_GATTS_UNREG_EVT:
        	break;
    	case ESP_GATTS_DELETE_EVT:
        	break;
    	case ESP_GATTS_START_EVT:
        	break;
    	case ESP_GATTS_STOP_EVT:
        	break;
    	case ESP_GATTS_CONNECT_EVT:
			ESP_LOGI( GATTS_TABLE_TAG, "mk uart profile app conn_id set to %d\n", param->connect.conn_id );
			mk_uart_profile_tab[MK_UART_PROFILE_APP_IDX].conn_id = param->connect.conn_id;
        	break;
    	case ESP_GATTS_DISCONNECT_EVT:
			// to do re-advertising
			ESP_LOGI( GATTS_TABLE_TAG, "Peer device disconnect, re-advertising\n" );
			esp_ble_gap_start_advertising(&heart_rate_adv_params);
		break;
    	case ESP_GATTS_OPEN_EVT:
		break;
    	case ESP_GATTS_CANCEL_OPEN_EVT:
		break;
    	case ESP_GATTS_CLOSE_EVT:
		break;
    	case ESP_GATTS_LISTEN_EVT:
		break;
    	case ESP_GATTS_CONGEST_EVT:
		break;
    case ESP_GATTS_CREAT_ATTR_TAB_EVT:{
		ESP_LOGE(GATTS_TABLE_TAG, "The number handle =%x\n",param->add_attr_tab.num_handle);
		if(param->add_attr_tab.num_handle == MK_IDX_NB){
			memcpy(mk_uart_handle_table, param->add_attr_tab.handles, sizeof(mk_uart_handle_table));
			esp_ble_gatts_start_service(mk_uart_handle_table[MK_IDX_SVC]);
			esp_ble_gatts_start_service(mk_uart_handle_table[MK_IDX_CTRL_SVC]);
		}

		break;
	}
		
    default:
        break;
    }
}


static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    ESP_LOGI(GATTS_TABLE_TAG, "GATT GEN EVT %d, gatts if %d\n", event, gatts_if);

    /* If event is register event, store the gatts_if for each profile */
    if (event == ESP_GATTS_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
			if (  ESP_MK_UART_APP_ID == param->reg.app_id ) {
				mk_uart_profile_tab[MK_UART_PROFILE_APP_IDX].gatts_if = gatts_if;
			}
			else {
				ESP_LOGE( GATTS_TABLE_TAG, "can not find the profile for APP_ID %d", param->reg.app_id);
			}
        } else {
            ESP_LOGI(GATTS_TABLE_TAG, "Reg app failed, app_id %04x, status %d\n",
                    param->reg.app_id, 
                    param->reg.status);
            return;
        }
    }
	
    do {
        int idx;
		for (idx = 0; idx < MK_UART_PROFILE_NUM; idx++) {		
			// ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function 
			if (gatts_if == ESP_GATT_IF_NONE || gatts_if == mk_uart_profile_tab[idx].gatts_if) {
				if (mk_uart_profile_tab[idx].gatts_cb) {
					mk_uart_profile_tab[idx].gatts_cb(event, gatts_if, param);
				}
			}
		}
    } while (0);
}

/******************************************************************************/
void mb_ble_device_init( void )
{
	esp_err_t ret;
	
	printf("mb_ble_device_init() been called\n");
	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable controller failed\n", __func__);
        return;
    }
	printf("esp_bt_controller_init() been called\n");

    ret = esp_bt_controller_enable(ESP_BT_MODE_BTDM);
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable controller failed\n", __func__);
        return;
    }

	printf("esp_bt_controller_enable() been called\n");

    ESP_LOGI(GATTS_TABLE_TAG, "%s init bluetooth\n", __func__);
    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s init bluetooth failed\n", __func__);
        return;
    }
	printf("esp_bluedroid_init() been called\n");
	
    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable bluetooth failed\n", __func__);
        return;
    }
	printf("esp_bluedroid_enable() been called\n");
	
    esp_ble_gatts_register_callback(gatts_event_handler);
    esp_ble_gap_register_callback(gap_event_handler);
	esp_ble_gatts_app_register(ESP_MK_UART_APP_ID);
}
