#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>

#include "err.h"
#include "sema.h"

/* everything* you might want to do with a semaphore has a method here that does it cleanly! */

union semun {
  int              val;    /* Value for SETVAL */
  struct  semid_ds *buf;     /*  Buffer for  IPC_STAT,
				 IPC_SET */
  unsigned short  *array;  /* Array for GETALL, SETALL */
  struct seminfo  *__buf;  /* Buffer for IPC_INFO
			      (Linux-specific) */
};

int sem_config(int key,int flags,int n,...){
  /* configures an array of n semaphores with initial values determined by the ... args, using `key` and `flags` */
  /* im really really proud of this :) */
  int i,r,semid;

  // create semaphore
  semid = semget(key,n,flags);
  exit_err(semid,"configuring semaphore array");

  // this handles the values in ...
  va_list init_vals;
  va_start(init_vals,n);

  short vals[n];
  // convert args in ... into an array of shorts that can be used in the semun union
  for( i=0; i<n; i++ ) vals[i] = (short)(va_arg(init_vals,int));

  union semun us;
  us.array = vals;
  r = semctl(semid,n,SETALL,us);
  exit_err(r,"initializing semaphore values");

  va_end(init_vals);
  return semid;
  // and thats it! :O
}
int sem_connect(int key,int n){
  /* returns semd for a semaphore array under key `key` with length `n` */
  int r = semget(key,n,0);
  exit_err(r,"connecting to semaphores");
  return r;
}
int sem_getval(int semid,int idx){
  /* returns current value of semaphore */
  int r = semctl(semid,idx,GETVAL);
  exit_err(r,"checking semaphore value");
  return r;
}
void sem_remove(int semid){
  /* destroys the semaphore array with id `semid` */
  int r = semctl(semid,0,IPC_RMID);
  exit_err(r,"removing semaphore");
}

void sem_claim(int semid,int idx){
  /* issues a down operation to semaphore at index `idx` of semaphore array `semid`; blocks until complete */
  struct sembuf op;
  op.sem_op = -1;
  op.sem_num = idx;
  int r = semop(semid,&op,1);
  exit_err(r,"claiming semaphore");
}
void sem_release(int semid,int idx){
  /* issues an up operation to semaphore at index `idx` of semaphore array `semid` */
  struct sembuf op;
  op.sem_op = 1;
  op.sem_num = idx;
  int r = semop(semid,&op,1);
  exit_err(r,"releasing semaphore");
}
