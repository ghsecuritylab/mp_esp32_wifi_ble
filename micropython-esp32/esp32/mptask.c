

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_task.h"

#include "py/stackctrl.h"
#include "py/nlr.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/gc.h"
#include "py/mphal.h"
#include "lib/mp-readline/readline.h"
#include "lib/utils/pyexec.h"

#include "uart.h"
#include "driver/uart.h"
#include "mb_makeblock.h"
#include "soc/uart_struct.h"

#include "lib/oofatfs/ff.h"
#include "extmod/vfs_fat.h"

//#include "diskio.h"
//#include "ffconf.h"

/******************************************************************************
 DEFINE TYPES
 ******************************************************************************/
union
{
  uint8_t byteVal[4];
  float floatVal;
  long longVal;
}val;

union
{
  uint8_t byteVal[4];
  double doubleVal;
}valDouble;

union
{
  uint8_t byteVal[2];
  int16_t shortVal;
}valShort;


/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
char            test_data[1024]=" "; 
char	        py_data[128] =" ";
FATFS           sflash_fatfs;

static uint8_t  prec;
static uint8_t  dataLen;
static int16_t  py_index;
static int16_t  py_data_index;
static bool	    isStart;
static bool	    fileRevStart;


//uint8_t *buffer_address_get()
//{
//  return py_data;
//}
void writeBuffer(int16_t index,uint8_t c)
{
  py_data[index] = c;
}

uint8_t readBuffer(int16_t index)
{
  return py_data[index];
}

int16_t readShort(int16_t idx)
{
  valShort.byteVal[0] = readBuffer(idx);
  valShort.byteVal[1] = readBuffer(idx+1);
  return valShort.shortVal; 
}

long readLong(int16_t idx)
{
  val.byteVal[0] = readBuffer(idx);
  val.byteVal[1] = readBuffer(idx+1);
  val.byteVal[2] = readBuffer(idx+2);
  val.byteVal[3] = readBuffer(idx+3);
  return val.longVal;
}

float readFloat(int16_t idx)
{
  val.byteVal[0] = readBuffer(idx);
  val.byteVal[1] = readBuffer(idx+1);
  val.byteVal[2] = readBuffer(idx+2);
  val.byteVal[3] = readBuffer(idx+3);
  return val.floatVal;
}

void writeSerial(uint8_t ch)
{
  const char data = ch;
  uart_write_bytes(UART_NUM_1,(const char*)&data,1);
}

void sendByte(uint8_t ch)
{
  writeSerial(1);
  writeSerial(ch);
}

void sendFloat(float value)
{ 
  writeSerial(2);
  val.floatVal = value;
  writeSerial(val.byteVal[0]);
  writeSerial(val.byteVal[1]);
  writeSerial(val.byteVal[2]);
  writeSerial(val.byteVal[3]);
}

void sendShort(int16_t value)
{
  writeSerial(3);
  valShort.shortVal = value;
  writeSerial(valShort.byteVal[0]);
  writeSerial(valShort.byteVal[1]);
}

void sendString(char* str,uint8_t size)
{
  writeSerial(4);
  writeSerial(size);
  for(int16_t i=0;i<size;i++)
  {
    writeSerial(*(str+1));
  }
}

void sendDouble(double value)
{ 
  writeSerial(5);
  valDouble.doubleVal = value;
  writeSerial(val.byteVal[0]);
  writeSerial(val.byteVal[1]);
  writeSerial(val.byteVal[2]);
  writeSerial(val.byteVal[3]);
}

void sendLong(long value)
{ 
  writeSerial(6);
  val.longVal = value;
  writeSerial(val.byteVal[0]);
  writeSerial(val.byteVal[1]);
  writeSerial(val.byteVal[2]);
  writeSerial(val.byteVal[3]);
}

void writeEnd(void)
{
  const char str[3] = {0x0d,0x0a,'\0'};
  mp_hal_stdout_tx_str(str);
  
}

void callOK(void)
{
  const char str[5] = {0xff,0x55,0x0d,0x0a,'\0'};
  mp_hal_stdout_tx_str(str);
}

void readSensor(uint8_t device)
{
  //uint8_t cmd = readBuffer(6);
  //uint8_t dataLen = readBuffer(2);
  switch(device)
  {
    case MICROPYTHON_ESP32:
      {
        //just for test;
      }
	  break;
    case LINEFOLLOWER:
      {
        uint8_t index = readBuffer(3);
        uint8_t port = readBuffer(6);
		mb_linefollower_value_cmd(index, port);
      }
	  break;
	case LIGHT_SENSOR:
	  { 
        uint8_t ls_index = readBuffer(3);
        uint8_t ls_port = readBuffer(6);
		mb_light_sensor_value_cmd(ls_index,ls_port);
	  }
	  break;	
	case ULTRASONIC_SENSOR:
      {
        uint8_t us_index = readBuffer(3);
        uint8_t us_port = readBuffer(6);
		mb_ultrasonic_sensor_value_cmd(us_index,us_port);
	  }
	  break;
	case TOUCH_SENSOR:
	  {
	    uint8_t ts_index = readBuffer(3);
	    uint8_t ts_port = readBuffer(6);
	    mb_touch_sensor_value_cmd(ts_index,ts_port);
	  }
	  break;
	case HUMITURE:
	  {
        uint8_t ht_index=readBuffer(3);
		uint8_t ht_port=readBuffer(6);
		uint8_t ht_sens=readBuffer(7);
 		mb_humiture_value_cmd(ht_index, ht_port, ht_sens);
	  }
	  break;	
	case LIMITSWITCH:
	  {
        uint8_t lis_index=readBuffer(3);
		uint8_t lis_port=readBuffer(6);
		uint8_t lis_slot=readBuffer(7);
 		mb_limitswitch_value_cmd(lis_index, lis_port, lis_slot);
	  }
	  break;	  
	case TEMPERATURE_SENSOR:
	  {
        uint8_t tem_index=readBuffer(3);
		uint8_t tem_port=readBuffer(6);
		uint8_t tem_slot=readBuffer(7);
 		mb_temperature_value_cmd(tem_index, tem_port, tem_slot);
	  }
	  break;	 
	case POTENTIONMETER:
	  {
	    uint8_t pm_index = readBuffer(3);
	    uint8_t pm_port = readBuffer(6);
	    mb_potentionmeter_value_cmd(pm_index,pm_port);
	  }
	  break;
	case SOUND_SENSOR:
	  {
	    uint8_t ss_index = readBuffer(3);
	    uint8_t ss_port = readBuffer(6);
	    mb_sound_sensor_value_cmd(ss_index,ss_port);
	  }
	  break;	 
	case FLAMESENSOR:
	  {
	    uint8_t fs_index = readBuffer(3);
	    uint8_t fs_port = readBuffer(6);
	    mb_flame_sensor_value_cmd(fs_index,fs_port);
	  }
	  break;
	case GASSENSOR:
	  {
	    uint8_t gs_index = readBuffer(3);
	    uint8_t gs_port = readBuffer(6);
	    mb_gas_sensor_value_cmd(gs_index,gs_port);
	  }
	  break;
	case COMPASS:
	  {
	    uint8_t cp_index = readBuffer(3);
	    uint8_t cp_port = readBuffer(6);
	    mb_compass_value_cmd(cp_index,cp_port);
	  }
	  break;
	case GYRO:
	  {
	    uint8_t gyro_index = readBuffer(3);
	    uint8_t gyro_port  = readBuffer(6);
		uint8_t gyro_axis  = readBuffer(7);
	    mb_gyro_value_cmd(gyro_index,gyro_port,gyro_axis);
	  }
	  break;
	case BUTTON_INNER:
	  {
	    uint8_t bi_index = readBuffer(3);
	    uint8_t bi_port = readBuffer(6);
		uint8_t bi_station=readBuffer(7);
	    mb_button_inner_value_cmd(bi_index,bi_port,bi_station);
	  }
	  break;	  
	case JOYSTICK:
	  {
	    uint8_t joys_index = readBuffer(3);
	    uint8_t joys_port  = readBuffer(6);
		uint8_t joys_axis  =readBuffer(7);
	    mb_joystick_value_cmd(joys_index,joys_port,joys_axis);
	  }
	  break;
	case PIRMOTION:
	  {
	    uint8_t pir_index = readBuffer(3);
	    uint8_t pir_port  = readBuffer(6);
	    mb_pirmotion_value_cmd(pir_index,pir_port);
	  }
	  break;
	case BUTTON:
	  {
	    uint8_t bu_index = readBuffer(3);
	    uint8_t bu_port  = readBuffer(6);
		uint8_t bu_keys  =readBuffer(7);
	    mb_button_value_cmd(bu_index,bu_port,bu_keys);
	  }
	  break;  
	case IRREMOTE:
	  {
	    uint8_t irr_index = readBuffer(3);
	    uint8_t irr_port  = readBuffer(6);
		uint8_t irr_keynum =readBuffer(7);
	    mb_irremote_value_cmd(irr_index,irr_port,irr_keynum);
	  }
	  break;  	  
  }
}

void runMoudle(uint8_t device)
{
  uint8_t cmd = readBuffer(6);
  uint8_t dataLen = readBuffer(2);
  FIL fp;
  UINT n;

  switch(device)
  {
    case MICROPYTHON_ESP32:
      {
        if(cmd == 0x01)
        {
          printf("cmd is 1\r\n");
          py_data_index = 0;
          fileRevStart = false;
        }
        else if(cmd == 0x02)
        { 
          printf("cmd is 2\r\n");
          py_data_index = 0;
          fileRevStart = true;
        }
        else if(cmd == 0x03)
        {
          printf("cmd is 3\r\n");
          memcpy(test_data+py_data_index,py_data+7,(dataLen - 4));
          py_data_index = (py_data_index + dataLen - 4);
        }
        else if(cmd == 0x04)
        {
          printf("cmd is 4,length(%d)\r\n",py_data_index);
          if(fileRevStart == true)
          {
            f_unlink(&sflash_fatfs,"/flash/main.py");
            f_open(&sflash_fatfs,&fp,"/flash/main.py",FA_WRITE | FA_CREATE_ALWAYS);
            f_write(&fp,test_data,py_data_index,&n);
            f_close(&fp);
          }
          fileRevStart = false;
          pyexec_mode_kind = PYEXEC_MODE_FRIENDLY_REPL;
        }
        else if(cmd == 0x05)
        {
          pyexec_mode_kind = PYEXEC_MODE_FRIENDLY_REPL;

        }
        else if(cmd == 0x06)
        {
          pyexec_mode_kind = PYEXEC_MODE_RAW_REPL;
        }
      }
      callOK();

      break;
    case MOTOR:
      {
	  	uint8_t index = readBuffer(3);
        uint8_t port = readBuffer(6);
		int16_t speed = readShort(7);
		mb_dcmotor_run_cmd(index, port,speed);
      }
	  break;
	case RGBLED:
	  {
        uint8_t rgb_index=readBuffer(3);
        uint8_t rgb_port=readBuffer(6);
	    uint8_t rgb_slot=readBuffer(7);
		uint8_t rgb_leds=readBuffer(8);
		uint8_t rgb_r=readBuffer(9);
		uint8_t rgb_g=readBuffer(10);
		uint8_t rgb_b=readBuffer(11);
		mb_rgbled_show_cmd(rgb_index, rgb_port,rgb_slot,rgb_leds,rgb_r,rgb_g,rgb_b);
	  }
	  break;
	case TONE:
	  {
        uint8_t buzzer_index=readBuffer(3);
        uint8_t buzzer_a=readBuffer(6);
	    uint8_t buzzer_b=readBuffer(7);
	    uint8_t buzzer_c=readBuffer(8);
	    uint8_t buzzer_d=readBuffer(9);
        mb_buzzer_play_cmd(buzzer_index,buzzer_a,buzzer_b,buzzer_c ,buzzer_d); 
	  }
	  break;
	case SEVSEG:
	  {
        uint8_t sevseg_index=readBuffer(3);
        uint8_t sevseg_port=readBuffer(6);
	    float   sevseg_val=readFloat(7);
        mb_sevseg_show_cmd(sevseg_index,sevseg_port,sevseg_val);
	  }
	  break;	
	case SERVO:
	  {
        uint8_t servo_index=readBuffer(3);
		uint8_t servo_port=readBuffer(6);
		uint8_t servo_slot=readBuffer(7);
		uint8_t servo_angle=readBuffer(8);
		mb_servo_run_cmd(servo_index, servo_port, servo_slot, servo_angle);  
	  }
	  break;
	case LEDMATRIX:
	  {
        uint8_t ledm_data_num=0;
		uint8_t *ledm_data;
		
		uint8_t ledm_index=readBuffer(3);
		uint8_t ledm_port=readBuffer(6);
		uint8_t ledm_mods=readBuffer(7);
		switch(ledm_mods)
	    {
          case 1:
		  	ledm_data_num=2+1+readBuffer(10);
		  	break;
			
		  case 2:
		  	ledm_data_num=18;
		  	break;
		  case 3:
		  	ledm_data_num=3;
		  	break;
		  case 4:
		  	ledm_data_num=4;
		  	break;
		}
		ledm_data=(uint8_t*)py_data+8;
		mb_led_matrix_show_cmd(ledm_index,ledm_port,ledm_mods,ledm_data_num,ledm_data);
	  }
	  break;	
	case  SHUTTER:
	  {
        uint8_t shu_index=readBuffer(3);
		uint8_t shu_port=readBuffer(6);
		uint8_t shu_sta=readBuffer(7);
		mb_shutter_run_cmd(shu_index, shu_port, shu_sta);  
	  }
	  break;
  }
}

void parseData(void)
{
  uint8_t action = readBuffer(4);
  uint8_t device = readBuffer(5);
  //printf("data:%d,%d,%d\r\n",py_data[2],py_data[3],py_data[4]);
  switch(action)
  {
    case GET_CMD:
    {
      readSensor(device);
      break;
    }
    case RUN_CMD:
    {
      runMoudle(device);
      break; 
    }
  }
}

void pyexec_pure_cmd_repl(void)
{
  byte c = mp_hal_stdin_rx_chr();
  if((c == START_FRAME_2) && (isStart == false))
  {
    if(prec == START_FRAME_1)
    {
      py_index = 1;
      isStart = true;
    }
  }
  else
  {
    prec = c;
    if(isStart)
    {
      if(py_index == 2)
      {
        dataLen  = c;
      }
      else if(py_index > 2)
      {
        dataLen--;
      }
      writeBuffer(py_index,c);
    }
  }
  py_index++;
  if(py_index > 140)
  {
    memset(py_data,0,128);
    py_index = 0;
    isStart = false;
  }
  if((isStart == true) && (dataLen == 0) && (py_index > 3))
  {
    isStart = false;
    parseData();
    py_index = 0;
	memset(py_data,0,128);
  }

}


