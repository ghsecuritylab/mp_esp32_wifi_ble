# 说明 #
1.将makeblock文件夹里所有文件全部放在外一层
2.加入了新的文件 esp32_hal.c 和esp32_hal.h文件
3.在modmakeblock.c文件中注释掉了 include "machuart.h" 该文件在实际操作中并未使用，且和uart.h文件存在冲突
4.在mpconfigport.c 文件中注册makeblock





# 新主板问题收集#
1.五向开关 管脚定义错误；


2.0,2,12,15,5 管脚需设定固定初始状态 否则很可能造成系统无法启动


3.esp-idf 未定义ADC2——api  自己添加了 ADC2 在adc.h 和Rtc_Moudle中做了修改和注释；

4.I2c总线出现问题；
