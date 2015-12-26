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
/* MACROS */
/****************************************************************************/
#define NMESH_PACKET_SIZE(packet) (packet.header.size + sizeof(NerfMesh_Header))

/****************************************************************************/
/* CONSTANTS */
/****************************************************************************/
typedef enum {
  EINVALID = -1,
  ENONEIGHBORS = -2,
  ENORESPONSE = -3,
  ENOTME = -4,
  ETOOMUCHTUNA = -5,
  EUNSUPPORTED = -6
} nmesh_errors_e;

typedef enum {
  INIT_ID = 0xFF,
  MY_PIPE = 1,
  RESERVED_ID = 0xFF,
  RESERVED_PIPE = 0,
  INFINITE_HOPS = -1
}nmesh_network_e;

typedef enum {
  NMESH_TYPE_PING = 1,
  NMESH_TYPE_ADDME = 2,
  NMESH_TYPE_HARK = 3,
  NMESH_TYPE_WHOHAS = 4,
  NMESH_TYPE_DATA = 5,
  NMESH_TYPE_ERROR = 6
} nmesh_packet_types_e;

typedef enum {
  ERROR_FUCKYOU = 1,
  ERROR_STFU = 2,
  ERROR_GTFO = 3,
  ERROR_TOOMUCHTUNA = 4,
  ERROR_NOBUENO = 5
} nmesh_errpack_e;

typedef enum {
  PING_OUT = 1,
  PING_BACK = 2
} nmesh_ping_e;

typedef enum {
  ADDME_ADD = 1,
  ADDME_GOTCHU = 2,
  ADDME_COPYTHAT = 3,
  ADDME_FUCKOFF = 4,
  ADDME_CHILLOUT = 5
} nmesh_addme_e;

typedef enum {
  WHOHAS_REQUEST = 1,
  WHOHAS_RESPONSE = 2
} nmesh_whohas_e;

typedef enum {
  HARK_ADD = 1,
  HARK_KILL = 2
} nmesh_hark_e;

typedef enum {
  MAX_NODES = 64,
  MAX_NEIGHBORS = 4,
  NUM_PIPES = 6,
  MAX_CACHE = 96
} nmesh_overview_e;

typedef enum {
  MAX_PAYLOAD = 20
} nmesh_packet_e;

typedef enum {
  RF24_CE_PIN = 7,
  RF24_CSN_PIN = 8,
  RF24_IRQ_PIN = 2
} rf24_pins_e;


const uint8_t RESERVED_ADDR[] = { "radio" };
const uint32_t MAGIC = 0x1eaf;

/****************************************************************************/
/* DATA TYPES */
/****************************************************************************/
struct NerfMesh_Header {
  uint8_t src;
  uint8_t dest;
  uint8_t type;
  uint8_t size;
  uint8_t last_hop;
  uint8_t num_hops;
  //uint16_t checksum;
  uint32_t magic;
};

struct NerfMesh_Packet {
  NerfMesh_Header header;
  uint8_t data[MAX_PAYLOAD - sizeof(NerfMesh_Header)];
};

struct NerfMesh_Error{
  uint8_t type;
};

struct NerfMesh_Ping {
  uint8_t type;
};

struct NerfMesh_WhoHas {
  uint8_t type;
  uint8_t address;
  uint8_t distance;
};

struct NerfMesh_AddMe {
  uint8_t type;
};

struct NerfMesh_Hark {
  uint8_t address;
  uint8_t distance;
  uint8_t type;
};

struct NerfMesh_Inquiry {
  NerfMesh_Header header;
};

struct RoutDir_Entry {
  int16_t num_hops;
  int16_t next_hop;
  //uint8_t PADDING;	// TODO : more fields
};

struct Locale {
  uint8_t addr[5];
};

const NerfMesh_Packet DEFAULT_PACKET =
  { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, MAGIC}, {0} };

typedef void (*data_handler)(NerfMesh_Packet packet);

/****************************************************************************/
/* CLASS: NerfMesh */
/****************************************************************************/
class NerfMesh {
 private:
  bool comm;
  static data_handler handler;
  static bool flag;
  static RoutDir_Entry directory[MAX_NODES];
  static int neighbors[MAX_NEIGHBORS];
  static uint8_t num_neighbors;
  static Locale __locale;
  static RF24 radio;
  static uint8_t my_id; //sketchy, but there will always only be one instance
  static char cache[MAX_CACHE]; // TODO : one of these for each channel????
  static int cache_idx;
  //static bool chache_full;
  
  int InitRoutingDirectory();
  int UpdateRoutingDirectory();
  static int UpdateDirectoryEntry(uint8_t address, int num_hops, int next_hop);
  static int AddNeighbor(uint8_t address);
  static bool Is_Neighbor(uint8_t address);
  static bool Is_In_Directory(uint8_t address);
  static bool DoWrite(NerfMesh_Packet *packet);

  bool Request_AddMe(uint8_t address);
  bool Request_Neighbor(uint8_t address);
  bool Request_RoutingTable(uint8_t address);

  static int Send_Data(uint8_t address, void * data, size_t size);
  static int Send_Ping(uint8_t address, uint8_t type);
  static int Send_AddMe(uint8_t address, uint8_t type);
  static int Send_Error(uint8_t address, uint8_t type);
  static int Send_WhoHas(uint8_t address, uint8_t addr_out, uint8_t dist, uint8_t type);


  static int Handle_Hark(NerfMesh_Packet packet);
  static int Handle_AddMe(NerfMesh_Packet packet);
  static int Handle_Ping(NerfMesh_Packet packet);
  static int Handle_Data(NerfMesh_Packet packet);
  static int Handle_Error(NerfMesh_Packet packet);
  static int Handle_WhoHas(NerfMesh_Packet packet);
  

  static int Hark(uint8_t address, uint8_t distance, uint8_t type);
  static int Forward_Packet(NerfMesh_Packet packet);
  static int Find_Next_Hop(uint8_t address);
  
 protected:
  int prot;
  
 public:
  NerfMesh();
  void begin(uint8_t id);
  void PrintRadioDetails();
  void PrintNeighbors();
  void PrintRoutingDirectory();

  int PingAddress(uint8_t address);
  int FindNeighbors();
  int PollNeighbors(uint8_t address);

  int Enable_Comm(data_handler dh);
  int Write(uint8_t address, void *data, size_t size);
  int Read(uint8_t address, void *buf, size_t size);
  int Disable_Comm();
  
  friend void check_radio();
};

#endif //__NERFMESH_H__
