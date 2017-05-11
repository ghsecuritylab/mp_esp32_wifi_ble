/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for makeblock version_check module
 * @file    mb_version_check.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/03/27
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
 * This file is a drive version_check module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 *  fftust            2017/03/27    1.0.0            build the new.
 * </pre>
 *
 */
	
#include <stdint.h>
#include <string.h>
#include <stdio.h>
	
	
#include "py/mpstate.h"
#include "py/runtime.h"
	
#include "driver/uart.h"
#include "driver/adc.h"
#include "soc/uart_struct.h"
#include "uart.h"
	
#include "mb_version_check.h"
#include "mb_processcmd.h"

	

	
/******************************************************************************
 MACRO DEFINITION
 ******************************************************************************/
#define MB_VERSION_CHECK_CHANNEL ADC1_CHANNEL_0 //GPIO 36
/******************************************************************************
 DECLARE CONSTANTS
 ******************************************************************************/

/******************************************************************************
 DEFINE TYPES
 ******************************************************************************/
typedef struct
{
  mp_obj_base_t base;
} mb_version_check_obj_t;

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/
STATIC mb_version_check_obj_t mb_version_check_obj = {};

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/


/******************************************************************************/



STATIC mp_obj_t mb_version_check_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
  // parse args
  mp_map_t kw_args;
  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);

  void mb_version_check_config();
  
  // setup the object
  mb_version_check_obj_t *self = &mb_version_check_obj;
  self->base.type = &mb_version_check_type;
  return self;
}

STATIC void mb_version_check_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{

}


void mb_version_check_config()
{
  adc1_config_width(ADC_WIDTH_12Bit);
  adc1_config_channel_atten(MB_VERSION_CHECK_CHANNEL,ADC_ATTEN_11db);

}

STATIC mp_obj_t mb_version_check_value(mp_uint_t n_args, const mp_obj_t *args)
{
  float value;
  value=(float)(adc1_get_voltage(MB_VERSION_CHECK_CHANNEL));
 
  return mp_obj_new_float(value);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mb_version_check_value_obj, 1, 1, mb_version_check_value);



STATIC mp_obj_t mb_version_check_call(mp_obj_t self_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args)
{
  mp_arg_check_num(n_args, n_kw, 0, 0, false);
  return mb_version_check_value(n_args,args);
}

STATIC const mp_map_elem_t mb_version_check_locals_dict_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_value), 			  (mp_obj_t)&mb_version_check_value_obj },
};

STATIC MP_DEFINE_CONST_DICT(mb_version_check_locals_dict, mb_version_check_locals_dict_table);

const mp_obj_type_t mb_version_check_type =
{
  { &mp_type_type },
  .name = MP_QSTR_version_check,
  .print = mb_version_check_print,
  .call = mb_version_check_call,
  .make_new = mb_version_check_make_new,
  .locals_dict = (mp_obj_t)&mb_version_check_locals_dict,
};



