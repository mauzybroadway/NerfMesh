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
#include "nRF24L01.h"

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
  //NerfMesh_Ping packet = DEFAULT_PING;
  //  Locale curr = __locale;
  //curr.addr[0] = RESERVED_ADDR;

  //radio.openReadingPipe(RESERVED_PIPE, curr.addr);
  //radio.openWritingPipe(curr.addr);

  Serial.print(F("Sending Ping"));
  radio.stopListening();
  radio.startWrite( &my_id, sizeof(uint8_t), 0 );

  //if(radio.write(&my_id, sizeof(uint8_t)) == false) {
  //  return ENORESPONSE;
  //}
  
  /*if(radio.write(&packet, sizeof(NerfMesh_Ping)) == false) {
    return ENORESPONSE;
    }*/
 
  return 0;
}

// TODO : a better algorithm (choose neighbors in a "smarter" manner)
int NerfMesh::FindNeighbors() {
  int i, count = 0;

  for(i = 0; i < MAX_NODES && count < MAX_NEIGHBORS; i++) {
    if(i == my_id) continue;

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
  
  directory[my_id].num_hops = 0;

  return 0;
}

int NerfMesh::UpdateRoutingDirectory() {
  /* For now, there will be a static routing directory with 3 nodes */ 
  
  return 0;
}

Locale NerfMesh::__locale;
RF24 NerfMesh::radio(7,8);
NerfMesh::NerfMesh() {
  my_id = INIT_ID;
  memset(&__locale, '\0', sizeof(Locale));
}

void check_radio() {
  bool tx,fail,rx;
  uint8_t buf[5];
  NerfMesh::radio.whatHappened(tx,fail,rx);
 
  // If data is available, handle it accordingly
  if ( rx ){
    if(NerfMesh::radio.getDynamicPayloadSize() < 1) {
      return; 
    }
    
    // Read in the data
    uint8_t received;
    NerfMesh::radio.read(&received,sizeof(received));

    Serial.print(F("Received: "));
    printf("%x\n", received);

    if(received < 0xFF) {
      Locale curr = NerfMesh::__locale;
      curr.addr[0] = received;
      uint8_t asdf = 0xFF;

      NerfMesh::radio.openWritingPipe(curr.addr);
      
      //NerfMesh::radio.read_register(TX_ADDR, buf, 5);
      //printf("TX:0x%02x%02x%02x%02x%02x\n", buf[4], buf[3], buf[2], buf[1], buf[0]);
      
      //NerfMesh::radio.read_register(RX_ADDR_P0, buf, 5);
      //printf("RX:0x%02x%02x%02x%02x%02x\n", buf[4], buf[3], buf[2], buf[1], buf[0]);

      NerfMesh::radio.stopListening();

      // (16 nop's @16mhz = 1us delay)
      for(uint32_t i=0; i<130;i++){
	__asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t"
		"nop\n\t""nop\n\t""nop\n\t""nop\n\t"
		"nop\n\t""nop\n\t""nop\n\t""nop\n\t"
		"nop\n\t""nop\n\t""nop\n\t""nop\n\t");
      }


      Serial.print("Reply");
      NerfMesh::radio.startWrite( &asdf, sizeof(uint8_t), 0 );
      
      //if(NerfMesh::radio.write(&asdf, sizeof(uint8_t)) == false) {
      //  return -1;
      //}
    } else {
      Serial.println(F("GOT ITEM"));
      NerfMesh::radio.flush_tx();
    }

    // If this is a ping, send back a pong
    //if(received == ping){
    //  NerfMesh::radio.stopListening();
    //  
    //  // (16nops @16mhz = 1us delay)
    //  for(uint32_t i=0; i<130;i++){
    //__asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t"
    //		"nop\n\t""nop\n\t""nop\n\t""nop\n\t"
    //		"nop\n\t""nop\n\t""nop\n\t""nop\n\t"
    //		"nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    //}
    //
    //NerfMesh::radio.startWrite(&pong,sizeof(pong),0);
    //Serial.print("pong");
    //} else if(received == pong){
    //Serial.print(F("Received Pong\n"));
    //}
  }

  // Start listening if transmission is complete
  if( tx || fail ){
    NerfMesh::radio.startListening();
    //NerfMesh::radio.flush_tx(); 
    Serial.println(tx ? F(":OK") : F(":Fail"));
    //NerfMesh::radio.read_register(RX_ADDR_P0, buf, 5);
    //printf("RX:0x%02x%02x%02x%02x%02x\n", buf[4], buf[3], buf[2], buf[1], buf[0]);  
  }
}

void NerfMesh::begin(uint8_t id) {
  radio.begin();
  radio.enableDynamicPayloads();
  radio.setAutoAck(false);
  //radio.enableAckPayload();

  //radio.maskIRQ(false, true, false);

  my_id = id;
  
  Locale my_locale = __locale;
  my_locale.addr[0] = id;

  uint8_t received;
  radio.read(&received,sizeof(received));
    
  radio.flush_tx();
  
  radio.openReadingPipe(RESERVED_PIPE, RESERVED_ADDR);
  radio.openReadingPipe(MY_PIPE, my_locale.addr);

  radio.openWritingPipe(RESERVED_ADDR);
  radio.startListening();

  attachInterrupt(0, check_radio, LOW);

  //FindNeighbors();
  //InitRoutingDirectory();
  //UpdateRoutingDirectory();  
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
