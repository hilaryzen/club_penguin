#ifndef PACKETS_H
#define PACKETS_H

#define MAX_CNX 8

#define PORT "2024"

enum packet_t {
  P_CNX_HEADER,
  P_CHATMSG,
  P_PLAYERMOVE,
  P_GOODBYE
};

struct packet_header {
  int id;
  int packet_size;
  enum packet_t packet_type;
};

struct chatmsg {
  char message[128];
};

struct playermove {
  int r;
  int c;
};

struct cnx_header {
  int id;
  int sd;
  char username[16];
  int room;
  struct playermove pos;
};

union packet {
  struct cnx_header CNX_HEADER;
  struct chatmsg CHATMSG;
  struct playermove PLAYERMOVE;
};

#endif
