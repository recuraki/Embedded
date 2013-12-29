#include <avr/io.h>
#include <util/delay.h>

// 点灯サイクル
#define WAITTIME 100 // ms
 
    char CounterC[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    char CounterD[8] = {0, 0, 0, 0, 0, 0, 0, 0};
/*
 ON「になる」までのインターバルとOFFになるまでのインターバル
 -1で「ならない」
 例: OFFinterval = -1でOFFにならない＝ずっとON
 */
    char ONintervalC[8];
    char ONintervalD[8];
    char OFFintervalC[8];
    char OFFintervalD[8];

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

int main(void){
mock_set_interval();
// PORT PCは
// PORT PDはすべて出力ポート
    DDRD = 0b11111111;
    DDRC = 0b00001111;
    PORTC = 0b00000000;
    PORTD = 0b00000000;
	char outC = 0;
	char outD = 0;
	int i;
    for(;;){
		for(i = 0; i < 8; i++)
		{
			if(CounterC[i] < 0) break;
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
        _delay_ms(WAITTIME);
    }
 
    return 0;
}