#if 1
/*
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/esp32-esp-now-wi-fi-web-server/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
//#include <Adafruit_Sensor.h>
//#include <DHT.h>

// Set your Board ID (ESP32 Sender #1 = BOARD_ID 1, ESP32 Sender #2 = BOARD_ID 2, etc)
#define BOARD_ID 2

// Digital pin connected to the DHT sensor
#define DHTPIN 4  
#define PUSHBUTTON D0
int buttonstate;

// Uncomment the type of sensor in use:
//#define DHTTYPE    DHT11     // DHT 11
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

//DHT dht(DHTPIN, DHTTYPE);

//MAC Address of the receiver 
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
float h = 0.02;
float t = 0.01;
int skip;
//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    int id;
    float temp;
    float hum;
    int readingId;
	int extra_cmd;
} struct_message;

esp_now_peer_info_t peerInfo;

//Create a struct_message called myData
struct_message myData;

unsigned long previousMillis = 0;   // Stores last time temperature was published
//const long interval = 2100;        // Interval at which to publish sensor readings
const long interval = 101000;        // Interval at which to publish sensor readings

unsigned int readingId = 0;

// Insert your SSID
constexpr char WIFI_SSID[] = "FAP_7BB3";

int32_t getWiFiChannel(const char *ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
      for (uint8_t i=0; i<n; i++) {
          if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
              return WiFi.channel(i);
          }
      }
  }
  return 0;
}
#endif
float readDHTTemperature(float t) {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Celsius (the default)
  //float t = dht.readTemperature();
  float t2 = t + 0.03;
  if(t2 > 20)
	  t2 = 0.01;
  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float t = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(t2)) {    
    Serial.println("Failed to read from DHT sensor!");
    return 0;
  }
  else {
    Serial.println(t2);
    return t2;
  }
}

float readDHTHumidity(float h) {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  //float h = dht.readHumidity();
  h = h + 0.05;
  if(h > 15)
	  h = 0.01;
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return 0;
  }
  else {
    Serial.println(h);
//	softSerial1.println(h);
    return h;
  }
}

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

/*	debounce
if (digitalRead(BUTTON_PIN) != lastButtonState) {
  lastDebounceTime = millis();  // Reset timer on change
}

if ((millis() - lastDebounceTime) > debounceDelay) {
  if (digitalRead(BUTTON_PIN) == LOW) {
    // Button is stable and pressed
  }
}
lastButtonState = digitalRead(BUTTON_PIN);
*/

void Handler()
{
	buttonstate = digitalRead(PUSHBUTTON);
	if(buttonstate == HIGH)
	{
		myData.extra_cmd = 74;
		myData.temp = 0;
		myData.hum = 0;
		myData.readingId = 3;
		myData.id = 3;
		esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
/*
		if (result == ESP_OK) 
		{
		Serial.println("extra cmd Sent with success");
		}
		else 
		{
		Serial.println("Error sending the extra cmd");
		}
*/
	}
}
 
void setup() {
  //Init Serial Monitor
  Serial.begin(115200);
  //dht.begin();
   pinMode(PUSHBUTTON,INPUT);
   attachInterrupt(digitalPinToInterrupt(D0), Handler, RISING);
   buttonstate = 0;
  // Set device as a Wi-Fi Station and set channel
  WiFi.mode(WIFI_STA);
  skip = 0;
  int32_t channel = getWiFiChannel(WIFI_SSID);

  WiFi.printDiag(Serial); // Uncomment to verify channel number before
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  WiFi.printDiag(Serial); // Uncomment to verify channel change after

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  //esp_now_register_send_cb(OnDataSent);
  esp_now_register_send_cb((esp_now_send_cb_t)OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}
 
void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // Save the last time a new reading was published
    previousMillis = currentMillis;
    //Set values to send
    myData.id = BOARD_ID;
    t = myData.temp = readDHTTemperature(t);
    h = myData.hum = readDHTHumidity(h);
    myData.readingId = readingId++;
	myData.extra_cmd = 0;
    //Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }
  }
}

