/*
 * �H�� OSL641501-A�@�����@���C�t�Q�[��
 * 8x8�h�b�g�}�g���N�XLED
 *
 */

/*
 * ����
 * �A�m�[�h��(PB)
 * High Active
 * PB0(14)--(13) COL1(A)
 * PB1(15)--(3)  COL2(A)
 * PB2(16)--(4)  COL3(A)
 * PB3(17)--(10) COL4(A)
 * PB4(18)--(6)  COL5(A)
 * PB5(19)--(11) COL6(A)
 * PB6(9) --(15) COL7(A)
 * PB7(10 --(16) COL8(A)
 * �J�\�[�h��(PD)
 * �������Ȃ̂�Low Active(Low�ł��̍s������)
 * PD0(2)--(9) ROW1(K)
 * PD1(3)--(14) ROW2(K)
 * PD2(4)--(8) ROW3(K)
 * PD3(5)--(12) ROW4(K)
 * PD4(6)--(1) ROW5(K)
 * PD5(11)--(7) ROW6(K)
 * PD6(12)--(2) ROW7(K)
 * PD7(13)--(5) ROW8(K)
 */
 
#define F_CPU 1000000L


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>
#include <stdlib.h>

// �h�b�g�̐����`
#define Xsize 16
#define Ysize 16

/*
 * ���C�t�Q�[���ɂ����Ắu���ӂW�Z���v�̏󋵂�m��K�v�����邽��
 * �㉺���E�P�Z���������̈���Ƃ�܂��B
 */
char nowMap[Xsize + 2][Ysize + 2];
char nextMap[Xsize + 2][Ysize + 2];

// �_���T�C�N��
#define WAITTIME 100 // ms

void timer_init(unsigned t)
{
	// ��r���W�X�^
	OCR1A = t;
	TCCR1A = 0b00000000;
	TCCR1B = 0b00001101;
	//              ^^^101 1024����
	TIMSK1 = 0b00000010;
}

/*
 * �Z���̏����z�u������܂��B
 * rand�g���Ă��܂����A�����̏����l�͌Œ�Ȃ̂ŁA���񓯂��B
 */
void MapInit(void)
{
	int i,j;
	cli();
	for(i = 1; i <= Xsize; i++){
		for(j = 1; j <= Ysize; j++){
			nowMap[i][j] = ((rand() % 3) == 0) ? 1 : 0;
		}
	}
	sei();
}


char curRow;
/*
 * �^�C�}�[�̊��荞�݃n���h��
 * ���荞�݂����邽�сA�u���̍s�v�̕`����s��
 */
ISR(TIMER1_COMPA_vect)
{

	// �C�����A�^�C�}�[���荞�݂��~�߂�
	cli();

	// out: ���̍s�ɓf���o�C�i���l
	char out = 0x00;
	char out2 = 0x00;
	int i;

	// ��������A���̗�̏o�͂��~�߂�
	/*
	 * Note:
	 * ����̓L���ŁA�u���܂ł̍s�v�̂܂�
	 * PORTD��ύX = �u���̍s�v�Ɉڂ��Ă��܂���
	 * �u���܂܂ł̍s�v�̎c�����u���̍s�Ɏc��v
	 * (���̍s�ɍ��̍s�̏o�͂͂����Ⴄ����)
	 */
	PORTB = 0x00;
	PORTA = 0x00;
	
	// ����ŁA���̍s��I������
	// LowActive�Ȃ̂ŁA�t��1����A���̍s�̃r�b�g��xor�Ƃ�
	if(curRow < 8)
	{
		PORTD = 0xff;
		PORTC = 0xff ^ (0x01 << curRow);
	}
	else
	{
		PORTD = 0xff ^ (0x01 << (curRow - 8));
		PORTC = 0xff;
	}
	// �e��𑀍�
	for(i = 0; i < 8; i++)
	{
		// �����A���̃t�B�[���h��1 = �����Ȃ�
		if(nowMap[i + 1][curRow + 1] == 1)
		{
			// ��̂��̏o�͂�High�ɂ���
			// �_�����Z��OR��out�ɂƂ�
			out |= (0x01 << (i));
		}
	}
	PORTB = out;
	
	for(i = 8; i < Xsize; i++)
	{
		// �����A���̃t�B�[���h��1 = �����Ȃ�
		if(nowMap[i + 1][curRow + 1] == 1)
		{
			// ��̂��̏o�͂�High�ɂ���
			// �_�����Z��OR��out�ɂƂ�
			out2 |= (0x01 << (i - 8));
		}
	}

	// �ŁA�����̌��ʂ��o��
	PORTA = out2;

	curRow ++;
	if(curRow >= Ysize)
	{
		curRow = 0;
	}

	// �^�C�}�[���荞�݂��ĊJ
	sei();
}


// x,y�̎���8�Z���̐����Z�����擾
int CountAround(int x, int y)
{
	int c = 0;
	c += (nowMap[x - 1][y - 1] == 1) ? 1 : 0;
	c += (nowMap[x    ][y - 1] == 1) ? 1 : 0;
	c += (nowMap[x + 1][y - 1] == 1) ? 1 : 0;
	c += (nowMap[x - 1][y    ] == 1) ? 1 : 0;
	// [x][y]�͎���
	c += (nowMap[x + 1][y    ] == 1) ? 1 : 0;
	c += (nowMap[x - 1][y + 1] == 1) ? 1 : 0;
	c += (nowMap[x    ][y + 1] == 1) ? 1 : 0;
	c += (nowMap[x + 1][y + 1] == 1) ? 1 : 0;
	return(c);
}

int reset_counter = 0;
int life_counter = 0;

// ���C�t�Q�[����step
/*
 * �Ղ́u���v�̏���nowMap�������A
 * �u���v�̏���nextMap�ɓ���Ă����܂��B
 *
 * �ʏ�̃��C�t�Q�[���ƈႢ�A���̓��͂��󂯕t���Ȃ��̂ŁA
 * 1. 50 steps���s���ꂽ�Ƃ��ɂ͑�������map�Ɉڂ�
 * 2. �ϓ����Ȃ��Ƃ���3step�҂��Ď���map�Ɉڂ�
 * �Ƃ������������Ă��܂�
 */
 
void step(void)
{
	// �ύX�t���O
	char IsChange = 0;

	int i,j;

	// ���̐������Ԃ��C���N�������g	
	life_counter += 1;
	
	// �e�Z���𑖍�
	for(i = 1; i <= Xsize; i++){
		for(j = 1; j <= Ysize; j++){
			if(nowMap[i][j] == 0)
			{
				// ���ɃZ���̏ꍇ
				if(CountAround(i, j) == 3)
				{
					nextMap[i][j] = 1;
					IsChange = 1;
				}
				else
				{
					nextMap[i][j] = 0;
				}
			}
			else
			{
				// �����Z���̏ꍇ
				if(CountAround(i, j) == 2)
				{
					nextMap[i][j] = 1;
				}
				else if(CountAround(i, j) == 3)
				{
					nextMap[i][j] = 1;
				}
				else
				{
					nextMap[i][j] = 0;
					IsChange = 1;
				}
			}
		}
	}

	// �����ŁA���̃}�b�v���u���̃}�b�v�v�ɃR�s�[���܂�
	for(i = 1; i <= Xsize; i++){
		for(j = 1; j <= Ysize; j++){
			nowMap[i][j] = nextMap[i][j];
		}
	}

	// ���step�����s����Ă���ꍇ�A���̃}�b�v��	
	if(life_counter > 70)
	{
		life_counter = 0;
		MapInit();
	}
	
	// �ύX���Ȃ��ꍇ��reset_counter���C���N�������g��
	// ��莞�ԗ��ƃ}�b�v��������
	if(IsChange == 0)
	{
		reset_counter += 1;
		if(reset_counter > 3)
		{
			reset_counter = 0;
			life_counter = 0;
			MapInit();
		}
	}
}

int main(void)
{
	MCUSR|= (1<<JTD);
	
	DDRA = 0b11111111;
	DDRB = 0b11111111;
	DDRC = 0b11111111;
	DDRD = 0b11111111;

	curRow = 0;
	MapInit();

    // �^�C�}�[������
    // 1024us���Ɋ��荞�� (100��4.88Hz) @ 1MHz
    timer_init(1);
    sei(); // enable timer
		
    while(1)
    {
		_delay_ms(20);
		// ���C�t�Q�[���̏���
		step();
    }
}