#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include "err.h"


#define READERS 2
#define WRITERS 1
#define NAP 5
#define BSIZE 32

struct readwrite {
  pthread_mutex_t lock;         
  pthread_cond_t readers;      
  pthread_cond_t writers;    
  int rcount, wcount, rwait, wwait;
  int change;
};

struct readwrite library;
char book[BSIZE];
int working = 1;

/* Initialize a buffer */

void init(struct readwrite *rw) {
  int err;
  if ((err = pthread_mutex_init(&rw->lock, 0) != 0))
    perror("mutex init failed");
  if ((err = pthread_cond_init(&rw->readers, 0)) != 0)
    perror("cond init readers failed");
  if ((err = pthread_cond_init(&rw->writers, 0)) != 0)
    perror("cond init writers failed");
  rw->rcount = 0;
  rw->wcount = 0;
  rw->rwait = 0;
  rw->wwait = 0;
  rw->change = 1;
  int l = rand()%10;
  snprintf(book, BSIZE, "6 times a number %d %d %d %d %d %d", l, l, l, l, l, l);
}

/* Destroy the buffer */

void destroy(struct readwrite *rw) {
  int err;
  if ((err = pthread_cond_destroy (&rw->readers)) != 0)
    perror("cond destroy readers failed");
  if ((err = pthread_cond_destroy (&rw->writers)) != 0)
    perror("cond destroy writers failed");
  if ((err = pthread_mutex_destroy (&rw->lock)) != 0)
    perror("mutex destroy failed");
}

void *reader(void *data)
{
   int err;
   if ((err = pthread_mutex_lock(&library.lock)) != 0)
    perror("lock failed");
   while (working) {
     // got lock
     library.rwait++;
     if (library.wcount == 0) {
       library.rcount++;
       if ((err = pthread_mutex_unlock(&library.lock)) != 0)
         perror("unlock failed");
     }
     else {
       if ((err = pthread_mutex_unlock(&library.lock)) != 0)
         perror("unlock failed");

       if ((err = pthread_cond_wait(&library.readers, &library.lock)) != 0)
         perror("cond wait failed");
        if (!working)
        break;
     }
     
     printf("reader read: %s\n", book); /* reading */
     if ((err = pthread_mutex_lock(&library.lock)) != 0)
       perror("lock failed");
     library.rwait--;
     library.rcount--;
     
     if (library.rcount == 0) {
       if (library.wcount < library.wwait) {
         library.wcount++;
         if ((err = pthread_cond_signal(&library.writers)) != 0)
           perror("cond signal failed");
       }
     }
     if ((err = pthread_mutex_unlock(&library.lock)) != 0)
         perror("unlock failed");
    sleep(1);
     if ((err = pthread_mutex_lock(&library.lock)) != 0)
       perror("lock failed");
     if (!working)
        break;
   }
   return 0;
}

void *writer(void *data)
{  
   int l;
   int err;
   if ((err = pthread_mutex_lock(&library.lock)) != 0)
    perror("lock failed");
   while (working) {
      library.wwait++;
      if (library.rcount == 0) {
        library.wcount++;
        if ((err = pthread_mutex_unlock(&library.lock)) != 0)
         perror("unlock failed");
      }
      else {
        if ((err = pthread_mutex_unlock(&library.lock)) != 0)
         perror("unlock failed");
        if ((err = pthread_cond_wait(&library.writers, &library.lock)) != 0)
         perror("cond wait failed");
       if (!working)
        break;
      }
     /* beginning of writing */
     l = rand()%10;
     if ((err = pthread_mutex_lock(&library.lock)) != 0)
       perror("lock failed");
     snprintf(book, BSIZE, "6 times a number %d %d %d %d %d %d", l, l, l, l, l, l);
     /* end of writing */
     library.wwait--;
     library.wcount--;
     library.change = 1 - library.change;
     if (library.change) {
       if (library.wcount < library.wwait) {
         library.wcount++;
         if ((err = pthread_cond_signal(&library.writers)) != 0)
           perror("cond signal failed");
       }
       else {
         while (library.rcount < library.rwait) {
          library.rcount++;
          if ((err = pthread_cond_signal(&library.readers)) != 0)
            perror("cond signal failed");
          }
        }
     }
     else {
         while (library.rcount < library.rwait) {
           library.rcount++;
           if ((err = pthread_cond_signal(&library.readers)) != 0)
             perror("cond signal failed");
        }
    }
      if ((err = pthread_mutex_unlock(&library.lock)) != 0)
         perror("unlock failed");
      sleep(1);
      if ((err = pthread_mutex_lock(&library.lock)) != 0)
       perror("lock failed");
     if (!working)
        break;
   }
   return 0;
}


int main() {
  pthread_t th[READERS+WRITERS];
  pthread_attr_t attr;
  int i, err;
  void *retval;


  srand((unsigned)time(0));
  
  if ((err = pthread_attr_init(&attr)) != 0 )
    perror("attr_init");

   if ((err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE)) != 0)
      perror("setdetach");

  init(&library);

  /* creating threads */

  for (i=0; i<WRITERS; i++) {
      if ((err = pthread_create(&th[i], &attr, writer, NULL)) != 0)
        perror("create");
  }
  
  for (i=WRITERS; i<READERS+WRITERS; i++) {
      if ((err = pthread_create(&th[i], &attr, reader, NULL)) != 0)
        perror("create");
  }

  sleep(NAP);
  if ((err = pthread_mutex_lock(&library.lock)) != 0)
    perror("lock failed");
  working = 0;
  //printf("WORKING 0\n");
  /* cmt */ exit(0);
  if ((err = pthread_mutex_unlock(&library.lock)) != 0)
    perror("unlock failed");
  
  /* waiting for threads */
  for (i = 0; i<READERS+WRITERS; i++) {
    if ((err = pthread_join(th[i], (void **) &retval)) != 0)
      perror("join");
  }

  destroy(&library);
  return 0;
}