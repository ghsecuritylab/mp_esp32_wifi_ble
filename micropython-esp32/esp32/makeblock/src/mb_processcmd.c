/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Process command with the second kernel.
 * @file    mb_processcmd.c
 * @author  Mark Yan
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
 * This file used for process the command from the second kernel.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * Mark Yan         2017/03/02     1.0.0            build the new.
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
#include "esp32_mphal.h"

#include "mb_processcmd.h"



/******************************************************************************
 MACRO DEFINITION
 ******************************************************************************/
#define BUF_SIZE (1024)

#define ECHO_TEST_TXD  (17)
#define ECHO_TEST_RXD  (16)
#define ECHO_TEST_RTS  (23) //fftust:the configer of RTS AND CTS will influence these two pins,it should be  
#define ECHO_TEST_CTS  (22) //configed to some pins that would not be used or just cancel these two configer

/******************************************************************************
 DECLARE CONSTANTS
 ******************************************************************************/

/******************************************************************************
 DEFINE TYPES
 ******************************************************************************/
union
{
  uint8_t byteVal[4];
  float floatVal;
  long longVal;
}val2;

union
{
  uint8_t byteVal[4];
  double doubleVal;
}valDouble2;

union
{
  uint8_t byteVal[2];
  int16_t shortVal;
}valShort2;

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/
static bool uart1_opened = false;
/******************************************************************************
 DECLARE PUBLIC DATA
 ******************************************************************************/
#define MAX_BUFFER_SIZE  52

uint8_t send_buffer_data[MAX_BUFFER_SIZE] = {0};
uint8_t read_buffer_data[MAX_BUFFER_SIZE] = {0};
uint8_t send_index = 0;
uint8_t read_index = 0;
volatile bool    rsp_be_received = false;
volatile bool    pure_command_mode = true;


/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/

/******************************************************************************/

void communication_channel_init(void)
{
  if(uart1_opened == false)
  {
    int uart_num = UART_NUM_1;
	uart1_opened = true;
    uart_config_t uart_config =
    {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 64,
    };
    uart_param_config(uart_num, &uart_config);
    uart_set_pin(uart_num, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS);
    uart_driver_install(uart_num, BUF_SIZE * 2, 0, 0, NULL, 0);
    //uart_enable_pattern_det_intr(uart_num, '+', 3, 10000, 10, 10);
  }
}

void process_serial_data(void *pvParameters)
{
  uint8_t* data = (uint8_t*) malloc(BUF_SIZE);
  communication_channel_init();
  while(1)
  {
	//Read data from UART1
	int len = uart_read_bytes(UART_NUM_1, data, BUF_SIZE, 20 / portTICK_RATE_MS);
    //Write data back to UART0
    if(len > 0)
    {
      //mp_hal_stdout_tx_strn((const char*)data,len);
	  rsp_be_received = true;
	  if(pure_command_mode == true)
      {
        //mp_hal_stdout_tx_strn((const char*)data,len);
	  }
	  memcpy(read_buffer_data,data,len);
    }
	vTaskDelay(20/portTICK_PERIOD_MS);
  }
}

bool check_start_frame(void)
{
  if((read_buffer_data[0] == 0xff) && (read_buffer_data[1] == 0x55))
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool is_data_byte(void)
{
  if(read_buffer_data[3] == TYPE_BYTE)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool is_data_float(void)
{
  if(read_buffer_data[3] == TYPE_FLOAT)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool is_data_short(void)
{
  if(read_buffer_data[3] == TYPE_SHORT)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool is_data_string(void)
{
  if(read_buffer_data[3] == TYPE_STRING)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool is_data_double(void)
{
  if(read_buffer_data[3] == TYPE_DOUBLE)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool is_data_long(void)
{
  if(read_buffer_data[3] == TYPE_LONG)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void free_the_read_buffer(void)
{
  memset(read_buffer_data,0,MAX_BUFFER_SIZE);
}

void write_serial(uint8_t ch)
{
  const char data = ch;
  uart_write_bytes(UART_NUM_1,(const char*)&data,1);
}

void send_byte(uint8_t ch)
{
  write_serial(ch);
}

void send_float(float value)
{ 
  val2.floatVal = value;
  write_serial(val2.byteVal[0]);
  write_serial(val2.byteVal[1]);
  write_serial(val2.byteVal[2]);
  write_serial(val2.byteVal[3]);
}

void send_short(int16_t value)
{
  valShort2.shortVal = value;
  write_serial(valShort2.byteVal[0]);
  write_serial(valShort2.byteVal[1]);
}

void send_string(char* str,uint8_t size)
{
  write_serial(size);
  for(int16_t i=0;i<size;i++)
  {
    write_serial(*(str+1));
  }
}

void send_double(double value)
{
  valDouble2.doubleVal = value;
  write_serial(valDouble2.byteVal[0]);
  write_serial(valDouble2.byteVal[1]);
  write_serial(valDouble2.byteVal[2]);
  write_serial(valDouble2.byteVal[3]);
}

void send_long(long value)
{ 
  val2.longVal = value;
  write_serial(val2.byteVal[0]);
  write_serial(val2.byteVal[1]);
  write_serial(val2.byteVal[2]);
  write_serial(val2.byteVal[3]);
}

uint8_t read_buffer(int16_t index)
{
  return read_buffer_data[index]; 
}

int16_t read_short(int16_t idx)
{
  valShort2.byteVal[0] = read_buffer(idx);
  valShort2.byteVal[1] = read_buffer(idx+1);
  return valShort2.shortVal; 
}

float read_float(int16_t idx)
{
  val2.byteVal[0] = read_buffer(idx);
  val2.byteVal[1] = read_buffer(idx+1);
  val2.byteVal[2] = read_buffer(idx+2);
  val2.byteVal[3] = read_buffer(idx+3);
  return val2.floatVal;
}

long read_long(int16_t idx)
{
  val2.byteVal[0] = read_buffer(idx);
  val2.byteVal[1] = read_buffer(idx+1);
  val2.byteVal[2] = read_buffer(idx+2);
  val2.byteVal[3] = read_buffer(idx+3);
  return val2.longVal;
}

void write_head(void)
{
  write_serial(0xff);
  write_serial(0x55);
}

void write_end(void)
{
  write_serial('\r');
  write_serial('\n'); 
}

