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

#include <Arduino.h>
#include "RF24.h"

/****************************************************************************/
/* CONSTANTS */
/****************************************************************************/
typedef enum { EINVALID = -1,
	       ENONEIGHBORS = -2,
	       ENORESPONSE = -3} nmesh_errors_e;

typedef enum { INIT_ID = 0xFF,
	       MY_PIPE = 1,
	       RESERVED_ID = 0xFF,
	       RESERVED_PIPE = 0,
	       INFINITE_HOPS = -1 } nmesh_network_e;

typedef enum { NMESH_TYPE_PING = 0,
	       NMESH_TYPE_WHOHAS = 1 } nmesh_packet_types_e;

typedef enum { MAX_NODES = 256, MAX_NEIGHBORS = 5 } nmesh_overview_e;
typedef enum { MAX_PAYLOAD = 30 } nmesh_packet_e;
typedef enum { RF24_CE_PIN = 7, RF24_CSN_PIN = 8, RF24_IRQ_PIN = 2 } rf24_pins_e;

const uint8_t RESERVED_ADDR[] = { "radio" };

/****************************************************************************/
/* DATA TYPES */
/****************************************************************************/
struct NerfMesh_Header { // 4 bytes
  uint8_t src;
  uint8_t dest;
  uint8_t type;
  uint8_t length;
  //uint16_t PADDING; 	// TODO : figure out more packet fields
  //uint16_t checksum;
};

struct NerfMesh_Packet {
  NerfMesh_Header header;
  uint8_t data[MAX_PAYLOAD];
};

struct NerfMesh_Inquiry {
  NerfMesh_Header header;
};

struct NerfMesh_Ping {
  NerfMesh_Header header;
};

struct RoutDir_Entry {
  int16_t num_hops;
  uint8_t next_hop;
  uint8_t PADDING;	// TODO : more fields
};

struct Locale {
  uint8_t addr[5];
};

const NerfMesh_Ping DEFAULT_PING =
  { { 0x00, 0x00, NMESH_TYPE_PING, sizeof(NerfMesh_Ping) } };

/****************************************************************************/
/* CLASS: NerfMesh */
/****************************************************************************/
class NerfMesh {
 private:
  uint8_t my_id;
  RoutDir_Entry directory[MAX_NODES];
  uint8_t neighbors[MAX_NEIGHBORS];
  static Locale __locale;

  int FindNeighbors();
  int InitRoutingDirectory();
  int UpdateRoutingDirectory();
  static RF24 radio;
  
 protected:
  int prot;
  
 public:
  NerfMesh();
  void begin(uint8_t id);
  void PrintRadioDetails();
  void PrintNeighbors();
  void PrintRoutingDirectory();

  int PingAddress(uint8_t address);
  
  friend void check_radio();  
};

#endif //__NERFMESH_H__
