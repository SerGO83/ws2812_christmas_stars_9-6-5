#ifndef WS2812_H_
#define WS2812_H_
#include "effects.h"

#define MAX_LEDS 280  //ВАЖНО!!! Обязательно указывать четное число

//--------------------------------------------------
typedef struct
{
		uint16_t H;
		uint8_t  S;
		uint8_t  V;
} HSV_TypeDef;

void ws2812_hsv_to_rgb(int pixel);
void ws2812_refresh(void);

#endif
