/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for makeblock sevseg module
 * @file    mb_sevseg.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/03/08
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
 * This file is a drive sevseg module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 *  fftust           2017/03/08        1.0.0              build the new.
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

#include "mb_sevseg.h"
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
  float   val;
} mb_sevseg_obj_t;

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/
STATIC mb_sevseg_obj_t mb_sevseg_obj = {.port = 0, .val = 0.0};

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/


/******************************************************************************/

STATIC mp_obj_t mb_sevseg_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
  // parse args
  mp_map_t kw_args;
  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
  communication_channel_init();

  // setup the object
  mb_sevseg_obj_t *self = &mb_sevseg_obj;
  self->base.type = &mb_sevseg_type;
  return self;
}

STATIC void mb_sevseg_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{

}

STATIC mp_obj_t mb_sevseg_show(mp_uint_t n_args, const mp_obj_t *args)
{
  uint8_t length = 1+1+1+1+4;   // index + action + device + port + val(float)
  uint8_t index = 0;
  mb_sevseg_obj_t *self = args[0];
  self->port = mp_obj_get_int(args[1]);
  self->val = mp_obj_get_float(args[2]);
  write_head();
  write_serial(length);
  write_serial(index);
  write_serial(RUN_CMD);
  write_serial(SEVSEG);
  write_serial(self->port);
  send_float(self->val);
  return mp_const_none;
}





//STATIC MP_DEFINE_CONST_FUN_OBJ_1(mb_sevseg_run_obj, mb_sevseg_run);
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mb_sevseg_show_obj, 3, 3, mb_sevseg_show);


void mb_sevseg_show_cmd(uint8_t index, uint8_t port,float val)
{
  uint8_t length = 1+1+1+1+4;   // index + action + device + port + val(float)
  write_head();
  write_serial(length);
  write_serial(index);
  write_serial(RUN_CMD);
  write_serial(SEVSEG);
  write_serial(port);
  send_float(val);
}

STATIC const mp_map_elem_t mb_sevseg_locals_dict_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_show),               (mp_obj_t)&mb_sevseg_show_obj },
};

STATIC MP_DEFINE_CONST_DICT(mb_sevseg_locals_dict, mb_sevseg_locals_dict_table);

const mp_obj_type_t mb_sevseg_type =
{
  { &mp_type_type },
  .name = MP_QSTR_sevseg,
  .print = mb_sevseg_print,
  .make_new = mb_sevseg_make_new,
  .locals_dict = (mp_obj_t)&mb_sevseg_locals_dict,
};

