/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for makeblock gyro_sensor module
 * @file    mb_gyro_sensor.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/03/20
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
 * This file is a drive gyro_sensor module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 *  fftust            2017/03/20        1.0.0            build the new.
 *  Mark Yan          2017/03/31        1.0.0            update for available version.
 * </pre>
 *
 */


/* Includes ------------------------------------------------------------------*/


#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>	

#include "driver/uart.h"
#include "driver/i2c.h"	
#include "driver/gpio.h"

#include "py/mpstate.h"
#include "py/runtime.h"
//#include "teensy/core/core_pins.h"	


#include "soc/uart_struct.h"
#include "uart.h"

#include "mb_gyro_board.h"
#include "mb_processcmd.h"
#include "modmachine.h"
#include "esp32_mphal.h"
#include "mb_sys.h"

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
  uint8_t axis;
} mb_gyro_board_obj_t;

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/
STATIC mb_gyro_board_obj_t mb_gyro_board_obj = {.axis = 0};
STATIC bool gyro_enabled = false;
/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/


/******************************************************************************/
STATIC mp_obj_t mb_gyro_board_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
  // parse args
  mp_map_t kw_args;
  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);

  // setup the object
  mb_gyro_board_obj_t *self = &mb_gyro_board_obj;
  self->base.type = &mb_gyro_board_type;

  i2c_master_init();  //fftust add
  gyro_board_init();
  return self;
}

STATIC void mb_gyro_board_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{

}

/**
 * @brief i2c master initialization
 */
void i2c_master_init(void)
{ 
  i2c_port_t i2c_master_port = I2C_NUM;
  i2c_config_t conf; 
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = I2C_SDA_IO;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_io_num = I2C_SCL_IO;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = I2C_FREQ_HZ;
  i2c_param_config(i2c_master_port, &conf);
  i2c_driver_install(i2c_master_port, conf.mode, I2C_RX_BUF_DISABLE, I2C_TX_BUF_DISABLE, 0); 
 
}

uint8_t i2c_write_gyro_reg(uint8_t reg, uint8_t data)
{
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (GYRO_DEFAULT_ADDRESS << 1) | WRITE_B, ACK_CHECK_E);

  i2c_master_write(cmd, &reg, 1,ACK_CHECK_E);
  i2c_master_write(cmd, &data, 1,ACK_CHECK_E);
  i2c_master_stop(cmd);
  esp_err_t ret = i2c_master_cmd_begin(I2C_NUM, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  return ret;
 
}

uint8_t i2c_read_gyro_data(i2c_port_t i2c_num ,uint8_t start, uint8_t *buffer, uint8_t size)
{
  if(size == 0)
  {
    return ESP_OK;
  }
		
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (GYRO_DEFAULT_ADDRESS << 1 ) | WRITE_B, ACK_CHECK_E);
  i2c_master_write(cmd, &start,1, ACK_CHECK_E);

  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (GYRO_DEFAULT_ADDRESS << 1 ) | READ_B, ACK_CHECK_E);
  if(size > 1) 
  {
    i2c_master_read(cmd, buffer, size - 1, ACK_V);
  }
  i2c_master_read_byte(cmd, buffer + size - 1, NACK_V);
  i2c_master_stop(cmd);
  esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  return ret;



}
void gyro_board_deviceCalibration(void)
{
  int8_t return_value;
  uint16_t x = 0;
  uint16_t num = 50;
  long xSum	= 0, ySum = 0, zSum = 0;
  for(x = 0; x < num; x++)
  {
    return_value =i2c_read_gyro_data(I2C_NUM,0x43, i2cData, 6);
    if(return_value != ESP_OK)
    {
  	  return;
    }
	xSum += ( (i2cData[0] << 8) | i2cData[1] );
    ySum += ( (i2cData[2] << 8) | i2cData[3] );
    zSum += ( (i2cData[4] << 8) | i2cData[5] );
  }
  gyrXoffs = xSum / num;
  gyrYoffs = ySum / num;
  gyrZoffs = zSum / num;
}

void gyro_board_init(void)
{
  gSensitivity = 65.5; //for 500 deg/s, check data sheet
  gx = 0;
  gy = 0;
  gz = 0;
  gyrX = 0;
  gyrY = 0;
  gyrZ = 0;
  accX = 0;
  accY = 0;
  accZ = 0;
  gyrXoffs = 0;
  gyrYoffs = 0;
  gyrZoffs = 0;
  i2c_write_gyro_reg(0x6b, 0x00);//close the sleep mode
  i2c_write_gyro_reg(0x1a, 0x01);//configurate the digital low pass filter
  i2c_write_gyro_reg(0x1b, 0x08);//set the gyro scale to 500 deg/s
  i2c_write_gyro_reg(0x19, 49);  //set the Sampling Rate   50Hz
  gyro_board_deviceCalibration();
  gyro_enabled = true;
}

bool gyro_board_enabled(void)
{
  return gyro_enabled;
}

void gyro_board_update(void)
{
  static unsigned long	last_time = 0;
  int8_t return_value;
  double dt, filter_coefficient;

  dt = (double)(millis() - last_time) / 1000;
  last_time = millis();

  /* read imu data */
  return_value =i2c_read_gyro_data(I2C_NUM,0x3b,i2cData, 14);
  if(return_value != ESP_OK)
  {
  	return;
  }
  double ax, ay;
  /* assemble 16 bit sensor data */
  accX = ( (i2cData[0] << 8) | i2cData[1] );
  accY = ( (i2cData[2] << 8) | i2cData[3] );
  accZ = ( (i2cData[4] << 8) | i2cData[5] );  
  int temp = ( (i2cData[6] << 8) | i2cData[7] ); 
  temperature = 36.53 + temp/340.0; 
  gyrX = ( ( (i2cData[8] << 8) | i2cData[9] )) / gSensitivity;
  gyrY = ( ( (i2cData[10] << 8) | i2cData[11] )) / gSensitivity;
  gyrZ = ( ( (i2cData[12] << 8) | i2cData[13] )) / gSensitivity;  
  ax = atan2(accX, sqrt( pow(accY, 2) + pow(accZ, 2) ) ) * 180 / 3.1415926;
  ay = atan2(accY, sqrt( pow(accX, 2) + pow(accZ, 2) ) ) * 180 / 3.1415926;  
  //printf("Mark ax: %f,ay: %f,accX:%d\n",ax,ay,accX);
  printf("accX:%d, accY:%d, accZ:%d, accX: %f, gyrY: %f,gyrZ:%f,temp:%f\n",accX,accY,accZ,gyrX,gyrY,gyrZ,temperature);
  if(accZ > 0)
  {
    gx = gx - gyrY * dt;
    gy = gy + gyrX * dt;
  }
  else
  {
    gx = gx + gyrY * dt;
    gy = gy - gyrX * dt;
  }
  gz += gyrZ * dt;
  gz = gz - 360 * floor(gz / 360);
  if(gz > 180)
  {
    gz = gz - 360;
  }

  filter_coefficient = 0.96; //0.5/(0.5+dt);
  gx = gx * filter_coefficient + ax * (1 - filter_coefficient);
  gy = gy * filter_coefficient + ay * (1 - filter_coefficient);
  printf("temperature:%f gx: %f,gy: %f,gz:%f,ax:%f,ay:%f\n",temperature,gx,gy,gz,ax,ay);
}

STATIC mp_obj_t mb_gyro_board_value(mp_uint_t n_args, const mp_obj_t *args)
{
  float value = 0;  
  
  mb_gyro_board_obj_t *self = args[0];
  //self->axis = mp_obj_get_int(args[1]);
  if(self->axis == 1)
  {
    value = gy;
  }
  else if(self->axis == 2)
  {
    value = gz;
  }
  else
  {
    value = gx;
  }
  return mp_obj_new_float(value);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mb_gyro_board_value_obj, 1, 1, mb_gyro_board_value);

void mb_gyro_board_value_cmd(uint8_t index, uint8_t port,uint8_t axis)
{

}

STATIC mp_obj_t mb_gyro_board_call(mp_obj_t self_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args)
{
  mp_arg_check_num(n_args, n_kw, 0, 0, false);
  return mb_gyro_board_value(n_args,args);
}

STATIC const mp_map_elem_t mb_gyro_board_locals_dict_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_value), 			  (mp_obj_t)&mb_gyro_board_value_obj },
};

STATIC MP_DEFINE_CONST_DICT(mb_gyro_board_locals_dict, mb_gyro_board_locals_dict_table);

const mp_obj_type_t mb_gyro_board_type =
{
  { &mp_type_type },
  .name = MP_QSTR_gyro_board,
  .print = mb_gyro_board_print,
  .call = mb_gyro_board_call,
  .make_new = mb_gyro_board_make_new,
  .locals_dict = (mp_obj_t)&mb_gyro_board_locals_dict,
};















