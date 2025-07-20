#include <Wire.h>
#include <DS3231.h>
#include <dht_nonblocking.h>
#define DHT_SENSOR_TYPE DHT_TYPE_11
#include <LiquidCrystal.h>


//Pin Assignments:
int latch = 12;
int clockPin = 13;
int data = 11;
int digitPins[] = {7, 8, 9, 10};
static const int DHT_SENSOR_PIN = 50;


//Char table for the digits
unsigned char table[] = {
  0x3f, 0x06, 0x5b, 0x4f, 0x66,
  0x6d, 0x7d, 0x07, 0x7f, 0x6f
};

//RTC Object definitions
DS3231 clock;
RTCDateTime dt;

//Needed Constants
int digits[4];

//DHT_sensor
DHT_nonblocking dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
float temp =0;
float humid = 0;

//LCD screen
LiquidCrystal lcd(30, 31, 32, 33, 34, 35);

//LEDs
int sensePin = A10;
int lightValue = 0;
int RED = 46;
int GREEN = 45;
int BLUE = 44;

void setup() {
  Serial.begin(9600);

  // Digit Screen
  pinMode(latch, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(data, OUTPUT);

  for (int i = 0; i < 4; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], HIGH); // turn off
  }

  //RTC module
  clock.begin();
  clock.setDateTime(__DATE__, __TIME__);


  //LCD screen
  lcd.begin(16, 2);
  lcd.print("Welcome!");

  //RGBs
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  digitalWrite(RED, LOW);
  digitalWrite(GREEN, LOW);
  digitalWrite(BLUE, LOW);

}

void loop() {
  clockMod();
  dhtInfo();
  lcdScreen();
}


////--------CLOCK MODULE--------////

void Display(unsigned char num) {
  digitalWrite(latch, LOW);
  shiftOut(data, clockPin, MSBFIRST, table[num]);
  digitalWrite(latch, HIGH);
}

void updateDigits() {
  dt = clock.getDateTime();
  digits[0] = dt.hour / 10;
  digits[1] = dt.hour % 10;
  digits[2] = dt.minute / 10;
  digits[3] = dt.minute % 10;
}

void clockMod() {

  // Increment time every minute
  updateDigits();

  // Display the time on 4-digit display
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) digitalWrite(digitPins[j], HIGH); // all off
    Display(digits[i]);
    digitalWrite(digitPins[i], LOW); // current digit on
    delay(2.5);
  }
}

////--------DHT MODULE--------////

static bool measure_environment(float *temperature, float *humidity)
{
  static unsigned long measurement_timestamp = millis( );

  /* Measure once every four seconds. */
  if(millis() - measurement_timestamp > 3000ul)
  {
    if(dht_sensor.measure(temperature, humidity) == true)
    {
      measurement_timestamp = millis();
      return(true);
    }
  }

  return(false);
}

void dhtInfo() {
  float temperature;
  float humidity;

  /* Measure temperature and humidity.  If the functions returns
     true, then a measurement is available. */
  if(measure_environment(&temperature, &humidity) == true)
  {
    temp = (temperature * 9/5) + 32;
    humid = humidity;    
  }
}
////--------LIGHT MODULE--------////
int redValue = 255;
int greenValue = 0;
int blueValue = 0;
void turnOn() {
 static int mode = 0;
  switch (mode) {
    case 0: // Red to Green
      redValue--;
      greenValue++;
      if (redValue <= 0) mode = 1;
      break;
    case 1: // Green to Blue
      greenValue--;
      blueValue++;
      if (greenValue <= 0) mode = 2;
      break;
    case 2: // Blue to Red
      blueValue--;
      redValue++;
      if (blueValue <= 0) mode = 0;
      break;
  }

  // Apply colors
  analogWrite(RED, redValue);
  analogWrite(GREEN, greenValue);
  analogWrite(BLUE, blueValue);
}

void lightUp() {
  lightValue = analogRead(sensePin);
  lcd.setCursor(0,1);
  if (lightValue < 600) {
    lcd.print("Its too dark");
    turnOn();
  } else {
    lcd.print("            ");
    analogWrite(BLUE, 0);
    analogWrite(RED, 0); 
    analogWrite(GREEN, 0); 
  }
}




////--------LCD MODULE--------////

void lcdScreen() {
  lcd.setCursor(9, 0);
  lcd.print(temp, 0);
  lcd.print("F,");
  lcd.print(humid, 0);
  lcd.print("%");
  lightUp();
}
