#ifndef PROCESSOR_H
#define PROCESSOR_H

void process( struct cnx_header *cnx_info, struct packet_header *header, union packet *packet , int qd);

int should_receive( struct cnx_header *cnx_info, struct packet_header *header, union packet *packet );

#endif
