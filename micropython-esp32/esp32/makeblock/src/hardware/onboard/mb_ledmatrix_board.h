/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Heard for ledmatrix_board.c.
 * @file    ledmatrix_board.h
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
 * This file is a drive ledmatrix_board module.
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



#ifndef MB_LEDMATRIX_BOARD_H_
#define MB_LEDMATRIX_BOARD_H_

#define DIGITAL_SCL_IO    12
#define DIGITAL_SDA_IO    22

typedef enum 
{
  DIGITAL_LOW =0,
  DIGITAL_HIGH =1,
} digital_status;

typedef enum
{
  Brightness_0 = 0,
  Brightness_1,
  Brightness_2,
  Brightness_3,
  Brightness_4,
  Brightness_5,
  Brightness_6,
  Brightness_7,
  Brightness_8
}LED_Matrix_Brightness_TypeDef;

typedef struct 
{
  uint8_t data[3];
}LED_Matrix_TimeFont_TypeDef;

typedef struct 
{
  uint8_t Character[1];
  uint8_t data[6];
}LED_Matrix_CharFont_TypeDef;


extern const LED_Matrix_CharFont_TypeDef char_font[];
extern const LED_Matrix_TimeFont_TypeDef time_font[];


extern  void mb_ledmatrix_board_config();  
extern  void mb_digital_write(gpio_num_t io_num , digital_status sta);
extern  void mb_digital_write_byte(uint8_t data);
extern  void mb_digital_writebytestoaddress(uint8_t Address, const uint8_t *P_data, uint8_t count_of_data);
extern  void mb_Ledmatrix_setbrightness(uint8_t bright);
extern  void mb_digital_write_delay();
extern  void mb_ledmatrix_board_init();
extern  void mb_ledmatrix_clean();
extern 	void mb_drawBitmap(int8_t x, int8_t y, uint8_t Bitmap_Width, const uint8_t *Bitmap);

	

extern const mp_obj_type_t mb_ledmatrix_board_type;

#endif /* MB_LEDMATRIX_BOARD_BOARD_H_*/



