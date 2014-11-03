/*
Le programme suivant est fourni a titre d'exemple pour illustrer un article.
Les instructions superflues telles que le test des codes de retour des
fonctions ont ete volontairement expurgees pour simplifier la lecture.

Rachid Koucha
*/
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#define __USE_GNU
#include <sys/time.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *thd_main(void *p)
{
struct timespec timeout;
int             rc;
struct timeval  tv;

  gettimeofday(&tv, NULL);
  tv.tv_sec += 10;
  TIMEVAL_TO_TIMESPEC(&tv, &timeout);

  printf("Echeance absolue : { %ld, %ld }\n", timeout.tv_sec, timeout.tv_nsec);

  rc = pthread_mutex_timedlock(&mutex, &timeout);
  printf("rc = %d\n", rc);  

  return NULL;
}

int main(int ac, char *av[])
{
int       rc;
pthread_t tid;

  pthread_mutex_lock(&mutex);

  pthread_create(&tid, NULL, thd_main, NULL);

  pthread_join(tid, NULL);

  return 0;
} // main
