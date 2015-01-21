/*
I2Cのデータシートの手順通りに設定し送信

I2C送信プログラム

I2C
	信号ライン
		PC4
		PC5

	送受信データ出力LED
		PD6
		PD7

	合図LED
		PB1	


相方の受信プログラム
I2Crecv_smpl


fuse)　デフォルト　内部発信　8MZ　1/8


結果） ok

*/


#include 	<avr/io.h>

//状態値
#define		START	0x08		//スタート状態値
#define		RECEIVE	0x60		//自分あての受信状態値

#define		MT_SLA_ACK	0x18	//アドレス:SLA_Wを正常送信チェック値
#define		MT_DATA_ACK	0x28	//データを正常送信チェック値


void wait( int );

int main(){

	unsigned char i;
	
	DDRB = 0x03;	//0b00000011;	//LED ピン　出力設定
	DDRD = 0xC0;	//0b1100 0000;	//LED ピン　出力設定

	TWBR = 0xFF;	//分周	2KHz

	PORTB = 0x02;	//0b00000001;	//初期値
	wait(1000);

	PORTB = 0x00;
	wait(1000);

		TWCR = 0xA4;						//①I2C開始条件送出

		while( !( TWCR & (1<<TWINT) ) );	//②結果待ち

		while( ( TWSR & 0xF8 ) != START );	//③送信可？

		TWDR = 0x80 ;					// あて先アドレス設定

		TWCR = ( 1<<TWINT )| ( 1<<TWEN ) ;		//アドレス送信開始
		while(  ! ( TWCR & ( 1<<TWINT ) ) );	//④あて先アドレス送信完了待ち

	PORTB = 0x02;	//送信先の応答待ち
		wait(1000);

		while(  ( TWSR & 0xF8 ) != MT_SLA_ACK );	//⑤アドレス:SLA_W 正常送信チェック

	PORTB = 0x00;	//送信先の応答確認受け取り　合図　LED消灯

			//データ送信
		i = 0xC0;		//送信データ
	PORTD = i;				//データ送信　合図
		wait(1000);

		TWDR = i;		//送信データセット

		TWCR = ( 1<<TWINT )| ( 1<<TWEN ) ;//送信開始
		while(  ! ( TWCR & ( 1<<TWINT ) ) );//⑥完了待ち

		while(  ( TWSR & 0xF8 ) != MT_DATA_ACK );	//⑦データ 正常送信チェック
	PORTD = 0x00;			//送信データ受け取り確認　合図　LED消灯

		TWCR = ( 1<<TWINT )| ( 1<<TWSTO )| ( 1<<TWEN ) ;	//I2C停止条件送出　バスの開放

	PORTB = 0x02;		//停止の目印

}

void wait( int time ){

	int j;
	unsigned char k;
	
	for ( j=0; j < time ;j++){
		for ( k=0; k<100 ;k++);
	}
}		 