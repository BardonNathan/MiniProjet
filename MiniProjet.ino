#include "MiniProjet.hpp"

#define PIN_LED1  5
#define PIN_LED2  6
#define PIN_BTN1  0
#define PIN_BTN2  0
#define PIN_BTN3  0

#define PIN_POTA3 A1
#define PIN_POTA2 A0
#define PIN_POTA1 A2
#define PIN_LUM   A3
#define PIN_TEMP  A4

#define PIN_7SEGA 9
#define PIN_7SEGB 2
#define PIN_REG_SIN 3
#define PIN_REG_CLK 8
#define PIN_REG_TRG 7

void setup() {
  Serial.begin(9600);

  Serial.println("Initializing Ethernet using DHCP...");

  switch (mp::init()) {
    case MP_ERR_NO_ETHERNET_SHIELD:
      Serial.println("Ethernet shield was not found.");
      while(1); // Block program
      
    case MP_ERR_ETHERNET_UNPLUGGED:
      Serial.println("Ethernet cable is not connected."); 
      while(1); // Block program
      
    case MP_ERR_NO_DHCP:
      Serial.println("No DHCP found. Using default IP configuration.");
  }


  Serial.println("== IP configuration ==");
  Serial.print("IP address : "); Serial.println(Ethernet.localIP());
  Serial.print("Mask : ");       Serial.println(Ethernet.subnetMask());
  Serial.print("Gateway : ");    Serial.println(Ethernet.gatewayIP());
  Serial.print("DNS Server : "); Serial.println(Ethernet.dnsServerIP());

  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  pinMode(PIN_REG_SIN, OUTPUT);
  pinMode(PIN_REG_CLK, OUTPUT);
  pinMode(PIN_REG_TRG, OUTPUT);
  pinMode(PIN_BTN1, INPUT_PULLUP);
  pinMode(PIN_BTN2, INPUT_PULLUP);
  pinMode(PIN_BTN3, INPUT_PULLUP);
}


// ----------------------------------------------------------------------------------------------------------------------------
  

void loop() {
  long uptimems = millis();

  int sync_result = mp::sync();
  if (sync_result == 0) {
    digitalWrite(PIN_LED1, mp::varval.led1);
    digitalWrite(PIN_LED2, mp::varval.led2);
  
  
  }
  
  if (sync_result != MP_ERR_ALREADY_SYNC) {
    Serial.print("Synchronized in ");
    Serial.print(millis() - uptimems);
    Serial.print("ms with error code ");
    Serial.println(sync_result);
  }
  
  // Update sensor values
  mp::varval.temp  = (analogRead(PIN_TEMP) *5 * 110)+ 50 / 1000;
  mp::varval.lum   = analogRead(PIN_LUM) / 10;
  
  mp::varval.pota1 = analogRead(PIN_POTA1) / 10; // 0-1024 => 0-100
  mp::varval.pota2 = analogRead(PIN_POTA2) / 10;
  mp::varval.pota3 = analogRead(PIN_POTA3) / 10;
  
  mp::varval.btn1  = digitalRead(PIN_BTN1);
  mp::varval.btn2  = digitalRead(PIN_BTN2);
  mp::varval.btn3  = digitalRead(PIN_BTN3);

  
  mp::update7seg(PIN_7SEGA, PIN_7SEGB, PIN_REG_SIN, PIN_REG_CLK, PIN_REG_TRG);
}
