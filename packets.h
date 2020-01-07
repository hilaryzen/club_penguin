#ifndef PACKETS_H
#define PACKETS_H

#define PORT "2024"

enum packet_t {
  P_CNX_HEADER,
  P_CHATMSG,
  P_PLAYERMOVE
};

struct packet_header {
  int id;
  char *username;
  int packet_size;
  enum packet_t packet_type;
};

struct cnx_header {
  int id;
  int sd;
  char username[16];
  int room;
};

struct chatmsg {
  char message[80];
};

struct playermove {
  int r;
  int c;
};

union packet {
  struct cnx_header CNX_HEADER;
  struct chatmsg CHATMSG;
  struct playermove PLAYERMOVE;
};

#endif
