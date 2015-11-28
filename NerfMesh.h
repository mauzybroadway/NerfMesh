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

#ifndef __NERFMESH_H__
#define __NERFMESH_H__

#include "RF24.h"

/* CONSTANTS */
typedef enum { MAX_NODES = 256 } nmesh_overview_e;
typedef enum { MAX_PAYLOAD = 30 } nmesh_packet_e;
typedef enum { RF24_CE_PIN = 7, RF24_CSN_PIN = 8 } rf24_pins_e;

/* DATA TYPES */
struct NerfMesh_Header {
  uint8_t src;
  uint8_t dest;
  uint8_t next;
  uint8_t length;
  uint16_t PADDING; 	// TODO : figure out more packet fields
  uint16_t checksum;
};

struct NerfMesh_Packet {
  NerfMesh_Header header;
  uint8_t data[MAX_PAYLOAD];
};

struct RoutDir_Entry {
  uint8_t dest;
  uint8_t next_hop;
  uint8_t num_hops;
  uint8_t PADDING; 	// TODO : figure out more entry fields
};

class NerfMesh {
 private:
  RF24 radio;
  RoutDir_Entry directory[MAX_NODES];
  
  int AddSelfToNetwork();
  
 protected:
  int prot;
  
 public:
  NerfMesh();
  
}

#endif //__NERFMESH_H__
