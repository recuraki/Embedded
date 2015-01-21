/*

I2C受信プログラム
	ATmega88

I2C
	信号ライン
		PC4
		PC5

	送受信データ出力LED
		PD6
		PD7

	合図LED
		PB1	

fuse)　デフォルト　内部発信　8MZ　1/8

相方送信プログラム
	I2Csend_smpl

結果） ok

*/


#include 	<avr/io.h>

//状態値
#define		START	0x08		//スタート状態値
#define		RECEIVE	0x60		//自分あてのアドレス受信状態値

#define		SR_DATA_ACK	0x80	//データを正常送信状態値

#define		P_S		0xA0	//停止または再送条件　状態値



void wait( int );

int main(){

	unsigned char x;
	
	DDRB = 0x03;	//0b00000011;	//LED ピン　出力設定
	DDRD = 0xC0;	//0b1100 0000;	//LED ピン　出力設定


	TWBR = 0xFF;		//分周	2KHz
	TWAR = 0x80;		//アドレス設定

	PORTB = 0x02;		//初期値

	wait(1000);


		TWCR =( ( 1<<TWINT )| ( 1<<TWEA ) | ( 1<<TWEN ) );//0xC4　0b1100 0100	//ｽﾚｰﾌﾞ受信動作設定

		while( !( TWCR & (1<<TWINT) ) );	//  設定結果待ち

		while( ( TWSR & 0xF8 ) != RECEIVE );	//自分あてのアドレス受信待ち

			PORTB = 0x00;	//アドレス受信　合図　LED消灯

				TWCR =( ( 1<<TWINT )| ( 1<<TWEA ) | ( 1<<TWEN ) );		//ｽﾚｰﾌﾞ受信動作
				while( !( TWCR & (1<<TWINT) ) );	// 設定結果待ち

				x = TWDR;			//受信データをLEDへ出力

			PORTD = x;

				while( ( TWSR & 0xF8 ) != SR_DATA_ACK );	//自分宛データ受信ACK応答　状態値チェック

			PORTB = 0x02;		//0b00000010;	データ受信終了合図

				//停止or再送条件検出
				TWCR =( ( 1<<TWINT )| ( 1<<TWEA ) | ( 1<<TWEN ) );		//ｽﾚｰﾌﾞ受信動作
				while( !( TWCR & (1<<TWINT) ) );	// 0b10000000 設定結果待ち
				while( (TWSR & 0xF8) != P_S );		//停止条件　状態値チェック

			PORTB = 0x00;	//0b00000000;	通信終了合図
			
}

void wait( int time ){

	int j;
	unsigned char k;
	
	for ( j=0; j<time ;j++){
		for ( k=0; k<100 ;k++);
	}
}
