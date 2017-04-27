/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for makeblock button_board module
 * @file    mb_button_board.c
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
 * This file is a drive button_board module.
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

#include "mb_button_board.h"
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
  uint8_t butt;
} mb_button_board_obj_t;

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/
STATIC mb_button_board_obj_t mb_button_board_obj = {.butt=0};

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/


/******************************************************************************/

STATIC mp_obj_t mb_button_board_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
  // parse args
  mp_map_t kw_args;
  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);

  mb_button_board_config();

  //printf("MAKEBLOCK API:call function-- value(1 or 2)-- to get the status of button1 or 2\n"); 
  // setup the object
  mb_button_board_obj_t *self = &mb_button_board_obj;
  self->base.type = &mb_button_board_type;
  return self;
}

STATIC void mb_button_board_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{

}



void mb_button_board_config()  
{
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_PIN_INTR_DISABLE;   
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pin_bit_mask = (1<<BUTTON1_IO) ;//GPIO_OUTPUT_PIN_SEL;
  io_conf.pull_down_en = 0;
  io_conf.pull_up_en = 1;
  gpio_config(&io_conf);

  io_conf.intr_type = GPIO_PIN_INTR_DISABLE;     
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pin_bit_mask = (1<<BUTTON1_IO) ;//GPIO_OUTPUT_PIN_SEL;
  io_conf.pull_down_en = 0;
  io_conf.pull_up_en = 1;
  gpio_config(&io_conf); 

}

STATIC mp_obj_t mb_button_board_value(mp_uint_t n_args, const mp_obj_t *args)
{
  float value=0;
  mb_button_board_obj_t *self = args[0];
  self->butt = mp_obj_get_int(args[1]);
  if(self->butt==1)
  {
    value= (float)(gpio_get_level(BUTTON1_IO));
  }
  else if(self->butt==2)
  {
    value= (float)(gpio_get_level(BUTTON2_IO));
  }	  
  else
  { 
    //printf("PARA ERROR: please input the para between 1 to 2\n");
    
  }
  
  return mp_obj_new_float(value);
}

//MP_DEFINE_CONST_FUN_OBJ_2(mb_button_board_value_obj, mb_button_board_value);
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mb_button_board_value_obj,2, 4, mb_button_board_value);
//MP_DEFINE_CONST_FUN_OBJ_3(mb_button_board_value_obj, mb_button_board_value);


STATIC mp_obj_t mb_button_board_call(mp_obj_t self_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args)
{
  mp_arg_check_num(n_args, n_kw, 0, 0, false);
  return mb_button_board_value(n_args,args);
}

STATIC const mp_map_elem_t mb_button_board_locals_dict_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_value), 			  (mp_obj_t)&mb_button_board_value_obj },
};

STATIC MP_DEFINE_CONST_DICT(mb_button_board_locals_dict, mb_button_board_locals_dict_table);

const mp_obj_type_t mb_button_board_type =
{
  { &mp_type_type },
  .name = MP_QSTR_button_board,
  .print = mb_button_board_print,
  .call = mb_button_board_call,
  .make_new = mb_button_board_make_new,
  .locals_dict = (mp_obj_t)&mb_button_board_locals_dict,
};

