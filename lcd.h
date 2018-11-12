#ifndef _MY_LCD_
#define _MY_LCD_
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "pinmacro.h"

#ifndef LCD_DATA
	#define LCD_DATA	D,4
#endif
#ifndef LCD_RS
	#define LCD_RS		B,0
#endif
#ifndef LCD_E
	#define LCD_E		D,3
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
 DDRx(LCD_DATA) |= (0x0F<<BITx(LCD_DATA));
 DDRx(LCD_RS) |= (1<<BITx(LCD_RS));
 DDRx(LCD_E) |= (1<<BITx(LCD_E));
 _delay_ms(10);
}
#else
 #define lcd_enable() {}
#endif
#ifndef LCD_DISABLE_OFF
void lcd_disable(){
 DDRx(LCD_DATA) &= ~(0x0F<<BITx(LCD_DATA));
 DDRx(LCD_RS) &=~(1<<BITx(LCD_RS));
 DDRx(LCD_E) &=~(1<<BITx(LCD_E));
}
#else
 #define lcd_disable() {}
#endif
//добавил невстраиваемую задержку 100 мкс, она используется чаще всего
NOINLINE void delay100us(){_delay_us(100);}
// Выбор задержек жутко шаманский, работает хз как
void lcd_send(unsigned char data){
  _delay_us(600);
 LCD_E_1;
 //_delay_us(100);
 delay100us();		
 PORTx(LCD_DATA) = (PORTx(LCD_DATA) & ~(0x0F<<BITx(LCD_DATA)))|((data>>4)<<BITx(LCD_DATA));//high half of byte
 LCD_E_0;
 //_delay_us(100);
 delay100us();
 LCD_E_1;
 //_delay_us(100);
 delay100us();
 PORTx(LCD_DATA) = (PORTx(LCD_DATA) & ~(0x0F<<BITx(LCD_DATA)))|((data & 0x0F)<<BITx(LCD_DATA));//low half of byte
 LCD_E_0;
 //_delay_us(100);
 delay100us();
}

void lcd_cmd(char cmd){LCD_RS_0; lcd_send(cmd);}
void lcd_data(char data){LCD_RS_1; lcd_send(data);}

#define lcd_goto(addr) lcd_cmd(0x80 | (char)(addr))
#define lcd_cur_small() lcd_cmd(0b00001110)
#define lcd_cur_large() lcd_cmd(0b00001101)
#define lcd_cur_two()   lcd_cmd(0b00001111)
#define lcd_cur_hide()  lcd_cmd(0b00001100)
#define lcd_img_off()   lcd_cmd(0b00001000)

NOINLINE void lcd_str(const char *ch){
 LCD_RS_1;
 while(*ch!=0)lcd_send(*ch++);
}
NOINLINE void lcd_str_P(const char *ch){
 char temp=pgm_read_byte(ch++);
 LCD_RS_1;
 do{lcd_send(temp); temp=pgm_read_byte(ch++);}while(temp!=0);
}

void lcd_init(void)
{
 DDRx(LCD_DATA) |= (0x0F<<BITx(LCD_DATA));
 DDRx(LCD_RS) |= (1<<BITx(LCD_RS));
 DDRx(LCD_E) |= (1<<BITx(LCD_E));
 _delay_ms(1);
 LCD_RS_0;
/* send_lcd(0b00110011);
 send_lcd(0b00110010);
 0011 - ждем 5 мс
 0011 - ждем 100 мкс
 0011 - ждем 2 мс*/
 LCD_E_1;
// _delay_us(100);
 delay100us();
 PORTx(LCD_DATA) = (PORTx(LCD_DATA) & ~(0x0F<<BITx(LCD_DATA)))|((0b00000011)<<BITx(LCD_DATA));
 LCD_E_0;
 _delay_ms(50);
 LCD_E_1;
// _delay_us(100);
 delay100us();
 LCD_E_0;
// _delay_us(100);
 delay100us();
 LCD_E_1;
 _delay_ms(20);
 LCD_E_0;
// _delay_us(100);
 delay100us();
 PORTx(LCD_DATA) = (PORTx(LCD_DATA) & ~(0x0F<<BITx(LCD_DATA)))|((0b00000010)<<BITx(LCD_DATA));				
 LCD_E_1;
// _delay_us(100);
 delay100us();
 LCD_E_0;

 lcd_send(0b00101000);
 lcd_send(0b00000110);

 lcd_send(0b00001110);
 lcd_send(0b00000010);
 lcd_send(0b00000001);
 _delay_ms(100);
}

#endif
