//Libraries for LoRa
#include <SPI.h>
#include <LoRa.h>
#include <SimpleDHT.h>
#include <Math.h>
//Libraries for OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Define the pins used by the LoRa transceiver module
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26
//DHT22 sensor definitions
byte pinDHT22 = 25;
SimpleDHT22 dht22;

//433E6 for our TTGO device
#define BAND 433E6

//OLED pins
#define OLED_SDA 21
#define OLED_SCL 22 
#define OLED_RST 16
#define SCREEN_WIDTH 128 //OLED display width, in pixels
#define SCREEN_HEIGHT 64 //OLED display height, in pixels
//RTC_DATA_ATTR used to save the variables in the RTC memory to keep them from deletion when going into deep sleep mode
//Default GPS coordinates
RTC_DATA_ATTR float gps1_lat = 45.838935;
RTC_DATA_ATTR float gps1_long = 12.339376;
int ID = 1;
//Define variables for the SOH
RTC_DATA_ATTR float max_temp=-100;
RTC_DATA_ATTR float min_temp=100;
RTC_DATA_ATTR float max_moi=1;
RTC_DATA_ATTR float min_moi=100;
RTC_DATA_ATTR float avg_temp;
RTC_DATA_ATTR float avg_moi;
float tempSOH;
float moiSOH;
float SOH;
//Frequency of sampling defined
RTC_DATA_ATTR int freq=30;
float hum = 0;
float temp = 0;
//message definition
String message;
//Define deep sleep options
uint64_t uS_TO_S_FACTOR = 1000000;  //Conversion factor for micro seconds to seconds
//Sleep for "freq" seconds
uint64_t TIME_TO_SLEEP = freq;
//Moisture Sensor variables
const int moisturePin = 35;
RTC_DATA_ATTR float soilMoisture;
int read_m;
//getConfigData function variables
String deviceID;
String new_lat;
String new_long;
String new_freq;
uint32_t period = 0.2 * 60000L;  // listening for 12 seconds is the default

//Definition for display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);
  //These functions can be used to set LoRa paramenters differently from default values 
  //these down here are the default ones
  //LoRa.setSpreadingFactor(7);
  //LoRa.setSignalBandwidth(125000);
  //LoRa.setCodingRate4(5);

  // Enable Timer wake_up
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  //reset OLED display via software
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { //Address 0x3C for 128x32
    //Serial.println(F("SSD1306 allocation failed"));
    for(;;); //Don't proceed, loop forever
  }
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Station1");
  display.setCursor(0,20);
  display.setTextSize(1);
  display.display();
////////////////////////LORA///////////////////////////////
  //SPI LoRa pins
  SPI.begin(SCK, MISO, MOSI, SS);
  //Setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);
  
  int counter = 0;  
  while (!LoRa.begin(BAND) && counter < 10) {
    //Serial.print(".");
    counter++;
    delay(500);
  }
  if (counter == 10) {
    // Start deep sleep
    //Serial.println("Failed to initialize LoRa. Going to sleep now");
    esp_deep_sleep_start();
  }
  //The sync word assures we don't get LoRa messages from other LoRa transceivers
  //ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  //Serial.println("LoRa initializing OK!");
  getReadings();
  //Serial.print("Soil moisture = ");
  //Serial.println(soilMoisture);
  sendReadings();
  //Serial.print("Message sent = ");
  //Serial.println(message);


//OLED
  display.print("LoRa packet sent.");   
  display.setCursor(0,40);
  display.print(message);      
  display.display();
//Start listening for maximum 12 seconds (default) for an eventual configuration packet coming from the Arduino
  for( uint32_t tStart = millis();  (millis()-tStart) < period;  ){
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      if(getConfigData()==1){ //if the function returns 1 then it's time to stop listening
        break;
      }
    }
  }

  
  //Start deep sleep
  //Serial.println("Going to sleep now.");
  esp_deep_sleep_start(); 
}
void loop() {
//The ESP32 will be in deep sleep
//it will never reach this loop
}

void getReadings() {
  //Measure moisture
  read_m = analogRead(moisturePin);
  //Serial.print("m=");
  //Serial.print(read_m);
  if (read_m>=4095){
   soilMoisture=0; //max value means no moisture
  }
  if (read_m<=3100){ //3100 arbitrary parameter calibrated from experiments to be 100% moisture
    soilMoisture=100;
  }
  else {
    soilMoisture=100*(1-(float)(read_m-3100)/(4095-3100)); //percentage of moisture calculated with linear interpolation  
  }
  //Measure temperature and humidity of the air
  dht22.read2(pinDHT22, &temp, &hum, NULL);
  //Serial.print("t= ");
  //Serial.print(temp);
  //Serial.print(" ");
  //Serial.print("h= ");
  //Serial.print(hum);
  //Serial.print(read_m);
  delay(500);
  if (isnan(hum) || isnan(temp)) {
    //Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  if (temp>max_temp){
    max_temp=temp;
  }
  if (temp<min_temp){
    min_temp=temp;
  }
   if (soilMoisture>max_moi){
    max_moi=soilMoisture;
  }
  if (soilMoisture<min_moi){
    min_moi=soilMoisture;
  }
  avg_temp = (max_temp-min_temp)/2;
  avg_moi = (max_moi-min_moi)/2;
  tempSOH = temp-avg_temp;
  moiSOH = soilMoisture-avg_moi;
  //Computing SOH as in the first project proposal
  SOH = 0.5*abs(tempSOH)+0.5*abs(moiSOH);
}

void sendReadings() {
  //Send data packet as string to use less bandwidth
  message = String(ID)+ "," + String(soilMoisture) + "," + String(temp)+ "," + String(hum)+ "," + String(SOH)+ "," + String(gps1_lat,6) + "," + String(gps1_long,6);
  delay(1000);
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();
}

int getConfigData() {
  //Serial.print("Lora packet received: ");
  //Read packet
  while (LoRa.available()) {
    String LoRaData = LoRa.readString();
    //LoRaData format: deviceID#new_lat/new_long&new_freq
    //String example: 1#12.43/54.12&80
    //Serial.print(LoRaData); 
    if (LoRaData == String(ID)){ //if the packet is the same as the ID of the device then the packet is not for this node or it has already been set we stop listening
      return 1;
    }
    else { 
      //Get deviceID, gps coordinates and requested frequency
      int pos1 = LoRaData.indexOf('#');
      int pos2 = LoRaData.indexOf('/');
      int pos3 = LoRaData.indexOf('&');
      deviceID = LoRaData.substring(0, pos1);
      new_lat = LoRaData.substring(pos1 +1, pos2);
      new_long = LoRaData.substring(pos2+1, pos3);
      new_freq = LoRaData.substring(pos3+1, LoRaData.length());   
      if (deviceID == String(ID)){ //the packet is a configuration packet for this station
        gps1_lat=new_lat.toFloat();
        gps1_long=new_long.toFloat();
        freq=new_freq.toInt();
      } 
    }
  }
  return 0;
}