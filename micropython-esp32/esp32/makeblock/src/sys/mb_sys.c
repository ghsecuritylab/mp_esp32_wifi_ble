/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   The basis of the function for makeblock.
 * @file    mb_sys.c
 * @author  Mark Yan
 * @version V1.0.0
 * @date    2017/03/30
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
 * This file include some system function.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 *   Mark Yan        2017/03/30     1.0.0            build the new.
 * </pre>
 *
 */
 
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "mb_sys.h"
#include "esp32_mphal.h"


uint32_t millis()
{
  return mp_hal_ticks_ms();
}