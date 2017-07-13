#ifndef __LQ12864_H__
#define __LQ12864_H__

#include "root.h"

#define OLED_DC(x)  GPIO_WriteBit(OELD_PORT, OELD_DC_PIN, x)
#define OLED_RST(x) GPIO_WriteBit(OELD_PORT, OELD_RST_PIN,x)
#define OLED_SCL(x) GPIO_WriteBit(OELD_PORT, OELD_SCL_PIN,x)
#define OLED_SDA(x) GPIO_WriteBit(OELD_PORT, OELD_SDA_PIN,x)

#define OLED_CLS() OLED_Fill(0x00)

// x: 0-127, y: 0-63
void OLED_Init(void);
void OLED_Fill(uint8_t bmp_data);
void OLED_PutPixel(uint8_t x, uint8_t y);
void OLED_Rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t gif);
void OLED_DrawBMP(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t * bmp); 
void OLED_DrawCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint8_t _ucColor);
void OLED_P6x8Str(uint8_t x, uint8_t y, uint8_t ch[]);
void OLED_P6x8Num(uint8_t x, uint8_t y, uint16_t num);

#endif
