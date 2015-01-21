/*
I2C�̃f�[�^�V�[�g�̎菇�ʂ�ɐݒ肵���M

I2C���M�v���O����

I2C
	�M�����C��
		PC4
		PC5

	����M�f�[�^�o��LED
		PD6
		PD7

	���}LED
		PB1	


�����̎�M�v���O����
I2Crecv_smpl


fuse)�@�f�t�H���g�@�������M�@8MZ�@1/8


���ʁj ok

*/


#include 	<avr/io.h>

//��Ԓl
#define		START	0x08		//�X�^�[�g��Ԓl
#define		RECEIVE	0x60		//�������Ă̎�M��Ԓl

#define		MT_SLA_ACK	0x18	//�A�h���X:SLA_W�𐳏푗�M�`�F�b�N�l
#define		MT_DATA_ACK	0x28	//�f�[�^�𐳏푗�M�`�F�b�N�l


void wait( int );

int main(){

	unsigned char i;
	
	DDRB = 0x03;	//0b00000011;	//LED �s���@�o�͐ݒ�
	DDRD = 0xC0;	//0b1100 0000;	//LED �s���@�o�͐ݒ�

	TWBR = 0xFF;	//����	2KHz

	PORTB = 0x02;	//0b00000001;	//�����l
	wait(1000);

	PORTB = 0x00;
	wait(1000);

		TWCR = 0xA4;						//�@I2C�J�n�������o

		while( !( TWCR & (1<<TWINT) ) );	//�A���ʑ҂�

		while( ( TWSR & 0xF8 ) != START );	//�B���M�H

		TWDR = 0x80 ;					// ���Đ�A�h���X�ݒ�

		TWCR = ( 1<<TWINT )| ( 1<<TWEN ) ;		//�A�h���X���M�J�n
		while(  ! ( TWCR & ( 1<<TWINT ) ) );	//�C���Đ�A�h���X���M�����҂�

	PORTB = 0x02;	//���M��̉����҂�
		wait(1000);

		while(  ( TWSR & 0xF8 ) != MT_SLA_ACK );	//�D�A�h���X:SLA_W ���푗�M�`�F�b�N

	PORTB = 0x00;	//���M��̉����m�F�󂯎��@���}�@LED����

			//�f�[�^���M
		i = 0xC0;		//���M�f�[�^
	PORTD = i;				//�f�[�^���M�@���}
		wait(1000);

		TWDR = i;		//���M�f�[�^�Z�b�g

		TWCR = ( 1<<TWINT )| ( 1<<TWEN ) ;//���M�J�n
		while(  ! ( TWCR & ( 1<<TWINT ) ) );//�E�����҂�

		while(  ( TWSR & 0xF8 ) != MT_DATA_ACK );	//�F�f�[�^ ���푗�M�`�F�b�N
	PORTD = 0x00;			//���M�f�[�^�󂯎��m�F�@���}�@LED����

		TWCR = ( 1<<TWINT )| ( 1<<TWSTO )| ( 1<<TWEN ) ;	//I2C��~�������o�@�o�X�̊J��

	PORTB = 0x02;		//��~�̖ڈ�

}

void wait( int time ){

	int j;
	unsigned char k;
	
	for ( j=0; j < time ;j++){
		for ( k=0; k<100 ;k++);
	}
}		 