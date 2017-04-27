/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   The python module for makeblock docking station.
 * @file    modmakeblock.h
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
 * The python module for makeblock docking station.
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
#include <stdio.h>
#include <string.h>

#include "py/mpconfig.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/objlist.h"
#include "py/stream.h"
#include "py/mphal.h"
#include "uart.h"

#include "mb_makeblock.h"
/// \module makeblock - functions related to the makeblock
///

/******************************************************************************/
// Micro Python bindings;

STATIC mp_obj_t makeblock_reset(void) {
    printf("makeblock_reset");
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(makeblock_reset_obj, makeblock_reset);

STATIC mp_obj_t makeblock_setup(void) {
    printf("makeblock_setup");
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(makeblock_setup_obj, makeblock_setup);


STATIC const mp_map_elem_t makeblock_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_reset),                     (mp_obj_t)(&makeblock_reset_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_setup),                     (mp_obj_t)(&makeblock_setup_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_linefollower),              (mp_obj_t)&mb_linefollower_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_dcmotor),                   (mp_obj_t)&mb_dcmotor_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ultrasonic_sensor),         (mp_obj_t)&mb_ultrasonic_sensor_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_light_sensor),              (mp_obj_t)&mb_light_sensor_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_rgbled),                    (mp_obj_t)&mb_rgbled_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_buzzer),                    (mp_obj_t)&mb_buzzer_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_touch_sensor),              (mp_obj_t)&mb_touch_sensor_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_sevseg),                    (mp_obj_t)&mb_sevseg_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_servo),                     (mp_obj_t)&mb_servo_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_humiture),                  (mp_obj_t)&mb_humiture_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_led_matrix),                (mp_obj_t)&mb_led_matrix_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_limitswitch),               (mp_obj_t)&mb_limitswitch_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_temperature),               (mp_obj_t)&mb_temperature_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_potentionmeter),            (mp_obj_t)&mb_potentionmeter_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_sound_sensor),              (mp_obj_t)&mb_sound_sensor_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_flame_sensor),              (mp_obj_t)&mb_flame_sensor_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_gas_sensor),                (mp_obj_t)&mb_gas_sensor_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_compass),                   (mp_obj_t)&mb_compass_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_gyro),                      (mp_obj_t)&mb_gyro_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_button_inner),              (mp_obj_t)&mb_button_inner_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_joystick),                  (mp_obj_t)&mb_joystick_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pirmotion),                 (mp_obj_t)&mb_pirmotion_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_shutter),                   (mp_obj_t)&mb_shutter_type }, 
    { MP_OBJ_NEW_QSTR(MP_QSTR_button),                    (mp_obj_t)&mb_button_type }, 
    { MP_OBJ_NEW_QSTR(MP_QSTR_irremote),                  (mp_obj_t)&mb_irremote_type }, 
    { MP_OBJ_NEW_QSTR(MP_QSTR_gyro_board),                (mp_obj_t)&mb_gyro_board_type }, 
    { MP_OBJ_NEW_QSTR(MP_QSTR_button_board),              (mp_obj_t)&mb_button_board_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ledmatrix_board),           (mp_obj_t)&mb_ledmatrix_board_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_light_sensor_board),        (mp_obj_t)&mb_light_sensor_board_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_potentionmeter_board),      (mp_obj_t)&mb_potentionmeter_board_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_fw_switch_board),           (mp_obj_t)&mb_fw_switch_board_type },   
    { MP_OBJ_NEW_QSTR(MP_QSTR_vibratingmotor_board),      (mp_obj_t)&mb_vibratingmotor_board_type }, 
    { MP_OBJ_NEW_QSTR(MP_QSTR_sound_sensor_board),        (mp_obj_t)&mb_sound_sensor_board_type }, 
    { MP_OBJ_NEW_QSTR(MP_QSTR_rmt_board),                 (mp_obj_t)&mb_rmt_board_type }, 
    { MP_OBJ_NEW_QSTR(MP_QSTR_wifi_test),                 (mp_obj_t)&mb_wifi_test_type },
};

STATIC MP_DEFINE_CONST_DICT(makeblock_module_globals, makeblock_module_globals_table);

const mp_obj_module_t makeblock_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&makeblock_module_globals,
};