#include "main.h"
#include "stm32f1xx_hal.h"
#include "ws2812b.h"

//внешние переменные для таймера1 и для ДМА  объявленые в main.h
extern TIM_HandleTypeDef htim1;
extern DMA_HandleTypeDef hdma_tim1_up;

volatile  uint16_t buff[144] = {0};  //буфер на два светодиода
		//videoRAM one kadr
volatile uint8_t image[MAX_LEDS * 3] = {0};
volatile uint8_t image1[MAX_LEDS * 3] = {0};
		//видеопамять одного кадра изображения в формате HSV
HSV_TypeDef	buffer_ws2812 [MAX_LEDS] = {0};
uint8_t color[3];

extern int flag_DMA;
int num_leds;
char first_entry_kostyl = 0;
volatile char DMA_busy;

void prepare_buf(void){
	for (int i=0; i<48; i++){
		buff[i*3    ] = 0xFFFF;
		buff[i*3 + 1] = 0<<1;
		buff[i*3 + 2] = 0<<1;
		
	}	
}
void clear_buf(void){
	for (int i  = 0; i < 100; i++){
		buff[i] = 0;
	}	
}

void kostyl(void){
	for (int i = 0; i < 35; i++){
		buff[i] = 0<<1;
	}
}
void prepare_buf_eff(void){
	for (int i = 0 ; i  < 8 ; i++){
		if ((image[1]>>(7-i)) & 0x01) {
			buff[i*3 + 1] = 1<<1;
		} else {
			buff[i*3 + 1] = 0<<1;
		}
		if ((image[4]>>(7-i)) & 0x01) {
			buff[72 + i*3 + 1] = 1<<1;
		} else {
			buff[72 + i*3 + 1] = 0<<1;
		}

	}
}

void from_image_to_buff(int num_led, _Bool first_or_second){
int offset;
	ws2812_hsv_to_rgb(num_led);
	//определим в какую половину буфера будем скидывать цвета
	if (first_or_second) {offset = 0;} else {offset = 72;}
		//перекидываем цвета в выходной буфер
		for (char i=0; i<8; i++){
			int ii = i*3;
			//red
			if (((color[0]>>(7-i))&0x01)) {
			   buff [offset+ ii + 1 + 0] |= 2;
		    } else {
		    	buff[offset+ ii + 1 + 0] &= ~(2);
		    }
			//green
			if (((color[1]>>(7-i))&0x01)) {
				buff[offset+ ii + 1 + 24] |= 1<<1;
			} else {
				buff[offset+ ii + 1 + 24] &= ~(1<<1);
			}
			//blue
			if (((color[2]>>(7-i))&0x01)) {
				buff[offset+ ii + 1 + 48] |= 1<<1;
			} else {
				buff[offset+ ii + 1 + 48] &= ~(1<<1);
			}
		}
}
void prepare_for_start(void){
	//перекидываем цвета в выходной буфер
	//для нулевого светодиода
	for (int colr = 0; colr<3; colr++){
			for (char i=0; i<8; i++){
				ws2812_hsv_to_rgb(0);  //для первого светодиода (в первую половину буфера)
				if (((color [colr]>>(7-i)))&0x01)  {buff [0+ i*3 + 1 + colr*24] |= 1<<1;}  else {buff[0 + i*3 + 1 + colr*24] &= ~(1<<1);}
				ws2812_hsv_to_rgb(1);  //для второго светодоида (во вторую половину буфера)
				if (((color [colr]>>(7-i)))&0x01)  {buff[72+ i*3 + 1 + colr*24] |= 1<<1;} else {buff[72+ i*3 + 1 + colr*24] &= ~(1<<1);}

			}
		}
}



//подпрограмма обработки прерывания от ДМА (при отправке половины и всего буфера)
void DmaHalfTransferCallback(DMA_HandleTypeDef * hdma){
	num_leds++;
	//если передали все светодиоды, то останавливаем ДМА
	if (num_leds>(MAX_LEDS-1)){
		num_leds = 0;
		first_entry_kostyl = 0;

		__HAL_TIM_DISABLE(&htim1);
		__HAL_TIM_DISABLE_DMA(&htim1, TIM_DMA_UPDATE);
		HAL_DMA_Abort_IT(&hdma_tim1_up);
		DMA_busy = 0;
	} else {
		//*************************************************************************************
		//* Костыль, на предпоследнем светодиоде.
		//*	в котором обнуляем начало выходного буфера. Если его не применить, то в конце посылки поступает еще несколько бит с начала буфера,
		//* из-за достаточно быстрой работой ДМА, а также режимиа Circular, его отключение происходит немного позже. Получается что ДМА еще фактически работает,
		//* но в ленту ws2812 летят уже нули.
		if ((num_leds==(MAX_LEDS-1))&&(!first_entry_kostyl)) {
				kostyl();
				first_entry_kostyl = 1;
		} else {
			//будем готовить переданную половину буфера следующим светодиодом
				from_image_to_buff(num_leds+1, num_leds&0x01);
			}
		}
}

void ws2812_refresh(){
	prepare_buf();
	prepare_for_start();
	//пропишем функции в которы попадем при прерывании по передаче половины и целого буфера ДМА, в которы будем возводить прорграммный флаг ДМА
	// это сделано для того, чтобы не писать код в файле _it.c.
	// наличие ссылок на эти функции и их запуск происходит в HAL-овском обработчике прерывания в функции HAL_DMA_IRQHandler();
	HAL_DMA_RegisterCallback(htim1.hdma[TIM_DMA_ID_UPDATE], HAL_DMA_XFER_HALFCPLT_CB_ID, DmaHalfTransferCallback);
	HAL_DMA_RegisterCallback(htim1.hdma[TIM_DMA_ID_UPDATE], HAL_DMA_XFER_CPLT_CB_ID, DmaHalfTransferCallback);
	//стартуем ДМА и таймер
	htim1.Instance->CNT = 0;
	HAL_DMA_Start_IT(htim1.hdma[TIM_DMA_ID_UPDATE], (uint32_t)buff, (uint32_t)&GPIOA->ODR, 144);
	__HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_UPDATE);
	__HAL_TIM_ENABLE(&htim1);
	DMA_busy = 1;
}

void ws2812_hsv_to_rgb(int j){
	volatile uint32_t  base_V;

	if (buffer_ws2812[j].S == 0)
		{
			color[0] = buffer_ws2812[j].V;
			color[1] = buffer_ws2812[j].V;
			color[2] = buffer_ws2812[j].V;
		} 
		else
		{
			base_V = ((255 - buffer_ws2812[j].S) * buffer_ws2812[j].V) >> 8;
			switch (buffer_ws2812[j].H / 60)
			{
				case 0:
					color[0] = buffer_ws2812[j].V;
					color[1] = (((buffer_ws2812[j].V - base_V) * buffer_ws2812[j].H) / 60) + base_V;
					color[2] = base_V;
					break;
				case 1:
					color[0] = (((buffer_ws2812[j].V - base_V) * (60 - (buffer_ws2812[j].H % 60))) / 60) + base_V;
					color[1] = buffer_ws2812[j].V;
					color[2] = base_V;
					break;
				case 2:
					color[0] = base_V;
					color[1] = buffer_ws2812[j].V;
					color[2] = (((buffer_ws2812[j].V - base_V) * (buffer_ws2812[j].H % 60)) / 60) + base_V;
					break;
				case 3:
					color[0] = base_V;
					color[1] = (((buffer_ws2812[j].V - base_V) * (60 - (buffer_ws2812[j].H % 60))) / 60) + base_V;
					color[2] = buffer_ws2812[j].V;
					break;
				case 4:
					color[0] = (((buffer_ws2812[j].V - base_V) * (buffer_ws2812[j].H % 60)) / 60) + base_V;
					color[1] = base_V;
					color[2] = buffer_ws2812[j].V;
					break;
				case 5:
					color[0] = buffer_ws2812[j].V;
					color[1] = base_V;
					color[2] = (((buffer_ws2812[j].V - base_V) * (60 - (buffer_ws2812[j].H % 60))) / 60) + base_V;
					break;
			}
		}
}

