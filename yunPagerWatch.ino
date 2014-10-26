//yunPagerWatch.ino

byte chordPatterns[] {1,5,48,56,2,24,33,6,4,14,28,12,40,30,7,18,31,3,16,32,51,45,8,35,54,49,};
//the pattern above is based on a bit of ergo research when braille fell short
//chordPatterns[0] == "a" possition
#define PATTERNSIZE sizeof(chordPatterns)
  
#define HAPTICTIMING 1000 // haptic char durration
#define PWM 200   // intensity- this is planed for upper/lower

#define MONITOR_MODE   0
#define START_INTERUPT 1 // removes output, zeros play point: messageHandlr
#define CAT_OUT        2 // set message to play : messageHandlr
#define JOB            3 // messageHandlr "is a job set?" argument
#define LINE_SIZE      80   //cause why should a line be longer?
#define TERMINATION    '\0'
#define NEW_LINE       '\n' //determines end of a message in buffer
#define BACKSPACE      8    // output keys

void setup() 
{
  Serial.begin(9600);
  pagersUp();
}

void loop() 
{
  listenForMessage();// grab potential messages over serial
  messageHandlr(MONITOR_MODE);//async message mangment-interupt with keystroke
}

/******* incoming messages **********************
Serial receive message
************************************************/
void listenForMessage()
{ 
  while(Serial.available())
  {
    char singleLetter = (char)Serial.read();
    messageHandlr(singleLetter);//fills up the handlr's lineBuffer
    if(singleLetter == NEW_LINE)
    {
      messageHandlr(START_INTERUPT);//In the middle of something? don't care
      messageHandlr(CAT_OUT);// flag to play 
      return;
    }
  }
}

boolean messageHandlr(byte mode)
{
  static byte lineBuffer[LINE_SIZE]={};
  static byte pos = 0; // in this way buffer can be no greater than 255
  static boolean playFlag = 0;
  
  switch(mode)
  {
    case MONITOR_MODE:
      if(playFlag)
      {
        if(lineBuffer[pos]==NEW_LINE)
        {// Check for end case before updating further
          playFlag=0; pos = 0;//reset possition and playflag
          return 0;
        }// END CASE: MESSAGE HAS BEEN PRINTED AND REMOVED
        if(hapticMessage(MONITOR_MODE))//<---Updates Letter display
        {//true == single letter display finished   
          hapticMessage(lineBuffer[pos]);       //start next letter vib
          pos++;//increment read possition
        }//false == waiting -> return -> continue main loop
      }//playFlag false == no directive to play ->continue main loop
      //else{patternVibrate(0);}//!!! sys wide release:turn pagers off!!!
      return 0;//in any case return to avoid falling thru
    case START_INTERUPT:// completly interupts message 
      if (playFlag) 
      {
        pos = 0; playFlag = 0;  //reset possition and playflag
      }
      return 0; 
    case CAT_OUT:
      playFlag = 1;
      hapticMessage(lineBuffer[pos]);
      pos++;
      return 0;
    case JOB: return playFlag;
    default://SPACE-Z cases concat into buffer
      if (mode > 128){break;}//ignore special cases
      if (mode == BACKSPACE){ pos--; break;} //delete buffer entry "RECORD"
      lineBuffer[pos] = mode; // assign incoming char to buffer
      if (mode == NEW_LINE){pos = 0;}//done recieving: zero possition
      else {pos++;} // increment write possition for more chars
      if(pos==LINE_SIZE){pos--;}//just take the head till the new line
  }  
}

byte charToPattern(byte letter)
{
  if(letter == 32){return 64;}//Express convertion: Space // Space also doubles as the first shift in a chord
  
  for (byte i=0; i<PATTERNSIZE; i++)   
  {// for all of the key mapping
    if ( letter == ('a'+ i) ){return chordPatterns[i];}//return typicall letter patterns
    if ( letter < 58 && letter == ('0' + i) ) {return chordPatterns[i] | 64;} // in numbers shift case return pattern with 6th bit shift
    if ( letter > 32 && letter < 48 && letter == (23 + i) ) {return chordPatterns[i] | 64;}//k-y cases ( !"#$%&'()*+'-./ )return 6th bit shift
    if ( letter < 65 && letter == (':' + i) ) {return chordPatterns[i] | 128;}//               a-g cases  (:;<=>?@ ), return 7th bit shift
    if ( letter > 90 && letter < 97 && letter == (84 + i) ) {return chordPatterns[i] | 128;}// h-m cases  ([\]^_`  ), return 7th bit shift
    if ( letter > 122 && letter < 127 && letter == (110 + i) ) {return chordPatterns[i] | 128;}//n-q cases( {|}~   ), return 7th bit shift
  }
  return 0;
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

//************lower level haptic logic***************
boolean ptimeCheck(uint32_t durration)
{//used for checking an setting timer
  static uint32_t ptimer[2] = { };// create timer to modify
  if(durration)
  {
    ptimer[1]=durration; //set durration
    ptimer[0]=millis();  // note the time set
  }
  else if(millis() - ptimer[0] > ptimer[1]){return true;}
  // if the durration has elapsed
  return false;
} 

boolean hapticMessage(byte letter) 
{ // updating function; passing a string sets course of action
  static boolean touchPause= 0; // pause between displays
  
  if(letter)
  {
    ptimeCheck(HAPTICTIMING);//set the time for a letter to display
    patternVibrate(charToPattern(letter));  //start vibrating that letter
    return false;//why bother checking time... we just set it
  }
  //---------- 0 or "monitor" case ------- aka no letter check if done
  if(ptimeCheck(0))
  {               //time to "display" a touch / pause elapsed
    if(touchPause)//given that we are at the pause stage FINAL
    {             //this case allows for a pause after "display"
      touchPause=!touchPause; //prep for next letter
      return true;//Send confirmation this letter has been "played"
    }
    else          //durring the letter buzz phase
    {
      touchPause=!touchPause;    //flag pause time to start
      patternVibrate(0);         //stop letter feedback
      ptimeCheck(HAPTICTIMING/2);//set pause time
    };
  }
  return false;  //signals letter in process of being played 
}
