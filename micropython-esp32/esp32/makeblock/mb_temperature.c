/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for makeblock temperature_sensor module
 * @file    mb_temperature.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/03/06
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
 * This file is a drive mb_temperature module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 *  fftust            2017/03/06     1.0.0            build the new.
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
	
#include "mb_temperature.h"
#include "mb_processcmd.h"
	
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
	  mp_obj_base_t base;
	  uint8_t port;
	  uint8_t slot;
	} mb_temperature_obj_t;
	
	/******************************************************************************
	 DECLARE PRIVATE DATA
	 ******************************************************************************/
	STATIC mb_temperature_obj_t mb_temperature_obj = {.port = 0, .slot=1};
	
	/******************************************************************************
	 DECLARE PRIVATE FUNCTIONS
	 ******************************************************************************/
	
	/******************************************************************************
	 DEFINE PUBLIC FUNCTIONS
	 ******************************************************************************/
	
	
	/******************************************************************************/
	
	STATIC mp_obj_t mb_temperature_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
	{
	  // parse args
	  mp_map_t kw_args;
	  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
	
	  communication_channel_init();        //fftust:???????
	
	  // setup the object
	  mb_temperature_obj_t *self = &mb_temperature_obj;
	  self->base.type = &mb_temperature_type;
	  return self;
	}
	
	STATIC void mb_temperature_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
	{
	
	}
	
	STATIC mp_obj_t mb_temperature_value(mp_uint_t n_args, const mp_obj_t *args)
	{
	  uint8_t length = 1+1+1+1+1;   // index + action + device + port+slot
	  uint8_t index = 0;
	  float value = 0;
	  mb_temperature_obj_t *self = args[0];
	  self->port = mp_obj_get_int(args[1]);
	  self->slot = mp_obj_get_int(args[2]);
	  rsp_be_received = false;
	  pure_command_mode = false;
	  free_the_read_buffer();
	
	  write_head();
	  write_serial(length);
	  write_serial(index);
	  write_serial(GET_CMD);
	  write_serial(TEMPERATURE_SENSOR);
	  write_serial(self->port);
	  write_serial(self->slot);
	
	  while(rsp_be_received == false)
	  {
		//需要加一个超时判断
	  }
	  if((check_start_frame() == true) && (is_data_float() == true))
	  {
		value = read_float(4);
	//	printf("value:%f\r\n",value);
	  }
	  else
	  {
	//	  printf("error frame!\r\n");
	  }
	  pure_command_mode = true;
	  return mp_obj_new_float(value);
	}
	STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mb_temperature_value_obj, 3, 3, mb_temperature_value);
	
	void mb_temperature_value_cmd(uint8_t index, uint8_t port,uint8_t slot)
	{
	  uint8_t length = 1+1+1+1+1;   // index + action + device + port + slot
	  rsp_be_received = false;
	  write_head();
	  write_serial(length);
	  write_serial(index);
	  write_serial(GET_CMD);
	  write_serial(TEMPERATURE_SENSOR);
	  write_serial(port);
	  write_serial(slot);

	}
	
	STATIC mp_obj_t mb_temperature_call(mp_obj_t self_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args)
	{
	  mp_arg_check_num(n_args, n_kw, 0, 0, false);
	  return mb_temperature_value(n_args,args);
	}
	
	STATIC const mp_map_elem_t mb_temperature_locals_dict_table[] =
	{
	  { MP_OBJ_NEW_QSTR(MP_QSTR_value), 			  (mp_obj_t)&mb_temperature_value_obj },
	};
	
	STATIC MP_DEFINE_CONST_DICT(mb_temperature_locals_dict, mb_temperature_locals_dict_table);
	
	const mp_obj_type_t mb_temperature_type =
	{
	  { &mp_type_type },
	  .name = MP_QSTR_mb_temperature,
	  .print = mb_temperature_print,
	  .call = mb_temperature_call,
	  .make_new = mb_temperature_make_new,
	  .locals_dict = (mp_obj_t)&mb_temperature_locals_dict,
	};

