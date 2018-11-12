/*
 * Автор:	COKPOWEHEU
 * Дата создания: 23.12.2015
 * Компилятор:	avr-gcc 4.8.1 (наверняка соберется и на других)
 * Размер (min): 106 байт flash ; 1 байт ОЗУ (плюс 1 байт на каждый разряд, то есть не меньше 3 байт)
 * Размер (max): 172 байта flash ; 18 байт ОЗУ (плюс 3 байта на каждый разряд, то есть не меньше 24 байт)
 */
/*
 *   ОПИСАНИЕ
 * Библиотека для динамической семисегментной индикации.
 * Поддерживаются индикаторы с общим катодом, общим анодом и даже с инверторами.
 * Можно включить поддержку выделения одного из разрядов миганием
 * Можно довольно гибко управлять отображением десятичной точки
 * ВСЕ сегменты должны висеть на одном порту (из соображений уменьшения размера кода,
 *  увеличения скорости работы и просто лени)
 * ВСЕ разряды также должны висеть на одном порту, но не на том же, что сегменты
 *  (там и так заняты как минимум 7 из 8 линий), по тем же соображениям.
 * Поддержки внешних сдвиговых регистров, счетчиков и прочих расширителей портов не предусмотрено.
 * Поддерживается, но не рекомендуется, пользовательский набор кодов символов
 */
/*
 *   !! ОСТОРОЖНО !!
 * 1. При НЕ-использовании вынесения констант в flash-память (макрос USE_PROGMEM)
 *      переопределяются модификатор PROGMEM и процедура чтения pgm_read_byte()
 *      теоретически, это может привести к проблемам, но простого способа обойти я не нашел
 * 
 * 2. seg_update() реализована как макрос, а не как процедура. В некоторых случаях опять-таки
 *      возможны проблемы (хотя пока я их представить толком не могу)
 * 
 * 3. Используемые макросы. Всего не предусмотришь, там тоже могут быть баги
 * 
 * 4. Может не хватать комментариев. Вроде старался добавлять где надо, но кому-то может не хватить
 */
#ifndef __7SEG_H__
#define __7SEG_H__

/******************************************************************************************
 * СИСТЕМНЫЕ МАКРОСЫ, ПРОЦЕДУРЫ И ПЕРЕМЕННЫЕ
 * ВСЕ ЧТО НУЖНО ИСПОЛЬЗОВАТЬ В ОСНОВНОЙ ПРОГРАММЕ
 ******************************************************************************************
 */
#if (1==0) //да, здесь заведомо ложное условие, но так видна подсветка
  //макрос управления десятичной точкой. Должен быть определен перед подключением этого файла.
  //Если не определен - точка всегда выключена.
  //В нем можно использовать свои переменные или специфичные для реализации динамической индикации
    //индицируемый сегмент
    static unsigned char seg_num=0;	
    //счетчик для мигания сегмента (определен только если включен макросом
    //SEG_BLINK), именно его бит номер SEG_BL_BIT проверяется для мигания. Считает от 0 до 255, не сбрасываясь
    static unsigned char seg_bcount=0;
  //пример использования
  // - зажечь точку синхронно с нажатием кнопки на PC0
    #define SEG_DOT_SRC()	(! (PINC & (1<<0)) )
    #include "7seg.h"
  // - зажечь точку после 0-го разряда
    #define SEG_DOT_SRC()	(seg_num == 0)
    #include "7seg.h"
  
  //экранный буфер. Определен в этом файле, поэтому переопределять нельзя
  unsigned char seg_buffer[];
  
  //номер мигающего разряда. Если мигание отключено (см. SEG_BLINK), переменная не объявлена
  //если мигание не нужно, установите ее за пределы буфера, например, присвоив 0xFF
  char seg_blink=0xFF;
  
  //инициализация направления портов
  inline void seg_init();
  
  //отключение индикатора. Например, для ухода в режим сна
  inline void seg_disable();
  
  //динамическая индикация. Частота ее вызова влияет на частоту мигания сегмента (см. SEG_BLINK и SEG_BL_BIT)
  seg_update();
#endif
/*****************************************************************************************
 * НАСТРОЙКИ
 *****************************************************************************************
 */
/*
// СЕГМЕНТЫ Они должны висеть на одном порту
#define SEG_PORT	D
#define SEG_A	0
#define SEG_B	1
#define SEG_C	2
#define SEG_D	3
#define SEG_E	4
#define SEG_F	5
#define SEG_G	6
#define SEG_DOT	7

// РАЗРЯДЫ Они тоже должны висеть на одном порту
//можно менять их количество, но только в пределах
// от 2 (потому что для 1 разряда динамическая индикация не нужна)
// до 8 (потому что все линии висят на одном порту, а в нем 8 линий)
#define DIG_PORT	B
#define DIG_0	0
#define DIG_1	1
#define DIG_2	2
#define DIG_3	3

// ПОЛЯРНОСТЬ сигнала. Какой уровень является "разрешающим" - лог.1 или лог.0
#define SEG_POL	1
#define DIG_POL	0

// Использование 16-ричных чисел, запись в seg_buffer[x] числа 0x0D приведет к выводу 'd' на дисплей
//#define SEG_HEX

// использование flash-памяти для хранения констант. Это чуть медленнее
//зато намного компактнее
#define USE_PROGMEM

// включить возможность выделения определенного разряда миганием
#define SEG_BLINK

// частота мигания. Какой бит из 8 использовать для мигания
//таким образом можно поделить тактовую на 2, 4, 8, ..., 256, 512
//тактовая частота - та, с которой вызывается seg_update()
#define SEG_BL_BIT	7

//внешнее объявление кодов символов. Не рекомендуется использовать, но если решитесь
//придется объявить массив const char seg_nums[] с модификатором PROGMEM или без него,
//в зависимости от макроса USE_PROGMEM. Вот тут стоит посмотреть реализацию seg_update()
#define SEG_EXT_NUMBERS	0
*/

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * СИСТЕМНЫЕ МАКРОСЫ И ПРОЦЕДУРЫ
 * НЕ ИЗМЕНЯТЬ ПОКА ЭТО НЕ ОСТАНЕТСЯ ЕДИНСТВЕННЫМ ВЫХОДОМ
 * !! DO NOT EDIT !!
 *!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */

// создание необъявленных макросов со значением false
#ifndef SEG_EXT_NUMBERS
  #define SEG_EXT_NUMBERS	0
#endif
#ifndef SEG_POL
  #define SEG_POL	0
#endif
#ifndef DIG_POL
  #define DIG_POL	0
#endif

//подсчет количества задействованных разрядов
#if defined DIG_7
  #define DIG_NUM 8
#elif defined DIG_6
  #define DIG_NUM 7
#elif defined DIG_5
  #define DIG_NUM 6
#elif defined DIG_4
  #define DIG_NUM 5
#elif defined DIG_3
  #define DIG_NUM 4
#elif defined DIG_2
  #define DIG_NUM 3
#else
  #define DIG_NUM 2
#endif

//заглушаем неиспользованные разряды на DIG_0. Для логических операций это не страшно
#ifndef DIG_2
  #define DIG_2 DIG_0
#endif
#ifndef DIG_3
  #define DIG_3 DIG_0
#endif
#ifndef DIG_4
  #define DIG_4 DIG_0
#endif
#ifndef DIG_5
  #define DIG_5 DIG_0
#endif
#ifndef DIG_6
  #define DIG_6 DIG_0
#endif
#ifndef DIG_7
  #define DIG_7 DIG_0
#endif

#define DIG_MASK (1<<DIG_0 | 1<<DIG_1 | 1<<DIG_2 | 1<<DIG_3 | 1<<DIG_4 | 1<<DIG_5 | 1<<DIG_6 | 1<<DIG_7)

#ifndef __PINMACRO_H__
  #define _DDRx(x)	DDR##x
  #define _PORTx(x)	PORT##x
  #define DDRx(x)	_DDRx(x)
  #define PORTx(x)	_PORTx(x)
#endif

#if (SEG_POL)
  #define SEG_OP(x)	((unsigned char)(x))
  #define SEG_DOT_ON()	do{ PORTx(SEG_PORT) |= (1<<SEG_DOT); }while(0)
  #define SEG_DOT_OFF()	do{ PORTx(SEG_PORT) &=~(1<<SEG_DOT); }while(0)
#else
  #define SEG_OP(x)	((unsigned char)(~(x)))
  #define SEG_DOT_ON()	do{ PORTx(SEG_PORT) &=~(1<<SEG_DOT); }while(0)
  #define SEG_DOT_OFF()	do{ PORTx(SEG_PORT) |= (1<<SEG_DOT); }while(0)
#endif
#if (DIG_POL)
  #define DIG_OP(x)	((unsigned char)(x))
  #define DIG_OFF()	do{ PORTx(DIG_PORT) &=~DIG_MASK; }while(0)
  #define DIG_ON(x)	do{ PORTx(DIG_PORT) |= x; }while(0)
#else
  #define DIG_OP(x)	((unsigned char)(~(x)))
  #define DIG_OFF()	do{ PORTx(DIG_PORT) |= DIG_MASK; }while(0)
  #define DIG_ON(x)	do{ PORTx(DIG_PORT) &= x; }while(0)
#endif

#ifdef USE_PROGMEM
  #include <avr/pgmspace.h>
#else //объявим заглушки
  #define PROGMEM
  #define pgm_read_byte(x) (*(x))
#endif

#ifdef __OPTIMIZE__
  #define SEG_INLINE	inline
#else
  #define SEG_INLINE
#endif

SEG_INLINE void seg_init(){
  DDRx(SEG_PORT) |= (1<<SEG_A | 1<<SEG_B | 1<<SEG_C | 1<<SEG_D | 1<<SEG_E | 1<<SEG_F | 1<<SEG_G | 1<<SEG_DOT);
  DDRx(DIG_PORT) |= DIG_MASK;
}

SEG_INLINE void seg_disable(){
#if (SEG_POL)
  PORTx(SEG_PORT) = 0x00;
#else
  PORTx(SEG_PORT) = 0xFF;
#endif
  DIG_OFF();
}
#if !(SEG_EXT_NUMBERS)
PROGMEM const unsigned char seg_nums[]={
  SEG_OP(1<<SEG_A | 1<<SEG_B | 1<<SEG_C | 1<<SEG_D | 1<<SEG_E | 1<<SEG_F),	//0
  SEG_OP(1<<SEG_B | 1<<SEG_C),							//1
  SEG_OP(1<<SEG_A | 1<<SEG_B | 1<<SEG_D | 1<<SEG_E | 1<<SEG_G),			//2
  SEG_OP(1<<SEG_A | 1<<SEG_B | 1<<SEG_C | 1<<SEG_D | 1<<SEG_G),			//3
  SEG_OP(1<<SEG_B | 1<<SEG_C | 1<<SEG_F | 1<<SEG_G),				//4
  SEG_OP(1<<SEG_A | 1<<SEG_C | 1<<SEG_D | 1<<SEG_F | 1<<SEG_G),			//5
  SEG_OP(1<<SEG_A | 1<<SEG_C | 1<<SEG_D | 1<<SEG_E | 1<<SEG_F | 1<<SEG_G),	//6
  SEG_OP(1<<SEG_A | 1<<SEG_B | 1<<SEG_C),					//7
  SEG_OP(1<<SEG_A | 1<<SEG_B | 1<<SEG_C | 1<<SEG_D | 1<<SEG_E | 1<<SEG_F | 1<<SEG_G),//8
  SEG_OP(1<<SEG_A | 1<<SEG_B | 1<<SEG_C | 1<<SEG_D | 1<<SEG_F | 1<<SEG_G)	//9
  #ifdef SEG_HEX
 ,SEG_OP(1<<SEG_A | 1<<SEG_B | 1<<SEG_C | 1<<SEG_E | 1<<SEG_F | 1<<SEG_G),	//A
  SEG_OP(1<<SEG_C | 1<<SEG_D | 1<<SEG_E | 1<<SEG_F | 1<<SEG_G),			//B
  SEG_OP(1<<SEG_A | 1<<SEG_D | 1<<SEG_E | 1<<SEG_F),				//C
  SEG_OP(1<<SEG_B | 1<<SEG_C | 1<<SEG_D | 1<<SEG_E | 1<<SEG_G),			//D
  SEG_OP(1<<SEG_A | 1<<SEG_D | 1<<SEG_E | 1<<SEG_F | 1<<SEG_G),			//E
  SEG_OP(1<<SEG_A | 1<<SEG_E | 1<<SEG_F | 1<<SEG_G)				//F
  #endif
};
#endif

PROGMEM const char seg_dig[]={
  DIG_OP(1<<DIG_0), DIG_OP(1<<DIG_1)
#if (DIG_NUM > 2)
  ,DIG_OP(1<<DIG_2)
#if (DIG_NUM > 3)
  ,DIG_OP(1<<DIG_3)
#if (DIG_NUM > 4)
  ,DIG_OP(1<<DIG_4)
#if (DIG_NUM > 5)
  ,DIG_OP(1<<DIG_5)
#if (DIG_NUM > 6)
  ,DIG_OP(1<<DIG_6)
#if (DIG_NUM > 7)
  ,DIG_OP(1<<DIG_7)
#endif
#endif
#endif
#endif
#endif
#endif
};

volatile unsigned char seg_buffer[DIG_NUM];

#ifdef SEG_DOT_SRC
  #define SEG_DOT_TEST() if(SEG_DOT_SRC())SEG_DOT_ON()
#else
  #define SEG_DOT_TEST() 
#endif

#ifdef SEG_BLINK
  volatile unsigned char seg_blink=0xFF;
  #define seg_update() \
  do{\
    static unsigned char seg_num=0;\
    static unsigned char seg_bcount=0;\
    if(++seg_num >= DIG_NUM)seg_num=0;\
    if(seg_num == seg_blink){\
      seg_bcount++;\
      if(seg_bcount & (1<<SEG_BL_BIT)){\
        DIG_OFF();\
        break;\
      }\
    }\
    DIG_OFF();\
    PORTx(SEG_PORT) = pgm_read_byte(&seg_nums[seg_buffer[seg_num]]);\
    DIG_ON(pgm_read_byte(&seg_dig[seg_num]));\
    SEG_DOT_TEST();\
  }while(0)
#else
  #define seg_update() \
  do{\
    static unsigned char seg_num=0;\
    if(++seg_num >= DIG_NUM)seg_num=0;\
    DIG_OFF();\
    PORTx(SEG_PORT) = pgm_read_byte(&seg_nums[seg_buffer[seg_num]]);\
    DIG_ON(pgm_read_byte(&seg_dig[seg_num]));\
    SEG_DOT_TEST();\
  }while(0);
#endif
/*
 //предыдущая версия, на которой тестировал. Только компилятор ругается на static в inline-функции,
 //поэтому теперь заменил на макросы
#ifdef SEG_BLINK
volatile unsigned char seg_blink=0xFF;
inline void seg_update(){
  static unsigned char seg_num=0;
  static unsigned char seg_bcount=0;
  if(++seg_num >= DIG_NUM)seg_num=0;
  if(seg_num == seg_blink){
    seg_bcount++;
    if(seg_bcount & (1<<SEG_BL_BIT)){
      DIG_OFF();
      return;
    }
  }
  DIG_OFF();
  PORTx(SEG_PORT) = pgm_read_byte(&seg_nums[seg_buffer[seg_num]]);
  DIG_ON(pgm_read_byte(&seg_dig[seg_num]));
  #ifdef SEG_DOT_SRC
  if(SEG_DOT_SRC())SEG_DOT_ON();
  #endif
}
#else
inline void seg_update(){
  do{
  static unsigned char seg_num=0;
  if(++seg_num >= DIG_NUM)seg_num=0;  
  DIG_OFF();
  PORTx(SEG_PORT) = pgm_read_byte(&seg_nums[seg_buffer[seg_num]]);
  DIG_ON(pgm_read_byte(&seg_dig[seg_num]));
  SEG_DOT_TEST();
  }while(0);
}
#endif
*/

#endif
