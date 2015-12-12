/**
 * NerfMesh: Decentralized, disestablishmentary
 *
 *	github.com/mauzybroadway/NerfMesh 
 *
 * Copyright (C) 2015 B. Wetzel <mauzybwy@gmail.com> 		
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#include "NerfMesh.h"

#define DEBUG
#ifdef DEBUG
#define Debug(args) Print("Nerf: " args);
#else
#define Debug(args)
#endif

//int nerfdebug = 0;
//#define Debug(args...) if (nerfdebug) Print("NERF: " args)

uint8_t ping = 111;
uint8_t pong = 222;

int NerfMesh::PingAddress(uint8_t address) {  
  NerfMesh_Ping packet = DEFAULT_PING;
  Locale curr = __locale;
  curr.addr[0] = address;

  radio.openReadingPipe(RESERVED_PIPE, curr.addr);
  radio.openWritingPipe(curr.addr);

  Serial.print(F("Sending Ping"));
  radio.stopListening();
  radio.startWrite( &ping, sizeof(uint8_t),0 );
  
  //if(radio.write(&packet, sizeof(NerfMesh_Ping)) == false) {
  //  return ENORESPONSE;
  //}
  
  return 0;
}

// TODO : a better algorithm (choose neighbors in a "smarter" manner)
int NerfMesh::FindNeighbors() {
  int i, count = 0;

  for(i = 0; i < MAX_NODES && count < MAX_NEIGHBORS; i++) {
    if(i == my_addr) continue;

    if (PingAddress(i) == 0) {
      //Debug("FOUND NEIGHBOR: %d",i);
      neighbors[count] = i;
      
      directory[i].num_hops = 1;
      directory[i].next_hop = 1;
	
      count++;
    }
  }

  return 0;
}

int NerfMesh::InitRoutingDirectory() {
  int i;

  memset(directory, '\0', sizeof(directory));
  for(i = 0; i < MAX_NODES; i++) {
    directory[i].num_hops = INFINITE_HOPS;
  }
  
  directory[my_addr].num_hops = 0;

  return 0;
}

int NerfMesh::UpdateRoutingDirectory() {
  /* For now, there will be a static routing directory with 3 nodes */ 
  
  return 0;
}

NerfMesh::NerfMesh()
  : radio(RF24_CE_PIN, RF24_CSN_PIN) {
  
  my_addr = INIT_ADDR;
  memset(&__locale, '\0', sizeof(Locale));
}

void check_radio(void) {
  bool tx,fail,rx;
  NerfMesh::radio.whatHappened(tx,fail,rx);
 
  // If data is available, handle it accordingly
  if ( rx ){
    if(radio.getDynamicPayloadSize() < 1) {
      return; 
    }
    
    // Read in the data
    uint8_t received;
    radio.read(&received,sizeof(received));

    // If this is a ping, send back a pong
    if(received == ping){
      radio.stopListening();
      
      // (16nops @16mhz = 1us delay)
      for(uint32_t i=0; i<130;i++){
	__asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t"
		"nop\n\t""nop\n\t""nop\n\t""nop\n\t"
		"nop\n\t""nop\n\t""nop\n\t""nop\n\t"
		"nop\n\t""nop\n\t""nop\n\t""nop\n\t");
      }
      
      radio.startWrite(&pong,sizeof(pong),0);
      Serial.print("pong");
    } else if(received == pong){
      Serial.print(F("Received Pong, Round Trip Time: "));
    }
  }

  // Start listening if transmission is complete
  if( tx || fail ){
    radio.startListening(); 
    Serial.println(tx ? F(":OK") : F(":Fail"));
  }
}

void NerfMesh::begin() {
  radio.begin();
  radio.enableDynamicPayloads();
  //radio.enableAckPayload();

  attachInterrupt(0, check_radio, LOW);

  FindNeighbors();
  
  InitRoutingDirectory();
  UpdateRoutingDirectory();  
}

void NerfMesh::PrintRadioDetails() {
  radio.printDetails();
}

void NerfMesh::PrintNeighbors() {
  
}

void NerfMesh::PrintRoutingDirectory() {
  int i;

  printf("[ ");
  for(i = 0; i < MAX_NODES; i++) {
    printf("%d{num_hops:%d next_hop:%d} ",i,directory[i].num_hops, directory[i].next_hop);
  }
  printf("]");
}
