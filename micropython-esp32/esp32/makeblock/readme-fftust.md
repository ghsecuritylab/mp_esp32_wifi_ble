# 需要阅读和修改的文件 #

1. mods/makeblock文件夹内的文件属于和mbot通信时所必须的一些模块的代码文件，和makeblock module相关的内容最好都整理在这个文件夹内

2. application.mk  这个Makefile文件用于新增文件的编译配置。

3. mptask.c 这个文件是esp32内核的主体程序，命令解析，python解析器，以及文件传输的代码的入口都在这个地方

4. mpconfigport.h   新增python模块时，需要在这里进行配置。


# 需要注意的修改规则 #

1. 文件头的一些简要介绍
2. 函数的命名方式要统一
3. 在函数文件中不要用table键来替代空格键，所有缩进都要按照两个空格的规则。
4. 每个功能做完都需要及时自测，避免代码误入歧途，浪费时间
5. 养成良好的编程习惯

# 控制协议 #
##下发命令格式##

/*

	ff 55 len idx action device dat1 dat2 dat3
	0  1  2   3   4      5      6    7    8
*/


## 上传命令格式 ##
/*

	ff 55  idx  type da1  dat2  dat3  da4   end1 end2
	0  1   2     3    4     5     6          0d   0a
*/


进入原始命名后，通过以下命令来切换到python repl模式


ff 55 04 01 02 50 05    切换到 PYEXEC_MODE_FRIENDLY_REPL

ff 55 04 01 02 50 06    切换到 PYEXEC_MODE_RAW_REPL

## 电机控制协议 ##
 
ff 55 06 00 02 0a 0a 64 00


/*

    head   len  idx  action  device   port  speed
    ff 55  06   00   02      0a       0a    64 00
*/

控制port10(电机接口2)的电机转动100
## RGBLed 控制协议 ##
ff 55 09 00 02 08 07 02 01 00 00 00  //板载端口07，slot 02为固定值，外接RGBLED（slot固定为02）及灯带device均为08
/*
   
    head   len  idx  act  dev   port  slot leds r  g  b
    ff 55   09  00   02   08    07    02     0    0  0  0
*/

python:

   from makeblock import rgbled 

   rgb=rgbled()

   rgb.show(7,2,0,255,255,255) //板载全亮

## LED_MATRIX控制协议 ##

### 显示字母模式 ###
ff 55 09 00 02 29 01 01 00 07 01 6c //显示字母 l

/*
   
     head len idx  act dev  port  mods  p_x  p_y data_num  data1 data2  ..
    ff 55 09  00   02  29   01    01    00   07  01        6c     ..
    

*/ 

python :

   from makeblock import led_matrix 
   ledm=led_matrix()
   ledm.show(01,01,00,07,01,108) 

### 显示绘图模式 ###
ff 55 09 00 02 29 01 02 00 00 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff //共16ff字节，代表16*8个灯，该命令全亮

/*
   
     head len idx  act dev  port  mods  p_x  p_y   data1 data2  ..  data16
    ff 55 09  00   02  29   01    02    00   00    ff     ff         ff
    

*/ 

python :

   from makeblock import led_matrix 

   ledm=led_matrix()

   ledm.show(01,02,00,00,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255)   
 
### 显示时间模式 ###
ff 55 09 00 02 29 01 03 01 14 14 //显示时间为20:20，中间的点显示出来

/*
   
     head len idx  act dev  port  mods  point  hour  min
    ff 55 09  00   02  29   01    03    01     14     14
    

*/ 

python :

   from makeblock import led_matrix 

   ledm=led_matrix()

   ledm.show(01,03,01,14,14) 
### 显示数字模式 ###
ff 55 09 00 02 29 01 04 00 00 80 3f //显示数字 1

/*
   
     head len idx  act dev  port  mods  f_d1  f_d2 f_d3  f_d4
    ff 55 09  00   02  29   01    04    00    00   80    3f
    

*/ 

python :

   from makeblock import led_matrix 

   ledm=led_matrix()

   ledm.show(01,04,1) 


## 数码管控制协议 ##
ff 55 08 00 02 09 04 00 00 5c 4  //数码管显示55.00

/*
   
     head len idx  act dev  port  f_d1  f_d2 f_d3  f_d4
    ff 55 09  00   02  09   04    00    00   5c    42
    

*/ 
python :

   from makeblock import sevseg 

   sev=sevseg()

   sev.show(04,55.00) 

## 舵机控制协议 ##
ff 55 06 00 02 0b 01 01 5a  //接口1 插头1的舵机角度90°

/*
   
     head len idx  act dev  port  slot  angle
    ff 55 06  00   02  0b   01    01    5a
    

*/


python :

   from makeblock import servo

   ser=servo()

   ser.run(01,01,90)

## 扬声器控制协议 ##
ff 55 07 00 02 22 06 01 f4 01  //c4 1/2节拍

/*
   
     head len idx  act dev  data1  data2   data3   节拍
    ff 55 07  00   02  22   06     01      f4       01
    

*/

python :

   from makeblock import buzzer 

   buz=buzzer()

   buz.play(06,01,244,01)

## 触摸板控制协议##

ff 55 04 00 01 33 01 //读port1的触摸板状态

/*
   
     head len idx  act dev  port
    ff 55 04  00   01  33   01     
    

*/

python :

   from makeblock import touch_sensor 

   tou=touch_sensor()

   tou.value(1)


## 超声波控制协议##
ff 55 04 00 01 01 03 //读port3中的超声波传感器数据

/*
   
     head len idx  act dev  port
    ff 55 04  00   01   01   03     
    

*/


python :

   from makeblock import ultrasonic_sensor 

   ult=ultrasonic_sensor()

   ult.value(3)

## 罗盘、火焰传感器、气体传感器、电位器控制协议##
  以上几种传感器协议类似 之传入一个port参数，回传一个float数据
  
ff 55 04 00 01 1a 01 compass
ff 55 04 00 01 18 03 flame_sensor
ff 55 04 00 01 19 03 gas_sensor
ff 55 04 00 01 04 03 potentionmeter

/*
   
     head len idx  act dev  port
    ff 55 04  00   01   1a   01     
    

*/


python:

   from makeblock import compass   //罗盘

   com=compass()

   com.value(1)



   from makeblock import flame_sensor   //火焰传感器

   fs=flame_sensor()

   fs.value(3)

   
   from makeblock import gas_sensor     //气体传感器

   gs=gas_sensor()

   gs.value(3)


   from makeblock import potentionmeter  //电位器
   pm=potentionmeter()

   pm.value(3)



## 相机快门控制协议 ##
ff 55 05 00 02 14 01 01  //设置接口一的快门按下


/*
   
     head len idx  act dev  port action
    ff 55 04  00   02  14   01     01  
    

*/


python：

   from makeblock import shutter 

   shu=shutter()

   shu.run(1,1)


## 陀螺仪控制协议 ##
ff 55 05 00 01 06 00 02   //读取y轴的角度

/*
   
     head len idx  act dev  port axis
    ff 55 04  00   01  06   00    02  
    

*/

python：

   from makeblock import gyro 

   gyr=gyro()

   gyr.value(0,2)

## 光线传感器控制协议 ##
ff 55 04 00 01 03 06  //读取板载光线传感器的数据

/*
   
     head len idx  act dev  port 
    ff 55 04  00   01  03   06    
    

*/

python：

   from makeblock import light_sensor

   lig=light_sensor()

   lig.value(6)


## 限位开关控制协议 ##
ff 55 05 00 01 15 01 01  //读取port1 slot1上限位开关状态

/*
   
     head len idx  act dev  port slot 
    ff 55 05  00   01  15   01    01
    

*/
python：

   from makeblock import limitswitch

   swi=limitswitch()

   swi.value(1,1)

## 温度传感器 ##
ff 55 05 00 01 02 03 01 //读取port3 slot的温度传感器数据

/*
   
     head len idx  act dev  port slot 
    ff 55 05  00   01  02   03    01
    

*/
python：

   from makeblock import temperature

   tem=temperature()

   tem.value(3,1)
## python的一些语句 ##

from makeblock import dcmotor

dc1 = dcmotor()

dc.run(10,100)        //端口10的电机设置100的速度

import makeblock
line = makeblock.linefollower()
line2 = seeed.linefollower()

line.value(2)
rgb=makeblock.rgbled()
rgb.rgbled(7,0,100,100,100)


## 巡线传感器 ##

ff 55 04 00 01 11 02
/*

    head   len  idx  action  device   port  speed
    ff 55  04   00   01      11       02
*/

返回值是 ff 55 00 02 00 00 40 40 0d 0a

/*

    head   idx  type  value         end
    ff 55  00   02    00 00 40 40   0d 0a
*/




## 外接按钮的控制协议 ##

ff 55 05 00 01 16 03 01 //port3 的key1是否按下

/*

    head   len  idx  action  device   port  keys
    ff 55  04   00   01      16       03    01
*/

python:

   from makeblock import button 

   but=button()

   but.value(3,1)

   

## 声音传感器控制协议 ##

ff 55 04 00 01 07 03    //读取port3声音传感器的值

/*

    head   len  idx  action  device   port  
    ff 55  04   00   01      07       03    
*/


python:

   from makeblock import sound_sensor 

   sou=sound_sensor()

   sou.value(3)



## 人体红外传感器控制协议 ##

ff 55 04 00 01 0f 02     //读取port2人体红外的值


/*

    head   len  idx  action  device   port  
    ff 55  04   00   01      0f       02    
*/


python:

   from makeblock import pirmotion 

   pir=pirmotion()

   pir.value(3)


##  板载按钮控制协议 ##

ff 55 05 00 01 23 07 00  //查询板载按钮是否已按下


/*

    head   len  idx  action  device   port 
    ff 55  05   00   01      07       00    

*/



python:

   from makeblock import button_inner

   bi=button_inner()

   bi.value(0)




## 温湿度传感器控制协议 ##

ff 55 05 00 01 17 01 00  //读取port1的湿度
 
/*

    head   len  idx  action  device   port  h/t  
    ff 55  05   00   01      07       01    00（01）

*/

python:

   from makeblock import humiture

   ht=humiture()

   ht.value(1,0) //读取湿度
   ht.value(1,1) //读取温度





# 新主板板载触感器的控制协议说明 #

## 板载表情面板 ##
from makeblock import ledmatrix_board
le=ledmatrix_board()
1.字符显示

le.char_show(0,0,1,98) //显示字符‘b’   char_show(0,0,2,98,98) //显示字符‘b’ ‘b’
  参数说明：1&2 位置  3 显示字符个数 4&..  字符码   

2.时间显示
le.time_show(12,12)  //显示12:12
  参数说明：1 时  2 分

3.图形显示
le.draw(0,0,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0) //第一列到第五列全亮 后面全灭
  参数说明： 1&2 位置 3-18 图形数据
   




## 板载电位器 ##
from makeblock import potentionmeter_board
pb=potentionmeter_board()
pb.value()

返回值：16位AD 采集数据


## 板载声音传感器 ##
from makeblock import sound_sensor_board
ss=sound_sensor_board()
ss.value()

返回值：16位AD 采集数据


## 板载光线传感器 ##
from makeblock import light_sensor_board
ss=sound_sensor_board()
ss.value()

返回值：16位AD 采集数据

## 板载按钮 ##
   from makeblock import button_board

   bb=button_board()

   bb.value(1)
参数说明： 板载按钮号（1 or 2）
返回值：开关状态




# 后期修改需要注意的 #

1. communication_channel_init 这个函数后期应该根据硬件状态自动调用和注销

2. 有一个超时判断需要补上

# fftust add #
#  程序编写注意事项 #
1.makeblock 相关函数名全部用小写+下划线格式；
2.函数 大括号另起一行，且代码全部缩进两格，不使用Tab键；

# 程序框架说明 #
1.ESP内核有三种工作模式，在mptask中配置默认模式 修改pyexec_mode_kind = PYEXEC_MODE_FRIENDLY_REPL可更改默认模式；
2.增加模块，程序写在makeblock文件夹里；modmakeblock.c注册py模块（链接.c和py模块）需要修改的程序如下：
 a.application.mk 增加新增的.c文件路径
 b.makeblock.c  注册新的模块 mb_makeblock.h 文件中加入.h文件
 c.pure_command模式下的处理函数需mptask.c中的runmoudle 和 readsensor函数中编写；

# 硬件接口说明 #
1.LED1=GPIO5 LED2=GPIO25
2.接在mbot上的串口为 u2 u2_TXD=16 u2_RXD=17


volatile bool    rsp_be_received = false;
volatile bool    pure_command_mode = true;

未volatile关键字 上发传感器数据死机；
