#include <WiFi.h>
#include <esp_now.h>
#include "SD.h"
#include <MicroNMEA.h>
#include "FS.h"
#include "SPI.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define VERSION "0.0.1"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// pinout
// PIN 5 - SD CS
// PIN 19 - SD MISO
// PIN 23 - SD MOSI
// PIN 18 - SD CLK
// PIN 22 - SCL
// PIN 21 - SDA
// PIN 16 - GPS TX
// PIN 17 - GPS RX

//Must match the receiver structure
typedef struct struct_message {
    char mac[6];
    char name[32];
} struct_message;

/*---------------------------------------------------------------------------------*/
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//These variables are used for buffering/caching GPS data.
char nmeaBuffer[100];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));
File filewriter;
struct_message READINGS;
/*---------------------------------------------------------------------------------*/
void setup_wifi(){
  //Gets the WiFi ready for scanning by disconnecting from networks and changing mode.
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
}

void clear_display(){
  //Clears the LCD and resets the cursor.
  display.clearDisplay();
  display.setCursor(0, 0);
}

// call back for ESPNow Data Sent
void data_sent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Callback when data is received
void data_receive(const uint8_t * mac, const uint8_t *incomingData, int len) {
  //memcpy(&sender_Readings, incomingData, sizeof(sender_Readings));
  Serial.print("Bytes received: ");
  Serial.println(len);
}

void setup() {
  // Debug Serial
  Serial.begin(115200);
  // GPS Serial
  Serial2.begin(9600);
  // Startup the Display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
  }
  display.setRotation(0);
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.println("Starting");
  display.print("Version ");
  display.println(VERSION);
  display.display();
  // Setup the SDCard
  if(!SD.begin()){
      Serial.println("SD Begin failed!");
      clear_display();
      display.println("SD Begin failed!");
      display.display();
      delay(4000);
  }
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE){
      Serial.println("No SD card attached!");
      clear_display();
      display.println("No SD Card!");
      display.display();
      delay(10000);
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  // Make sure the SDCard is writable.
  while (!filewriter){
    filewriter = SD.open("/test.txt", FILE_APPEND);
    if (!filewriter){
      Serial.println("Failed to open file for writing.");
      clear_display();
      display.println("SD File open failed!");
      display.display();
      delay(1000);
    }
  }
  int wrote = filewriter.print("\n_BOOT_");
  filewriter.print(VERSION);
  filewriter.flush();
  if (wrote < 1){
    while(true){
      Serial.println("Failed to write to SD card!");
      clear_display();
      display.println("SD Card write failed!");
      display.display();
      delay(4000);
    }
  }
  // setup ESP Now
  if ( esp_now_init() != ESP_OK )
  {
    Serial.println("Erorr setting up ESPNOW");
  }
  esp_now_register_send_cb(data_sent);
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;   
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(data_receive);


  // Serial.println("Opening destination file for writing");
  // String filename = "/wd3-";
  // filename = filename + bootcount;
  // filename = filename + ".csv";
  // Serial.println(filename);
  // filewriter = SD.open(filename, FILE_APPEND);
  // filewriter.print("WigleWifi-1.4,appRelease=" + VERSION + ",model=wardriver.uk Rev3 ESP32,release=1.0.0,device=wardriver.uk Rev3 ESP32,display=i2c LCD,board=wardriver.uk Rev3 ESP32,brand=JHewitt\nMAC,SSID,AuthMode,FirstSeen,Channel,RSSI,CurrentLatitude,CurrentLongitude,AltitudeMeters,AccuracyMeters,Type\n");
  // filewriter.flush();
  
  // clear_display();
  // display.println("Starting main..");
  // display.display();

  // xTaskCreatePinnedToCore(
  //   primary_scan_loop, /* Function to implement the task */
  //   "primary_scan_loop", /* Name of the task */
  //   10000,  /* Stack size in words */
  //   NULL,  /* Task input parameter */
  //   3,  /* Priority of the task */
  //   &primary_scan_loop_handle,  /* Task handle. */
  //   0); /* Core where the task should run */
}

void loop() {
  memcpy(READINGS.mac, "00:01:02:03:04:05", 6);
  memcpy(READINGS.name, "Test", 4);
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &READINGS, sizeof(READINGS));

}

void do_scan()
{
  int n = WiFi.scanNetworks(false, true);
  
}

