/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for makeblock buzzer module
 * @file    mb_buzzer.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/03/07
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
 * This file is a drive buzzer module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * fftust           2017/03/07          1.0.0            build the new.
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

#include "mb_buzzer.h"
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
  uint8_t buzzer_a;
  uint8_t buzzer_b;
  uint8_t buzzer_c;
  uint8_t buzzer_d;
} mb_buzzer_obj_t;

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/
STATIC mb_buzzer_obj_t mb_buzzer_obj = { .buzzer_a=196, .buzzer_b=0, .buzzer_c= 250, .buzzer_d=0};
/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/


/******************************************************************************/

STATIC mp_obj_t mb_buzzer_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
  // parse args
  mp_map_t kw_args;
  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
  communication_channel_init();

  // setup the object
  mb_buzzer_obj_t *self = &mb_buzzer_obj;
  self->base.type = &mb_buzzer_type;
  return self;
}

STATIC void mb_buzzer_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{

}

STATIC mp_obj_t mb_buzzer_play(mp_uint_t n_args, const mp_obj_t *args)
{
  uint8_t length = 1+1+1+4;   // index + action + device  + buzzer_a + _b + _c +_d
  uint8_t index = 0;  
  mb_buzzer_obj_t *self = args[0];
  self->buzzer_a        = mp_obj_get_int(args[1]);
  self->buzzer_b        = mp_obj_get_int(args[2]);
  self->buzzer_c        = mp_obj_get_int(args[3]);
  self->buzzer_d        = mp_obj_get_int(args[4]);
  
  write_head();
  write_serial(length);
  write_serial(index);
  write_serial(RUN_CMD);
  write_serial(TONE );
  write_serial(self->buzzer_a);
  write_serial(self->buzzer_b); 
  write_serial(self->buzzer_c);
  write_serial(self->buzzer_d);
  return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mb_buzzer_play_obj, 5, 5, mb_buzzer_play);

STATIC mp_obj_t mb_buzzer_off(mp_uint_t n_args, const mp_obj_t *args)
{
  uint8_t length = 1+1+1+4;   // index + action + device  + buzzer_a + _b + _c +_d
  uint8_t index = 0;  
  mb_buzzer_obj_t *self = args[0];
  self->buzzer_a   = 0;
  self->buzzer_b   = 0;
  self->buzzer_c   = 0;
  self->buzzer_d   = 0;
  
  write_head();
  write_serial(length);
  write_serial(index);
  write_serial(RUN_CMD);
  write_serial(TONE );
  write_serial(self->buzzer_a);
  write_serial(self->buzzer_b); 
  write_serial(self->buzzer_c);
  write_serial(self->buzzer_d);

  return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mb_buzzer_off_obj, 1, 1, mb_buzzer_off);



//STATIC MP_DEFINE_CONST_FUN_OBJ_1(mb_dcmotor_run_obj, mb_dcmotor_run);



void mb_buzzer_play_cmd(uint8_t index, int8_t buzzer_a ,uint8_t buzzer_b ,uint8_t buzzer_c ,uint8_t buzzer_d  ) 
{
	uint8_t length = 1+1+1+4;	// index + action + device	+ buzzer_a + _b + _c +_d
	
	write_head();
	write_serial(length);
	write_serial(index);
	write_serial(RUN_CMD);
	write_serial(TONE);
	write_serial(buzzer_a);
	write_serial(buzzer_b); 
	write_serial(buzzer_c);
	write_serial(buzzer_d);
}

STATIC const mp_map_elem_t mb_buzzer_locals_dict_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_play),               (mp_obj_t)&mb_buzzer_play_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_off),                (mp_obj_t)&mb_buzzer_off_obj },
  		
};

STATIC MP_DEFINE_CONST_DICT(mb_buzzer_locals_dict, mb_buzzer_locals_dict_table);

const mp_obj_type_t mb_buzzer_type =
{
  { &mp_type_type },
  .name = MP_QSTR_buzzer,
  .print = mb_buzzer_print,
  .make_new = mb_buzzer_make_new,
  .locals_dict = (mp_obj_t)&mb_buzzer_locals_dict,
};

