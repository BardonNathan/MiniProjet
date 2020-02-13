#include <Ethernet.h>


/******************
 * TODO : 
 *  - Test if MP_ERR_NO_ETHERNET_SHIELD working fine
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

#define MP_ERR_CONNECTION_FAILED        6
#define MP_ERR_PROXY_CONNECTION_FAILED  7
#define MP_ERR_ALREADY_SYNC             8

#define MP_ERR_UNKNOWN_ERR             -1

#define MP_VARNAME_LED1 "led1"
#define MP_VARNAME_LED2 "led2"
#define MP_VARNAME_7SEG "7segs"

namespace mp {
  byte MAC[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
  
  // Default addresses if no DHCP server found
  IPAddress IP(192, 168, 0, 1);
  IPAddress gateway(192, 168, 0, 254);
  IPAddress subnet(255, 255, 255, 0);

  bool proxyNeeded = false;
  long nextSync = 0;

  struct {
    bool led1, led2, btn1, btn2, btn3;
    short lum, temp, pota1, pota2, pota3, segments;
  } varval;
  
  byte segs_template[] {
    0b10000010, // 0
    0b10111011, // 1
    0b10000101, // 2
    0b10010001, // 3
    0b10111000, // 4
    0b11010000, // 5
    0b11000000, // 6
    0b10011011, // 7
    0b10000000, // 8
    0b10010000  // 9
  };

  // ----------------------------------------------------------------------------------------------------------------------------

  /** Init MiniProj
    *  @return 0 on success or error code. See MP_ERR_* preprocessor defines.
    *  
    * Potential errors : MP_ERR_NO_ETHERNET_SHIELD, MP_ERR_ETHERNET_UNPLUGGED, MP_ERR_NO_DHCP (warning, not an error).
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
  /** Function used internaly to change variables values (led state and 7 seg value) after a sync.
    */
  void execVariable(String varname, String varval) {

    if (varname == MP_VARNAME_LED1)
      mp::varval.led1 = (varval == "1");
    
    else if (varname == MP_VARNAME_LED2)
      mp::varval.led2 = (varval == "1");

    else if (varname == MP_VARNAME_7SEG)
      mp::varval.segments = varval.toInt();
  }


  // ----------------------------------------------------------------------------------------------------------------------------
  /** Process an HTTP request to the server in order to sync data and push others.
    * @return 0 on success or error code. See MP_ERR_* preprocessor defines.
    * 
    * Potential errors : MP_ERR_TIMEOUT, MP_ERR_CONNECTION_FAILED, MP_ERR_PROXY_CONNECTION_FAILED.
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
    c.print("/sync.php?arduino");

    // Send sensor values
    c.print("&btn1=");  c.print(varval.btn1);
    c.print("&btn2=");  c.print(varval.btn2);
    c.print("&btn3=");  c.print(varval.btn3);
    c.print("&pota1="); c.print(varval.pota1);
    c.print("&pota2="); c.print(varval.pota2);
    c.print("&pota3="); c.print(varval.pota3);
    c.print("&temp=");  c.print(varval.temp);
    c.print("&lum=");   c.print(varval.lum);
    
    c.println(" HTTP/1.1");

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
          execVariable(varname, varval);
          
          varname = "";
          varval  = "";
          isName  = true;
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
    *  
    * Potential errors : MP_ERR_ALREADY_SYNC and processHTTPRequest()'s errors.
    */
  int sync() {
    if (nextSync > millis())
      return MP_ERR_ALREADY_SYNC;

    nextSync = millis() + MP_SYNC_DELAY;
    
    return processHTTPRequest();
  }



  
  // ----------------------------------------------------------------------------------------------------------------------------
  /** Update 7 segments display from mp::varval.segments.
    */
  void update7seg(int act1Pin, int act2Pin, int sigPin, int clkPin, int trigPin) {
    bool first = true;
    
    do {
      digitalWrite(act1Pin,  first);
      digitalWrite(act2Pin, !first);
      
      //                 get tens                       get units
      byte n = first ? (mp::varval.segments / 10) % 100 : mp::varval.segments % 10;
      n = (n < 0 || n > 9) ? 0b01111111 : segs_template[n];
      
      // For each bits, write HIGH level for a 1 or LOW for a 0 to register's SIG pin
      //                Pulse clock
      for (int i = 0 ; i < 8 ; i++) {
        digitalWrite(sigPin, (n<<i) & 0x80);
        digitalWrite(clkPin, HIGH);
        delayMicroseconds(10);
        
        digitalWrite(sigPin, LOW);
        digitalWrite(clkPin, LOW);
        delayMicroseconds(10);
      }
    
      // Trigger data to output
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
      delayMicroseconds(10);
      first = !first;
    } while (!first);

    digitalWrite(act2Pin, LOW);
  }
}
