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
#define Debug(args...) printf(args);
#else
#define Debug(args)
#endif

//int nerfdebug = 0;
//#define Debug(args...) if (nerfdebug) Print("NERF: " args)

// TODO : a better algorithm (choose neighbors in a "smarter" manner)

RF24 NerfMesh::radio(7,8);
Locale NerfMesh::__locale;
uint8_t NerfMesh::my_id;
bool NerfMesh::flag;
RoutDir_Entry NerfMesh::directory[MAX_NODES];
int NerfMesh::neighbors[MAX_NEIGHBORS];
uint8_t NerfMesh::num_neighbors;
data_handler NerfMesh::handler;
int NerfMesh::cache_idx;
char NerfMesh::cache[MAX_CACHE];

NerfMesh::NerfMesh() {
  int i;

  handler = NULL;
  comm = false;
  
  num_neighbors = 0;
  my_id = INIT_ID;
  memset(&__locale, '\0', sizeof(Locale));

  for(i = 0; i < MAX_NEIGHBORS; i++) {
    neighbors[i] = -1;
  }

  for(i = 0; i < MAX_NODES; i++) {
    directory[i].num_hops = -1;
    directory[i].next_hop = -1;
  }
  
}

bool NerfMesh::DoWrite(NerfMesh_Packet * packet) {
  bool out;
  Locale curr = __locale;

  /* Prepare chip for writing */
  curr.addr[0] = packet->header.dest;
  radio.openWritingPipe(curr.addr);
  radio.stopListening();

  /* WRITE */
  out = radio.write(packet, packet->header.size + sizeof(NerfMesh_Header));

  if(out) {
    Debug(":SUCC\n");
  } else {
    Debug(":FAIL\n");
  }  

  /* Get back into Standby-I */
  radio.startListening();

    // (16 nop's @16mhz = 1us delay)
  for(uint32_t i=0; i<130;i++){
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t"
	    "nop\n\t""nop\n\t""nop\n\t""nop\n\t"
	    "nop\n\t""nop\n\t""nop\n\t""nop\n\t"
	    "nop\n\t""nop\n\t""nop\n\t""nop\n\t");
  }
  

  return out;
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

/* Returns bytes written? */
int NerfMesh::Send_Data(uint8_t address, void *data, size_t size) {
  NerfMesh_Packet packet = DEFAULT_PACKET;
  
  /* Set up header */
  packet.header.src = my_id;
  packet.header.dest = address;
  packet.header.type = NMESH_TYPE_DATA;
  packet.header.size = size;

  memcpy(&packet.data, data, size);
    
  Debug("Sending Data:%d bytes:", size);

  if(!DoWrite(&packet))
    return ENORESPONSE;

  return size; //meh for now at least, doubt I could do any better though...
}

int NerfMesh::Send_Error(uint8_t address, uint8_t type) {
  NerfMesh_Packet packet = DEFAULT_PACKET;
  NerfMesh_Error *error = ((NerfMesh_Error *)packet.data);
  
  /* Set up header */
  packet.header.src = my_id;
  packet.header.dest = address;
  packet.header.type = NMESH_TYPE_ERROR;
  packet.header.size = sizeof(NerfMesh_Error);

  /* Set up data */
  error->type = type;
    
  Debug("Sending Error:0x%x:%d:", address, type);
  
  return DoWrite(&packet) ? 0 : ENORESPONSE;
}

int NerfMesh::Send_AddMe(uint8_t address, uint8_t type) {
  NerfMesh_Packet packet = DEFAULT_PACKET;
  NerfMesh_AddMe *addme = ((NerfMesh_AddMe *)packet.data);
  
  /* Set up header */
  packet.header.src = my_id;
  packet.header.dest = address;
  packet.header.type = NMESH_TYPE_ADDME;
  packet.header.size = sizeof(NerfMesh_AddMe);

  /* Set up data */
  addme->type = type;
    
  Debug("Sending AddMe:0x%x:%d:", address, type);

  return DoWrite(&packet) ? 0 : ENORESPONSE;
}

int NerfMesh::Send_Ping(uint8_t address, uint8_t type) {
  NerfMesh_Packet packet = DEFAULT_PACKET;
  NerfMesh_Ping *ping = ((NerfMesh_Ping *)packet.data);
  
  /* Set up header */
  packet.header.src = my_id;
  packet.header.dest = address;
  packet.header.type = NMESH_TYPE_PING;
  packet.header.size = sizeof(NerfMesh_Ping);

  /* Set up data */
  ping->type = type;

  if(type == PING_OUT) {
    Debug("Sending Ping:0x%x", address);
  } else if(type == PING_BACK) {
    Debug("Returning Pong:0x%x", address);
  } else {
    // AYYY
  }

  return DoWrite(&packet) ? 0 : ENORESPONSE;
}

int NerfMesh::PingAddress(uint8_t address) {
  return Send_Ping(address, PING_OUT);
}

int NerfMesh::Hark(uint8_t address, uint8_t distance, uint8_t type) {
  int i;
  NerfMesh_Packet packet = DEFAULT_PACKET;
  NerfMesh_Hark *hark = ((NerfMesh_Hark *)packet.data);

  /* Set up header */
  packet.header.src = my_id;
  packet.header.size = sizeof(NerfMesh_Hark);
  packet.header.type = NMESH_TYPE_HARK;

  /* Set up data */
  hark->address = address;
  hark->distance = distance;
  hark->type = type;
  
  for(i = 0; i < MAX_NEIGHBORS; i++) {
    if(neighbors[i] >= 0 && neighbors[i] != address) {
      packet.header.dest = i;
      
      Debug("Sending Hark:add:");
      radio.stopListening();
      radio.startWrite( &packet, NMESH_PACKET_SIZE(packet), 0 );
    }
  }

  return 0;
}

bool NerfMesh::Request_AddMe(uint8_t address) {
  int trial = 0;
  NerfMesh_AddMe *addme;
  NerfMesh_Packet packet;
  
  if(!Send_AddMe(address, ADDME_ADD)) {
    return false;
  }
    
  /* Try connecting 5 times */
  //success = false;
  for(trial = 0; trial < 5; trial ++) {
    delay(10); // wait 1 ms SHOULD I DO THIS THE FIRST TIME??
    
    if(radio.available(NULL)) {
      NerfMesh::radio.read(&packet.header,sizeof(NerfMesh_Header));
      NerfMesh::radio.read(&packet.data,packet.header.size);

      /* Take out the trash */
      if(packet.header.type != NMESH_TYPE_ADDME) {
	continue;
      }

      addme = (NerfMesh_AddMe *)packet.data;
      
      if(addme->type == ADDME_FUCKOFF) return false;
      if(addme->type == ADDME_GOTCHU) {
	Send_AddMe(address, ADDME_COPYTHAT);
	return true;
      }
    }
  }
  
  return false;
}

bool NerfMesh::Request_RoutingTable(uint8_t address) {

  return false;
}

bool NerfMesh::Request_Neighbor(uint8_t address) {
  
  return false;
}

int NerfMesh::FindNeighbors() {
  int count = 0;
  uint8_t i, old_num_nei = num_neighbors;
  
  /* Disable RX interrupts */
  //radio.maskIRQ(false, false, true);
  
  for(i = 0; i < MAX_NODES && num_neighbors < MAX_NEIGHBORS; i++) {
    if(i == my_id) continue;

    //if (Request_AddMe(i) == true) {
    if(Send_AddMe(i, ADDME_ADD) == 0) {
      delay(1);
      //count++;
    }
  }

  /*Enable RX interrupts*/
  //radio.unmaskIRQ(false, false, true);

  count = num_neighbors - old_num_nei;

  if(count == 0) {
    Debug("NO NEW NEIGHBORS!");
    return ENONEIGHBORS;
  }

  Debug("New neighbors: %d\n", count);

  return 0;
}

bool NerfMesh::Is_Neighbor(uint8_t address) {
  return directory[address].num_hops == 1;
}

bool NerfMesh::Is_In_Directory(uint8_t address) {
  return directory[address].num_hops >= 0;
}

int NerfMesh::UpdateDirectoryEntry(uint8_t address, int num_hops, int next_hop) {
  directory[address].num_hops = num_hops;
  directory[address].next_hop = next_hop;

  return 0;
}

int NerfMesh::Handle_Error(NerfMesh_Packet packet) {
  
  return 0;
}

int NerfMesh::Handle_Data(NerfMesh_Packet packet) {
  //int amt;

  if(cache_idx >= MAX_CACHE) {
    //Send_Error(packet.header.src, ERROR_TOOMUCHTUNA);
    return EUNSUPPORTED;
  }
  
  if(handler){
    handler(packet);
  } else {
    while(radio.available() && cache_idx < MAX_CACHE) {
      radio.read(&cache[cache_idx++], 1); // generalize in case setPayload is called elsewhere
    }
  }
    
  return 0;
}

int NerfMesh::Handle_Hark(NerfMesh_Packet packet) {
  NerfMesh_Hark *hark = (NerfMesh_Hark *)packet.data;

  /* Node is already taken care of */
  if(Is_In_Directory(hark->address) && directory[hark->address].num_hops <= hark->distance) {
    return 0; 
  }

  /* Update node in directory */
  UpdateDirectoryEntry(hark->address, hark->distance + 1, packet.header.src);

  /* HARK */
  Hark(hark->address, hark->distance + 1, hark->type);
  
  return 0;
}

// TODO : check if already neighbor and shit like that
int NerfMesh::AddNeighbor(uint8_t address) {
  int i = 0; 
  
  if (num_neighbors >= MAX_NEIGHBORS) return ETOOMUCHTUNA;

  for(i = 0; i < MAX_NEIGHBORS; i++) {
    if(neighbors[i] < 0) {
      neighbors[i] = address;
      num_neighbors++;
      return 0;
    }
  }

  return EINVALID;
}

int NerfMesh::Handle_AddMe(NerfMesh_Packet packet) {
  NerfMesh_AddMe *addme = (NerfMesh_AddMe *)packet.data;
  
  if(packet.header.dest != my_id) {
    Debug("NOT ME\n");
    return ENOTME; 
  }

  if(addme->type == ADDME_ADD) {

    if(num_neighbors >= MAX_NEIGHBORS) {
      Send_AddMe(packet.header.src, ADDME_FUCKOFF);
    } else if(Is_Neighbor(packet.header.src)) {
      Send_AddMe(packet.header.src, ADDME_CHILLOUT);
    } else {
      Send_AddMe(packet.header.src, ADDME_GOTCHU);
    }
            
  } else if( addme->type == ADDME_GOTCHU ) {

    Send_AddMe(packet.header.src, (uint8_t)ADDME_COPYTHAT);
    UpdateDirectoryEntry(packet.header.src, 1, packet.header.src);
    AddNeighbor(packet.header.src);
    num_neighbors++;
    
  } else if(addme->type == ADDME_COPYTHAT) {
    Debug("COOL\n");
    /* Add new guy to neighbors */    
    AddNeighbor(packet.header.src);
    UpdateDirectoryEntry(packet.header.src, 1, packet.header.src);
    
    /* HARK */
    //Hark(packet.header.src, 1, HARK_ADD);
    
  } else if(addme->type == ADDME_CHILLOUT) {
    // TODO: check neighbor stuff
  } else {
    Debug("INVALID ADDME: received %d\n",addme->type);
    return EINVALID;
  }
  
  return 0;
}

int NerfMesh::Handle_Ping(NerfMesh_Packet packet) {
  NerfMesh_Ping *ping = (NerfMesh_Ping *)packet.data;

  if(ping->type == PING_OUT) {
    Send_Ping(packet.header.src, PING_BACK);	    
  } else if(ping->type == PING_BACK) {
    Debug("GOT ITEM:0x%x\n", packet.header.src);
  } else {
    Debug("PING ERROR\n");
    return EINVALID;
  }

  return 0;
}

int NerfMesh::Find_Next_Hop(uint8_t address) {
  if(my_id == 0x11 || my_id == 0x22)
    return 0x33;

  if(my_id == 0x33 && address)
    return 0x11;
    
  return ENONEIGHBORS;
}

int NerfMesh::Forward_Packet(NerfMesh_Packet packet) {
  Locale curr = __locale;
  int next_hop = 0;

  // TODO : routing table stuffs
  if(my_id == 0x33) {
    if(packet.header.src == 0x11){
      next_hop = 0x22;
    } else if(packet.header.src == 0x22) {
      next_hop = 0x11;
    } else {
      Debug("HOW THE HELL IS THIS EVEN POSSIBLE\n");
      return ENONEIGHBORS;
    }
  } else {
    Debug("YOU ARENT 0x33, SHUT UP\n");
    return ENONEIGHBORS;
  }
  
  curr.addr[0] = next_hop;
  
  radio.openWritingPipe(curr.addr);
  radio.stopListening();
  
  Debug("Forwarding to 0x%x", packet.header.dest);
  radio.startWrite( &packet, NMESH_PACKET_SIZE(packet), 0 );
  
  return 0;
}

void check_radio() {
  bool tx,fail,rx;
  
  NerfMesh::radio.whatHappened(tx,fail,rx);
 
  // If data is available, handle it accordingly
  if ( rx ){
    if(NerfMesh::radio.getDynamicPayloadSize() < 1) {
      return; 
    }
    
    // Read in the data
    NerfMesh_Packet packet;
    NerfMesh::radio.read(&packet.header,sizeof(NerfMesh_Header));

    if(packet.header.magic != MAGIC) {
      Debug("GET FUCKED\n");
      return;
    }
    
    NerfMesh::radio.read(&packet.data,packet.header.size);
    
    if(packet.header.dest != NerfMesh::my_id &&
       packet.header.type != NMESH_TYPE_ADDME) {
      NerfMesh::Forward_Packet(packet);
    } else {
      switch(packet.header.type) {
      case NMESH_TYPE_PING: NerfMesh::Handle_Ping(packet); break;
      case NMESH_TYPE_ADDME: NerfMesh::Handle_AddMe(packet); break;
      case NMESH_TYPE_DATA: NerfMesh::Handle_Data(packet); break;
      case NMESH_TYPE_ERROR: NerfMesh::Handle_Error(packet); break;
        default: Debug("BAD PACKET\n");
      }
    }
  }
  
  // Start listening if transmission is complete
  if( tx || fail ){
    NerfMesh::radio.startListening();
    Serial.println(tx ? F(":OK") : F(":Fail"));
    NerfMesh::flag = true;
  }
}


void NerfMesh::begin(uint8_t id) {
  radio.begin();
  radio.enableDynamicPayloads();
  radio.setAutoAck(true);
  radio.setRetries(0,15);
  radio.setPayloadSize(MAX_PAYLOAD);
  //radio.enableAckPayload();

  radio.maskIRQ(true, true, false);
  
  my_id = id;
  UpdateDirectoryEntry(my_id, 0, -1);
  
  Locale my_locale = __locale;
  my_locale.addr[0] = id;

  uint8_t received;
  radio.read(&received,sizeof(received));
    
  radio.flush_tx();

  //uint8_t ADDR11[] = { 0x11,0,0,0,0 };
  //uint8_t ADDR22[] = { 0x22,0,0,0,0 };

  
  radio.openReadingPipe(MY_PIPE, my_locale.addr);
  
  /*if(my_id != 0x33) {
    radio.openReadingPipe(RESERVED_PIPE, RESERVED_ADDR);
  } else {
    uint8_t SEXY_ADDR[] = { "sexyy" };
    
    radio.openReadingPipe(RESERVED_PIPE, SEXY_ADDR); // just ignore shit on this pipe
    radio.openReadingPipe(2, ADDR11);
    radio.openReadingPipe(3, ADDR22);
    }*/
  
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
  int i;
  
  for(i = 0; i < MAX_NEIGHBORS; i++) {
    printf("Neighbor #%d ", i);
    
    if(neighbors[i] >= 0) {
      printf("0x%x\n",neighbors[i]);
    } else {
      printf("NOT FOUND\n");
    }
  }
}

void NerfMesh::PrintRoutingDirectory() {
  int i;

  printf("[ ");
  for(i = 0; i < MAX_NODES; i++) {
    printf("0x%x{num_hops:%d next_hop:0x%x}\n",i,directory[i].num_hops, directory[i].next_hop);
  }
  printf("]");
}

int NerfMesh::Enable_Comm(data_handler dh) {
  comm = true;
  handler = dh;
  return 0;
}

int NerfMesh::Disable_Comm() {
  comm = true;
  handler = NULL; //necessary?
  return 0;
}

int NerfMesh::Write(uint8_t address, void *data, size_t size) {
  int rc, amt, diff, paysize = radio.getPayloadSize() - sizeof(NerfMesh_Header);
  uint32_t bytes_written = 0;
  
  while(bytes_written < size) {
    diff = size - bytes_written;
    amt = diff < paysize ? diff : paysize;
    
    rc = Send_Data(address, ((char*)data) + bytes_written, amt);
    delay(10);
    if(rc < 0) return rc;

    bytes_written += rc;    
  }
  
  return 0;
}

int NerfMesh::Read(uint8_t address, void * data, size_t size) {
  
  
  return 0;
}
