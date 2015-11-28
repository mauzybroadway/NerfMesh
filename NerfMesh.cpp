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

int NerfMesh::AddSelfToNetwork() {

  return 0;
}

NerfMesh::NerfMesh()
  : radio(RF24_CE_PIN, RF24_CSN_PIN) {
  
}
