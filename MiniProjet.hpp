#include <Ethernet.h>


/******************
 * TODO : 
 *  - At the end of init(), sync ip addr with server
 *  - When HTTP req failed => retry with 5 times.
 *  -     \ If 5 fails occurs, stop program check why (no shield, Ethernet unplugged..) and inform the user
 *  - After each http requests, exec updateActuators(bool led1, led2, led3, int 7seg...)
 *  - Process an http requests each .5 secs (mesure response time) if user dont proccess one last .5s
 *  - At the end of the init() func, check if proxy if needed (process http without proxy and check if response gone)
 */


#define MP_SRV "branly.lazyfox.fr"
//#define MP_SRV "192.168.1.74"
#define MP_PORT 80

#define MP_PROXY_SRV "172.22.16.168"
#define MP_PROXY_PORT 80

#define MP_TIMEOUT 5000
#define MP_SYNC_DELAY 500 // delay between 2 syncs

#define MP_ERR_NO_ETHERNET_SHIELD       1
#define MP_ERR_ETHERNET_UNPLUGGED       2
#define MP_ERR_NO_DHCP                  3
#define MP_ERR_TIMEOUT                  4
#define MP_ERR_BUFFER_OVERFLOW          5
#define MP_ERR_CONNECTION_FAILED        6
#define MP_ERR_PROXY_CONNECTION_FAILED  7
#define MP_ERR_ALREADY_SYNC             8

#define MP_ERR_UNKNOWN_ERR             -1

namespace mp {
  byte MAC[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
  
  // Default addresses if no DHCP server found
  IPAddress IP(192, 168, 0, 1);
  IPAddress gateway(192, 168, 0, 254);
  IPAddress subnet(255, 255, 255, 0);

  bool proxyNeeded = false;
  long lastSync = 0;

  // ----------------------------------------------------------------------------------------------------------------------------

  /** Init MiniProj
    *  @return 0 on success or error code. See MP_ERR_* preprocessor defines.
    */
  int init() {
    if (!Ethernet.begin(MAC)) {
      if (Ethernet.hardwareStatus() == EthernetNoHardware)
        return MP_ERR_NO_ETHERNET_SHIELD;
      
      if (Ethernet.linkStatus() == LinkOFF)
        return MP_ERR_ETHERNET_UNPLUGGED;
      
      
      Ethernet.begin(MAC, IP, gateway, gateway, subnet);
      return MP_ERR_NO_DHCP;
    }
    
    delay(10);
    return 0;
  }



  // ----------------------------------------------------------------------------------------------------------------------------
  /** Process an HTTP request to the server in order to sync data and push others.
    * @return 0 on success or error code. See MP_ERR_* preprocessor defines.
    * 
    * Potential errors : MP_ERR_TIMEOUT, MP_ERR_BUFFER_OVERFLOW, MP_ERR_CONNECTION_FAILED.
    */
  int processHTTPRequest() {
    EthernetClient c;
    if (proxyNeeded) {
      if (!c.connect(MP_PROXY_SRV, MP_PROXY_PORT))
        return MP_ERR_PROXY_CONNECTION_FAILED;
        
    } else {
      if (!c.connect(MP_SRV, MP_PORT))
        return MP_ERR_CONNECTION_FAILED;
    }
    

    // Send HTTP request
    c.print("GET http://");
    c.print(MP_SRV);
    c.println("/sync.php HTTP/1.1");

    c.print("Host: ");
    c.println(MP_SRV);

    c.println("User-Agent: Arduino WarningShield");
    c.println("Connection: close");
    c.println();

    // Read HTTP request
    
    // Wait for data. Return MP_ERR_TIMEOUT if no data is received during MP_TIMEOUT milliseconds.
    long t = millis() + MP_TIMEOUT;
    while (!c.available()) {
      if (millis() > t)
        return MP_ERR_TIMEOUT;
    }


    // Wait for a # next to a line feed
    do {
      while (c.read() != '\n');
    } while (c.read() !=  '#');

    String varname, varval;
    bool isName = true; // When true=Reading the name ; when false=Reading the value
    char cbuff;
    
    // Read while stream isnt finished and the caracter read isnt \r or \n
    while ((cbuff = c.read()) != -1 && cbuff != '\r' && cbuff != '\n') {
      
      // Parse variable names and values
      if (isName) {
        if (cbuff == '=')
          isName = false;
        else
          varname += (char)cbuff;
          
      } else {
        if (cbuff == ';') {  
          varname = "";
          varval  = "";
          isName  = true;

          execVariable(varname, varval);
        } else
          varval += (char)cbuff;
      }
    }

    
    c.stop();
    return 0;
  }
  
  // ----------------------------------------------------------------------------------------------------------------------------
  /** Synchronize local sensors with server and server's data with actuators.
    *  @return 0 on success or error code. See MP_ERR_* preprocessor defines.
    */
  int sync() {
    if (nextSync > millis())
      return MP_ERR_ALREADY_SYNC;

    nextSync = millis() + MP_SYNC_DELAY;
    
    processHTTPRequest();
  }
}
