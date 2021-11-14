

void setup() {
  Serial.begin(9600);         // Set the communication speed
  pinMode(12, INPUT); // Set the GPIO pin as an output
  pinMode(13, OUTPUT);
}

int threshold = 450; //Enter Your threshold value here
int abs_value = 0;

int ledCount = 10; //number of Bargraph LEDs
int Sound_signal;
int sum;
int num_Measure = 128 ; // Set the number of measurements 
int level;
int soundlow = 3575;
int soundmedium = 3400;

void askForPicture(){
  // ESPNow o PIR?
  digitalWrite(13, 1);
  Serial.println("Disparando camara"); 
  digitalWrite(13, 0);
}

void loop() {
  sum = 0;
  for ( int i = 0 ; i <num_Measure; i ++)  
  {  
   Sound_signal = analogRead (12);  
    sum =sum + Sound_signal;  
  }  
 
  level = (sum / num_Measure) - 33; // Calculate the average value   
  Serial.println("El nivel es: "+String(level));
  //if (level<=soundlow){
  //  Serial.println("Low"); 
  //} else 
  if(level>soundlow){
    askForPicture();
    delay(45000); //Duerme 10 segundos para esperar la imagen?
  }
  
}
