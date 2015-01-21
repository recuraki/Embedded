/*

I2C��M�v���O����
	ATmega88

I2C
	�M�����C��
		PC4
		PC5

	����M�f�[�^�o��LED
		PD6
		PD7

	���}LED
		PB1	

fuse)�@�f�t�H���g�@�������M�@8MZ�@1/8

�������M�v���O����
	I2Csend_smpl

���ʁj ok

*/


#include 	<avr/io.h>

//��Ԓl
#define		START	0x08		//�X�^�[�g��Ԓl
#define		RECEIVE	0x60		//�������ẴA�h���X��M��Ԓl

#define		SR_DATA_ACK	0x80	//�f�[�^�𐳏푗�M��Ԓl

#define		P_S		0xA0	//��~�܂��͍đ������@��Ԓl



void wait( int );

int main(){

	unsigned char x;
	
	DDRB = 0x03;	//0b00000011;	//LED �s���@�o�͐ݒ�
	DDRD = 0xC0;	//0b1100 0000;	//LED �s���@�o�͐ݒ�


	TWBR = 0xFF;		//����	2KHz
	TWAR = 0x80;		//�A�h���X�ݒ�

	PORTB = 0x02;		//�����l

	wait(1000);


		TWCR =( ( 1<<TWINT )| ( 1<<TWEA ) | ( 1<<TWEN ) );//0xC4�@0b1100 0100	//�ڰ�ގ�M����ݒ�

		while( !( TWCR & (1<<TWINT) ) );	//  �ݒ茋�ʑ҂�

		while( ( TWSR & 0xF8 ) != RECEIVE );	//�������ẴA�h���X��M�҂�

			PORTB = 0x00;	//�A�h���X��M�@���}�@LED����

				TWCR =( ( 1<<TWINT )| ( 1<<TWEA ) | ( 1<<TWEN ) );		//�ڰ�ގ�M����
				while( !( TWCR & (1<<TWINT) ) );	// �ݒ茋�ʑ҂�

				x = TWDR;			//��M�f�[�^��LED�֏o��

			PORTD = x;

				while( ( TWSR & 0xF8 ) != SR_DATA_ACK );	//�������f�[�^��MACK�����@��Ԓl�`�F�b�N

			PORTB = 0x02;		//0b00000010;	�f�[�^��M�I�����}

				//��~or�đ��������o
				TWCR =( ( 1<<TWINT )| ( 1<<TWEA ) | ( 1<<TWEN ) );		//�ڰ�ގ�M����
				while( !( TWCR & (1<<TWINT) ) );	// 0b10000000 �ݒ茋�ʑ҂�
				while( (TWSR & 0xF8) != P_S );		//��~�����@��Ԓl�`�F�b�N

			PORTB = 0x00;	//0b00000000;	�ʐM�I�����}
			
}

void wait( int time ){

	int j;
	unsigned char k;
	
	for ( j=0; j<time ;j++){
		for ( k=0; k<100 ;k++);
	}
}
