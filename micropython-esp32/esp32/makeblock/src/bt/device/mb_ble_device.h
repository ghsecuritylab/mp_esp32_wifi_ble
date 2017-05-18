/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Heard for mb_ble_device.c.
 * @file    mb_ble_device.h
 * @author  Leo lu
 * @version V1.0.0
 * @date    2017/04/27
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
 * This file setup ble to device and start ble device with a makeblock profile.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * Leo lu             2017/04/27   	  1.0.0              Initial version
 * </pre>
 *
 */



#ifndef MB_BLE_DEVICE_H_
#define MB_BLE_DEVICE_H_

extern void mb_ble_device_init( void );
extern int mb_get_ble_char( void );
extern int mb_push_to_ble( uint8_t v );

#endif /* MB_BLE_DEVICE_H_*/

