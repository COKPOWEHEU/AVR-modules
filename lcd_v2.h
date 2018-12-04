#ifndef _MY_LCD_
#define _MY_LCD_
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "pinmacro.h"

#ifndef LCD_D4
	#define LCD_D4	D,4
#endif
#ifndef LCD_D5
	#define LCD_D5	D,5
#endif
#ifndef LCD_D6
	#define LCD_D6	D,6
#endif
#ifndef LCD_D7
	#define LCD_D7	D,7
#endif
#ifndef LCD_RS
	#define LCD_RS	B,0
#endif
#ifndef LCD_E
	#define LCD_E	D,3
#endif
#ifndef LCD_NUM_STR
	#define LCD_NUM_STR 2
#endif


#define LCD_RS_0	PORTx(LCD_RS) &=~(1<<BITx(LCD_RS))
#define LCD_RS_1	PORTx(LCD_RS) |= (1<<BITx(LCD_RS))
#define LCD_E_0		PORTx(LCD_E)  &=~(1<<BITx(LCD_E))
#define LCD_E_1		PORTx(LCD_E)  |= (1<<BITx(LCD_E))

#if LCD_NUM_STR==4
	#define LCD_STR_1	0x00
	#define LCD_STR_2	0x40
	#define LCD_STR_3	0x10
	#define LCD_STR_4	0x50
#endif
#if LCD_NUM_STR==2
	#define LCD_STR_1	0x00
	#define LCD_STR_2	0x40
#endif

#ifndef LCD_ENABLE_OFF
void lcd_enable(){
  DDR_1( LCD_D4 );
  DDR_1( LCD_D5 );
  DDR_1( LCD_D6 );
  DDR_1( LCD_D7 );
  DDR_1( LCD_RS );
  DDR_1( LCD_E );
  _delay_ms(10);
}
#else
 #define lcd_enable() {}
#endif
#ifndef LCD_DISABLE_OFF
void lcd_disable(){
  DDR_0( LCD_D4 );
  DDR_0( LCD_D5 );
  DDR_0( LCD_D6 );
  DDR_0( LCD_D7 );
  DDR_0( LCD_RS );
  DDR_0( LCD_E );
  _delay_ms(10);
}
#else
 #define lcd_disable() {}
#endif
//добавил невстраиваемую задержку 100 мкс, она используется чаще всего
NOINLINE void delay100us(){_delay_us(100);}
// Выбор задержек жутко шаманский, работает хз как
void lcd_send(unsigned char data){
  _delay_us(600);
  PORT_1( LCD_E );
  delay100us();
  //попытка оптимизации, чтобы каждая линия выставлялась за 3 такта
  PORT_0( LCD_D4 ); if(data  & (1<<4))PORT_1( LCD_D4 );
  PORT_0( LCD_D5 ); if(data  & (1<<5))PORT_1( LCD_D5 );
  PORT_0( LCD_D6 ); if(data  & (1<<6))PORT_1( LCD_D6 );
  PORT_0( LCD_D7 ); if(data  & (1<<7))PORT_1( LCD_D7 );
  
  PORT_0( LCD_E );
  delay100us();
  PORT_1( LCD_E );
  delay100us();
  
  PORT_0( LCD_D4 ); if(data  & (1<<0))PORT_1( LCD_D4 );
  PORT_0( LCD_D5 ); if(data  & (1<<1))PORT_1( LCD_D5 );
  PORT_0( LCD_D6 ); if(data  & (1<<2))PORT_1( LCD_D6 );
  PORT_0( LCD_D7 ); if(data  & (1<<3))PORT_1( LCD_D7 );
  
  PORT_0( LCD_E );
  delay100us();
}

void lcd_cmd(char cmd){PORT_0( LCD_RS ); lcd_send(cmd);}
void lcd_data(char data){PORT_1( LCD_RS ); lcd_send(data);}

#define lcd_goto(addr) lcd_cmd(0x80 | (char)(addr))
#define lcd_cur_small() lcd_cmd(0b00001110)
#define lcd_cur_large() lcd_cmd(0b00001101)
#define lcd_cur_two()   lcd_cmd(0b00001111)
#define lcd_cur_hide()  lcd_cmd(0b00001100)
#define lcd_img_off()   lcd_cmd(0b00001000)

NOINLINE void lcd_str(const char *ch){
  PORT_1( LCD_RS );
  while(*ch!=0)lcd_send(*ch++);
}
NOINLINE void lcd_str_P(const char *ch){
  char temp=pgm_read_byte(ch++);
  PORT_1( LCD_RS );
  do{lcd_send(temp); temp=pgm_read_byte(ch++);}while(temp!=0);
}

void lcd_init(void){
  DDR_1( LCD_D4 );
  DDR_1( LCD_D5 );
  DDR_1( LCD_D6 );
  DDR_1( LCD_D7 );
  DDR_1( LCD_RS );
  DDR_1( LCD_E );
  _delay_ms(1);
  PORT_0( LCD_RS );
  PORT_1( LCD_E );
  delay100us();
   
  //send 0011
  PORT_1( LCD_D4 );
  PORT_1( LCD_D5 );
  PORT_0( LCD_D6 );
  PORT_0( LCD_D7 );
  
  PORT_0( LCD_E );
  _delay_ms(50);
  PORT_1( LCD_E );
  delay100us();
  PORT_0( LCD_E );
  delay100us();
  PORT_1( LCD_E );
  _delay_ms(10);
  PORT_0( LCD_E );
  delay100us();
  
  //send 0010
  PORT_0( LCD_D4 );
  PORT_1( LCD_D5 );
  PORT_0( LCD_D6 );
  PORT_0( LCD_D7 );
  
  PORT_1( LCD_E );
  delay100us();
  PORT_0( LCD_E );

  lcd_send(0b00101000);
  lcd_send(0b00000110);

  lcd_send(0b00001110);
  lcd_send(0b00000010);
  lcd_send(0b00000001);
  _delay_ms(10);
}

#endif
