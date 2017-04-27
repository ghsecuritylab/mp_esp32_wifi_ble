/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for makeblock vibratingmotor_board module
 * @file    mb_vibratingmotor_board.c
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
 * This file is a drive vibratingmotor_board module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 *  fftust            2017/03/28     1.0.0            build the new.
 * </pre>
 *
 */
	
#include <stdint.h>
#include <string.h>
#include <stdio.h>
	
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_task.h"


	
#include "py/mpstate.h"
#include "py/runtime.h"
	
#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "uart.h"
#include "driver/gpio.h"
	
#include "mb_vibratingmotor_board.h"
#include "mb_processcmd.h"
	
/******************************************************************************
 MACRO DEFINITION
 ******************************************************************************/
 #define MB_VIBRATINGMOTOR_BOARD_IO    21
 
 #define MB_VIB_TASK_STACK_LEN       ((16*1024) / sizeof(StackType_t))  //fftust:set it smaller
/******************************************************************************
 DECLARE CONSTANTS
 ******************************************************************************/

/******************************************************************************
 DEFINE TYPES
 ******************************************************************************/
typedef struct
{
  mp_obj_base_t base;
  uint8_t frequence;  //0-100
  uint8_t runtime_s;  //0-255
} mb_vibratingmotor_board_obj_t;

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/
STATIC mb_vibratingmotor_board_obj_t mb_vibratingmotor_board_obj = {};

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/


/******************************************************************************/

STATIC mp_obj_t mb_vibratingmotor_board_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
  // parse args
  mp_map_t kw_args;
  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);

  mb_vibratingmotor_board_config();   //fftust:GPIO config
  // setup the object
  mb_vibratingmotor_board_obj_t *self = &mb_vibratingmotor_board_obj;
  self->base.type = &mb_vibratingmotor_board_type;
  return self;
}

STATIC void mb_vibratingmotor_board_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{

}

void mb_vibratingmotor_board_config()  
{  
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_PIN_INTR_DISABLE;  
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = (1<<MB_VIBRATINGMOTOR_BOARD_IO);
  io_conf.pull_down_en = 0;
  io_conf.pull_up_en = 1;
  gpio_config(&io_conf); 
}


STATIC void mb_vibratingmotor_TASK(void *pvParameters)
{
  uint16_t tim;
  if(mb_vibratingmotor_board_obj.frequence>=100)
  {
    mb_vibratingmotor_board_obj.frequence=100;
  }
  if(mb_vibratingmotor_board_obj.runtime_s<0)
  {
    mb_vibratingmotor_board_obj.runtime_s=0;
  }
  for(;;)
  {
   	for(tim=0;tim< mb_vibratingmotor_board_obj.runtime_s* mb_vibratingmotor_board_obj.frequence/2;tim++)
    {
	  gpio_set_level(MB_VIBRATINGMOTOR_BOARD_IO,MOTOR_RUN);
      vTaskDelay((1000/ mb_vibratingmotor_board_obj.frequence) / portTICK_RATE_MS);
      //vTaskDelay((100) / portTICK_RATE_MS);
      gpio_set_level(MB_VIBRATINGMOTOR_BOARD_IO,MOTOR_STOP);
	  vTaskDelay((1000/ mb_vibratingmotor_board_obj.frequence) / portTICK_RATE_MS);
	  //vTaskDelay((100) / portTICK_RATE_MS);
   	}
	break;
  }
 	

  vTaskDelete( NULL );


}
	
STATIC mp_obj_t mb_vibratingmotor_board_run(mp_uint_t n_args, const mp_obj_t *args)
{

  //mb_vibratingmotor_board_obj_t *self=args[0];
  mb_vibratingmotor_board_obj.frequence = mp_obj_get_int(args[1]);
  mb_vibratingmotor_board_obj.runtime_s = mp_obj_get_int(args[2]);
  
  xTaskCreatePinnedToCore(mb_vibratingmotor_TASK, "mb_vibratingmotor_TASK", MB_VIB_TASK_STACK_LEN , NULL, 1, NULL, 0);
  return mp_const_none;
  #if 0
  uint16_t tim;
  mb_vibratingmotor_board_obj_t *self=args[0];
  self->frequence = mp_obj_get_int(args[1]);
  self->runtime_s = mp_obj_get_int(args[2]);

  if(self->frequence<=0 || self->runtime_s<=0)
  	 return mp_const_none;
  else
  {
    if(self->frequence>=100)
	{
      self->frequence=100;
	}
  }
  			
  for(tim=0;tim<self->runtime_s*self->frequence/2;tim++)
  {
    gpio_set_level(MB_VIBRATINGMOTOR_BOARD_IO,MOTOR_RUN);
    vTaskDelay((1000/self->frequence) / portTICK_RATE_MS);
    gpio_set_level(MB_VIBRATINGMOTOR_BOARD_IO,MOTOR_STOP);
	vTaskDelay((1000/self->frequence) / portTICK_RATE_MS);
  }
  return mp_const_none;
  #endif
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mb_vibratingmotor_board_run_obj,3, 3, mb_vibratingmotor_board_run);
//MP_DEFINE_CONST_FUN_OBJ_3(mb_potention_meter_board_value_obj, mb_potention_meter_board_value);


STATIC mp_obj_t mb_vibratingmotor_board_call(mp_obj_t self_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args)
{
  mp_arg_check_num(n_args, n_kw, 0, 0, false);
  return mb_vibratingmotor_board_run(n_args,args);
}

STATIC const mp_map_elem_t mb_vibratingmotor_board_locals_dict_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_run), 			  (mp_obj_t)&mb_vibratingmotor_board_run_obj },
};

STATIC MP_DEFINE_CONST_DICT(mb_vibratingmotor_board_locals_dict, mb_vibratingmotor_board_locals_dict_table);

const mp_obj_type_t mb_vibratingmotor_board_type =
{
  { &mp_type_type },
  .name = MP_QSTR_vibratingmotor_board,
  .print = mb_vibratingmotor_board_print,
  .call = mb_vibratingmotor_board_call,
  .make_new = mb_vibratingmotor_board_make_new,
  .locals_dict = (mp_obj_t)&mb_vibratingmotor_board_locals_dict,
};



