// CLK8DIVを無効にすること
#define F_CPU 1000000UL 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#define ChipSelect1 PORTC = (PORTC & 0b11111101) | 0b00000001
#define ChipSelect2 PORTC = (PORTC & 0b11111110) | 0b00000010

#define Mode_Data  PORTC |= (1 << PORTC2)
#define Mode_Cmd PORTC   &= (0xff ^ (1 << PORTC2))

#define Dir_Read  PORTC |= (1 << PORTC3)
#define Dir_Write PORTC &= (0xff ^ (1 << PORTC3))



/*
 * PDx = DBx (x=0to7)
 * PC0: CS1
 * PC1: CS2
 * PC2: Register(H:dat, L:cmd)
 * PC3: RW(H: Read, L, Write)
 * PC4: Enable(H: Enable) 
 */

void E_enable()
{
	PORTC |= (1 << PORTC4);
	_delay_us(1000);
}

void E_disable(){
	PORTC &= (0xff ^ (1 << PORTC4));
	_delay_us(1000);
}

void wait_busy()
{
	// ビジー待機
	DDRD = 0b00000000;
	Dir_Read;
	_delay_us(1000);
	Mode_Cmd;
	_delay_us(1000);
	E_enable();
	_delay_us(1000);
	E_disable();
	_delay_us(1000);
	DDRD = 0b11111111;
	_delay_us(1000);
	Mode_Data;
	_delay_us(1000);
	Dir_Write;
	_delay_us(1000);
}

int main(void)
{
	int i;
	
	DDRD = 0b11111111;
	DDRC = 0b00011111;

	// 初期化
	_delay_ms(100);
	
	Mode_Cmd;
	Dir_Write;
	ChipSelect1;
	_delay_us(1000);
	PORTD = 0b11000000;
	_delay_us(1000);
	PORTD = 0b00111111;
	_delay_us(1000);
	ChipSelect2;
	_delay_us(1000);
	_delay_us(1000);
	PORTD = 0b11000000;
	_delay_us(1000);
	PORTD = 0b00111111;
	
	
	// Y指定
	wait_busy();
	Mode_Cmd;
	_delay_us(1000);
	Dir_Write;
	_delay_us(1000);
	PORTD = 0b01000000 || (0b00000000);
	_delay_us(1000);
	E_enable();
	_delay_us(1000);
	E_disable();
	_delay_us(1000);

	// X指定
	wait_busy();
	_delay_us(1000);
	Mode_Cmd;
	_delay_us(1000);
	Dir_Write;
	_delay_us(1000);
	PORTD = 0b10111000 || (0b00000000);
	_delay_us(1000);
	E_enable();
	_delay_us(1000);
	E_disable();
	_delay_us(1000);
	
	// write
	wait_busy();
	_delay_us(1000);
	Mode_Data;
	_delay_us(1000);
	Dir_Write;
	_delay_us(1000);
	PORTD = 0xaa;
	_delay_us(1000);
	E_enable();
	_delay_us(1000);
	E_disable();
	_delay_us(1000);
	


	while(1)
	{
		
	}

}