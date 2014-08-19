//----------PINOUT DEFINITIONS-------------------
byte pagers[]=//Pagers //Haptic feedback hardware
{//arangement for the 32u4
  3,5,6,9,10,11,// NOTE ---set the desired button pins here--- NOTE
};//these really need to be assigned in corrispondence with the input pin arrangement
//starting from least significant bit on

#define NUMPAGERS sizeof(pagers)

//------------HARDWARE SETUP --------------------------

void pagersUp()
{ //setup the pager motor pins as outputs
  for (byte set=0;set<NUMPAGERS;set++)
  { 
    pinMode(pagers[set], OUTPUT);
  }
}

//-------------- actuating pagers---------------

void patternVibrate(byte pins, byte pwm)//
{
  for (byte i=0; i<NUMPAGERS; i++) // !! only the first 6 bits from the least significant are necisarry !!
  {//!! convert to read from least significant bit!!
    if (pins & (1 << i)) // show desired bit (pin)
    { // imagine incoming byte as an array of 8 bits, one for each pager
      analogWrite(pagers[i], pwm);
    }
    else
    {
      analogWrite(pagers[i], 0);
    }
  }
}

void pagerTest()
{
  for (byte i=0; i<NUMPAGERS; i++) 
  {  
      analogWrite(pagers[i], 255);
      delay(2000);
      analogWrite(pagers[i], 0);
  }
}
