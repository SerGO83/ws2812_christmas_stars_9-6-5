#include "effects.h"
#include "stm32f1xx_hal.h"
#include "ws2812b.h"
#include "main.h"



extern HSV_TypeDef	buffer_ws2812 [MAX_LEDS];

typedef struct{
	int offset;
	int lenght;
} Star;

const uint8_t gamma8[] = {
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2,
 2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5,
 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10,
 10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
 25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
 37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
 51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
 69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
 90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
 115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
 144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
 177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
 215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };



char corr_num_row[6] = {0,3,5,1,2,4}; //номера лучей в конструкции звезды для движении эффекта по часовой стрелке
int cnt_i = 0, cnt_j = 0, cnt_k = 0;
int stage = 0, num_kadr = 0;
Star stars[9] = {{0,31},{31,31},{ 62,31}, { 93,31}, { 124,31}, { 155,31}, { 186,31}, { 217,31}, { 248,31}};


/*----------------  UTILS   begin  ---------*/
void clear_img (void){
	for (int i = 0; i < MAX_LEDS; i++){
		buffer_ws2812[i].V = 0;
	}
}
void segment_on_stars (Star segs,uint16_t h,uint8_t s, uint8_t v)
{
				for (int j = 0; j < segs.lenght; j++)
				{
					buffer_ws2812[ segs.offset + j].H = h;
					buffer_ws2812[ segs.offset + j].S = s;
					buffer_ws2812[ segs.offset + j].V = v;
				}
}
/*----------------  UTILS   end  ---------*/



void effect_test(){
	buffer_ws2812[0].H = cnt_i;
	buffer_ws2812[0].S = 255;
	buffer_ws2812[0].V = 5;

	buffer_ws2812[4].H = 0;
	buffer_ws2812[4].S = 255;
	buffer_ws2812[4].V = gamma8[cnt_j];


	buffer_ws2812[1].H = 0;
	buffer_ws2812[1].S = 255;
	buffer_ws2812[1].V = 5;
	buffer_ws2812[2].H = 120;
	buffer_ws2812[2].S = 255;
	buffer_ws2812[2].V = 5;
	buffer_ws2812[3].H = 240;
	buffer_ws2812[3].S = 255;
	buffer_ws2812[3].V = 5;



	ws2812_refresh();
	HAL_Delay(50);
	cnt_i += 5;
	if (cnt_i>359) {cnt_i=0;}
	cnt_j += 1;
	if (cnt_j>255) {cnt_j=0;}

}

void effect001(void){  //вращающиеся лучи
int i,j,num_kadr,cnt_kadr;

	for (num_kadr = 0; num_kadr < NUM_ROW; num_kadr++){ // цикл на изменения кадров.
		for (i = 0; i < NUM_STARS; i++) {  //цикл на кол-во звезд
			for (j = 0; j < LEN_ROW; j++) {  //цикл на кол-во светодиодов в луче.

				cnt_kadr = corr_num_row[(((i+num_kadr-1) % NUM_ROW) >= 0) ? (i+num_kadr-1) % NUM_ROW : LEN_ROW] * LEN_ROW;

				if (cnt_kadr > 4) {cnt_kadr++;} // //коррекция на центральную точку
				buffer_ws2812[i * LEN_STAR + j + cnt_kadr].H = 0;
				buffer_ws2812[i * LEN_STAR + j + cnt_kadr].S = 255;
				buffer_ws2812[i * LEN_STAR + j + cnt_kadr].V = 5;

				cnt_kadr = corr_num_row[(i + num_kadr) % NUM_ROW] * LEN_ROW;
				if (cnt_kadr >4) {cnt_kadr++;} //коррекция на центральную точку
				buffer_ws2812[i * LEN_STAR + j + cnt_kadr].H = 0;
				buffer_ws2812[i * LEN_STAR + j + cnt_kadr].S = 255;
				buffer_ws2812[i * LEN_STAR + j + cnt_kadr].V = 255;
			}
		}
		ws2812_refresh();
		HAL_Delay(200);
	}
}
void effect002(void){  //бегущий огонь среди звезд
int j,num_kadr,cnt_kadr;

	for (num_kadr = 0; num_kadr < NUM_STARS; num_kadr++){ // цикл на изменения кадров. по количеству звезд
			for (j = 0; j < LEN_STAR; j++) {  //цикл на кол-во светодиодов в звезде

				cnt_kadr = (num_kadr-1) * LEN_STAR;
				if (cnt_kadr < 0) {cnt_kadr = (NUM_STARS-1)*LEN_STAR;}
				buffer_ws2812[ j + cnt_kadr].H = 120;
				buffer_ws2812[ j + cnt_kadr].S = 255;
				buffer_ws2812[ j + cnt_kadr].V = 5;

				cnt_kadr = num_kadr * LEN_STAR;
				buffer_ws2812[ j + cnt_kadr].H = 120;
				buffer_ws2812[ j + cnt_kadr].S = 255;
				buffer_ws2812[ j + cnt_kadr].V = 255;
			}
			ws2812_refresh();
			HAL_Delay(200);
	}
}
void test_spinner_spiral_dot(){  //заполнение звезды точкой движущейся по спирали от внешнего края во внутрь
int i,j,m,temp,colr;



    for (i = 0; i < NUM_STARS; i++) {// цикл перебора звезд
      colr = 359 - 30*i;
      j = corr_num_row[cnt_j];  //номер луча
          m = cnt_k;
            if ((j%2)==0) {   //расчет светодиода из-за разного направления лучей(ленты) относительно центра. чётные лучи направлены к центру, а нечетные от центра
                  temp = m + i*LEN_STAR + j*LEN_ROW;
              if (j>0) {temp++;}              //коррекция на центральную точку
              } else{
                  temp = 4 - m + i*LEN_STAR + j*LEN_ROW;
                  if (j>0) {temp++;}               //коррекция на центральную точку
                  }
                  if ((temp>=0)&&(temp<MAX_LEDS)){  //проверка выхода за границы ленты
                  buffer_ws2812[ temp].H = colr;
                  buffer_ws2812[ temp].S = 255;
                  buffer_ws2812[ temp].V = 255;}
                  temp = 0;
    }
	ws2812_refresh();
    HAL_Delay(120);

    cnt_j++;

    if (cnt_j>5) {
        cnt_j=0;
        cnt_k++;
        if (cnt_k>4) {
            cnt_k=0;
					HAL_Delay(500);//конец эффекта. Пауза перед выключением
            for (i = 0; i < MAX_LEDS; i++){  //
              buffer_ws2812[i].V = 0;    //яркость ставим в ноль
            }
          }
    }
     num_kadr++;
 if (num_kadr>LEN_ROW) {num_kadr = 0;}
}
void effect003(){   //попиксельное заполнение звёзд слева направо
int i=0,j=0,m=0,temp,colr;

int temp_row;

/*  for (i = 0; i < MAX_LEDS; i++){  //
    buffer_ws2812[i].S = 255;  //насыщенность ставим на максимум
    buffer_ws2812[i].H = 240;  //выберем нужный цвет
    buffer_ws2812[i].V = 00;    //яркость ставим в ноль
  }*/
  //заполняющий огонь от краёв к центру
//  for (num_kadr = 0; num_kadr < LEN_ROW+1; num_kadr++){ // цикл на изменения кадров. по количеству светодиодов в луче
    //for (i = 0; i < NUM_STARS; i++) {// цикл перебора звезд
      i = cnt_i;
      colr =  36*i;
      switch (stage){  // этапы выполнения эффектов
        case 0:  //нулевой - заполнение левых лучей от края к центру
          for (temp_row = 1; temp_row < 3; temp_row++) { // цикл перебора лучей
          j = corr_num_row[temp_row];
              for (m = 0; m < num_kadr; m++) {  //цикл для зажигания луча
              //m = 0;
                if ((j%2)==0) {   //расчет светодиода из-за разного направления лучей(ленты) относительно центра. чётные лучи направлены к центру, а нечетные от центра
                      temp = m + i*LEN_STAR + j*LEN_ROW;
                  if (j>0) {temp++;}              //коррекция на центральную точку
                  } else{
                      temp = 4 - m + i*LEN_STAR + j*LEN_ROW;
                      if (j>0) {temp++;}               //коррекция на центральную точку
                      }
                      if ((temp>=0)&&(temp<MAX_LEDS)){  //проверка выхода за границы ленты
                      buffer_ws2812[ temp].H = colr;
                      buffer_ws2812[ temp].S = 255;
                      buffer_ws2812[ temp].V = 180;}
                      temp = 0;
              } //цикл для зажигания луча
            } // цикл перебора лучей
          break;

         case 1:  //первый этап - включение вертикальных лучей
            for (j = 0; j < 2; j++) { // цикл перебора лучей
//              j = corr_num_row[temp_row];
                for (m = 0; m < 5; m++) {  //цикл для зажигания луча
                //m = 0;
                  if ((j%2)==0) {   //расчет светодиода из-за разного направления лучей(ленты) относительно центра. чётные лучи направлены к центру, а нечетные от центра
                        temp = m + i*LEN_STAR + j*LEN_ROW;
                    if (j>0) {temp++;}              //коррекция на центральную точку
                    } else{
                        temp = 4 - m + i*LEN_STAR + j*LEN_ROW;
                        if (j>0) {temp++;}               //коррекция на центральную точку
                        }
                        if ((temp>=0)&&(temp<MAX_LEDS)){  //проверка выхода за границы ленты
                        buffer_ws2812[ temp].H = colr;
                        buffer_ws2812[ temp].S = 255;
                        buffer_ws2812[ temp].V = 180;}
                        temp = 0;
                } //цикл для зажигания луча
            } // цикл перебора лучей
            num_kadr += 4;
            break;

          case 2:  // второй этап - заполнение правых лучей от центра к краю
          for (temp_row = 4; temp_row < 6; temp_row++) { // цикл перебора лучей
          j = corr_num_row[temp_row];
              for (m = 0; m < num_kadr%6; m++) {  //цикл для зажигания луча
              //m = 0;
                if ((j%2)!=0) {   //расчет светодиода из-за разного направления лучей(ленты) относительно центра. чётные лучи направлены к центру, а нечетные от центра
                      temp = m + i*LEN_STAR + j*LEN_ROW;
                  if (j>0) {temp++;}              //коррекция на центральную точку
                  } else{
                      temp = 4 - m + i*LEN_STAR + j*LEN_ROW;
                      if (j>0) {temp++;}               //коррекция на центральную точку
                      }
                      if ((temp>=0)&&(temp<MAX_LEDS)){  //проверка выхода за границы ленты
                      buffer_ws2812[ temp].H = colr;
                      buffer_ws2812[ temp].S = 255;
                      buffer_ws2812[ temp].V = 180;}
                      temp = 0;
              } //цикл для зажигания луча
            } // цикл перебора лучей

            break;

        default:
          break;
      }
    //} // цикл перебора звезд
  	ws2812_refresh();
    HAL_Delay(50);
//  }
     num_kadr++;
 if (num_kadr>LEN_ROW) {
     num_kadr = 0;
     stage += 1; //
     if (stage>2) {
         stage = 0;
         cnt_i += 1;
         if (cnt_i>NUM_STARS) {
             cnt_i = 0;
             clear_img();
           }
       }
   }

}  //void effect003(){   //заполнение звезды слева направо
void effect004(){  //(вращающиеся 2 луча по часовой стрелке)
int i,j,cnt_kadr;
    for (i = 0; i < NUM_STARS; i++) {  //цикл на кол-во звезд
      for (j = 0; j < LEN_ROW; j++) {  //цикл на кол-во светодиодов в луче.

        cnt_kadr = corr_num_row[((num_kadr-1)>=0) ? num_kadr-1 : LEN_ROW] * LEN_ROW;
        if (cnt_kadr > 4) {cnt_kadr++;} // //коррекция на центральную точку
        buffer_ws2812[i * LEN_STAR + j + cnt_kadr].H = 180;
        buffer_ws2812[i * LEN_STAR + j + cnt_kadr].S = 255;
        buffer_ws2812[i * LEN_STAR + j + cnt_kadr].V = 3;

        cnt_kadr = corr_num_row[(((num_kadr+3)%NUM_ROW-1)>=0) ? (num_kadr+3)%NUM_ROW-1 : LEN_ROW] * LEN_ROW;
        if (cnt_kadr > 4) {cnt_kadr++;} // //коррекция на центральную точку
        buffer_ws2812[i * LEN_STAR + j + cnt_kadr].H = 180;
        buffer_ws2812[i * LEN_STAR + j + cnt_kadr].S = 255;
        buffer_ws2812[i * LEN_STAR + j + cnt_kadr].V = 3;

        cnt_kadr = corr_num_row[num_kadr] * 5;
        if (cnt_kadr >4) {cnt_kadr++;} //коррекция на центральную точку
        buffer_ws2812[i * LEN_STAR + j + cnt_kadr].H = 180;
        buffer_ws2812[i * LEN_STAR + j + cnt_kadr].S = 255;
        buffer_ws2812[i * LEN_STAR + j + cnt_kadr].V = 255;

        cnt_kadr = corr_num_row[(num_kadr+3)%6] * 5;
        if (cnt_kadr >4) {cnt_kadr++;} //коррекция на центральную точку
        buffer_ws2812[i * LEN_STAR + j + cnt_kadr].H = 180;
        buffer_ws2812[i * LEN_STAR + j + cnt_kadr].S = 255;
        buffer_ws2812[i * LEN_STAR + j + cnt_kadr].V = 255;
      }
    }
	ws2812_refresh();
    HAL_Delay(150);
    num_kadr++;
    if (num_kadr>LEN_ROW) {num_kadr = 0;}
}
void effect005(void){  //стробоскоп зеленый зеленый
#define PAUSE 25
#define CNT 8
#define COLOR1 0
#define COLOR2 120
#define VAL1 180
#define VAL2 30
#define VAL3 5


	for (char cnt = 0; cnt < CNT; cnt++){ //помигаем красным
		for (int i = 0; i < NUM_STARS;) {  //цикл на кол-во звезд
			for (int j = 0; j < LEN_STAR; j++) {  //цикл на кол-во светодиодов в звезде. Длина звезды
				buffer_ws2812[i * LEN_STAR + j ].H = COLOR1;
				buffer_ws2812[i * LEN_STAR + j ].S = 255;
				buffer_ws2812[i * LEN_STAR + j ].V = VAL1;
				}
			i++;
			i++;
		}
		ws2812_refresh();
		HAL_Delay(PAUSE);
		for (int i = 0; i < NUM_STARS;) {  //цикл на кол-во звезд
			for (int j = 0; j < LEN_STAR; j++) {  //цикл на кол-во светодиодов в звезде. Длина звезды
				buffer_ws2812[i * LEN_STAR + j ].H = COLOR1;
				buffer_ws2812[i * LEN_STAR + j ].S = 255;
				buffer_ws2812[i * LEN_STAR + j ].V = VAL2;
				}
			i++;
			i++;
		}
		ws2812_refresh();
		HAL_Delay(PAUSE);
	}
	//поставим красный на минимум яркости
	for (int i = 0; i < NUM_STARS;) {  //цикл на кол-во звезд
		for (int j = 0; j < LEN_STAR; j++) {  //цикл на кол-во светодиодов в звезде. Длина звезды
			buffer_ws2812[i * LEN_STAR + j ].H = COLOR1;
			buffer_ws2812[i * LEN_STAR + j ].S = 255;
			buffer_ws2812[i * LEN_STAR + j ].V = VAL3;
			}
		i++;
		i++;
		}

	for (char cnt = 0; cnt < CNT; cnt++){ //помигаем зеленым
		for (int i = 1; i < NUM_STARS;) {  //цикл на кол-во звезд
			for (int j = 0; j < LEN_STAR; j++) {  //цикл на кол-во светодиодов в звезде. Длина звезды
				buffer_ws2812[i * LEN_STAR + j ].H = COLOR2;
				buffer_ws2812[i * LEN_STAR + j ].S = 255;
				buffer_ws2812[i * LEN_STAR + j ].V = VAL1;
				}
			i++;
			i++;
		}
		ws2812_refresh();
		HAL_Delay(PAUSE);
		for (int i = 1; i < NUM_STARS;) {  //цикл на кол-во звезд
			for (int j = 0; j < LEN_STAR; j++) {  //цикл на кол-во светодиодов в звезде. Длина звезды
				buffer_ws2812[i * LEN_STAR + j ].H = COLOR2;
				buffer_ws2812[i * LEN_STAR + j ].S = 255;
				buffer_ws2812[i * LEN_STAR + j ].V = VAL2;
				}
			i++;
			i++;
		}
		ws2812_refresh();
		HAL_Delay(PAUSE);
	}
	//поставим зеленый на минимум
	for (int i = 1; i < NUM_STARS;) {  //цикл на кол-во звезд
			for (int j = 0; j < LEN_STAR; j++) {  //цикл на кол-во светодиодов в звезде. Длина звезды
				buffer_ws2812[i * LEN_STAR + j ].H = COLOR2;
				buffer_ws2812[i * LEN_STAR + j ].S = 255;
				buffer_ws2812[i * LEN_STAR + j ].V = VAL3;
				}
			i++;
			i++;
		}
}
void effect006(void){  //заполняющий огонь по всем лучам от краев к центру, затем погасание от краев к центру
int i,j,m,num_kadr,temp,color;


	for (i = 0; i < MAX_LEDS; i++){  //
		buffer_ws2812[i].S = 255;	//насыщенность ставим на максимум
		buffer_ws2812[i].H = 240;	//выберем нужный цвет
		buffer_ws2812[i].V = 00;		//яркость ставим в ноль
	}
	//заполняющий огонь от краёв к центру
	for (num_kadr = 0; num_kadr < LEN_ROW+1; num_kadr++){ // цикл на изменения кадров. по количеству светодиодов в луче
		for (i = 0; i < NUM_STARS; i++) {// цикл перебора звезд
			color =  36*i;
			for (j = 0; j <NUM_ROW; j++) { // цикл перебора лучей
					for (m = 0; m < num_kadr; m++) {  //цикл для зажигания луча
						if ((j%2)==0) {   //расчет светодиода из-за разного направления лучей(ленты) относительно центра. чётные лучи направлены к центру, а нечетные от центра
									temp = m + i*LEN_STAR + j*LEN_ROW;
							if (j>0) {temp++;}							//коррекция на центральную точку
							} else{
									temp = 4 - m + i*LEN_STAR + j*LEN_ROW;
									if (j>0) {temp++;} 							//коррекция на центральную точку
									}
									if ((temp>=0)&&(temp<MAX_LEDS)){  //проверка выхода за границы ленты
									buffer_ws2812[ temp].H = color;
									buffer_ws2812[ temp].S = 255;
									buffer_ws2812[ temp].V = 180;}
									temp = 0;
								}
							}
						}
				ws2812_refresh();
				HAL_Delay(120);
	}
	// включение центральной точки
	for (i = 0; i < NUM_STARS; i++){

			buffer_ws2812[ i * LEN_STAR + 5].H = 36*i;
			buffer_ws2812[ i * LEN_STAR + 5].S = 255;
			buffer_ws2812[ i * LEN_STAR + 5].V = 180;
	}
	ws2812_refresh();
	HAL_Delay(120);

	for (num_kadr = 0; num_kadr < LEN_ROW+1; num_kadr++){ // цикл на изменения кадров. по количеству светодиодов в луче
		for (i = 0; i < NUM_STARS; i++) {// цикл перебора звезд
			for (j = 0; j <NUM_ROW; j++) { // цикл перебора лучей
					for (m = 0; m < num_kadr; m++) {  //цикл для зажигания луча
						if ((j%2)==0) {   //расчет светодиода из-за разного направления лучей(ленты) относительно центра. чётные лучи направлены к центру, а нечетные от центра
									temp = m + i*LEN_STAR + j*LEN_ROW;
									if (j>0) {temp++;}
							} else{
									temp = 4 - m + i*LEN_STAR + j*LEN_ROW;
									if (j>0) {temp++;}
									}
									if ((temp>=0)&&(temp<MAX_LEDS)){
									buffer_ws2812[ temp].H = 180;
									buffer_ws2812[ temp].S = 255;
									buffer_ws2812[ temp].V = 0;}
									temp = 0;
								}
							}
						}
				ws2812_refresh();
				HAL_Delay(120);
	}
	// выключение центральной точки
	for (i = 0; i < NUM_STARS; i++){
			buffer_ws2812[ i * LEN_STAR + 5].H = 180;
			buffer_ws2812[ i * LEN_STAR + 5].S = 255;
			buffer_ws2812[ i * LEN_STAR + 5].V = 0;
	}
	ws2812_refresh();
	HAL_Delay(120);
}


void effect007(void){  //бегущий огонь среди звезд
int j,num_kadr,cnt_kadr;

	for (num_kadr = 0; num_kadr < NUM_STARS; num_kadr++){ // цикл на изменения кадров. по количеству звезд
			for (j = 0; j < LEN_STAR; j++) {  //цикл на кол-во светодиодов в звезде

				cnt_kadr = (num_kadr-1) * LEN_STAR;
				if (cnt_kadr < 0) {cnt_kadr = (NUM_STARS-1)*LEN_STAR;}
				buffer_ws2812[j + cnt_kadr].H = 220;
				buffer_ws2812[j + cnt_kadr].S = 255;
				buffer_ws2812[j + cnt_kadr].V = 1;

				cnt_kadr = num_kadr * LEN_STAR;
				buffer_ws2812[j + cnt_kadr].H = 180;
				buffer_ws2812[j + cnt_kadr].S = 255;
				buffer_ws2812[j + cnt_kadr].V = 255;
			}
			ws2812_refresh();
			HAL_Delay(150);
	}
}


void effect008(void){  //бегущий огонь среди звезд со шлейфом
int j,m,num_kadr,cnt_kadr;
int lenShlf = 6;

	clear_img();

	for (num_kadr = 0; num_kadr < NUM_STARS + lenShlf - 1; num_kadr++){ // цикл на изменения кадров. по количеству звезд
		for (j = 0 ; j < lenShlf; j++)
		{
			if (((num_kadr-j)>=0)&&((num_kadr-j)<NUM_STARS)) {
				for (m = 0; m < LEN_STAR; m++) {  //цикл для зажигания одной цвезды одним цветом
					cnt_kadr = num_kadr * LEN_STAR;
					int temp = m + cnt_kadr - j*LEN_STAR;
					if ((temp>=0)&&(temp<MAX_LEDS)) {
						buffer_ws2812[ temp].H = 180;
						buffer_ws2812[ temp].S = 255;
						buffer_ws2812[ temp].V = (250-(j+1)*49)<0?3:(250-(j+1)*49);
					}
				}
			}
		}

				for (m = 0; m < LEN_STAR; m++) {  //цикл для зажигания одной цвезды одним цветом
					cnt_kadr = num_kadr * LEN_STAR;
					int temp = m + cnt_kadr - lenShlf*LEN_STAR;
					if ((temp>=0)&&(temp<MAX_LEDS)) {
						buffer_ws2812[ temp].H = 240;
						buffer_ws2812[ temp].S = 255;
						buffer_ws2812[ temp].V = 2;
					}
				}
				ws2812_refresh();
				HAL_Delay(110);
	}
}
void effect009(void){  //эффект дыхание
int i,j;
int yark    [9]={0,112,30,248,170,50,145,112,30};
int yark_dev[9]={0};

	for (j = 0; j < 500; j++){
			for (i = 0; i < 9; i++){
				segment_on_stars (stars [i] ,120,255,gamma8[yark[i]]);
			}
			ws2812_refresh();
			HAL_Delay(1);
			for (i = 0; i < 9; i++)
				{
				if (yark_dev[i]) {
					yark[i] -= 5;
					if (yark[i]<5) {yark[i]=5; if (yark_dev[i]) {yark_dev[i]=0;}else {yark_dev[i]=1;}}
					} else {
					yark[i] += 5;
					if (yark[i]>250) {yark[i]=250; if (yark_dev[i]) {yark_dev[i]=0;}else {yark_dev[i]=1;}}
				}
			}
	}
}
