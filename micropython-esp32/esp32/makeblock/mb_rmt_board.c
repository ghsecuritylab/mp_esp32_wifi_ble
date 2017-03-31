/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for makeblock rmt_board module
 * @file    mb_rmt_board.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/03/24
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
 * This file is a drive rmt_board module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 *  fftust            2017/03/24     1.0.0            build the new.
 * </pre>
 *
 */
	
#include <stdint.h>
#include <string.h>
#include <stdio.h>
		
#include "py/mpstate.h"
#include "py/runtime.h"
#include "esp_log.h"

#include "py/nlr.h"
#include "py/obj.h"

#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "uart.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_err.h"


#include "driver/rmt.h"
#include "driver/periph_ctrl.h"
#include "soc/rmt_reg.h"

	
#include "mb_rmt_board.h"
#include "mb_processcmd.h"

	
	/******************************************************************************
	 MACRO DEFINITION
	 ******************************************************************************/

	static const char* NEC_TAG = "NEC";
	
	
    #define RMT_RX_ACTIVE_LEVEL  0  
    #define RMT_TX_CARRIER_EN    1   
	
	
    #define RMT_RX_CHANNEL    0     /*!< RMT channel for transmitter */
    #define RMT_RX_GPIO_NUM   26     /*!< GPIO number for transmitter signal */
    #define RMT_CLK_DIV       100    /*!< RMT counter clock divider */
    #define RMT_TICK_10_US    (80000000/RMT_CLK_DIV/100000)   /*!< RMT counter value for 10 us.(Source clock is APB clock) */
    	
    #define NEC_HEADER_HIGH_US    9000                         /*!< NEC protocol header: positive 9ms */
    #define NEC_HEADER_LOW_US     4500                         /*!< NEC protocol header: negative 4.5ms*/

	#define NEC_BIT_ONE_HIGH_US    560                         /*!< NEC protocol data bit 1: positive 0.56ms */
    #define NEC_BIT_ONE_LOW_US    (2250-NEC_BIT_ONE_HIGH_US)   /*!< NEC protocol data bit 1: negative 1.69ms */
    #define NEC_BIT_ZERO_HIGH_US   560                         /*!< NEC protocol data bit 0: positive 0.56ms */
    #define NEC_BIT_ZERO_LOW_US   (1120-NEC_BIT_ZERO_HIGH_US)  /*!< NEC protocol data bit 0: negative 0.56ms */
    #define NEC_BIT_END            560                         /*!< NEC protocol end: positive 0.56ms */
    #define NEC_BIT_MARGIN         150//20                          /*!< NEC parse margin time */
    	
    #define NEC_ITEM_DURATION(d)  ((d & 0x7fff)*10/RMT_TICK_10_US)  /*!< Parse duration time from memory register value */
    #define NEC_DATA_ITEM_NUM   34  /*!< NEC code item number: header + 32bit data + end */
    #define RMT_TX_DATA_NUM  100    /*!< NEC tx test data number */
    #define rmt_item32_tIMEOUT_US  9500   /*!< RMT receiver timeout value(us) */
	
	/******************************************************************************
	 DECLARE CONSTANTS
	 ******************************************************************************/
	/******************************************************************************
	 DEFINE TYPES
	 ******************************************************************************/
	typedef struct
	{
	  mp_obj_base_t base;
	  
	} mb_rmt_board_obj_t;
	
	/******************************************************************************
	 DECLARE PRIVATE DATA
	 ******************************************************************************/
	STATIC mb_rmt_board_obj_t mb_rmt_board_obj = {};
	
	/******************************************************************************
	 DECLARE PRIVATE FUNCTIONS
	 ******************************************************************************/
	
	/******************************************************************************
	 DEFINE PUBLIC FUNCTIONS
	 ******************************************************************************/
	
	
	/******************************************************************************
     drive functions start
	******************************************************************************/

	inline bool nec_check_in_range(int duration_ticks, int target_us, int margin_us)
	{
		if(( NEC_ITEM_DURATION(duration_ticks) < (target_us + margin_us))
			&& ( NEC_ITEM_DURATION(duration_ticks) > (target_us - margin_us))) {
			return true;
		} else {
			return false;
		}
	}
	
	/*
	 * @brief Check whether this value represents an NEC header
	 */
	static bool nec_header_if(rmt_item32_t* item)
	{
		if((item->level0 == RMT_RX_ACTIVE_LEVEL && item->level1 != RMT_RX_ACTIVE_LEVEL)
			&& nec_check_in_range(item->duration0, NEC_HEADER_HIGH_US, NEC_BIT_MARGIN)
			&& nec_check_in_range(item->duration1, NEC_HEADER_LOW_US, NEC_BIT_MARGIN)) {
			return true;
		}
		return false;
	}
	
	/*
	 * @brief Check whether this value represents an NEC data bit 1
	 */
	static bool nec_bit_one_if(rmt_item32_t* item)
	{
		if((item->level0 == RMT_RX_ACTIVE_LEVEL && item->level1 != RMT_RX_ACTIVE_LEVEL)
			&& nec_check_in_range(item->duration0, NEC_BIT_ONE_HIGH_US, NEC_BIT_MARGIN)
			&& nec_check_in_range(item->duration1, NEC_BIT_ONE_LOW_US, NEC_BIT_MARGIN)) {
			return true;
		}
		return false;
	}
	
	/*
	 * @brief Check whether this value represents an NEC data bit 0
	 */
	static bool nec_bit_zero_if(rmt_item32_t* item)
	{
		if((item->level0 == RMT_RX_ACTIVE_LEVEL && item->level1 != RMT_RX_ACTIVE_LEVEL)
			&& nec_check_in_range(item->duration0, NEC_BIT_ZERO_HIGH_US, NEC_BIT_MARGIN)
			&& nec_check_in_range(item->duration1, NEC_BIT_ZERO_LOW_US, NEC_BIT_MARGIN)) {
			return true;
		}
		return false;
	}
	
	
	/*
	 * @brief Parse NEC 32 bit waveform to address and command.
	 */
	static int nec_parse_items(rmt_item32_t* item, int item_num, uint16_t* addr, uint16_t* data)
	{
		int w_len = item_num;
		if(w_len < NEC_DATA_ITEM_NUM) {
			return -1;
		}
		int i = 0, j = 0;
		if(!nec_header_if(item++)) {
			return -1;
		}
		uint16_t addr_t = 0;
		for(j = 0; j < 16; j++) {
			if(nec_bit_one_if(item)) {
				addr_t |= (1 << j);
			} else if(nec_bit_zero_if(item)) {
				addr_t |= (0 << j);
			} else {
				return -1;
			}
			item++;
			i++;
		}
		uint16_t data_t = 0;
		for(j = 0; j < 16; j++) {
			if(nec_bit_one_if(item)) {
				data_t |= (1 << j);
			} else if(nec_bit_zero_if(item)) {
				data_t |= (0 << j);
			} else {
				return -1;
			}
			item++;
			i++;
		}
		*addr = addr_t;
		*data = data_t;
		return i;
	}
	
	
	/*
	 * @brief RMT receiver initialization
	 */
	void rmt_rx_init()
	{
		rmt_config_t rmt_rx;
		rmt_rx.channel = MB_RMT_BOARD_CHANNEL;
		rmt_rx.gpio_num = MB_RMT_BOARD_GPIO ;
		rmt_rx.clk_div = RMT_CLK_DIV;
		rmt_rx.mem_block_num = 1;
		rmt_rx.rmt_mode = RMT_MODE_RX;
		rmt_rx.rx_config.filter_en = true;
		rmt_rx.rx_config.filter_ticks_thresh = 100;
		rmt_rx.rx_config.idle_threshold = rmt_item32_tIMEOUT_US / 10 * (RMT_TICK_10_US);
		rmt_config(&rmt_rx);
		rmt_driver_install(rmt_rx.channel, 1000, 0);
	}
	
	/**
	 * @brief RMT receiver demo, this task will print each received NEC data.
	 *
	 */
	void rmt_nec_rx_task(void* pvParameter)
	{
		int channel = MB_RMT_BOARD_CHANNEL;
		//rmt_rx_init();
		RingbufHandle_t rb = NULL;
		//get RMT RX ringbuffer
		
		rmt_get_ringbuf_handler(channel, &rb);
		rmt_rx_start(channel, 1);

		while(rb) {
			size_t rx_size = 0;
			//try to receive data from ringbuffer.
			//RMT driver will push all the data it receives to its ringbuffer.
			//We just need to parse the value and return the spaces of ringbuffer.
			rmt_item32_t* item = (rmt_item32_t*) xRingbufferReceive(rb, &rx_size, 5000);
            //vTaskDelay(30 / portTICK_RATE_MS);
			if(item) {
				uint16_t rmt_addr;
				uint16_t rmt_cmd;
				int offset = 0;
				while(1) {
					//parse data value from ringbuffer.
					int res = nec_parse_items(item + offset, rx_size / 4 - offset, &rmt_addr, &rmt_cmd);
					if(res > 0) {
						offset += res + 1;
						ESP_LOGI(NEC_TAG, "RMT RCV --- addr: 0x%04x cmd: 0x%04x", rmt_addr, rmt_cmd);
					} else {
					   // ESP_LOGI(NEC_TAG, "error");
						break;
					}
				   // vTaskDelay(30 / portTICK_RATE_MS);	
				}
				//after parsing the data, return spaces to ringbuffer.
				vRingbufferReturnItem(rb, (void*) item);
			} else {
				break;
			}
		}
		vTaskDelete(NULL);
	}
	


	/******************************************************************************
     drive functions end
	******************************************************************************/




	
	
    STATIC mp_obj_t mb_rmt_board_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
	{
	  // parse args
	  mp_map_t kw_args;
	  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
	
	  //mb_rmt_board_config();
      rmt_rx_init();
	  printf("MAKEBLOCK API:call function-- value()-- to get the receive data of rmt\n"); 
	  // setup the object
	  mb_rmt_board_obj_t *self = &mb_rmt_board_obj;
	  self->base.type = &mb_rmt_board_type;
	  return self;
	}
	
	STATIC void mb_rmt_board_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
	{
	
	}



    void mb_rmt_board_config()  
	{


	}

	STATIC mp_obj_t mb_rmt_board_value(mp_uint_t n_args, const mp_obj_t *args)
	{
      float value=0;
      mb_rmt_board_obj_t *self = args[0];
	  //self->butt = mp_obj_get_int(args[1]);
	  xTaskCreate(rmt_nec_rx_task, "rmt_nec_rx_task", 2048, NULL, 2, NULL);
      //xTaskCreatePinnedToCore(rmt_nec_rx_task, "rmt_nec_rx_task", 2048, NULL, 2, NULL,0);
      //rmt_nec_rx_task(NULL);
	  return mp_obj_new_float(value);
	}
	
    //MP_DEFINE_CONST_FUN_OBJ_2(mb_rmt_board_value_obj, mb_rmt_board_value);
	STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mb_rmt_board_value_obj,1, 1, mb_rmt_board_value);
    //MP_DEFINE_CONST_FUN_OBJ_3(mb_rmt_board_value_obj, mb_rmt_board_value);
	
	
	STATIC mp_obj_t mb_rmt_board_call(mp_obj_t self_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args)
	{
	  mp_arg_check_num(n_args, n_kw, 0, 0, false);
	  return mb_rmt_board_value(n_args,args);
	}
	
	STATIC const mp_map_elem_t mb_rmt_board_locals_dict_table[] =
	{
	  { MP_OBJ_NEW_QSTR(MP_QSTR_value), 			  (mp_obj_t)&mb_rmt_board_value_obj },
	};
	
	STATIC MP_DEFINE_CONST_DICT(mb_rmt_board_locals_dict, mb_rmt_board_locals_dict_table);
	
	const mp_obj_type_t mb_rmt_board_type =
	{
	  { &mp_type_type },
	  .name = MP_QSTR_rmt_board,
	  .print = mb_rmt_board_print,
	  .call = mb_rmt_board_call,
	  .make_new = mb_rmt_board_make_new,
	  .locals_dict = (mp_obj_t)&mb_rmt_board_locals_dict,
	};


