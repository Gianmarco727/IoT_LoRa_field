#include <SPI.h>
#include <LoRa.h>
String msg;
void setup() {
  Serial.begin(9600);
  while (!Serial);
  

  //Serial.println("LoRa Receiver");

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  LoRa.setSyncWord(0xF3); //setting a default syncword to filter out unwanted packets
}

void loop() {
  //receiver//
  
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // read packet
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    } //print packet received to serial port
 Serial.print((char)',');
 Serial.print(String(LoRa.packetRssi())); //also print the RSSI in the serial port
 Serial.print((char)'\n');
    // print RSSI of packet
 //sending data from the serial
 
   delay(500); //wait for NodeRed to process data and provice configurations parameters to be sent

  //sender//
  msg=Serial.readString(); //read the setting parameters from the serial of the raspberry
  LoRa.beginPacket();
  LoRa.print(msg);
  LoRa.endPacket();
  }
 
}
