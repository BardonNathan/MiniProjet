#include <Ethernet.h>


/******************
 * TODO : 
 *  - At the end of init(), sync ip addr with server
 *  - When HTTP req failed => retry 5 times.
 *  -     \ If 5 fails occurs, stop program and tell user
 *  - After each http requests, exec updateActuators(bool led1, led2, led3, int 7seg...)
 *  - Process an http requests each .5 secs (mesure response time) if user dont proccess one last .5s
 *  - At the end of the init() func, check if proxy if needed (process http without proxy and check if response gone)
 */

#define MP_ERR_NO_ETHERNET_SHIELD 1
#define MP_ERR_ETHERNET_UNPLUGGED 2

namespace mp {
  byte MAC[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
  
  // Default addresses if no DHCP server found
  IPAddress IP(192, 168, 0, 1);
  IPAddress gateway(192, 168, 0, 254);
  IPAddress subnet(255, 255, 255, 0);

  // ----------------------------------------------------------------------------------------------------------------------------

  /** Init MiniProj
    * @return 0 on success or error code. See MP_ERR_* preprocessors defines.
    */
  int init() {
    Serial.println("Initializing Ethernet using DHCP...");
    
    if (!Ethernet.begin(MAC)) {
      
      if (Ethernet.hardwareStatus() == EthernetNoHardware) {
        Serial.println("Ethernet shield was not found.");
        return MP_ERR_NO_ETHERNET_SHIELD;
      }
      
      if (Ethernet.linkStatus() == LinkOFF) {
        Serial.println("Ethernet cable is not connected.");
        return MP_ERR_ETHERNET_UNPLUGGED;
      }
      
      
      Serial.println("No DHCP response, using default IP configuration.");
      Ethernet.begin(MAC, IP, gateway, gateway, subnet);
    }

    Serial.println("== IP configuration ==");
    Serial.print("IP address : "); Serial.println(Ethernet.localIP());
    Serial.print("Mask : "); Serial.println(Ethernet.subnetMask());
    Serial.print("Gateway : "); Serial.println(Ethernet.gatewayIP());
    Serial.print("DNS Server : "); Serial.println(Ethernet.dnsServerIP());
    return 0;
  }


  // ----------------------------------------------------------------------------------------------------------------------------
  /** Process an HTTP request to the server in order to sync data and push others.
   *  @return 0 or error code. See MP_ERR_* preprocessor defines.
   */
  int processHTTPRequest() {



    
  }
}
