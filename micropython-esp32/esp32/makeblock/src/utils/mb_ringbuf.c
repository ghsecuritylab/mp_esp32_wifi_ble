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
#include <stdio.h>
#include "mb_ringbuf.h"


/*****************************************************************
 Macro definition
******************************************************************/


/*****************************************************************
 Type definition
******************************************************************/


/*****************************************************************
 Local varible definition
******************************************************************/


/*****************************************************************
 Local function deleration
******************************************************************/


/*****************************************************************
 External function definition
******************************************************************/
int mb_ringbuf_get(mb_ringbuf_t *r) {
    if (r->iget == r->iput) {
        return -1;
    }
    uint8_t v = r->buf[r->iget++];
    if (r->iget >= r->size) {
        r->iget = 0;
    }
    return v;
}

int mb_ringbuf_put(mb_ringbuf_t *r, uint8_t v) {
    uint32_t iput_new = r->iput + 1;
    if (iput_new >= r->size) {
        iput_new = 0;
    }
    if (iput_new == r->iget) {
        return -1;
    }
    r->buf[r->iput] = v;
    r->iput = iput_new;
    return 0;
}

