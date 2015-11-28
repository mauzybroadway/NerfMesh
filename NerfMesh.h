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

#define __STDC_LIMIT_MACROS
#include <stdint.h>

#define INT8_MAX 0x7f

#include <Arduino.h>
#include "RF24.h"

/****************************************************************************/
/* CONSTANTS */
/****************************************************************************/
typedef enum { MAX_NODES = INT8_MAX } nmesh_overview_e;
typedef enum { MAX_PAYLOAD = 30 } nmesh_packet_e;
typedef enum { RF24_CE_PIN = 7, RF24_CSN_PIN = 8 } rf24_pins_e;

/****************************************************************************/
/* DATA TYPES */
/****************************************************************************/
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

struct NefMesh_Inquiry {
  
};

struct RoutDir_Entry {
  int8_t next_hop;
  uint8_t num_hops;
};

/****************************************************************************/
/* CLASS: NerfMesh */
/****************************************************************************/
class NerfMesh {
 private:
  uint8_t my_id;
  RF24 radio;
  RoutDir_Entry directory[MAX_NODES];
  
  int UpdateRoutingDirectory();
  
 protected:
  int prot;
  
 public:
  NerfMesh();
  
};

#endif //__NERFMESH_H__
