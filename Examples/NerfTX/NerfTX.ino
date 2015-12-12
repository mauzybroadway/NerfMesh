#include <SPI.h>
#include <printf.h>

#include "RF24.h"
#include "NerfMesh.h"

NerfMesh nerf;

void handler(NerfMesh_Packet packet);

void setup() {
  Serial.begin(115200);
  printf_begin();

  nerf.begin(0x22);
  nerf.Enable_Comm(handler);
}

void loop() {
  //uint8_t buf[] = {1};
  char buf = 'f';
  
  //nerf.Write(0x11, &buf, 1);
  //delay(10);
  
  if(Serial.available()){
    switch(toupper(Serial.read())){
    case 'T':
      nerf.PingAddress(0x11);
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
    case 'R':
      nerf.Write(0x11, &buf, 1);
      break;
    }
  }
}

void handler(NerfMesh_Packet packet) {
  printf("%c\n", packet.data[0]);
}