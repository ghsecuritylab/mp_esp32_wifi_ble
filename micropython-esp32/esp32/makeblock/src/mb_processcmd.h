/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Heard for mb_processcmd.c.
 * @file    mb_processcmd.h
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

#ifndef MB_PROCESSCMD_H_
#define MB_PROCESSCMD_H_

/*************************/
 #include "esp_log.h"
 #define  MB_TAG "Makeblock: "
/*************************/





#define GET_CMD   1      //action
#define RUN_CMD   2

#define START_FRAME_1   0xff  //head
#define START_FRAME_2   0x55

#define TYPE_BYTE       0x01
#define TYPE_FLOAT      0x02
#define TYPE_SHORT      0x03
#define TYPE_STRING     0x04
#define TYPE_DOUBLE     0x05
#define TYPE_LONG       0x06



#define VERSION                0  //device
#define ULTRASONIC_SENSOR      1  //超声波
#define TEMPERATURE_SENSOR     2  //光温度传感器
#define LIGHT_SENSOR           3  //光线传感器
#define POTENTIONMETER         4  //电位器
#define JOYSTICK               5  //操作杆
#define GYRO                   6  //陀螺仪
#define SOUND_SENSOR           7  //音量传感器
#define RGBLED                 8  //RGB灯
#define SEVSEG                 9  //数码管
#define MOTOR                  10 //电机
#define SERVO                  11 //舵机 
#define ENCODER                12 //编码电机
#define IR                     13 //红外????????
#define IRREMOTE               14 //红外遥控
#define PIRMOTION              15 //人体红外
#define INFRARED               16  
#define LINEFOLLOWER           17 //巡线
#define IRREMOTECODE           18 
#define SHUTTER                20 //相机快门
#define LIMITSWITCH            21 //限位开关
#define BUTTON                 22 //按钮
#define HUMITURE               23 //温湿度传感器
#define FLAMESENSOR            24 //火焰传感器
#define GASSENSOR              25 //气体传感器
#define COMPASS                26 //罗盘
#define TEMPERATURE_SENSOR_1   27 //
#define DIGITAL                30 //
#define ANALOG                 31
#define PWM                    32 //pwm
#define SERVO_PIN              33 
#define TONE                   34 //蜂鸣器
#define BUTTON_INNER           35 //内部按钮
#define ULTRASONIC_ARDUINO     36 
#define PULSEIN                37 
#define STEPPER                40 //步进电机
#define LEDMATRIX              41 //表情板
#define TIMER                  50 //定时器
#define TOUCH_SENSOR           51 //触摸板
#define JOYSTICK_MOVE          52 //
#define COMMON_COMMONCMD       60
  //Secondary command
  #define SET_STARTER_MODE     0x10
  #define SET_AURIGA_MODE      0x11
  #define SET_MEGAPI_MODE      0x12
  #define GET_BATTERY_POWER    0x70
  #define GET_AURIGA_MODE      0x71
  #define GET_MEGAPI_MODE      0x72
#define ENCODER_BOARD          61
  //Read type
  #define ENCODER_BOARD_POS    0x01
  #define ENCODER_BOARD_SPEED  0x02

#define ENCODER_PID_MOTION     62
  //Secondary command
  #define ENCODER_BOARD_POS_MOTION         0x01
  #define ENCODER_BOARD_SPEED_MOTION       0x02
  #define ENCODER_BOARD_PWM_MOTION         0x03
  #define ENCODER_BOARD_SET_CUR_POS_ZERO   0x04
  #define ENCODER_BOARD_CAR_POS_MOTION     0x05
  
#define PM25SENSOR             63
  //Secondary command
  #define GET_PM1_0         0x01
  #define GET_PM2_5         0x02
  #define GET_PM10          0x03

#define MICROPYTHON_ESP32      0x50

/******************************************************************************
 DECLARE PUBLIC DATA
 ******************************************************************************/
extern uint8_t send_buffer_data[52];
extern uint8_t read_buffer_data[52];
extern uint8_t send_index;
extern uint8_t read_index;
extern volatile bool rsp_be_received;
extern volatile bool pure_command_mode;


void communication_channel_init(void);
void process_serial_data(void *pvParameters);
bool check_start_frame(void);
bool is_data_byte(void);
bool is_data_float(void);
bool is_data_short(void);
bool is_data_string(void);
bool is_data_double(void);
bool is_data_long(void);
void free_the_read_buffer(void);


void write_serial(uint8_t ch);
void send_byte(uint8_t ch);
void send_string(char* str,uint8_t size);
void send_short(int16_t value);
void send_float(float value);
void send_long(long value);
void send_double(double value);

uint8_t read_buffer(int16_t index);
int16_t read_short(int16_t idx);
float read_float(int16_t idx);
long read_long(int16_t idx);


void write_head(void);
void write_end(void);

#endif /* MB_PROCESSCMD_H_ */
