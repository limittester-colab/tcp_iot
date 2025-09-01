#if 1
// Demonstration code to transfer data from server to client using TCP
// This code sends a 'request' in for form of a single character to the corresponding server, which responds with the value of Millis

//#include <ESP8266WiFi.h>		 for 8266
#include <WiFi.h>
#include <HTTPClient.h>

WiFiClient client_A;
//WiFiClient Client_B;

#define STASSID "Outer Limits"
#define STAPSK  "twister56"

const char * ssid = STASSID;
const char * pass = STAPSK;
IPAddress serverIP(192, 168, 88, 239);  // Replace this IP address with the IP address of your server
#define portNumber 5193
#define DATA_SIZE 200
typedef unsigned char UCHAR;

char xfer_data[DATA_SIZE];

static char pre_preamble[] = {0xF8,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00};

void send_msg(int msg_len, char *msg, char msg_type, char dest);

void setup() {
	int i;
	unsigned char xbyte = 0x7e;
    setup_serial_port();
    setup_wifi();
	for(i = 0;i < DATA_SIZE;i++)
	{
		xfer_data[i] = xbyte;
		if(--xbyte < 0x22)
			xbyte = 0x7f;
	}
	
//	for(i = 0;i < DATA_SIZE;i++)
//		Serial.print(xbyte);
}
#endif
void loop() 
{
	TCP_Client_A();
}

void TCP_Client_A() 
{
	uint16_t i;
	uint32_t currentMillis = millis();
	static uint32_t lastMillis;
	const uint32_t interval = 5000;
	char tempx[30];
	char tempx2[5];
	int msg_len;

	if (!client_A.connected()) 
	{
		if (client_A.connect(serverIP, portNumber)) 
		{                                         // Connects to the server
			//Serial.print("Connected to Gateway IP = "); Serial.println(serverIP);
		} else 
		{
			//Serial.print("Could NOT connect to Gateway IP = "); Serial.println(serverIP);
			delay(500);
		}
	} else 
	{
		strcpy((char *)&tempx[0],"wifi client\0");
		msg_len = strlen((const char *)tempx);

		if (client_A.connected())
		{
			Serial.println("sending client name");
			send_msg(msg_len, tempx, 72, 5);		// 72 is SET_CLIENT_NAME
		}
		delay(5000);

//		Serial.print(client_A.available());
//		while (client_A.available())
		msg_len = strlen(tempx);
		i = 0;
		while(1)
		{
			bzero(tempx2,sizeof(tempx2));
			bzero(tempx,sizeof(tempx));
			strcpy(&tempx[0],"test asdf \0");
			itoa(i, tempx2, 10);
//			Serial.println(tempx2);
			strcat(tempx,tempx2);
			Serial.println(tempx);
			msg_len = strlen(tempx);
			send_msg(msg_len, tempx, 73, 5);
			delay(5000);
			i++;
//			Serial.printf("%d\n",i);
		}
			//Serial.write(client_A.read());
			// Receives data from the server and sends to the serial port
		if (currentMillis - lastMillis >= interval) 
		{
			// Sends the letter A (could be anything) to the server once every 'interval'
			lastMillis += interval;
//			client_A.print("Abc");
		}
		delay(1000);
	}
}

//Setup functions
void setup_serial_port() 
{
	uint32_t baudrate = 115200;
	//uint32_t baudrate = 9600;
	Serial.begin(baudrate);
	Serial.println("");
	Serial.print("Serial port connected: ");
	Serial.println(baudrate);
}

void setup_wifi() 
{
	uint8_t wait = 0;

	IPAddress local_IP(192, 168, 88, 237);
	// Set your Gateway IP address
	IPAddress gateway(192, 168, 88, 1);

	IPAddress subnet(255, 255, 255, 0);
	IPAddress primaryDNS(8, 8, 8, 8);   //optional
	IPAddress secondaryDNS(8, 8, 4, 4); //optional

	// Configures static IP address
	if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) 
	{
		Serial.println("STA Failed to configure");
	}

	while (WiFi.status() != WL_CONNECTED) 
	{
		if (wait) 
		{
			--wait;
		} else 
		{
			wait = 40;
			WiFi.mode(WIFI_STA);
			WiFi.begin(ssid, pass);
		}
		delay(100);
	}
	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
}

void send_msg(int msg_len, char *msg, char msg_type, char dest)
{
	int ret;
	int i;
	char temp[4];

	//	if(dest > MAX_CLIENTS)
	//		return;

	//	if(test_sock())
	if(1)
	{
		client_A.write((uint8_t *)&pre_preamble[0],8);
		temp[0] = (UCHAR)(msg_len & 0x0F);
		temp[1] = (UCHAR)((msg_len & 0xF0) >> 4);
		temp[2] = msg_type;
		temp[3] = dest;
		client_A.write((uint8_t *)&temp[0],4);
/*
		client_A.write((uint8_t *)&temp[0],1);
//		Serial.printf("%02x ",temp[0]);
		client_A.write((uint8_t *)&temp[1],1);
//		Serial.printf("%02x ",temp[1]);
		client_A.write((uint8_t *)&msg_type,1);
//		Serial.printf("%02x ",msg_type);
		client_A.write((uint8_t *)&dest,1);
//		Serial.printf("%02x ",dest);
*/
		for(i = 0;i < msg_len;i++)
		{
			client_A.write((uint8_t *)&msg[i],1);
//			Serial.printf("%02x ",msg[i]);
		}
//		Serial.println("");
	}
}
