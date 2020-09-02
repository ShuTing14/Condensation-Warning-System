////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
// Condensation Warning System
// Real time data collection system
// Use DHT22 and Type-K thermocouple with MAX6675 module to collect 
// Humidity, Temperature and Window Surface Temperature.
// Dew Point is calculated to predict condensation.
//
// Use ThingSpeak database to store all these data.
// GET HTTP command is used to send four field data to database.
// Use ESP8266 Wi-Fi module. 

// Hardware		: ATMEGA328P with 16MHz crystal oscillator
// Author		: Wong Shu Ting 
// SID			: 201376607
// For			: University of Leeds Final Year Project
// Supervisor	: Dr. Craig Evans
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Include Libraries used in this project
#include <Wire.h>
#include <math.h> 
#include <SoftwareSerial.h>

// Downloaded Libraries for I2C 20x4 LCD, MAX6675 module for Type-K thermocouple and DHT22 Sensor.
#include "DHT.h"
#include <LiquidCrystal_I2C.h>
#include <max6675.h>

// Define RX and TX pins of ESP8266 Wi-Fi module
#define RX 2
#define TX 3

// Define DHT output pin, and DHT sensor type. 
#define DHTPIN 4     
#define DHTTYPE DHT22 

// Define Wi-FI AP and password.
String AP = "WongST";     // AP NAME
String PASS = "12345678"; // AP PASSWORD

// Define ThingSpeak Write API KEY, HOST address, Port
String API = "CXWATF1EZGCI3IZ9";   // Write API KEY
String HOST = "api.thingspeak.com";
String PORT = "80";

// Define Fieldnames of ThingSpeak
String field = "field1";
String field2 = "field2";
String field3 = "field3";
String field4 = "field4";

// Variables used in sendCommand function.
// countTimeCommand counts how many sucessful command is sent. 
// countTimeCommand tracks the try time and keep retrying to sendCommand as long as countTimeCommand < max time. 
// boolean found used to break out if the reply is found. 
int countTrueCommand;
int countTimeCommand; 
boolean found = false; 


// Variables used for Dew Point Calculation
const float A = 17.625;
const float B = 243.04;

// Variables used to store temperature, humidity, window surface temperature and dew point. 
double valSensor     = 1;
double humidSensor   = 1;
double surfaceSensor = 1;
double dewpoint      = 1;

// ThermoCouple Pin Confuguration 
int thermo_gnd_pin = 13;
int thermo_vcc_pin = 12;
int thermo_so_pin  = 9;
int thermo_cs_pin  = 10;
int thermo_sck_pin = 11;

// Set ESP8266 chip as SoftwareSerial. 
SoftwareSerial esp8266(RX,TX); 

// Set DHT with previously defined DHTPIN and DHTTYPE
DHT dht(DHTPIN, DHTTYPE);

// Initialise I2C LCD
LiquidCrystal_I2C lcd(0x27,20,4);

// Initialise MAX6675 with previously defined pinout
MAX6675 thermocouple(thermo_sck_pin, thermo_cs_pin, thermo_so_pin);

void setup() {

  // Start Serial Output for debug
  Serial.begin(9600);
  
  // Set onboard LED as output
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Set ThermoCouple Vcc and GND as output. 
  pinMode(thermo_vcc_pin, OUTPUT); 
  pinMode(thermo_gnd_pin, OUTPUT); 
  
  // Set Vcc as high for power supply, GND as low for Ground. 
  digitalWrite(thermo_vcc_pin, HIGH);
  digitalWrite(thermo_gnd_pin, LOW);
  
  // Start software serial with ESP8266 chip and connect to Wi-FI AP. 
  esp8266.begin(115200);
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");

  // Initialise DHT sensor, LCD 
  dht.begin();
  lcd.init();
  
  // Turn on LCD backlight and clear anything on the LCD
  lcd.backlight();
  lcd.clear();
}

void loop() {
    while(1){
		
		// Obtain sensors data
		valSensor = getSensorData();
		humidSensor = getHumidData();
		surfaceSensor = getSurfaceData(); 
		
		// Calculate Dew Point with equation.
		dewpoint = (B*(((A*valSensor)/(B+valSensor))+log(humidSensor/100)))/(A-(((A*valSensor)/(B+valSensor))+log(humidSensor/100)));
    
		// Check if surface temperature is lower than dew point, if it is, turn on alert LED, else turn off alert LED. 
		if (surfaceSensor < dewpoint) {
			digitalWrite(LED_BUILTIN, HIGH);   
		}
		else {
			digitalWrite(LED_BUILTIN, LOW);
      }

		// Print first line of Humidity reading
		lcd.setCursor(0,0);
		lcd.print("Humidity: ");
		lcd.print(humidSensor);
		lcd.print("%");

		// Print second line of Temperature reading
		lcd.setCursor(0,1);
		lcd.print("Temp: ");
		lcd.print(valSensor);
		lcd.print((char)223);
		lcd.print("C");

		// Print third line of Surface Temperature reading
		lcd.setCursor(0,2);
		lcd.print("SurfaceTemp: ");
		lcd.print(surfaceSensor);
		lcd.print((char)223);
		lcd.print("C");

		// Print forth line of calculated Dew Point
		lcd.setCursor(0,3);
		lcd.print("Dewpoint: ");
		lcd.print(dewpoint);
		lcd.print((char)223);
		lcd.print("C");
    
		// Initialise the GET HTTP command used to store data on ThingSpeak
		String getData = "GET /update?api_key="+ API +"&"+ field +"="+String(valSensor)+"&"+ field2 +"="+String(humidSensor)+"&"+ field3 +"="+String(surfaceSensor)+"&"+ field4 +"="+String(dewpoint);

		// Sent the GET HTTP command
		sendCommand("AT+CIPMUX=1",5,"OK");
		sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
		sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
		esp8266.println(getData);delay(1500);countTrueCommand++;
		sendCommand("AT+CIPCLOSE=0",5,"OK");

  
    }
  }

// Function to read MAX6675 reading for window surface temperature
float getSurfaceData(){

	float STemp = thermocouple.readCelsius();
  
  return STemp; 
}

// Function to read DHT22 temperature 
float getSensorData(){

	float t = dht.readTemperature();
  
  return t; // Replace with your own sensor code
}

// Function to read DHT22 humidity
float getHumidData(){
    
		float h = dht.readHumidity();

    return h;
}

// sendCommand function to send command with ESP8266 chip
void sendCommand(String command, int maxTime, char readReplay[]) {
		
  // Print out the sucessful tries		
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  
  // Keep retrying as long as maxTime is not reached 
  while(countTimeCommand < (maxTime*1))
  {
	// Send the command and if expected result is obtained, found becomes true and break out.   
    esp8266.println(command);
    if(esp8266.find(readReplay))
    {
      found = true;
      break;
    }
  
    countTimeCommand++;
  }
  
  // If expected result is found, printout OYI, else printout Fail. 
  if(found == true)
  {
    Serial.println("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  
  if(found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  
  found = false;
 }
