#define BLYNK_PRINT Serial

#include <SPI.h>
#include <Ethernet.h>
#include <BlynkSimpleEthernet.h>

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#include <DHT.h>

#include <OneWire.h>
#include <DallasTemperature.h>

char auth[] = "hWE4p7JAq17YX9oQJ7a2SjsDZNjG2WA3";

byte arduino_mac[] = { 0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress arduino_ip ( 192, 168,   1, 120); // dirección IP del ARDUINO
IPAddress dns_ip     ( 192, 168,   1,   1);
IPAddress gateway_ip ( 192, 168,   1,   1);
IPAddress subnet_mask( 255, 255, 255,   0);

IPAddress server_ip  ( 192, 168,   1,  78); // dirección IP del servidor BLYNK. En local es 192.168.1.78
int server_port( 8080 ); // puerto del servidor BLYNK. En local es 9443

#define W5100_CS  10
#define SDCARD_CS 4

// Definimos el pin digital donde se conecta el sensor de humedad
#define DHTPIN 45
// Dependiendo del tipo de sensor, en este caso DHT11
#define DHTTYPE DHT11
 
// Inicializamos el sensor DHT11
DHT dht(DHTPIN, DHTTYPE);

// Sensores de temperatura
const int oneWirePin = 47;
 
OneWire oneWireBus(oneWirePin);
DallasTemperature sensors(&oneWireBus);
 
DeviceAddress SplitOut    = { 0x28, 0xAC, 0x9F, 0x2C, 0x27, 0x19, 0x01, 0x6B };
DeviceAddress RackDown    = { 0x28, 0x4E, 0xBD, 0x24, 0x27, 0x19, 0x01, 0x4B };
DeviceAddress RackUp      = { 0x28, 0xC5, 0x3B, 0x2C, 0x27, 0x19, 0x01, 0x12 };
DeviceAddress XtractorIn  = { 0x28, 0x13, 0xCC, 0x23, 0x27, 0x19, 0x01, 0x09 };

const int ledverdePIN    = 39;
const int ledrojoPIN     = 41;
const int ledamarilloPIN = 43;

float tempC1;
float tempC2;
float tempC3;
float tempC4;

int h;

byte grado[8] =
 {
    0b00001100,     // Los definimos como binarios 0bxxxxxxx
    0b00010010,
    0b00010010,
    0b00001100,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000
 };

byte caraNOTOK[8] = {
B00000,
B10001,
B00000,
B00000,
B01110,
B10001,
B00000,
};

byte caraOK[8] = {
B00000,
B10001,
B00000,
B00000,
B10001,
B01110,
B00000,
};

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display


void WRITE_TEMP()
{
  digitalWrite(ledrojoPIN , HIGH);
  
  tempC1 = sensors.getTempC(XtractorIn);
  tempC2 = sensors.getTempC(RackUp);
  tempC3 = sensors.getTempC(SplitOut);
  tempC4 = sensors.getTempC(RackDown);
  Blynk.virtualWrite(V1, tempC1); // assigning incoming value from pin V1 to a variable
  Blynk.virtualWrite(V2, tempC2); // assigning incoming value from pin V1 to a variable
  Blynk.virtualWrite(V3, tempC3); // assigning incoming value from pin V1 to a variable
  Blynk.virtualWrite(V4, tempC4); // assigning incoming value from pin V1 to a variable

  h = dht.readHumidity();
  Blynk.virtualWrite(V5, h); // assigning incoming value from pin V5 to a variable
  if (tempC1>=24 || tempC2>=24 || tempC3>=24 || tempC4>=24 )
  {
     digitalWrite(ledamarilloPIN , HIGH);
  }else{
    digitalWrite(ledamarilloPIN , LOW);
  }
  digitalWrite(ledrojoPIN , LOW);
}


void setup()
{
//  pinMode(SDCARD_CS, OUTPUT);
//  digitalWrite(SDCARD_CS, HIGH); // Deselect the SD card

  pinMode(ledverdePIN , OUTPUT); 
  pinMode(ledrojoPIN , OUTPUT); 
  pinMode(ledamarilloPIN , OUTPUT); 
  
  digitalWrite(ledverdePIN , HIGH);
  
  //digitalWrite(ledamarilloPIN , HIGH);

  dht.begin();

  sensors.begin();
  sensors.setResolution(SplitOut, 10);
  sensors.setResolution(RackUp, 10);
  sensors.setResolution(RackDown, 10);
  sensors.setResolution(XtractorIn, 10);
    
  lcd.begin();
  // Print a message to the LCD.
  lcd.backlight();

  lcd.createChar (1,grado);  
  lcd.createChar (2,caraOK); 
  lcd.createChar (3,caraNOTOK); 
        
  Blynk.begin(auth, server_ip, 25600, arduino_ip, dns_ip, gateway_ip, subnet_mask, arduino_mac); // el puerto para mandar datos en local es el 8080
}


void loop()
{
  Blynk.run();
  
  // leer temperaturas
  sensors.requestTemperatures();  

  WRITE_TEMP();
  lcd.setCursor(0,0);
  lcd.print("EXTRACT: ");
  lcd.print(tempC1);
  lcd.write (byte (1));
  lcd.print("C");
  lcd.setCursor(0,1);
  lcd.print("OUT A/A: ");
  lcd.print(tempC3);
  lcd.write (byte (1));
  lcd.print("C");
  lcd.print("HUM: ");
  lcd.print(h);
  lcd.print("%");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("RACK UP :");
  lcd.print(tempC2);
  lcd.write (byte (1));
  lcd.print("C");
  lcd.setCursor(0,1);
  lcd.print("RACK DWN:");
  lcd.print(tempC4);
  lcd.write (byte (1));
  lcd.print("C");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("HUMEDAD : ");
  lcd.print(h);
  lcd.print("%");
  lcd.setCursor(0,1);
  lcd.print("ESTADO ");
  if (tempC1<24 && tempC2<24 && tempC3<24 && tempC4<24 )
  {
    lcd.write (byte(2));
    lcd.print(" OK");
  }else{
    lcd.write (byte(3));
    lcd.print(" WARNING");
  }
  delay(3000);

}
