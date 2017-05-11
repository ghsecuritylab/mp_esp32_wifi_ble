/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Heard for mb_button_board.c.
 * @file    mb_button_board.h
 * @author  fftust
 * @version V1.0.0
 * @date    2017/03/24
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
 * This file is a drive  button_board module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * fftust              2017/03/24    1.0.0            build the new.
 * </pre>
 *
 */



#ifndef MB_BUTTON_BOARD_H_
#define MB_BUTTON_BOARD_H_

#define BUTTON1_IO    15
#define BUTTON2_IO    12
#define BUTTON3_IO    0
#define BUTTON4_IO    2

extern void mb_button_board_config();  

extern const mp_obj_type_t mb_button_board_type;

#endif /* MB_BUTTON_BOARD_H_*/


