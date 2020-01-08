#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "packets.h"
#include "processor.h"
#include "server.h"

/* this file is where serverside handling of stuff should be done! */

void process( struct cnx_header *cnx_info, struct packet_header *header, union packet *packet , int qd){
  /* this method will be called whenever a message is received from a user */
  /* cnx_info contain's the user's information: username, id, etc. */
  /* header and packet are what were sent to the server by the user */
  /* qd is the descriptor for the outbox queue pipe: DO NOT WRITE DIRECTLY TO IT! */
  /* instead of writing to qd, use qwrite(), which waits on a semaphore so we don't have to worry abt messes */

  // currently im doing no processing, simply adding the id of the user who sent to the header and writing it to the queue
  // in the future, you could filter out packets that, say, move a player onto the spot of another player, and instead write an error message to the queue to send back to the user
  header->id = cnx_info->id;
  strcpy(header->username, cnx_info->username);
  qwrite(header, packet,qd);
}

int should_receive( struct cnx_header *cnx_info, struct packet_header *header, union packet *packet ){
  /* this method will be called for each connected client when a message is about to be sent out */
  /* cnx_info contains the info for the potential recipient */
  /* header and packet contain the message to be sent */
  /* return 0 if this user does NOT need to receive the message (in another room, etc), return non-zero if they DO need to receive the message */

  // currently, all packets are sent to everyone
  return 1;
}
