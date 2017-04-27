/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for makeblock rgbled module
 * @file    mb_rgbled.c
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
 * This file is a drive rgbled module.
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

#include "mb_RGBLed.h"
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
  uint8_t rgb_slot;   //fftust:it's means is not clear 
  uint8_t rgb_leds;      //fftust:choose one led of the two leds on the mbot board 0: all 1:led1 2:led2 
  uint8_t rgb_r;
  uint8_t rgb_g;
  uint8_t rgb_b;
} mb_rgbled_obj_t;

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/
STATIC mb_rgbled_obj_t mb_rgbled_obj = {.port = 0, .rgb_slot=0, .rgb_leds=0, .rgb_r = 0, .rgb_g=0, .rgb_b=0};

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/


/******************************************************************************/

STATIC mp_obj_t mb_rgbled_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
  // parse args
  mp_map_t kw_args;
  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
  communication_channel_init();

  // setup the object
  mb_rgbled_obj_t *self = &mb_rgbled_obj;
  self->base.type = &mb_rgbled_type;
  return self;
}

STATIC void mb_rgbled_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{

}

STATIC mp_obj_t mb_rgbled_show(mp_uint_t n_args, const mp_obj_t *args)
{
  uint8_t length = 1+1+1+1+1+1+3;   // index + action + device + port/board +??+LEDs+rgb_r+rgb_g+rgb_b 
  uint8_t index = 0;  
  mb_rgbled_obj_t *self = args[0];
  self->port       = mp_obj_get_int(args[1]);
  self->rgb_slot= mp_obj_get_int(args[2]);
  self->rgb_leds   = mp_obj_get_int(args[3]);
  self->rgb_r      = mp_obj_get_int(args[4]);
  self->rgb_g      = mp_obj_get_int(args[5]);
  self->rgb_b      = mp_obj_get_int(args[6]);
  
  
  write_head();
  write_serial(length);
  write_serial(index);
  write_serial(RUN_CMD);
  write_serial(RGBLED );
  write_serial(self->port);
  write_serial(self->rgb_slot); 
  write_serial(self->rgb_leds);
  write_serial(self->rgb_r);
  write_serial(self->rgb_g);
  write_serial(self->rgb_b);
  return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mb_rgbled_show_obj, 7, 7, mb_rgbled_show);

STATIC mp_obj_t mb_rgbled_off(mp_uint_t n_args, const mp_obj_t *args)
{
  uint8_t length = 1+1+1+1+1+1+3;   // index + action + device + port/board +??+LEDs+rgb_r+rgb_g+rgb_b 
  uint8_t index = 0;  
  mb_rgbled_obj_t *self = args[0];
  self->port    = mp_obj_get_int(args[1]);
  self->rgb_leds= mp_obj_get_int(args[2]);
  self->rgb_slot= mp_obj_get_int(args[3]);
  self->rgb_r   = 0;
  self->rgb_g   = 0;
  self->rgb_b   = 0;
  
  
  write_head();
  write_serial(length);
  write_serial(index);
  write_serial(RUN_CMD);
  write_serial(RGBLED );
  write_serial(self->port);
  write_serial(self->rgb_slot); 
  write_serial(self->rgb_leds);
  write_serial(self->rgb_r);
  write_serial(self->rgb_g);
  write_serial(self->rgb_b);
  return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mb_rgbled_off_obj, 4, 4, mb_rgbled_off);



//STATIC MP_DEFINE_CONST_FUN_OBJ_1(mb_dcmotor_run_obj, mb_dcmotor_run);



void mb_rgbled_show_cmd(uint8_t index, uint8_t port,uint8_t rgb_slot,uint8_t rgb_leds,uint8_t rgb_r,uint8_t rgb_g,uint8_t rgb_b ) 
{
  uint8_t length = 1+1+1+1+1+1+3;   // index + action + device + port/board +slot+LEDs+rgb_r+rgb_g+rgb_b 
  
  write_head();
  write_serial(length);
  write_serial(index);
  write_serial(RUN_CMD);
  write_serial(RGBLED );
  write_serial(port);
  write_serial(rgb_slot); 
  write_serial(rgb_leds);
  write_serial(rgb_r);
  write_serial(rgb_g);
  write_serial(rgb_b);

}

STATIC const mp_map_elem_t mb_rgbled_locals_dict_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_show),               (mp_obj_t)&mb_rgbled_show_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_off),                (mp_obj_t)&mb_rgbled_off_obj },
  		
};

STATIC MP_DEFINE_CONST_DICT(mb_rgbled_locals_dict, mb_rgbled_locals_dict_table);

const mp_obj_type_t mb_rgbled_type =
{
  { &mp_type_type },
  .name = MP_QSTR_rgbled,
  .print = mb_rgbled_print,
  .make_new = mb_rgbled_make_new,
  .locals_dict = (mp_obj_t)&mb_rgbled_locals_dict,
};

