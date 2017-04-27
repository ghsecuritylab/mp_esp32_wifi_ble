/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Heard for mb_dcmotor.c.
 * @file    mb_dcmotor.h
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
 * This file is a drive dcmotor module.
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

#ifndef MB_DCMOTOR_H_
#define MB_DCMOTOR_H_

extern const mp_obj_type_t mb_dcmotor_type;
extern void mb_dcmotor_run_cmd(uint8_t index, uint8_t port,int16_t speed);

#endif /* MB_DCMOTOR_H_ */
