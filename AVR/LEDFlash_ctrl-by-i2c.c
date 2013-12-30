//  �{�R�[�h��
//  I2C�����͒��� �i�l�̈ȉ��R�[�h���Q�l�ɂ��Ă��܂�
//  http://www.eleki-jack.com/mycom2/2007/12/avri2c8twii2c_4.html#more
//  07/11/15	www.wsnak.com T.Nakao

// �^�C�}�[�R�[�h������sim00�l�̈ȉ��̃R�[�h���Q�l�ɂ��Ă��܂�
// http://blog.goo.ne.jp/sim00/e/1ab8d1564106581d1168edc059042c30

/*
recv data: DDDD -PPP
D: �A�h���X: 4bit = 0-15
�A�h���X:
 0-7 = PD0-7
 8-11 = PC0-3
P: �p�^�[��: 3bit = 0-7
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

typedef unsigned char  BYTE;
typedef unsigned short WORD;

// PD4��LED1, PD5��LED2���ڑ�����Ă�����̂Ƃ���

#define	bitLED1 6				// PD4
#define	bitLED2 7				// PD5
#define	LED1 (1<<bitLED1)
#define	LED2 (1<<bitLED2)

#define LED1_ON PORTD|=LED1		// LED1 ON
#define LED1_OFF PORTD&=~LED1	// LED1 OFF
#define LED2_ON PORTD|=LED2		// LED2 ON
#define LED2_OFF PORTD&=~LED2	// LED2 OFF

#define	rgTWISlEna (1<<TWEA)|(1<<TWEN)		// �X���[�u ACK����
#define	rgClrTWInt rgTWISlEna|(1<<TWINT)	// TWINT���荞�ݗv���t���O�̃N���A


BYTE Phase;			// ��M�t�F�[�Y
BYTE MyAdrs;		// My I2C�A�h���X
BYTE CByte;			// ��M�����R���g���[���E�o�C�g
BYTE I2CData;

// �v���g�^�C�v
void I2CSlInit(BYTE adrs);
void I2CSlCom();

void SlaveInit(BYTE dat);
void SlaveReceive(BYTE dat);
BYTE SlaveSend(void);

// �J�E���^
char CounterC[8] = {0, 0, 0, 0, 0, 0, 0, 0};
char CounterD[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// �C���^�[�o�����X�g(3bit = 8��)
/*
 ON�u�ɂȂ�v�܂ł̃C���^�[�o����OFF�ɂȂ�܂ł̃C���^�[�o��
 255�Łu�Ȃ�Ȃ��v
 ��: OFFinterval = 255��OFF�ɂȂ�Ȃ���������ON
 */
unsigned char ONinterval[8]  = {255, 0 , 10, 1, 0, 0, 0, 0};
unsigned char OFFinterval[8] = {0,  255, 10, 1, 0, 0, 0, 0};

    unsigned char ONintervalC[8];
    unsigned char ONintervalD[8];
    unsigned char OFFintervalC[8];
    unsigned char OFFintervalD[8];

	char outC;
	char outD;

void mock_set_interval(void)
{
	ONintervalD[0] = 1;
	ONintervalD[1] = 2;
	ONintervalD[2] = 1;
	ONintervalD[3] = 1;
	ONintervalD[4] = 1;
	ONintervalD[5] = 1;
	ONintervalD[6] = 1;
	ONintervalD[7] = 1;
	OFFintervalD[0] = -1;
	OFFintervalD[1] = 2;
	OFFintervalD[2] = 1;
	OFFintervalD[3] = 1;
	OFFintervalD[4] = 1;
	OFFintervalD[5] = 1;
	OFFintervalD[6] = 1;
	OFFintervalD[7] = 1;

	ONintervalC[0] = 1;
	ONintervalC[1] = 1;
	ONintervalC[2] = 1;
	ONintervalC[3] = 1;
	OFFintervalC[0] = 1;
	OFFintervalC[1] = 10;
	OFFintervalC[2] = 1;
	OFFintervalC[3] = 1;
}

//-----------------------------------------------------------
// ���荞�ݏ����n���h��
ISR(TIMER1_COMPA_vect)
{
	int i;
		for(i = 0; i < 8; i++)
		{
			if((ONintervalC[i] == 0xff) ||
			  (OFFintervalC[i] == 0xff)) continue;
			
			if( (outC >> i) & 0x01)
			{
				// ��1�̏ꍇ
				if(CounterC[i] == 0)
				{
					outC = outC & (0xff ^ (0x01 << i));
					CounterC[i] = ONintervalC[i];
				}
			}
			else
			{
				// ��0�̏ꍇ
				if(CounterC[i] == 0)
				{
					outC = outC | 0x01 << i;
					CounterC[i] = OFFintervalC[i];
				}
			}
			CounterC[i]--;
		}

		for(i = 0; i < 8; i++)
		{
			if( (ONintervalD[i]  == 0xff) ||
   		 	    (OFFintervalD[i] == 0xff) ) continue;

			if( (outD >> i) & 0x01)
			{
				// ��1�̏ꍇ
				if(CounterD[i] == 0)
				{
					outD = outD & (0xff ^ (0x01 << i));
					CounterD[i] = ONintervalD[i];
				}
			}
			else
			{
				// ��0�̏ꍇ
				if(CounterD[i] == 0)
				{
					outD = outD | 0x01 << i;
					CounterD[i] = OFFintervalD[i];
				}
			}
			CounterD[i]--;
		}

		PORTC = outC;
		PORTD = outD;

}

//-----------------------------------------------------------
// �^�C�}�[������
void timer_init(unsigned t)
{
	// 15.11.5 �^�C�}�^�J�E���^1��r���W�X�^A
	OCR1A = t;

	// 15.11.1 �^�C�}�^�J�E���^1���䃌�W�X�^A (�����l��0x00�Ȃ̂ŕK�v�Ȃ�)
	//         ++-------COM1A1:COM1A0 00 OC1A�ؒf
	//         ||++---- COM1B1:COM1B0 00 OC1B�ؒf
	//         ||||  ++ WGM11:WGM10   00 �g�`�������(4bit�̉���2bit)
	TCCR1A = 0b00000000;

	// 15.11.2 �^�C�}�^�J�E���^1���䃌�W�X�^B
	//         +------- ICNC1          0
	//         |+------ ICES1          0
	//         || ++--- WGM13:WGM12    01  �g�`�������(4bit�̏��2bit) CTC top=OCR1A
	//         || ||+++ CS12:CS11:CS10 101 1024����
	TCCR1B = 0b00001101;

	// 15.11.8 �^�C�}�^�J�E���^1���荞�݃}�X�N���W�X�^
	//           +----- ICIE1  0
	//           |  +-- OCIE1B 0
	//           |  |+- OCIE1A 1 �^�C�}�^�J�E���^1��rA���荞�݋���
	//           |  ||+ TOIE1  0
	TIMSK1 = 0b00000010;

	sei(); // ���荞�݋���
}

//-----------------------------------------------------------
// �v���O�����{��
int main(void) {
	uint8_t result;
	eeprom_busy_wait();
	result =  eeprom_read_byte(0x0000);
	//MyAdrs = 0x21;				// ��I2C�X���[�u�E�A�h���X
	MyAdrs = result;				// ��I2C�X���[�u�E�A�h���X
	if(MyAdrs > 0x77 || MyAdrs < 0x03)
	 {
	 MyAdrs = 0x20;				// ��I2C�X���[�u�E�A�h���X
	 }

	timer_init(200); // 100 * 1024us���Ɋ��荞�� (��4.88Hz) @ 1MHz

    mock_set_interval();

	//LED�p�|�[�g���o�͂ɐݒ�
	// PORT PD�͂��ׂďo�̓|�[�g
	// PORT PC��LSB������
    DDRD = 0b11111111;
    DDRC = 0b00001111;
    PORTD = 0b00000000;
    PORTC = 0b00000000;
	
	outC = 0;
	outD = 0;

	I2CSlInit(MyAdrs);			// I2C�X���[�u������

	while(1) {
		I2CSlCom();
	}

}


// -----------------------------------------------------------
// I2C�X���[�u������
// 	IN adrs:My I2C�X���[�u�E�A�h���X
// -----------------------------------------------------------
void I2CSlInit(BYTE adrs) {
	adrs <<= 1;

	// adrs |= 1;		// �W�F�l�����E�R�[���E�A�h���X����
	TWAR = adrs;
	TWCR = rgTWISlEna;
}


// -----------------------------------------------------------
//  I2C�X���[�u����(���C���E���[�v�ɓ����)
// -----------------------------------------------------------
void I2CSlCom() {

	if(!(TWCR & (1<<TWINT))) {
		return;					// TWINT�����Z�b�g�̂Ƃ��I��
	}

	// TWINT���Z�b�g����Ă���Ƃ�
	switch(TWSR) {						// ��ԃR�[�h
		case 0x60:			// CB(Write)��M
		// ---- CB(W)��M ----
		SlaveInit(TWDR);			// CB��M�C�x���g�E�n���h��
		TWCR = rgClrTWInt;			// INT�v���t���O�E�N���A
		break;

		case 0x80: 			// Data��M
		// ---- �X���[�u��M ----
		SlaveReceive(TWDR);			// ��M�C�x���g�E�n���h��
		TWCR = rgClrTWInt;			// INT�v���t���O�E�N���A
		break;

		case 0xA8:			// CB(Read)��M
		// ---- CB(R)��M ----
		SlaveInit(TWDR);			// CB��M�C�x���g�E�n���h��
		// �X���[�u���M��1�o�C�g�擾
		TWDR = SlaveSend();			// �X���[�u���M�C�x���g�E�n���h��
		//   ���M�f�[�^�擾
		TWCR = rgClrTWInt;			// INT�v���t���O�E�N���A
		break;

		case 0xB8:			// Data���M(ACK��M)
		// ---- �X���[�u���M(ACK��M)(�X���[�u���M�p��) ----
		// �X���[�u���M��2�o�C�g�ȍ~�擾
		TWDR =  SlaveSend();		// �X���[�u���M�C�x���g�E�n���h��
		//   ���M�f�[�^�擾
		TWCR = rgClrTWInt;			// INT�v���t���O�E�N���A

		break;

		case 0xC0:			// Data���M(NOACK��M) �ŏI�f�[�^ �X���[�u���M�I��
		case 0xA0:			// �X���[�u��M���̃X�g�b�v�E�R���f�B�V����
		TWCR = rgClrTWInt;			// INT�v���t���O�E�N���A
		break;
	}
}


// -----------------------------------------------------------
// CB��M�C�x���g�E�n���h��
// 	IN dat: ��M�����f�[�^(�R���g���[���E�o�C�g)
// -----------------------------------------------------------
void SlaveInit(BYTE dat) {

}



// -----------------------------------------------------------
// �X���[�u��M�C�x���g�E�n���h��
// 	IN dat: ��M�����f�[�^
// -----------------------------------------------------------
void SlaveReceive(BYTE dat) {
    BYTE chAddr;
	BYTE chPattern;
	chAddr = dat >> 4;
	chPattern = dat & 0x07;
	if(chAddr <= 7)
	{
		//PortD�̃A�h���X
		ONintervalD[chAddr] = ONinterval[chPattern];
		OFFintervalD[chAddr] = OFFinterval[chPattern];
        // ON�ɂȂ�Ȃ��Ȃ�
		if(ONinterval[chPattern] == 255)
		{
			// OFF�ɂ���
			outD = outD & (0xff ^ (0x01 << chAddr));
		}
        // OFF�ɂȂ�Ȃ��Ȃ�
		else if(OFFinterval[chPattern] == 255)
		{
			// ON�ɂ���
			outD = outD | 0x01 << chAddr;
		}
		CounterD[chAddr] = 0;
	}
	
	else if(chAddr > 7)
	{
		chAddr = chAddr - 8;
		//PortD�̃A�h���X
		ONintervalC[chAddr] = ONinterval[chPattern];
		OFFintervalC[chAddr] = OFFinterval[chPattern];
		// ON�ɂȂ�Ȃ��Ȃ�
		if(ONinterval[chPattern] == 255)
		{
			// OFF�ɂ���
			outC = outC & (0xff ^ (0x01 << chAddr));
		}
		// OFF�ɂȂ�Ȃ��Ȃ�
		else if(OFFinterval[chPattern] == 255)
		{
			// ON�ɂ���
			outC = outC | 0x01 << chAddr;
		}
		CounterC[chAddr] = 0;
	}

	I2CData = dat;
    PORTC = outC;
	PORTD = outD;
}


// -----------------------------------------------------------
// �X���[�u���M�C�x���g�E�n���h��
// 	OUT Acc: ���M����f�[�^
// -----------------------------------------------------------
BYTE SlaveSend(void) {
	return I2CData;		// �ߋ��Ƀ}�X�^���瑗���Ă����f�[�^�𑗂�Ԃ�
}

