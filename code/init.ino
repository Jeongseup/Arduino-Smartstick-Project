#include <SPI.h>
#include "WizFi250.h"

char ssid[] = "Jeongseup-iphone";    // your network SSID (name)
char pass[] = "87654321g";          // your network password
int status = WL_IDLE_STATUS;       // the Wifi radio's status

//Ubidots information
#define APIKEY  "YOUR-PULIC-KEY" //공개키
#define ROUTEID "165000012" 
#define BSTOPID "164000395" 


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
void Warning();


//진동모터
#define SWITCH 6
#define Vibration 7
#define Vibration_Length 100


// 부저
#define ALARM 5
#define ALARM_Length 100                
#define ALARM_bHz 1000
#define Basic_Delay 100
int ALARM_Hz = 1500;

// 초음파센서
#define TrigPin 4 //검정
#define EchoPin 3 // 갈색 
#define DISTANCE_DATA 5


void setup()
{
  // put your setup code here, to run once:
  //기본 셋팅
  Serial.begin(115200);
  Serial.println(F("\r\nSerial Init"));
 
  pinMode(SWITCH, INPUT_PULLUP);
  pinMode(EchoPin,INPUT);
  pinMode(TrigPin,OUTPUT);
  pinMode(Vibration,OUTPUT);
  pinMode(ALARM,OUTPUT);
  
  WiFi.init();
  
  // check for the presence of the shield 쉴드 상태
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network  Wifi 상태
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("You're connected to the network");

  printWifiStatus();  
}

void loop()
{
  Warning();
  while (client.available()) {
    char c = client.read();
    if ( c != NULL ) {
      if (rcvbuf.length() > 130)
        rcvbuf = "";
      rcvbuf += c;
      Serial.write(c);
     
      if (rcvbuf.endsWith("<REST_STOP_COUNT>")) {
        readingVal = true;
        REST_STOP_COUNT = "";
      }
      
      if (readingVal) 
        dataParser(c, REST_STOP_COUNT, readingVal);
    }
  }
  Serial.print(digitalRead(SWITCH));

  
  if(!digitalRead(SWITCH)){
    httpRequest();
    Serial.print("REST_STOP_COUNT : ");
    Serial.println(REST_STOP_COUNT);

    int R = REST_STOP_COUNT.toInt();
    delay(3000);    
    for( int i=0 ; i < R; i++)
    {
      digitalWrite(Vibration,HIGH); 
      delay(Vibration_Length); 
      digitalWrite(Vibration,LOW); 
      delay(500);
    }
    delay(3000);
  }
  rcvbuf = "";
}
  
void httpRequest() {
  Serial.println();

  // close any connection before send a new request
  // this will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection
  if (client.connect(server, 80)) {
    Serial.println("Connecting...");

    // send the HTTP PUT request
    client.print(F("GET /6280000/busArrivalService/getBusArrivalList?serviceKey="));
    client.print(APIKEY);
    client.print(F("&pageNo=1&numOfRows=10&bstopId="));
                                    //bstopId=164000395&routeId=165000012
                                    //#define ROUTEID "165000012" 
                                    //#define BSTOPID "164000395" 
    client.print(BSTOPID);
    client.print(F("&routeId="));
    client.print(ROUTEID);
    
    client.print(F(" HTTP/1.1\r\n"));
    client.print(F("Host: apis.data.go.kr\r\n"));
    client.print(F("Connection: close\r\n"));
    client.print(F("\r\n\r\n"));

  }
  else {
    // if you couldn't make a connection
    Serial.println("Connection failed");
  }
}


void printWifiStatus() {
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void dataParser(char c, String &data, boolean &b) {
  if (c != '<') {   
    if (c != '>')
      data += c;
  }
  else {
    b = false;
  }
}

void Warning()
{
    //초음파센서
  long duration, distance;
  float DATA_ARRANGE[DISTANCE_DATA] = {};
  int SUM = 0;
  
 // 거리데이터 만들기
  for( int i = 0; i < DISTANCE_DATA; i++)
  {
    digitalWrite(TrigPin,HIGH);
    delayMicroseconds(10);
    digitalWrite(TrigPin,LOW);
    
    duration = pulseIn(EchoPin,HIGH);
    DATA_ARRANGE[i] = ((float)(340 * duration) / 10000) / 2;
    Serial.print("거리 데이터 배열 : ");
    Serial.print(DATA_ARRANGE[i]);
    Serial.println(" ");
  }

// 측정치의 정확도 향상을 위한 알고리즘 
// '20% 절삭평균' 이용

  int SAVE_MIN = DATA_ARRANGE[0];
  int SAVE_MAX = DATA_ARRANGE[4];
  
  for( int x = 0; x < DISTANCE_DATA; x++)
  {
    if(SAVE_MIN > DATA_ARRANGE[x])
    {
        SAVE_MIN = DATA_ARRANGE[x];
     }
    if(SAVE_MAX < DATA_ARRANGE[x])
    {
        SAVE_MAX = DATA_ARRANGE[x];
    }
        SUM = SUM + DATA_ARRANGE[x];
    
  }
  distance = (SUM - SAVE_MAX -SAVE_MIN) / 3;

  if(distance>200)
  {
    distance = 0;
  }
  Serial.print("알고리즘 후 거리:  ");
  Serial.println(distance);
  Serial.print("현재 Hz: ");
  Serial.println(ALARM_Hz);

  
// 거리에 따른 알람소리 알고리즘
 if(distance<150 && distance>=100)     
  {
    digitalWrite(Vibration,HIGH); delay(100); digitalWrite(Vibration,LOW);    
    tone(ALARM,ALARM_Hz,ALARM_Length);delay(1000);
     
  }
  else if(distance<100 && distance>=50)
  {
    digitalWrite(Vibration,HIGH); delay(100); digitalWrite(Vibration,LOW);     
    tone(ALARM,ALARM_Hz,ALARM_Length);delay(500);

    
  }
  else if(distance<50 && distance>=15)
  { 
    digitalWrite(Vibration,HIGH); delay(100);   digitalWrite(Vibration,LOW); 
    tone(ALARM,ALARM_Hz,ALARM_Length);delay(300);

  }
  else if(distance<15 && distance>=2)
  { 
    digitalWrite(Vibration,HIGH); delay(100); digitalWrite(Vibration,LOW); 
    tone(ALARM,ALARM_Hz,ALARM_Length);delay(100);

  }
  else if(distance<2)
  { 
    delay(100);
  }
}
