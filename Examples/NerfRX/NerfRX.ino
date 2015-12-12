#include <SPI.h>
#include <printf.h>

#include "RF24.h"
#include "NerfMesh.h"

NerfMesh nerf;

void handler(NerfMesh_Packet packet);

void setup() {
  pinMode(4,INPUT);
  digitalWrite(4, HIGH);
  
  Serial.begin(115200);
  printf_begin();

  nerf.begin(0x11);
  nerf.Enable_Comm(handler);
}

void loop() {
  if(Serial.available()){
    switch(toupper(Serial.read())){
    case 'T':
      nerf.PingAddress(0x22);
      break;
    case 'S':
      nerf.PrintRadioDetails();
      break;
    case 'F':
      nerf.FindNeighbors();
      break;
    case 'N':
      nerf.PrintNeighbors();
      break;
    case 'D':
      nerf.PrintRoutingDirectory();
      break;
    }
  }
}

void handler(NerfMesh_Packet packet) {
  //char buf[] = { 'f','u','c','k','\0'};
  //char buf = 'a';
  uint8_t y[] = { 'y' };
  uint8_t n[] = { 'n' };
  uint8_t jump = 0;
  
  printf("%d\n", digitalRead(4));
  
  if(packet.data[0] == 'f') {
    if(digitalRead(4)) {
      nerf.Write(0x22, n, 1);
    } else {
      nerf.Write(0x22, y, 1);
    }
  }
  
}
