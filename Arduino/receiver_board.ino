#if 1
/*
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/esp32-esp-now-wi-fi-web-server/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/
#include <esp_now.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include "ESPAsyncWebServer.h"
#include <Arduino_JSON.h>
//#include <SoftwareSerial.h>

// Replace with your network credentials (STATION)
//const char* ssid = "extra ssid";
//const char* password = "twister56";
const char* ssid = "FAP_7BB3";
const char* password = "twister56";
typedef unsigned char UCHAR;
// Structure example to receive data
// Must match the sender structure

typedef struct struct_message {
  int id;
  float temp;
  float hum;
  unsigned int readingId;
  int extra_cmd;
} struct_message;

struct_message incomingReadings;

// server to log into 
const char* host = "192.168.88.239";	
const uint16_t port = 5193;

UCHAR msg1[30];
UCHAR msg2[200];

JSONVar board;

WiFiClient gclient;
int msg_len;
UCHAR cmd, dest;

static UCHAR pre_preamble[] = {0xF8,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00};

void send_msg(int msg_len, UCHAR *msg, UCHAR msg_type, UCHAR dest);
int get_msg(void);
/*
#define SOFTSERIAL_BAUD 9600
// Define pins for the first software serial port: TX on pin 6, RX on pin 7.
#define SS1_TX 6
#define SS1_RX 7
SoftwareSerial softSerial1(SS1_RX, SS1_TX);
*/
char xbyte = ' ';

AsyncWebServer server(80);
AsyncEventSource events("/events");
int new_data;

#endif
// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) { 
  // Copies the sender mac address to a string
/*
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
*/
	memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
	board["id"] = incomingReadings.id;
	board["temperature"] = incomingReadings.temp;
	board["humidity"] = incomingReadings.hum;
	board["readingId"] = String(incomingReadings.readingId);
	board["extra_cmd"] = String(incomingReadings.extra_cmd);
	String jsonString = JSON.stringify(board);
	events.send(jsonString.c_str(), "new_readings", millis());

	Serial.printf("\r\nBoard ID %u: %u bytes\r\n", incomingReadings.id, len);
	Serial.printf("t value: %4.2f \r\n", incomingReadings.temp);
	Serial.printf("h value: %4.2f \r\n", incomingReadings.hum);
	Serial.printf("readingID value: %d \r\n", incomingReadings.readingId);
	Serial.printf("extra cmd: %d \r\n", incomingReadings.extra_cmd);
	Serial.println();

	sprintf((char *)msg2, "%u %4.2f %4.2f %d %d",incomingReadings.id, incomingReadings.temp, incomingReadings.hum, incomingReadings.readingId, incomingReadings.extra_cmd);
	msg_len = strlen((const char *)msg2);
	if(incomingReadings.extra_cmd == 74)
		cmd = 74;
	else cmd = 73;
	send_msg(msg_len, msg2, cmd, dest);
}
#if 1
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP-NOW DASHBOARD</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p {  font-size: 1.2rem;}
    body {  margin: 0;}
    .topnav { overflow: hidden; background-color: #2f4468; color: white; font-size: 1.7rem; }
    .content { padding: 20px; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); }
    .cards { max-width: 500px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(120px, 1fr)); }
    .reading { font-size: 2.8rem; }
    .packet { color: #bebebe; }
    .card.temperature { color: #fd7e14; }
    .card.humidity { color: #1b78e2; }
  </style>
</head>
<body>
  <div class="topnav">
    <h3>ESPNOW DASHBOARD</h3>
  </div>
  <div class="content">
    <div class="cards">
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> BOARD #1 - TEMPERATURE</h4><p><span class="reading"><span id="t1"></span> &deg;C</span></p><p class="packet">Reading ID: <span id="rt1"></span></p>
      </div>
      <div class="card humidity">
        <h4><i class="fas fa-tint"></i> BOARD #1 - HUMIDITY</h4><p><span class="reading"><span id="h1"></span> &percnt;</span></p><p class="packet">Reading ID: <span id="rh1"></span></p>
      </div>
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> BOARD #2 - TEMPERATURE</h4><p><span class="reading"><span id="t2"></span> &deg;C</span></p><p class="packet">Reading ID: <span id="rt2"></span></p>
      </div>
      <div class="card humidity">
        <h4><i class="fas fa-tint"></i> BOARD #2 - HUMIDITY</h4><p><span class="reading"><span id="h2"></span> &percnt;</span></p><p class="packet">Reading ID: <span id="rh2"></span></p>
      </div>
    </div>
  </div>
<script>

if (!!window.EventSource) {
 var source = new EventSource('/events');
 
 source.addEventListener('open', function(e) {
  console.log("Events Connected");
 }, false);
 source.addEventListener('error', function(e) {
  if (e.target.readyState != EventSource.OPEN) {
    console.log("Events Disconnected");
  }
 }, false);
 
 source.addEventListener('message', function(e) {
  console.log("message", e.data);
 }, false);
 
 source.addEventListener('new_readings', function(e) {
  console.log("new_readings", e.data);
  var obj = JSON.parse(e.data);
  document.getElementById("t"+obj.id).innerHTML = obj.temperature.toFixed(2);
  document.getElementById("h"+obj.id).innerHTML = obj.humidity.toFixed(2);
  document.getElementById("rt"+obj.id).innerHTML = obj.readingId;
  document.getElementById("rh"+obj.id).innerHTML = obj.readingId;
 }, false);
}
</script>
</body>
</html>)rawliteral";
#endif
void setup() 
{
// Initialize Serial Monitor
//	softSerial1.begin(SOFTSERIAL_BAUD);
	Serial.begin(115200);
	new_data = 0;
  // Set the device as a Station and Soft Access Point simultaneously
	WiFi.mode(WIFI_AP_STA);
//  WiFi.mode(WIFI_STA);
  
  // Set device as a Wi-Fi Station
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) 
	{
		delay(1000);
		Serial.println("Setting as a Wi-Fi Station..");
	}
	Serial.print("Station IP Address: ");
	Serial.println(WiFi.localIP());
	Serial.print("Wi-Fi Channel: ");
	Serial.println(WiFi.channel());

	// Init ESP-NOW
	if (esp_now_init() != ESP_OK) 
	{
		Serial.println("Error initializing ESP-NOW");
		return;
	}

// Once ESPNow is successfully Init, we will register for recv CB to
// get recv packer info

	esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
	{
		request->send(200, "text/html", index_html);
	});

	//  server.onNotFound(notFound);
	server.begin();

	events.onConnect([](AsyncEventSourceClient *client)
	{
		if(client->lastId())
		{
			Serial.printf("Client reconnected! Last message ID that it got is: %u\n", (unsigned int)client->lastId());
		}
		// send event with message "hello!", id current millis
		// and set reconnect delay to 1 second
		client->send("hello!", NULL, millis(), 10000);
	});
	server.addHandler(&events);
	server.begin();
}

void loop() 
{
	int i;
	static unsigned long lastEventTime = millis();
	static const unsigned long EVENT_INTERVAL_MS = 5000;
	if ((millis() - lastEventTime) > EVENT_INTERVAL_MS) 
	{
		events.send("ping",NULL,millis());
		Serial.println("ping");
		lastEventTime = millis();
	}

	if (!gclient.connect(host, port)) 
	{
		Serial.println("connection failed");
		delay(5000);
		return;
	}
	// This will send a string to the server
	//  Serial.println("sending data to server");
	msg1[0] = 0;	// destination
	strcpy((char *)msg2,"wifi client\0");
	msg_len = strlen((const char *)msg2);
	strncpy((char *)(msg1+1), (const char *)msg2, msg_len);

	if (gclient.connected())
	{
		Serial.println("sending client name");
		send_msg(msg_len, msg2, 72, 5);		// 72 is SET_CLIENT_NAME
	}
	Serial.println("connected");
	cmd = 73;		// SEND_IOT_VALUES cmd
	dest = 5;
	Serial.println("start sending data");
	for(;;)
	{
		delay(100);
	}
}
#if 1
/*********************************************************************/
// get preamble & msg len from client
// preamble is: {0xF8,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,
// msg_len(lowbyte),msg_len(highbyte),0x00,0x00,0x00,0x00,0x00,0x00}
// returns message length
int get_msg(void)
{
int len;
UCHAR low, high;
int ret;
int i;
/*
UCHAR preamble[10];
ret = recv_tcp(preamble,8,1);
//	printf("ret: %d\n",ret);
if(ret < 0)
{
printf("ret < 0");
}
if(memcmp(preamble,pre_preamble,8) != 0)
{
printf("bad preamble\n");
//		uSleep(2,0);
return -1;
}
ret = recv_tcp(&low,1,1);
ret = recv_tcp(&high,1,1);
//	printf("%02x %02x\n",low,high);
len = 0;
len = (int)(high);
len <<= 4;
len |= (int)low;
*/
return len;
}
/*********************************************************************/
// send the preamble, msg len, msg_type & dest (dest is index into client table)
void send_msg(int msg_len, UCHAR *msg, UCHAR msg_type, UCHAR dest)
{
	int ret;
	int i;
	UCHAR temp[2];

	//	if(dest > MAX_CLIENTS)
	//		return;

	//	if(test_sock())
	if(1)
	{
		gclient.write((uint8_t *)&pre_preamble[0],8);
		temp[0] = (UCHAR)(msg_len & 0x0F);
		temp[1] = (UCHAR)((msg_len & 0xF0) >> 4);
		//		printf("%02x %02x\n",temp[0],temp[1]);

		gclient.write((uint8_t *)&temp[0],1);
		delay(1);
		gclient.write((uint8_t *)&temp[1],1);
		delay(1);
		gclient.write((uint8_t *)&msg_type,1);
		delay(1);
		gclient.write((uint8_t *)&dest,1);
		delay(1);

		for(i = 0;i < msg_len;i++)
		{
			gclient.write((uint8_t *)&msg[i],1);
			delay(1);
		}
	}
}
#endif
