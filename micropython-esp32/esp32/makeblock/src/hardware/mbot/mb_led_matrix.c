/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for makeblock ledmatrix module
 * @file    mb_led_matrix.c
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
 * This file is a drive ledmatrix module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * fftust           2017/03/08          1.0.0            build the new.
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

#include "mb_led_matrix.h"
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
  uint8_t ledm_mods;    
   union ledm_modset_t
  {
    struct chars_dis_t
   	{
      uint8_t pos_c_x;
	  uint8_t pos_c_y;
	  uint8_t charnums;
	  uint8_t chars_data[4];
    }chars_dis;
   
    struct paint_dis_t
   	{
      uint8_t pos_p_x;
	  uint8_t pos_p_y;
	  uint8_t paint_data[16]; 
    }paint_dis;

    struct time_dis_t
   	{
      uint8_t havepoint;
      uint8_t time_data[2];
    }time_dis;

    struct num_dis_t
   	{
      float num_data;
    }num_dis;	
  }ledm_modset;
} mb_led_matrix_obj_t;

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/
STATIC mb_led_matrix_obj_t mb_led_matrix_obj = {.port = 0, .ledm_mods=0};

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/


/******************************************************************************/

STATIC mp_obj_t mb_led_matrix_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
  // parse args
  mp_map_t kw_args;
  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
  communication_channel_init();

  // setup the object
  mb_led_matrix_obj_t *self = &mb_led_matrix_obj;
  self->base.type = &mb_led_matrix_type;
  return self;
}

STATIC void mb_led_matrix_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{

}

STATIC mp_obj_t mb_led_matrix_show(mp_uint_t n_args, const mp_obj_t *args)
{
  uint8_t length=0;
  uint8_t index = 0;  
  uint8_t n;
  mb_led_matrix_obj_t *self = args[0];
  self->port    = mp_obj_get_int(args[1]);
  self->ledm_mods=mp_obj_get_int(args[2]);
  
  switch(self->ledm_mods)
  {
    case 1:
      self->ledm_modset.chars_dis.pos_c_x=mp_obj_get_int(args[3]);
	  self->ledm_modset.chars_dis.pos_c_y=mp_obj_get_int(args[4]);
	  self->ledm_modset.chars_dis.charnums=mp_obj_get_int(args[5]);
	  for(n=0;n<self->ledm_modset.chars_dis.charnums;n++)
	  {
	    self->ledm_modset.chars_dis.chars_data[n]=mp_obj_get_int(args[6+n]);
	  }
	  length=1+1+1+1+1+2+1+self->ledm_modset.chars_dis.charnums;
	break;

	case 2:
      self->ledm_modset.paint_dis.pos_p_x=mp_obj_get_int(args[3]);
	  self->ledm_modset.paint_dis.pos_p_y=mp_obj_get_int(args[4]);

	  for(n=0;n<16;n++)
	  {
        self->ledm_modset.paint_dis.paint_data[n]=mp_obj_get_int(args[5+n]);

	  }
	  length=1+1+1+1+1+2+16;
	break;

	case 3:
      self->ledm_modset.time_dis.havepoint=mp_obj_get_int(args[3]);
	  self->ledm_modset.time_dis.time_data[0]=mp_obj_get_int(args[4]);
	  self->ledm_modset.time_dis.time_data[1]=mp_obj_get_int(args[5]);
	  
	  length=1+1+1+1+1+1+2;
	break;

	case 4:
      self->ledm_modset.num_dis.num_data=mp_obj_get_float(args[3]);
	  length=1+1+1+1+1+4;
    break;		



  }
  
  write_head();
  write_serial(length);
  write_serial(index);
  write_serial(RUN_CMD);
  write_serial(LEDMATRIX);
  write_serial(self->port);
  write_serial(self->ledm_mods); 
  if(self->ledm_mods==1)
  {
    write_serial(self->ledm_modset.chars_dis.pos_c_x); 
	write_serial(self->ledm_modset.chars_dis.pos_c_y);
	write_serial(self->ledm_modset.chars_dis.charnums);
	for(n=0;n<self->ledm_modset.chars_dis.charnums;n++)
	{
	  write_serial(self->ledm_modset.chars_dis.chars_data[n]);
	}  
  }
  else if(self->ledm_mods==2)
  {
    write_serial(self->ledm_modset.paint_dis.pos_p_x); 
	write_serial(self->ledm_modset.paint_dis.pos_p_y);
	for(n=0;n<16;n++)
	{
	  write_serial(self->ledm_modset.paint_dis.paint_data[n]);
	}  
  }
  else if(self->ledm_mods==3)
  {
    write_serial(self->ledm_modset.time_dis.havepoint);
    write_serial(self->ledm_modset.time_dis.time_data[0]);
	write_serial(self->ledm_modset.time_dis.time_data[1]);

  }
  else if(self->ledm_mods==4)
  {
    send_float(self->ledm_modset.num_dis.num_data);
  }
  
  return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mb_led_matrix_show_obj,4, 21, mb_led_matrix_show);

STATIC mp_obj_t mb_led_matrix_charsshow(mp_uint_t n_args, const mp_obj_t *args)
{
  uint8_t length=0;
  uint8_t index = 0;  
  uint8_t n;
  mb_led_matrix_obj_t *self = args[0];
  self->port    = mp_obj_get_int(args[1]);
  self->ledm_mods=mp_obj_get_int(args[2]);
  self->ledm_modset.chars_dis.pos_c_x=mp_obj_get_int(args[3]);
  self->ledm_modset.chars_dis.pos_c_y=mp_obj_get_int(args[4]);
  self->ledm_modset.chars_dis.charnums=mp_obj_get_int(args[5]);
  for(n=0;n<self->ledm_modset.chars_dis.charnums;n++)
  {
	self->ledm_modset.chars_dis.chars_data[n]=mp_obj_get_int(args[6+n]);
  }
  length=1+1+1+1+1+2+1+n;

  write_head();
  write_serial(length);
  write_serial(index);
  write_serial(RUN_CMD);
  write_serial(LEDMATRIX);
  write_serial(self->port);
  write_serial(self->ledm_mods); 
  write_serial(self->ledm_modset.chars_dis.pos_c_x); 
  write_serial(self->ledm_modset.chars_dis.pos_c_y);
  write_serial(self->ledm_modset.chars_dis.charnums);
  for(n=0;n<self->ledm_modset.chars_dis.charnums;n++)
  {
	write_serial(self->ledm_modset.chars_dis.chars_data[n]);
  }  

  return mp_const_none;

}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mb_led_matrix_charsshow_obj,7, 10, mb_led_matrix_charsshow);

STATIC mp_obj_t mb_led_matrix_paintshow(mp_uint_t n_args, const mp_obj_t *args)
{
  uint8_t length=0;
  uint8_t index = 0;  
  uint8_t n;
  mb_led_matrix_obj_t *self = args[0];
  self->port    = mp_obj_get_int(args[1]);
  self->ledm_mods=mp_obj_get_int(args[2]);
  self->ledm_modset.paint_dis.pos_p_x=mp_obj_get_int(args[3]);
  self->ledm_modset.paint_dis.pos_p_y=mp_obj_get_int(args[4]);
  
  for(n=0;n<16;n++)
  {
	self->ledm_modset.paint_dis.paint_data[n]=mp_obj_get_int(args[5+n]);
  
  }
  length=1+1+1+1+1+2+16;


  write_head();
  write_serial(length);
  write_serial(index);
  write_serial(RUN_CMD);
  write_serial(LEDMATRIX);
  write_serial(self->port);
  write_serial(self->ledm_mods); 
  write_serial(self->ledm_modset.paint_dis.pos_p_x); 
  write_serial(self->ledm_modset.paint_dis.pos_p_y);
  for(n=0;n<16;n++)
  {
    write_serial(self->ledm_modset.paint_dis.paint_data[n]);
  }  

  return mp_const_none;

}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mb_led_matrix_paintshow_obj,21, 21, mb_led_matrix_paintshow);



STATIC mp_obj_t mb_led_matrix_timeshow(mp_uint_t n_args, const mp_obj_t *args)
{
  uint8_t length=0;
  uint8_t index = 0;  
  mb_led_matrix_obj_t *self = args[0];
  self->port    = mp_obj_get_int(args[1]);
  self->ledm_mods=mp_obj_get_int(args[2]);
  self->ledm_modset.time_dis.havepoint=mp_obj_get_int(args[3]);
  self->ledm_modset.time_dis.time_data[0]=mp_obj_get_int(args[4]);
  self->ledm_modset.time_dis.time_data[1]=mp_obj_get_int(args[5]);
  
  length=1+1+1+1+1+1+2;



  write_head();
  write_serial(length);
  write_serial(index);
  write_serial(RUN_CMD);
  write_serial(LEDMATRIX);
  write_serial(self->port);
  write_serial(self->ledm_mods); 
  write_serial(self->ledm_modset.time_dis.havepoint);
  write_serial(self->ledm_modset.time_dis.time_data[0]);
  write_serial(self->ledm_modset.time_dis.time_data[1]);


  return mp_const_none;

}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mb_led_matrix_timeshow_obj,6, 6, mb_led_matrix_timeshow);

STATIC mp_obj_t mb_led_matrix_numshow(mp_uint_t n_args, const mp_obj_t *args)
{
  uint8_t length=0;
  uint8_t index = 0;  
  mb_led_matrix_obj_t *self = args[0];
  self->port    = mp_obj_get_int(args[1]);
  self->ledm_mods=mp_obj_get_int(args[2]);
  self->ledm_modset.num_dis.num_data=mp_obj_get_float(args[3]);

  length=1+1+1+1+1+4;

  write_head();
  write_serial(length);
  write_serial(index);
  write_serial(RUN_CMD);
  write_serial(LEDMATRIX);
  write_serial(self->port);
  write_serial(self->ledm_mods); 
  send_float(self->ledm_modset.num_dis.num_data);


  return mp_const_none;

}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mb_led_matrix_numshow_obj,4, 4, mb_led_matrix_numshow);



void mb_led_matrix_show_cmd(uint8_t index, uint8_t port,int8_t ledm_mods,uint8_t ledm_data_num,uint8_t*  ledm_data ) 
{
  uint8_t length_table[4]={10,23,8,9};
  uint8_t n=0;
  
  write_head();
  write_serial(length_table[ledm_mods-1]);
  write_serial(index);
  write_serial(RUN_CMD);
  write_serial(LEDMATRIX);
  write_serial(port);
  write_serial(ledm_mods); 
 
  for(n=0;n<ledm_data_num;n++)
  {
	write_serial(ledm_data[n]);
  }  

}

STATIC const mp_map_elem_t mb_led_matrix_locals_dict_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_show),               (mp_obj_t)&mb_led_matrix_show_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_charsshow),          (mp_obj_t)&mb_led_matrix_charsshow_obj },		
  { MP_OBJ_NEW_QSTR(MP_QSTR_paintshow),          (mp_obj_t)&mb_led_matrix_paintshow_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_timeshow),           (mp_obj_t)&mb_led_matrix_timeshow_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_numshow),            (mp_obj_t)&mb_led_matrix_numshow_obj },
};

STATIC MP_DEFINE_CONST_DICT(mb_led_matrix_locals_dict, mb_led_matrix_locals_dict_table);

const mp_obj_type_t mb_led_matrix_type =
{
  { &mp_type_type },
  .name = MP_QSTR_led_matrix,
  .print = mb_led_matrix_print,
  .make_new = mb_led_matrix_make_new,
  .locals_dict = (mp_obj_t)&mb_led_matrix_locals_dict,
};

