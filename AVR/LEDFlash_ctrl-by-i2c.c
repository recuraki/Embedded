//  本コードの
//  I2C部分は中尾 司様の以下コードを参考にしています
//  http://www.eleki-jack.com/mycom2/2007/12/avri2c8twii2c_4.html#more
//  07/11/15	www.wsnak.com T.Nakao

// タイマーコード部分はsim00様の以下のコードを参考にしています
// http://blog.goo.ne.jp/sim00/e/1ab8d1564106581d1168edc059042c30

/*
recv data: DDDD -PPP
D: アドレス: 4bit = 0-15
アドレス:
 0-7 = PD0-7
 8-11 = PC0-3
P: パターン: 3bit = 0-7
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

typedef unsigned char  BYTE;
typedef unsigned short WORD;

// PD4にLED1, PD5にLED2が接続されているものとする

#define	bitLED1 6				// PD4
#define	bitLED2 7				// PD5
#define	LED1 (1<<bitLED1)
#define	LED2 (1<<bitLED2)

#define LED1_ON PORTD|=LED1		// LED1 ON
#define LED1_OFF PORTD&=~LED1	// LED1 OFF
#define LED2_ON PORTD|=LED2		// LED2 ON
#define LED2_OFF PORTD&=~LED2	// LED2 OFF

#define	rgTWISlEna (1<<TWEA)|(1<<TWEN)		// スレーブ ACK応答
#define	rgClrTWInt rgTWISlEna|(1<<TWINT)	// TWINT割り込み要因フラグのクリア


BYTE Phase;			// 受信フェーズ
BYTE MyAdrs;		// My I2Cアドレス
BYTE CByte;			// 受信したコントロール・バイト
BYTE I2CData;

// プロトタイプ
void I2CSlInit(BYTE adrs);
void I2CSlCom();

void SlaveInit(BYTE dat);
void SlaveReceive(BYTE dat);
BYTE SlaveSend(void);

// カウンタ
char CounterC[8] = {0, 0, 0, 0, 0, 0, 0, 0};
char CounterD[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// インターバルリスト(3bit = 8個)
/*
 ON「になる」までのインターバルとOFFになるまでのインターバル
 255で「ならない」
 例: OFFinterval = 255でOFFにならない＝ずっとON
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
// 割り込み処理ハンドラ
ISR(TIMER1_COMPA_vect)
{
	int i;
		for(i = 0; i < 8; i++)
		{
			if((ONintervalC[i] == 0xff) ||
			  (OFFintervalC[i] == 0xff)) continue;
			
			if( (outC >> i) & 0x01)
			{
				// 今1の場合
				if(CounterC[i] == 0)
				{
					outC = outC & (0xff ^ (0x01 << i));
					CounterC[i] = ONintervalC[i];
				}
			}
			else
			{
				// 今0の場合
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
				// 今1の場合
				if(CounterD[i] == 0)
				{
					outD = outD & (0xff ^ (0x01 << i));
					CounterD[i] = ONintervalD[i];
				}
			}
			else
			{
				// 今0の場合
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
// タイマー初期化
void timer_init(unsigned t)
{
	// 15.11.5 タイマ／カウンタ1比較レジスタA
	OCR1A = t;

	// 15.11.1 タイマ／カウンタ1制御レジスタA (初期値は0x00なので必要ない)
	//         ++-------COM1A1:COM1A0 00 OC1A切断
	//         ||++---- COM1B1:COM1B0 00 OC1B切断
	//         ||||  ++ WGM11:WGM10   00 波形生成種別(4bitの下位2bit)
	TCCR1A = 0b00000000;

	// 15.11.2 タイマ／カウンタ1制御レジスタB
	//         +------- ICNC1          0
	//         |+------ ICES1          0
	//         || ++--- WGM13:WGM12    01  波形生成種別(4bitの上位2bit) CTC top=OCR1A
	//         || ||+++ CS12:CS11:CS10 101 1024分周
	TCCR1B = 0b00001101;

	// 15.11.8 タイマ／カウンタ1割り込みマスクレジスタ
	//           +----- ICIE1  0
	//           |  +-- OCIE1B 0
	//           |  |+- OCIE1A 1 タイマ／カウンタ1比較A割り込み許可
	//           |  ||+ TOIE1  0
	TIMSK1 = 0b00000010;

	sei(); // 割り込み許可
}

//-----------------------------------------------------------
// プログラム本体
int main(void) {
	uint8_t result;
	eeprom_busy_wait();
	result =  eeprom_read_byte(0x0000);
	//MyAdrs = 0x21;				// 自I2Cスレーブ・アドレス
	MyAdrs = result;				// 自I2Cスレーブ・アドレス
	if(MyAdrs > 0x77 || MyAdrs < 0x03)
	 {
	 MyAdrs = 0x20;				// 自I2Cスレーブ・アドレス
	 }

	timer_init(200); // 100 * 1024us毎に割り込み (約4.88Hz) @ 1MHz

    mock_set_interval();

	//LED用ポートを出力に設定
	// PORT PDはすべて出力ポート
	// PORT PCはLSB側だけ
    DDRD = 0b11111111;
    DDRC = 0b00001111;
    PORTD = 0b00000000;
    PORTC = 0b00000000;
	
	outC = 0;
	outD = 0;

	I2CSlInit(MyAdrs);			// I2Cスレーブ初期化

	while(1) {
		I2CSlCom();
	}

}


// -----------------------------------------------------------
// I2Cスレーブ初期化
// 	IN adrs:My I2Cスレーブ・アドレス
// -----------------------------------------------------------
void I2CSlInit(BYTE adrs) {
	adrs <<= 1;

	// adrs |= 1;		// ジェネラル・コール・アドレス許可
	TWAR = adrs;
	TWCR = rgTWISlEna;
}


// -----------------------------------------------------------
//  I2Cスレーブ処理(メイン・ループに入れる)
// -----------------------------------------------------------
void I2CSlCom() {

	if(!(TWCR & (1<<TWINT))) {
		return;					// TWINTが未セットのとき終了
	}

	// TWINTがセットされているとき
	switch(TWSR) {						// 状態コード
		case 0x60:			// CB(Write)受信
		// ---- CB(W)受信 ----
		SlaveInit(TWDR);			// CB受信イベント・ハンドラ
		TWCR = rgClrTWInt;			// INT要求フラグ・クリア
		break;

		case 0x80: 			// Data受信
		// ---- スレーブ受信 ----
		SlaveReceive(TWDR);			// 受信イベント・ハンドラ
		TWCR = rgClrTWInt;			// INT要求フラグ・クリア
		break;

		case 0xA8:			// CB(Read)受信
		// ---- CB(R)受信 ----
		SlaveInit(TWDR);			// CB受信イベント・ハンドラ
		// スレーブ送信第1バイト取得
		TWDR = SlaveSend();			// スレーブ送信イベント・ハンドラ
		//   送信データ取得
		TWCR = rgClrTWInt;			// INT要求フラグ・クリア
		break;

		case 0xB8:			// Data送信(ACK受信)
		// ---- スレーブ送信(ACK受信)(スレーブ送信継続) ----
		// スレーブ送信第2バイト以降取得
		TWDR =  SlaveSend();		// スレーブ送信イベント・ハンドラ
		//   送信データ取得
		TWCR = rgClrTWInt;			// INT要求フラグ・クリア

		break;

		case 0xC0:			// Data送信(NOACK受信) 最終データ スレーブ送信終了
		case 0xA0:			// スレーブ受信中のストップ・コンディション
		TWCR = rgClrTWInt;			// INT要求フラグ・クリア
		break;
	}
}


// -----------------------------------------------------------
// CB受信イベント・ハンドラ
// 	IN dat: 受信したデータ(コントロール・バイト)
// -----------------------------------------------------------
void SlaveInit(BYTE dat) {

}



// -----------------------------------------------------------
// スレーブ受信イベント・ハンドラ
// 	IN dat: 受信したデータ
// -----------------------------------------------------------
void SlaveReceive(BYTE dat) {
    BYTE chAddr;
	BYTE chPattern;
	chAddr = dat >> 4;
	chPattern = dat & 0x07;
	if(chAddr <= 7)
	{
		//PortDのアドレス
		ONintervalD[chAddr] = ONinterval[chPattern];
		OFFintervalD[chAddr] = OFFinterval[chPattern];
        // ONにならないなら
		if(ONinterval[chPattern] == 255)
		{
			// OFFにする
			outD = outD & (0xff ^ (0x01 << chAddr));
		}
        // OFFにならないなら
		else if(OFFinterval[chPattern] == 255)
		{
			// ONにする
			outD = outD | 0x01 << chAddr;
		}
		CounterD[chAddr] = 0;
	}
	
	else if(chAddr > 7)
	{
		chAddr = chAddr - 8;
		//PortDのアドレス
		ONintervalC[chAddr] = ONinterval[chPattern];
		OFFintervalC[chAddr] = OFFinterval[chPattern];
		// ONにならないなら
		if(ONinterval[chPattern] == 255)
		{
			// OFFにする
			outC = outC & (0xff ^ (0x01 << chAddr));
		}
		// OFFにならないなら
		else if(OFFinterval[chPattern] == 255)
		{
			// ONにする
			outC = outC | 0x01 << chAddr;
		}
		CounterC[chAddr] = 0;
	}

	I2CData = dat;
    PORTC = outC;
	PORTD = outD;
}


// -----------------------------------------------------------
// スレーブ送信イベント・ハンドラ
// 	OUT Acc: 送信するデータ
// -----------------------------------------------------------
BYTE SlaveSend(void) {
	return I2CData;		// 過去にマスタから送られてきたデータを送り返す
}

