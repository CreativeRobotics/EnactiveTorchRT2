


typedef struct espStatus_t{
  bool serverEnabled    = false;
  bool startServer      = false; //set to true to trigger server start
  bool telnetEnabled    = false;
  bool startTelnet      = false; //set to true to trigger telnet server start
  bool telnetSending    = false;
  bool wifiConnected    = false;
  bool connectToWiFi    = false;
  bool serverSending    = false;
  uint8_t wifiListIndex = 0; //used by serial commander to tell the unit to connect to a particular wifi
  unsigned long clientDisconnectTime = 0;
  unsigned long clientConnectionTimeout = 5000; //millis
  unsigned long wifiConnectTimeout = 10000; //ten seconds
  bool clientConnected  = false;
  bool telnetConnected  = false;
  bool btEnabled        = false;
  bool btConnected      = false;
  bool startBT          = false; //set to true to trigger BT start
  bool udpEnabled       = false;
  bool startUDP         = false; //set to true to trigger UDP start
  int udpPort           = 3333;
  String UDPAddress     = "192.168.0.255";
  String btName         = "ETRT-2";
  String timeServer     = "pool.ntp.org";
  long  gmtOffset_sec   = 3600;
  int   daylightOffset_sec = 0;
  int webServerPort = 80;
  int telnetPort = 23;
} espStatus_t;

espStatus_t device;

#define WIFI_LIST 16
String ctSSID = "";
String ctPswd = "";

typedef struct wifiSettings_t{
  String ssid = "";
  String pswd = "";
} wifiSettings_t;

wifiSettings_t wifiLogins[WIFI_LIST];

WiFiClient serverClient;
WiFiClient telnetClient;
String webFormSubmit = ""; //holds data sent from a client

#define SBUFFER 256
#define DEBUG Serial
bool DEBUGGING = false;
bool PRINT_CLIENT_DATA = false; //streams all the client info over the serial port
typedef enum bufferState_t{
  WAITING_FOR_START = 0,
  BUFFERING_PACKET,
  PACKET_RECEIVED,
} bufferState_t;

typedef enum serialPortMode_t{
  NO_OPERATION = 0,
  COMMAND_MODE,
  PASSTHROUGH,
  PASSTHROUGH_COMMAND,
  FILEREADER_TOSERIAL,
  FILEREADER_COMMAND
} serialPortMode_t;

typedef struct serialBuffer_t{
  Stream *port = &Serial; //initialised to something sensible . . .
  String bufferString = "";
  unsigned long bytesWritten = 0;
  bool newData = false;
  bool newLine = false;
  bool bufferFull = false;
  serialPortMode_t mode = COMMAND_MODE;
  Stream *altPort = &Serial; //for pass through - initialised to something sensible
  uint8_t commandVal = 0;
  bufferState_t parseState = WAITING_FOR_START;
  const uint8_t END_OF_PACKET = '\n';
  //File fileReader;
  bool endOfFile = false;
} serialBuffer_t;
serialBuffer_t sBuffer;


typedef bool (*commandHandler)(serialBuffer_t &);

typedef struct commander_t{
  String commandString;
  commandHandler handler;
} commander_t;





String blankPage = "<!DOCTYPE html><html><body><center><p>Enactive Torch RT Configuration page.</p></center><center><p>Reply Timeout Error 01</p></center></body></html>";


String pageMenu = "<center><a href=\"commandPage\">Command Input Page</a></center>";
String devName = "ETRT2.0 The NEw One!";
const String header = "<!DOCTYPE html><html><body>";
//String formStart = "<form action=\"/action_page.php\">";
const String configPageTitle = "<p>Enactive Torch RT Configuration page.</p>";
const String testLine1 = "<br><br><center><p>Command X received.</p></center><br></p></center>";
//const String testLine2 = "<center><form action=\"/commandPage\" method=\"get\">Command: <input type=\"text\" name=\"Command\" value=\"\" size=\"64\"> <br><br><input type=\"submit\" value=\"Submit\"></form></center>";
const String testLine2 = "<center><form>Command: <input type=\"text\" name=\"Command\" value=\"\" size=\"64\"> <br><br><input type=\"submit\" value=\"Submit\"></form></center>";
const String testLine3 = "<center><p>Click the \"Submit\" button to send the command to the Enactive Torch.</p></center>";
const String pageEnd = "</body></html>";
//action="/action_page.php" method="get"
/*
<!DOCTYPE html><html><body><p>Enactive Torch RT Configuration page.</p><br><br><center><p>Command X received.</p></center><br></p></center><center><form action="/commandPage" method="get">Command: <input type="text" name="Command" value="" size="64"> <br><br><input type="submit" value="Submit"></form></center><center><form>Command: <input type="text" name="Command" value="" size="64"> <br><br><input type="submit" value="Submit"></form></center>const String testLine3 = "<center><p>Click the "Submit" button to send the command to the Enactive Torch.</p></center></body></html>

connect to VM4789902 vrtqvfDxr8hq
serverSend:<!DOCTYPE html><html><body><p>Enactive Torch RT Configuration page.</p></body></html>
*/
