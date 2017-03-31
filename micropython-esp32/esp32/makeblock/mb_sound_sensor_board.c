/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for makeblock sound_sensor_board module
 * @file    mb_sound_sensor_board.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/03/028
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
 * This file is a drive sound_sensor_board module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 *  fftust            2017/03/28      1.0.0            build the new.
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
#include "driver/adc.h"
	
#include "mb_sound_sensor_board.h"
#include "mb_processcmd.h"
	
	/******************************************************************************
	 MACRO DEFINITION
	 ******************************************************************************/
	#define MB_POTENTIONMETER_BOARD_CHANNEL ADC2_CHANNEL_6 
	/******************************************************************************
	 DECLARE CONSTANTS
	 ******************************************************************************/
	
	/******************************************************************************
	 DEFINE TYPES
	 ******************************************************************************/
	typedef struct
	{
	  mp_obj_base_t base;
	} mb_sound_sensor_board_obj_t;
	
	/******************************************************************************
	 DECLARE PRIVATE DATA
	 ******************************************************************************/
	STATIC mb_sound_sensor_board_obj_t mb_sound_sensor_board_obj = {};
	
	/******************************************************************************
	 DECLARE PRIVATE FUNCTIONS
	 ******************************************************************************/
	
	/******************************************************************************
	 DEFINE PUBLIC FUNCTIONS
	 ******************************************************************************/
	
	
	/******************************************************************************/
	
    STATIC mp_obj_t mb_sound_sensor_board_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
	{
	  // parse args
	  mp_map_t kw_args;
	  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
	
	  mb_sound_sensor_board_config();
	  // setup the object
	  mb_sound_sensor_board_obj_t *self = &mb_sound_sensor_board_obj;
	  self->base.type = &mb_sound_sensor_board_type;
	  return self;
	}
	
	STATIC void mb_sound_sensor_board_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
	{
	
	}
  
    void mb_sound_sensor_board_config()  
	{  
    adc2_config_width(ADC_WIDTH_12Bit);
    adc2_config_channel_atten(MB_SOUND_SENSOR_BOARD_CHANNEL,ADC_ATTEN_11db);

	}

	STATIC mp_obj_t mb_sound_sensor_board_value(mp_uint_t n_args, const mp_obj_t *args)
	{
      float value=0;
	  value=(float)(adc2_get_voltage(MB_SOUND_SENSOR_BOARD_CHANNEL));

	  return mp_obj_new_float(value);
	}
	
	STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mb_sound_sensor_board_value_obj,1, 1, mb_sound_sensor_board_value);
    //MP_DEFINE_CONST_FUN_OBJ_3(mb_potention_meter_board_value_obj, mb_potention_meter_board_value);
	
	
	STATIC mp_obj_t mb_sound_sensor_board_call(mp_obj_t self_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args)
	{
	  mp_arg_check_num(n_args, n_kw, 0, 0, false);
	  return mb_sound_sensor_board_value(n_args,args);
	}
	
	STATIC const mp_map_elem_t mb_sound_sensor_board_locals_dict_table[] =
	{
	  { MP_OBJ_NEW_QSTR(MP_QSTR_value), 			  (mp_obj_t)&mb_sound_sensor_board_value_obj },
	};
	
	STATIC MP_DEFINE_CONST_DICT(mb_sound_sensor_board_locals_dict, mb_sound_sensor_board_locals_dict_table);
	
	const mp_obj_type_t mb_sound_sensor_board_type =
	{
	  { &mp_type_type },
	  .name = MP_QSTR_sound_sensor_board,
	  .print = mb_sound_sensor_board_print,
	  .call = mb_sound_sensor_board_call,
	  .make_new = mb_sound_sensor_board_make_new,
	  .locals_dict = (mp_obj_t)&mb_sound_sensor_board_locals_dict,
	};



