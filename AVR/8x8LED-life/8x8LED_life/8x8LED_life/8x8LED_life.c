/*
 * 秋月 OSL641501-A　向け　ライフゲーム
 * 8x8ドットマトリクスLED
 *
 */

/*
 * 結線
 * アノード側(PB)
 * High Active
 * PB0(14)--(13) COL1(A)
 * PB1(15)--(3)  COL2(A)
 * PB2(16)--(4)  COL3(A)
 * PB3(17)--(10) COL4(A)
 * PB4(18)--(6)  COL5(A)
 * PB5(19)--(11) COL6(A)
 * PB6(9) --(15) COL7(A)
 * PB7(10 --(16) COL8(A)
 * カソード側(PD)
 * 引く方なのでLow Active(Lowでその行が発光)
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

// ドットの数を定義
#define Xsize 8
#define Ysize 8

/*
 * ライフゲームにおいては「周辺８セル」の状況を知る必要があるため
 * 上下左右１セル分多く領域をとります。
 */
char nowMap[Xsize + 2][Ysize + 2];
char nextMap[Xsize + 2][Ysize + 2];

// 点灯サイクル
#define WAITTIME 100 // ms

void timer_init(unsigned t)
{
	// 比較レジスタ
	OCR1A = t;
	TCCR1A = 0b00000000;
	TCCR1B = 0b00001101;
	//              ^^^101 1024分周
	TIMSK1 = 0b00000010;
}

/*
 * セルの初期配置をつくります。
 * rand使っていますが、乱数の初期値は固定なので、毎回同じ。
 */
void MapInit(void)
{
	int i,j;
	for(i = 1; i <= Xsize; i++){
		for(j = 1; j <= Ysize; j++){
			nowMap[i][j] = ((rand() % 3) == 0) ? 1 : 0;
		}
	}
}


char curRow;
/*
 * タイマーの割り込みハンドラ
 * 割り込みがあるたび、「次の行」の描画を行う
 */
ISR(TIMER1_COMPA_vect)
{

	// 気持ち、タイマー割り込みを止める
	cli();

	// out: 今の行に吐くバイナリ値
	char out = 0x00;
	int i;

	// いったん、今の列の出力を止める
	/*
	 * Note:
	 * これはキモで、「今までの行」のまま
	 * PORTDを変更 = 「次の行」に移してしまうと
	 * 「今ままでの行」の残像が「次の行に残る」
	 * (次の行に今の行の出力はいちゃうから)
	 */
	PORTB = 0x00;
	
	// それで、次の行を選択する
	// LowActiveなので、フル1から、今の行のビットのxorとる
	PORTD = 0xff ^ (0x01 << curRow);

	// 各列を操作
	for(i = 0; i < Xsize; i++)
	{
		// もし、今のフィールドが1 = 生存なら
		if(nowMap[i + 1][curRow + 1] == 1)
		{
			// 列のその出力をHighにする
			// 論理演算でORをoutにとる
			out |= (0x01 << (i));
		}
	}

	// で、それらの結果を出力
	PORTB = out;

	curRow ++;
	if(curRow >= Ysize)
	{
		curRow = 0;
	}

	// タイマー割り込みを再開
	sei();
}


// x,yの周囲8セルの生存セルを取得
int CountAround(int x, int y)
{
	int c = 0;
	c += (nowMap[x - 1][y - 1] == 1) ? 1 : 0;
	c += (nowMap[x    ][y - 1] == 1) ? 1 : 0;
	c += (nowMap[x + 1][y - 1] == 1) ? 1 : 0;
	c += (nowMap[x - 1][y    ] == 1) ? 1 : 0;
	// [x][y]は自分
	c += (nowMap[x + 1][y    ] == 1) ? 1 : 0;
	c += (nowMap[x - 1][y + 1] == 1) ? 1 : 0;
	c += (nowMap[x    ][y + 1] == 1) ? 1 : 0;
	c += (nowMap[x + 1][y + 1] == 1) ? 1 : 0;
	return(c);
}

int reset_counter = 0;
int life_counter = 0;

// ライフゲームのstep
/*
 * 盤の「今」の情報はnowMapが持ち、
 * 「次」の情報をnextMapに入れていきます。
 *
 * 通常のライフゲームと違い、何の入力も受け付けないので、
 * 1. 50 steps実行されたときには即座次のmapに移る
 * 2. 変動がないときは3step待って次のmapに移る
 * という処理を入れています
 */
 
void step(void)
{
	// 変更フラグ
	char IsChange = 0;

	int i,j;

	// 今の生存時間をインクリメント	
	life_counter += 1;
	
	// 各セルを走査
	for(i = 1; i <= Xsize; i++){
		for(j = 1; j <= Ysize; j++){
			if(nowMap[i][j] == 0)
			{
				// 死にセルの場合
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
				// 生きセルの場合
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

	// ここで、次のマップを「今のマップ」にコピーします
	for(i = 1; i <= Xsize; i++){
		for(j = 1; j <= Ysize; j++){
			nowMap[i][j] = nextMap[i][j];
		}
	}

	// 一定step数実行されている場合、次のマップに	
	if(life_counter > 70)
	{
		life_counter = 0;
		MapInit();
	}
	
	// 変更がない場合はreset_counterをインクリメントし
	// 一定時間立つとマップを初期化
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
	DDRB = 0b11111111;
	DDRD = 0b11111111;

	curRow = 0;
	MapInit();

    // タイマー初期化
    // 1024us毎に割り込み (100で4.88Hz) @ 1MHz
    timer_init(1);
    sei(); // enable timer
		
    while(1)
    {
		_delay_ms(100);
		// ライフゲームの処理
		step();
    }
}