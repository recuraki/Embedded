#define F_CPU 1000000L

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

unsigned int disp_i = 0;
unsigned char flag_dot = 0;

// 7セグLEDの何を選択しているか
unsigned  char led_counter = 0;

// 点灯サイクル
#define WAITTIME 100 // ms

#define Bus_PORT PORTB
#define Bus_DDR  DDRB
#define Bus_PIN  PINB
#define Bus_BIT  (1 << PORTB0)

#define Bus_WaitH  while((Bus_PIN & Bus_BIT) != Bus_BIT) {}
#define Bus_WaitL  while((Bus_PIN & Bus_BIT) == Bus_BIT) {}
#define Bus_Dout   Bus_DDR  |=  Bus_BIT
#define Bus_Din    Bus_DDR  &= ~Bus_BIT
#define Bus_SetH   Bus_PORT |=  Bus_BIT
#define Bus_SetL   Bus_PORT &= ~Bus_BIT
#define Bus_Status ((Bus_PIN & Bus_BIT) ? 1 : 0)

    char CounterC[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    char CounterD[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	char LEDmap[10] = {
			0b11000000, // 0
			0b11111001, // 1
			0b10100100, // 2
			0b10110000, // 3
			0b10011001, // 4
			0b10010010, // 5
			0b10000010, // 6
			0b11011000, // 7
			0b10000000, // 8
			0b10011000, // 9
		};

unsigned char Get_TH(unsigned char *temp, unsigned char *humi)
{
	unsigned char c[5];
	unsigned char buf;
	int i;
	int j;

	cli();

	// シーケンススタート
	// Out Portにセット後、HからLに落とし18ms以上まってHにする
	Bus_Dout;
	Bus_SetH;
	Bus_SetL;
	sei();
	_delay_ms(30);
	cli();
	Bus_SetH;
	Bus_Din;
	
	// センサポートはH pullしている
	// Lが80ns続きHになったらスタート
	Bus_WaitL;
	Bus_WaitH;
	
	// 8 * 5 char読み込む
	for(i = 0; i < 5; i ++)
	{
	    //disp_i++;
		buf = 0;
		for(j = 0; j < 8; j++)
		{
			Bus_WaitL;
			Bus_WaitH;
			_delay_us(40);
			buf = (buf << 1) | Bus_Status;
		}
		c[i] = buf;
	}
	
	*humi = c[0];
	*temp = c[2];

	sei(); // enable timer
	//_delay_ms(50);

	return(0);
}

void timer_init(unsigned t)
{ 
	// 比較レジスタ
	OCR1A = t;
	TCCR1A = 0b00000000;
	TCCR1B = 0b00001101;
	//              ^^^101 1024分周
	TIMSK1 = 0b00000010;
}
// handler for timer_a
ISR(TIMER1_COMPA_vect)
{
	cli();
	unsigned int disp_tmp = disp_i;
	unsigned int b[4];
	
	b[0] = (disp_tmp % 10);
	disp_tmp = disp_tmp / 10;

	b[1] = (disp_tmp % 10);
	disp_tmp = disp_tmp / 10;

	b[2] = (disp_tmp % 10);
	disp_tmp = disp_tmp / 10;

	b[3] = (disp_tmp % 10);
	disp_tmp = disp_tmp / 10;
	
	led_counter++;
	if (led_counter == 4)
	led_counter = 0;

	switch(led_counter)
	{
		case 0:
		PORTC = 0x01;
		PORTD = LEDmap[b[3]];
		break;
		case 1:
		PORTC = 0x02;
		PORTD = LEDmap[b[2]];
		break;
		case 2:
		PORTC = 0x04;
		PORTD = LEDmap[b[1]];
		break;
		case 3:
		PORTC = 0x08;
		PORTD = LEDmap[b[0]];
		break;
	}	
	sei();
}

int main(void){
	
	unsigned char temp, humi;
	unsigned count = 0;

    // タイマー初期化
	// 1024us毎に割り込み (100で4.88Hz) @ 1MHz
	timer_init(1); 
	sei(); // enable timer

	// PDはすべて7セグに割り当て
    DDRD = 0b11111111;
	// 桁の選択はPC 0-3を使う
    DDRC = 0b00001111;

	PORTC = 0x01;
	PORTD = 0x00;

	disp_i = 8888;
	_delay_ms(2000);


	while(1){
		Get_TH(&temp, &humi);
		disp_i = temp*100 + humi;
		_delay_ms(2000);
		//disp_i++;
		//count++;
		flag_dot = flag_dot ^ 0b10000000;
	}
 
    return 0;
}