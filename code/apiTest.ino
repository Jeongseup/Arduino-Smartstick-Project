#include <SPI.h>
#include "WizFi250.h"

char ssid[] = "Jeongseup-iphone";    // your network SSID (name)
char pass[] = "87654321g";          // your network password
int status = WL_IDLE_STATUS;       // the Wifi radio's status

//Ubidots information
#define APIKEY    "YOUR-PULIC-KEY" //공개키
#define ROUTEID   "165000012" //노선번호
#define BSTOPID "164000395" //정류장번호
#define pageNo
#define numOfRows 10

char server[] = "apis.data.go.kr";
boolean readingVal;
boolean viewData;
boolean getIsConnected = false;
String REST_STOP_COUNT;

String rcvbuf;

// Initialize the Ethernet client object
WiFiClient client;

void httpRequest();
void printWifiStatus();

//
