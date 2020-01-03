#ifndef SEMA_H
#define SEMA_H

int sem_config(int key,int flags,int n,...);
int sem_connect(int key,int n);
int sem_getval(int semid,int idx);
void sem_disconnect(int semid);

void sem_claim(int semid,int idx);
void sem_release(int semid,int idx);

#endif
