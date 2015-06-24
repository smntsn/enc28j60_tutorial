// WebRelay Module by lucadentella https://github.com/lucadentella/enc28j60_tutorial/blob/master/_9_WebRelay
// Fixed and re-edited for HIGHasLOW relay boards and added DHCP support by smntsn
// For one relay module. 

#include <EtherCard.h>
#include <IPAddress.h>
#define RELAY_PIN  2

// DHCP Support starts here
#define STATIC 0  // set to 1 to disable DHCP (adjust myip/gwip values below)

#if STATIC 
// ethernet interface ip address
static byte myip[] = { 192,168,1,200 };
// gateway ip address
static byte gwip[] = { 192,168,1,1 };
#endif

static byte mymac[] = { 0x70,0x69,0x69,0x2D,0x30,0x31 };
// static byte myip[]   = {192,168,1,113};
byte Ethernet::buffer[700];

char* on  = "ON";
char* off = "OFF";

boolean relayStatus;
char* relayLabel;
char* linkLabel;

void setup () {
 
  Serial.begin(57600);
  Serial.println("WebRelay Demo");

  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) 
    Serial.println( "Failed to access Ethernet controller");
#if STATIC
  ether.staticSetup(myip, gwip);
#else
  if (!ether.dhcpSetup())
    Serial.println("DHCP failed");
#endif

    ether.printIp("IP:  ", ether.myip);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  
  relayStatus = true;
  relayLabel = off;
  linkLabel = on;
}
  
void loop() {
 
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);
  
  if(pos) {
        
    if(strstr((char *)Ethernet::buffer + pos, "GET /?ON") != 0) {
      relayStatus = false;
      relayLabel = on;
      linkLabel = off;
    } else if(strstr((char *)Ethernet::buffer + pos, "GET /?OFF") != 0) {
      relayStatus = true;
      relayLabel = off;
      linkLabel = on;
    }
    digitalWrite(RELAY_PIN, relayStatus); 
    
    BufferFiller bfill = ether.tcpOffset();
    bfill.emit_p(PSTR("HTTP/1.0 200 OK\r\n"
      "Content-Type: text/html\r\nPragma: no-cache\r\n\r\n"
      "<html><head><meta name='viewport' content='width=200px'/></head><body>"
      "<div style='position:absolute;width:200px;height:200px;top:50%;left:50%;margin:-100px 0 0 -100px'>"
      "<div style='font:bold 14px verdana;text-align:center'>Relay is $S</div>"
      "<br><div style='text-align:center'>"
      "<a href='/?$S'><img src='http://www.lucadentella.it/files/bt_$S.png'></a>"
      "</div></div></body></html>"
      ), relayLabel, linkLabel, linkLabel);

      ether.httpServerReply(bfill.position());
    }
}
