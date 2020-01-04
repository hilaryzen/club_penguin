#include <stdio.h>
#include <stdlib.h>

#include "packets.h"
#include "processor.h"
#include "server.h"


void process( struct cnx_header *cnx_info, struct packet_header *header, union packet *packet , int qd){
  qwrite(packet, header->id, header->packet_type,qd);
}

int should_receive( struct cnx_header *cnx_info, struct packet_header *header, union packet *packet ){
  return 1;
}
