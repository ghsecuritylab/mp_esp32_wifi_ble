/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Ring buffer
 * @file    mb_ringbuf.h
 * @author  Leo lu
 * @version V1.0.0
 * @date    2017/05/17
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
 * This file is a header for ring function.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 *   Leo lu          2017/05/17       1.0.0              build the new.
 * </pre>
 *
 */
  

#ifndef MB_RINGBUF_H_
#define MB_RINGBUF_H_
#include <stdint.h>
#include <string.h>
#include <stdio.h>

/*****************************************************************
 Macro definition
******************************************************************/
#define mb_ringbuf_alloc(r, sz) \
{ \
    (r)->buf = malloc( sz ); \
    (r)->size = sz; \
    (r)->iget = (r)->iput = 0; \
}

/*****************************************************************
 Type definition
******************************************************************/
typedef struct mb_ringbuf_t {
    uint8_t *buf;
    uint16_t size;
    uint16_t iget;
    uint16_t iput;
} mb_ringbuf_t;

/*****************************************************************
 Local varible definition
******************************************************************/


/*****************************************************************
 External vaible decleration
******************************************************************/


/*****************************************************************
 External function decleration
******************************************************************/
/*
*/
int mb_ringbuf_get(mb_ringbuf_t *r);

/*
*/
int mb_ringbuf_put(mb_ringbuf_t *r, uint8_t v);

#endif