#include "MiniProjet.hpp"

void setup() {
  Serial.begin(9600);
  
  Serial.println("Initializing Ethernet using DHCP...");
  // Init Mini Projet (ethernet shiled and variables)
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

  long aze = micros();
  int i = mp::processHTTPRequest();
  aze = (micros() - aze) / 1000;
  
  Serial.print("\n== Done in ");
  Serial.print(aze);
  Serial.print("ms with error code ");
  Serial.println(i);
}


// ----------------------------------------------------------------------------------------------------------------------------
  

void loop() {
  mp::sync();

}
