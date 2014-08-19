#define LED 13

void setup() 
{
  Serial.begin(9600);
  pinMode(LED, OUTPUT);
  pagersUp();
  digitalWrite(13, HIGH);
  delay(2000);
  pagerTest();
  delay(500);
  pagerTest();
  Serial.println("done");
}

void loop() 
{
  //pagerTest(); 
}
