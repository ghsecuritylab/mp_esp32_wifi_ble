/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for makeblock shutter module
 * @file    mb_shutter.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/03/03
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
 * This file is a drive shutter module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * fftust          2017/03/02     1.0.0            build the new.
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

#include "mb_shutter.h"
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
  int8_t  sta;
} mb_shutter_obj_t;

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/
STATIC mb_shutter_obj_t mb_shutter_obj = {.port = 0, .sta = 1};

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/


/******************************************************************************/

STATIC mp_obj_t mb_shutter_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
  // parse args
  mp_map_t kw_args;
  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
  communication_channel_init();

  // setup the object
  mb_shutter_obj_t *self = &mb_shutter_obj;
  self->base.type = &mb_shutter_type;
  return self;
}

STATIC void mb_shutter_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{

}

STATIC mp_obj_t mb_shutter_run(mp_uint_t n_args, const mp_obj_t *args)
{
  uint8_t length = 1+1+1+1+1;   // index + action + device + port + sta
  uint8_t index = 0;
  mb_shutter_obj_t *self = args[0];
  self->port = mp_obj_get_int(args[1]);
  self->sta  = mp_obj_get_int(args[2]);
  write_head();
  write_serial(length);
  write_serial(index);
  write_serial(RUN_CMD);
  write_serial(SHUTTER);
  write_serial(self->port);
  send_short(self->sta);
  return mp_const_none;
}





//STATIC MP_DEFINE_CONST_FUN_OBJ_1(mb_dcmotor_run_obj, mb_dcmotor_run);
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mb_shutter_run_obj, 3, 3, mb_shutter_run);


void mb_shutter_run_cmd(uint8_t index, uint8_t port,int16_t sta)
{
  uint8_t length = 1+1+1+1+2;   // index + action + device + port + speed
  write_head();
  write_serial(length);
  write_serial(index);
  write_serial(RUN_CMD);
  write_serial(SHUTTER);
  write_serial(port);
  send_short(sta);
}

STATIC const mp_map_elem_t mb_shutter_locals_dict_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_run),               (mp_obj_t)&mb_shutter_run_obj },
};

STATIC MP_DEFINE_CONST_DICT(mb_shutter_locals_dict, mb_shutter_locals_dict_table);

const mp_obj_type_t mb_shutter_type =
{
  { &mp_type_type },
  .name = MP_QSTR_shutter,
  .print = mb_shutter_print,
  .make_new = mb_shutter_make_new,
  .locals_dict = (mp_obj_t)&mb_shutter_locals_dict,
};

