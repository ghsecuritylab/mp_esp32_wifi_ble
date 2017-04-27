/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for makeblock ledmatrix_board module
 * @file    mb_ledmatrix_board.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/03/024
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
 * This file is a drive ledmatrix_board module.
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
#include <math.h>
	
	
#include "py/mpstate.h"
#include "py/runtime.h"
	
#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "uart.h"
#include "driver/gpio.h"
#include "driver/timer.h"
	
#include "mb_ledmatrix_board.h"
#include "mb_processcmd.h"



/******************************************************************************
 MACRO DEFINITION
 ******************************************************************************/
#define TIMER_INTR_SEL TIMER_INTR_LEVEL  /*!< Timer level interrupt */
#define TIMER_GROUP    TIMER_GROUP_1     /*!< Test on timer group 0 */
#define TIMER_DIVIDER   16               /*!< Hardware timer clock divider */

#define LED_BUFFER_SIZE   16
#define STRING_DISPLAY_BUFFER_SIZE 20


//Define Data Command Parameters
#define Mode_Address_Auto_Add_1  0x40     //0100 0000 B
#define Mode_Permanent_Address   0x44     //0100 0100 B


//Define Address Command Parameters
#define ADDRESS(addr)  (0xC0 | addr)

#define LED_MATRIX_CHARFONT_ROW  8
#define LED_MATRIX_CHARFONT_COLUMN  6

#define LED_MATRIX_TIMEFONT_ROW  8
#define LED_MATRIX_TIMEFONT_COLUMN  3

/******************************************************************************
 DECLARE CONSTANTS
 ******************************************************************************/
const LED_Matrix_CharFont_TypeDef char_font[]=
{
  {{' '},  {0x00,0x00,0x00,0x00,0x00,0x00}},
  	
  {{'0'},  {0x00,0x7C,0x82,0x82,0x7C,0x00}}, 
  {{'1'},  {0x00,0x42,0xFE,0x02,0x00,0x00}},
  {{'2'},  {0x00,0x46,0x8A,0x92,0x62,0x00}},
  {{'3'},  {0x00,0x44,0x92,0x92,0x6C,0x00}},  
  {{'4'},  {0x00,0x1C,0x64,0xFE,0x04,0x00}},
  {{'5'},  {0x00,0xF2,0x92,0x92,0x8C,0x00}},
  {{'6'},  {0x00,0x7C,0x92,0x92,0x4C,0x00}},
  {{'7'},  {0x00,0xC0,0x8E,0x90,0xE0,0x00}},
  {{'8'},  {0x00,0x6C,0x92,0x92,0x6C,0x00}},
  {{'9'},  {0x00,0x64,0x92,0x92,0x7C,0x00}},

  {{'a'},  {0x00,0x04,0x2A,0x2A,0x1E,0x00}},
  {{'b'},  {0x00,0xFE,0x12,0x12,0x0C,0x00}},
  {{'c'},  {0x00,0x0C,0x12,0x12,0x12,0x00}},
  {{'d'},  {0x00,0x0C,0x12,0x12,0xFE,0x00}},
  {{'e'},  {0x00,0x1C,0x2A,0x2A,0x18,0x00}},
  {{'f'},  {0x00,0x10,0x3E,0x50,0x50,0x00}},
  {{'g'},  {0x00,0x08,0x15,0x15,0x1E,0x00}},
  {{'h'},  {0x00,0xFE,0x10,0x1E,0x00,0x00}},
  {{'i'},  {0x00,0x00,0x2E,0x00,0x00,0x00}},
  {{'j'},  {0x00,0x02,0x01,0x2E,0x00,0x00}},
  {{'k'},  {0x00,0xFE,0x08,0x14,0x12,0x00}},
  {{'l'},  {0x00,0x00,0xFE,0x02,0x00,0x00}},
  {{'m'},  {0x00,0x1E,0x10,0x0E,0x10,0x0E}},
  {{'n'},  {0x00,0x1E,0x10,0x10,0x0E,0x00}},
  {{'o'},  {0x00,0x0C,0x12,0x12,0x0C,0x00}},
  {{'p'},  {0x00,0x1F,0x12,0x12,0x0C,0x00}},
  {{'q'},  {0x00,0x0C,0x12,0x12,0x1F,0x00}},
  {{'r'},  {0x00,0x1E,0x08,0x10,0x10,0x00}},
  {{'s'},  {0x00,0x12,0x29,0x25,0x12,0x00}},
  {{'t'},  {0x00,0x10,0x3E,0x12,0x00,0x00}},
  {{'u'},  {0x00,0x1C,0x02,0x02,0x1E,0x00}},
  {{'v'},  {0x18,0x04,0x02,0x04,0x18,0x00}},
  {{'w'},  {0x18,0x06,0x1C,0x06,0x18,0x00}},
  {{'x'},  {0x00,0x12,0x0C,0x0C,0x12,0x00}},
  {{'y'},  {0x00,0x18,0x05,0x05,0x1E,0x00}},
  {{'z'},  {0x00,0x12,0x16,0x1A,0x12,0x00}},

  {{'A'},  {0x00,0x7E,0x88,0x88,0x7E,0x00}},
  {{'B'},  {0x00,0xFE,0x92,0x92,0x6C,0x00}},
  {{'C'},  {0x00,0x7C,0x82,0x82,0x44,0x00}},
  {{'D'},  {0x00,0xFE,0x82,0x82,0x7C,0x00}},
  {{'E'},  {0x00,0xFE,0x92,0x92,0x82,0x00}},
  {{'F'},  {0x00,0xFE,0x90,0x90,0x80,0x00}},
  {{'G'},  {0x00,0x7C,0x82,0x92,0x5C,0x00}},
  {{'H'},  {0x00,0xFE,0x10,0x10,0xFE,0x00}},
  {{'I'},  {0x00,0x82,0xFE,0x82,0x00,0x00}},
  {{'J'},  {0x00,0x0C,0x02,0x02,0xFC,0x00}},
  {{'K'},  {0x00,0xFE,0x10,0x28,0xC6,0x00}},
  {{'L'},  {0x00,0xFE,0x02,0x02,0x02,0x00}},
  {{'M'},  {0x00,0xFE,0x40,0x30,0x40,0xFE}},
  {{'N'},  {0x00,0xFE,0x40,0x30,0x08,0xFE}},
  {{'O'},  {0x00,0x7C,0x82,0x82,0x82,0x7C}},
  {{'P'},  {0x00,0xFE,0x90,0x90,0x60,0x00}},
  {{'Q'},  {0x00,0x7C,0x82,0x8A,0x84,0x7A}},
  {{'R'},  {0x00,0xFE,0x98,0x94,0x62,0x00}},
  {{'S'},  {0x00,0x64,0x92,0x92,0x4C,0x00}},
  {{'T'},  {0x00,0x80,0xFE,0x80,0x80,0x00}},
  {{'U'},  {0x00,0xFC,0x02,0x02,0xFC,0x00}},
  {{'V'},  {0x00,0xF0,0x0C,0x02,0x0C,0xF0}},
  {{'W'},  {0x00,0xFE,0x04,0x38,0x04,0xFE}},
  {{'X'},  {0x00,0xC6,0x38,0x38,0xC6,0x00}},
  {{'Y'},  {0xC0,0x20,0x1E,0x20,0xC0,0x00}},
  {{'Z'},  {0x00,0x86,0x9A,0xB2,0xC2,0x00}},
  {{','},  {0x00,0x01,0x0e,0x0c,0x00,0x00}},
  {{'.'},  {0x00,0x00,0x06,0x06,0x00,0x00}},
  {{'%'},  {0x72,0x54,0x78,0x1e,0x2a,0x4e}},
  {{'!'},  {0x00,0x00,0x7a,0x00,0x00,0x00}},
  {{'?'},  {0x00,0x20,0x4a,0x30,0x00,0x00}},
  {{'-'},  {0x00,0x10,0x10,0x10,0x10,0x00}},
  {{'+'},  {0x08,0x08,0x3e,0x08,0x08,0x00}},
  {{'/'},  {0x00,0x02,0x0c,0x30,0x40,0x00}},
  {{'*'},  {0x22,0x14,0x08,0x14,0x22,0x00}},
  {{':'},  {0x00,0x00,0x14,0x00,0x00,0x00}},
  {{'"'},  {0x00,0xC0,0x00,0xC0,0x00,0x00}},
  {{'#'},  {0x28,0xFE,0x28,0xFE,0x28,0x00}},
  {{'('},  {0x00,0x00,0x7C,0x82,0x00,0x00}},
  {{')'},  {0x00,0x00,0x82,0x7C,0x00,0x00}},
  {{';'},  {0x00,0x02,0x24,0x00,0x00,0x00}},
  {{'~'},  {0x00,0x40,0x80,0x40,0x80,0x00}},
  {{';'},  {0x00,0x02,0x24,0x00,0x00,0x00}},
  {{'='},  {0x00,0x28,0x28,0x28,0x28,0x00}},
  {{'|'},  {0x00,0x00,0xFE,0x00,0x00,0x00}},
  {{'>'},  {0x00,0x82,0x44,0x28,0x10,0x00}},
  {{'<'},  {0x00,0x10,0x28,0x44,0x82,0x00}},  
  {{'@'},  {0x00,0x00,0x00,0x00,0x00,0x00}},
};  //85

const LED_Matrix_TimeFont_TypeDef time_font[]=
{
  {{0x7C,0x44,0x7C}},      //0
  {{0x00,0x7C,0x00}},       
  {{0x5C,0x54,0x74}},
  {{0x54,0x54,0x7C}},
  {{0x70,0x10,0x7C}},
  {{0x74,0x54,0x5C}},
  {{0x7C,0x54,0x5C}},
  {{0x40,0x40,0x7C}},
  {{0x7C,0x54,0x7C}},
  {{0x74,0x54,0x7C}},      //9
  {{0x00,0x44,0x00}},      //:
  {{0x20,0x20,0x20}},      //-
  {{0x00,0x00,0x00}},      //
  };
/******************************************************************************
 DEFINE TYPES
 ******************************************************************************/
 bool b_Color_Index;
 bool b_Draw_Str_Flag;
 uint8_t u8_Display_Buffer[LED_BUFFER_SIZE];

 
 typedef struct
 {
   mp_obj_base_t base;
   //uint8_t ledm_mods;    
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
} mb_ledmatrix_board_obj_t;

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/
STATIC mb_ledmatrix_board_obj_t mb_ledmatrix_board_obj = {};

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/


/******************************************************************************/

STATIC mp_obj_t mb_ledmatrix_board_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
  // parse args
  mp_map_t kw_args;
  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);

  mb_ledmatrix_board_config(); //gpio config
  mb_ledmatrix_board_init();   //LED_matrix init
  
  // setup the object
  mb_ledmatrix_board_obj_t *self = &mb_ledmatrix_board_obj;
  self->base.type = &mb_ledmatrix_board_type;
  return self;
}

STATIC void mb_ledmatrix_board_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{

}



void mb_ledmatrix_board_config()  
{
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_PIN_INTR_DISABLE;      
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = (1<<DIGITAL_SCL_IO) ;//GPIO_OUTPUT_PIN_SEL;
  io_conf.pull_down_en = 0;
  io_conf.pull_up_en = 1;
  gpio_config(&io_conf);


  io_conf.intr_type = GPIO_PIN_INTR_DISABLE;       
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = (1<<DIGITAL_SDA_IO) ;//GPIO_OUTPUT_PIN_SEL;
  io_conf.pull_down_en = 0;
  io_conf.pull_up_en = 1;
  gpio_config(&io_conf); 
 
  int timer_group = TIMER_GROUP;
  int timer_idx = TIMER_1;
  timer_config_t config;
  config.alarm_en = TIMER_ALARM_DIS;
  config.auto_reload = TIMER_AUTORELOAD_DIS;
  config.counter_dir = TIMER_COUNT_UP;
  config.divider = TIMER_DIVIDER; 
  config.intr_type = TIMER_INTR_SEL;
  config.counter_en = TIMER_PAUSE;
  timer_init(timer_group, timer_idx, &config);
  timer_pause(timer_group, timer_idx);
  timer_set_counter_value(timer_group, timer_idx, 0x00000000ULL);

}


 
void mb_digital_write(gpio_num_t io_num , digital_status sta)
{
  gpio_set_level(io_num,sta);
  mb_digital_write_delay();
}

void mb_digital_write_byte(uint8_t data)
{
 //Start
 mb_digital_write(DIGITAL_SCL_IO, DIGITAL_HIGH);
 mb_digital_write(DIGITAL_SDA_IO, DIGITAL_HIGH);
 mb_digital_write(DIGITAL_SDA_IO, DIGITAL_LOW);


for(uint8_t i=0;i<8;i++)
{
  mb_digital_write(DIGITAL_SCL_IO, DIGITAL_LOW);
  mb_digital_write(DIGITAL_SDA_IO, (data & 0x01));
  mb_digital_write(DIGITAL_SCL_IO, DIGITAL_HIGH);
  data = data >> 1;
}

//End
mb_digital_write(DIGITAL_SCL_IO, DIGITAL_LOW);
mb_digital_write(DIGITAL_SDA_IO, DIGITAL_LOW);
mb_digital_write(DIGITAL_SCL_IO, DIGITAL_HIGH);
mb_digital_write(DIGITAL_SDA_IO, DIGITAL_HIGH);

}


void mb_digital_writebytestoaddress(uint8_t address, const uint8_t *P_data, uint8_t count_of_data)
{
  uint8_t T_data;

  if(address > 15 || count_of_data==0)
    return;

  address = ADDRESS(address);

  //Start
  mb_digital_write(DIGITAL_SCL_IO, DIGITAL_HIGH);
  mb_digital_write(DIGITAL_SDA_IO, DIGITAL_HIGH);
  mb_digital_write(DIGITAL_SDA_IO, DIGITAL_LOW);


  //write Address
  for(uint8_t i=0;i<8;i++)
  {
    mb_digital_write(DIGITAL_SCL_IO, DIGITAL_LOW);
    mb_digital_write(DIGITAL_SDA_IO, (address & 0x01));
    mb_digital_write(DIGITAL_SCL_IO, DIGITAL_HIGH);
    address = address >> 1;
  }

  //write data
  for(uint8_t k=0; k<count_of_data; k++)
  {
    T_data = *(P_data + k);

    for(char i=0;i<8;i++)
    {
	  mb_digital_write(DIGITAL_SCL_IO, DIGITAL_LOW);
	  mb_digital_write(DIGITAL_SDA_IO, (T_data & 0x01));
	  mb_digital_write(DIGITAL_SCL_IO, DIGITAL_HIGH);
	  T_data =T_data >> 1;
    }
  }

  //End
  mb_digital_write(DIGITAL_SCL_IO, DIGITAL_LOW);
  mb_digital_write(DIGITAL_SDA_IO, DIGITAL_LOW);
  mb_digital_write(DIGITAL_SCL_IO, DIGITAL_HIGH);
  mb_digital_write(DIGITAL_SDA_IO, DIGITAL_HIGH);

}


void mb_Ledmatrix_setbrightness(uint8_t bright)
{
  if((uint8_t)bright>8)
  {
    bright = Brightness_8;
  }

  if((uint8_t)bright != 0)
  {
    bright = (LED_Matrix_Brightness_TypeDef)((uint8_t)(bright-1)|0x08);  
  }
  mb_digital_write_byte(0x80 | (uint8_t)bright);

}

void mb_digital_write_delay()
{
  uint64_t  timer_value=0;
  timer_pause(TIMER_GROUP,  TIMER_1);
  timer_set_counter_value(TIMER_GROUP, TIMER_1, 0x00000000ULL);
  timer_start(TIMER_GROUP,  TIMER_1);
  while(timer_value<100)  //50us
  {
    timer_get_counter_value(TIMER_GROUP, TIMER_1,&timer_value); 
  }
  timer_pause(TIMER_GROUP,  TIMER_1);
  timer_set_counter_value(TIMER_GROUP, TIMER_1, 0x00000000ULL);
}

void mb_ledmatrix_board_init()
{
  mb_digital_write(DIGITAL_SCL_IO, DIGITAL_HIGH);
  mb_digital_write(DIGITAL_SDA_IO, DIGITAL_HIGH);
  mb_digital_write_byte(Mode_Address_Auto_Add_1);
  mb_Ledmatrix_setbrightness(Brightness_5);
  mb_ledmatrix_clean();

}

void mb_ledmatrix_clean()
{
  for(uint8_t i=0;i<LED_BUFFER_SIZE;i++)
  {
    u8_Display_Buffer[i] = 0x00;
  }

  b_Color_Index = 1;
  b_Draw_Str_Flag = 0;

  mb_digital_writebytestoaddress(0,u8_Display_Buffer,LED_BUFFER_SIZE);

}

STATIC uint8_t mb_char_font_inv(uint8_t data ) 
{
  uint8_t tempdata;
   
  tempdata=0;
  tempdata+=(data & 0x80)>>7;
  tempdata+=(data & 0x40)>>5;
  tempdata+=(data & 0x20)>>3; 
  tempdata+=(data & 0x10)>>1;
  tempdata+=(data & 0x08)<<1;
  tempdata+=(data & 0x04)<<3;
  tempdata+=(data & 0x02)<<5;
  tempdata+=(data & 0x01)<<7;
  return tempdata;
} 


void mb_drawBitmap(int8_t x, int8_t y, uint8_t Bitmap_Width, const uint8_t *Bitmap)
{

	if(x>15 || y>7 || Bitmap_Width==0)
		return;

	if(b_Color_Index == 1)
	{
		for(uint8_t k=0;k<Bitmap_Width;k++)
		{
		  if(x+k>=0){
			u8_Display_Buffer[x+k] = (u8_Display_Buffer[x+k] & (0xff << (8-y))) | (y>0?(Bitmap[k] >> y):(Bitmap[k] << (-y)));
			u8_Display_Buffer[x+k] =mb_char_font_inv(u8_Display_Buffer[x+k]);

		  }
		}
	}
	else if(b_Color_Index == 0)
	{
		for(uint8_t k=0;k<Bitmap_Width;k++)
		{
			if(x+k>=0){
			  u8_Display_Buffer[x+k] = (u8_Display_Buffer[x+k] & (0xff << (8-y))) | (y>0?(~Bitmap[k] >> y):(~Bitmap[k] << (-y)));
			}
		}
	}

	mb_digital_writebytestoaddress(0,u8_Display_Buffer,LED_BUFFER_SIZE);
}

mp_obj_t mb_ledmatrix_board_char_show(mp_uint_t n_args, const mp_obj_t *args)
{
  uint8_t n=0,m=0;
  mb_ledmatrix_clean();
  mb_ledmatrix_board_obj_t *self=args[0];
  self->ledm_modset.chars_dis.pos_c_x=mp_obj_get_int(args[1]);
  self->ledm_modset.chars_dis.pos_c_y=mp_obj_get_int(args[2]);
  self->ledm_modset.chars_dis.charnums=mp_obj_get_int(args[3]);

  for(n=0;n<self->ledm_modset.chars_dis.charnums;n++)
  {
	self->ledm_modset.chars_dis.chars_data[n]=mp_obj_get_int(args[4+n]);
	for(m=0;m<85;m++)
	{    
      if(self->ledm_modset.chars_dis.chars_data[n]==char_font[m].Character[0])
	    break;
	}
    const uint8_t* temp=&(char_font[m].data[0]);
	mb_drawBitmap( self->ledm_modset.chars_dis.pos_c_x+n*6, self->ledm_modset.chars_dis.pos_c_y,LED_MATRIX_CHARFONT_COLUMN,temp);
  }
  
  return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mb_ledmatrix_board_char_show_obj,3, 7, mb_ledmatrix_board_char_show);
//MP_DEFINE_CONST_FUN_OBJ_3(mb_ledmatrix_board_value_obj, mb_ledmatrix_board_value);

mp_obj_t mb_ledmatrix_board_time_show(mp_uint_t n_args, const mp_obj_t *args)
{
  mb_ledmatrix_clean();
  mb_ledmatrix_board_obj_t *self=args[0];
  self->ledm_modset.time_dis.time_data[0]=mp_obj_get_int(args[1]);
  self->ledm_modset.time_dis.time_data[1]=mp_obj_get_int(args[2]);
  //self->ledm_modset.time_dis.havepoint   =mp_obj_get_int(args[3]);
  
  const uint8_t* temp=time_font[self->ledm_modset.time_dis.time_data[0] / 10].data ;
  mb_drawBitmap(0, 0,LED_MATRIX_TIMEFONT_COLUMN,temp);
  temp = time_font[self->ledm_modset.time_dis.time_data[0] % 10].data ;
  mb_drawBitmap(3, 0,LED_MATRIX_TIMEFONT_COLUMN,temp);

  temp = time_font[10].data;
  mb_drawBitmap(6, 0,LED_MATRIX_TIMEFONT_COLUMN,temp);
  
  temp=time_font[self->ledm_modset.time_dis.time_data[1] / 10].data;
  mb_drawBitmap(9, 0,LED_MATRIX_TIMEFONT_COLUMN,temp);
  temp = time_font[self->ledm_modset.time_dis.time_data[1] % 10].data;
  mb_drawBitmap(12, 0,LED_MATRIX_TIMEFONT_COLUMN,temp);

  return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mb_ledmatrix_board_time_show_obj,3, 3, mb_ledmatrix_board_time_show);


//MP_DEFINE_CONST_FUN_OBJ_3(mb_ledmatrix_board_value_obj, mb_ledmatrix_board_value);

mp_obj_t mb_ledmatrix_board_draw(mp_uint_t n_args, const mp_obj_t *args)
{
  uint8_t n;
  uint8_t px,py;
  mb_ledmatrix_clean();
  mb_ledmatrix_board_obj_t *self=args[0];
  
  self->ledm_modset.paint_dis.pos_p_x=mp_obj_get_int(args[1]);
  self->ledm_modset.paint_dis.pos_p_y=mp_obj_get_int(args[2]);

  px=self->ledm_modset.paint_dis.pos_p_x;
  py=self->ledm_modset.paint_dis.pos_p_y;
  
  for(n=0;n<16;n++)
  {
   self->ledm_modset.paint_dis.paint_data[n]=mp_obj_get_int(args[n+3]);
  }
  

  if(px > 15 || py > 7 )
	return mp_const_none;


  for(uint8_t n=0;n<16;n++)
  {
    if(px+n>=0)
	{
	  u8_Display_Buffer[px+n] = self->ledm_modset.paint_dis.paint_data[n];
	  //u8_Display_Buffer[px+n] = char_font_inv(u8_Display_Buffer[px+n]);
	}
  }
  printf("paranum:%d",n_args);
  mb_digital_writebytestoaddress(0,u8_Display_Buffer,LED_BUFFER_SIZE);

  return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mb_ledmatrix_board_draw_obj,3, 20, mb_ledmatrix_board_draw);

/*STATIC mp_obj_t mb_ledmatrix_board_call(mp_obj_t self_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args)
{
  mp_arg_check_num(n_args, n_kw, 0, 0, false);
  return mb_ledmatrix_board_char_show(n_args,args);
}
*/
STATIC const mp_map_elem_t mb_ledmatrix_board_locals_dict_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_char_show), 			  (mp_obj_t)&mb_ledmatrix_board_char_show_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_time_show), 			  (mp_obj_t)&mb_ledmatrix_board_time_show_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_draw), 			          (mp_obj_t)&mb_ledmatrix_board_draw_obj },
};

STATIC MP_DEFINE_CONST_DICT(mb_ledmatrix_board_locals_dict, mb_ledmatrix_board_locals_dict_table);

const mp_obj_type_t mb_ledmatrix_board_type =
{
  { &mp_type_type },
  .name = MP_QSTR_ledmatrix_board,
  .print = mb_ledmatrix_board_print,
  //.call = mb_ledmatrix_board_call,
  .make_new = mb_ledmatrix_board_make_new,
  .locals_dict = (mp_obj_t)&mb_ledmatrix_board_locals_dict,
};


