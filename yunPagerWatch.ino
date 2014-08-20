//yunPagerWatch.ino

#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>

YunServer server;

byte chordPatterns[] {1,5,48,56,2,24,33,6,4,14,28,12,40,30,7,18,31,3,16,32,51,45,8,35,54,49,};
//the pattern above is based on a bit of ergo research when braille fell short
//chordPatterns[0] == "a" possition
#define PATTERNSIZE sizeof(chordPatterns)
  
#define TIMING 1000 // haptic char durration
#define PWM 200   // intensity- this is planed for upper/lower

void setup() 
{
  Serial.begin(9600);
  pagersUp();
  Bridge.begin();
  Console.begin();// "telnet localhost 6571" type that sshed into the yun
  while (!Console){;}// wait for Console port to connect.!!SKETCH ONLY RUNS WHEN CONNECTED!!
}

void loop() 
{
  playAlpha();
}

void playAlpha()
{
	for(int i=0; i<PATTERNSIZE; i++)
	{
	  Console.write(patternToChar(chordPatterns[i]));
		patternVibrate(chordPatterns[i], PWM);
		delay(TIMING);
		patternVibrate(chordPatterns[i],0);
		delay(TIMING);
	}
}

byte patternToChar(byte base)
{
  if(base == 128){return 8;}//Express convertion: Backspace // Backspace doubles as second level shift for special chars
  if(base == 64){return 32;}//Express convertion: Space // Space also doubles as the first shift in a chord
  if(base == 63){return 13;}//Express convertion: Cariage return
  
  for (byte i=0; i<PATTERNSIZE; i++)   
  {// for all of the key mapping   
    if ( (base & 63) == chordPatterns[i] ) 
    {//patern match regardless most significant 2 bits // 63 = 0011-1111 // mask the 6th and 7th bit out
      if ((base & 192) == 192){break;}//third level shift *combination holding space and backspace
      if (base & 64)//first level shift *combination with space
      {// 64 = 0100-0000 // if( 6th bit is fliped high )
        //if(lower shift, less than 10th result) {return corrisponding number}
        if(i<10){return '0' + i;} //a-j cases (ascii numbers)
        if(i<25){return 23 + i;}  //k-y cases ( !"#$%&'()*+'-./ )
        if(i==26){break;}         //z case (unassigned)
      } 
      if (base & 128)//second level shift *combination with backspace
      {//128 = 1000-0000 // if(7th bit is high) 
        if(i<7){return ':' + i;}//a-g cases ( :;<=>?@ )
        if(i<13){return 84 + i;}//h-m cases ( [\]^_`  )
        if(i<17){return 110 + i;}//n-q cases( {|}~    ) 
        break;                   //other casses unassigned
      }
      return 'a' + i;// return plain char based on possition in the array given no shift
    }
  }
  return 0;
}
