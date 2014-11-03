/*
Le programme suivant est fourni a titre d'exemple pour illustrer un article.
Les instructions superflues telles que le test des codes de retour des
fonctions ont ete volontairement expurgees pour simplifier la lecture.

Rachid Koucha
*/
#define _GNU_SOURCE
#include <sched.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <errno.h>
#include <assert.h>

#define futex_lock_pi_to(addr, to) \
  syscall(SYS_futex, addr, FUTEX_LOCK_PI, 0, to)

#define futex_unlock_pi(addr) \
  syscall(SYS_futex, addr, FUTEX_UNLOCK_PI, 0, 0)

#define futex_wait_to(addr, to) \
           syscall(SYS_futex, addr, FUTEX_WAIT_PRIVATE, 0, to)

#define gettid() syscall(__NR_gettid)

int futex_var;

void *thd_main(void *p)
{
struct sched_param param;
int                old;
int                mytid = gettid();
struct timespec    timeout;
int                rc;
struct timeval     t;
unsigned int       seconds = *((unsigned int *)p);

  gettimeofday(&t, NULL);
  fprintf(stderr, "0x%x : Date courante %lu s / %lu us\n", mytid, t.tv_sec, t.tv_usec);

  old = __sync_val_compare_and_swap(&futex_var, 0, mytid);

  fprintf(stderr, "0x%x : La valeur du futex@%p avant LOCK est : 0x%x\n", mytid, &futex_var, old);

  switch(old)
  {
    case 0 : // Le futex etait libre
    {
      // Desormais le thread courant detient le futex
      assert(0);
    }
    break;

    default : // Le futex n'est pas libre
    {
      timeout.tv_sec = (t.tv_sec += seconds);
      timeout.tv_nsec = t.tv_usec * 1000;
      fprintf(stderr, "0x%x : timeout a la date %lu s / %lu us\n", mytid, t.tv_sec, t.tv_usec);
      rc = futex_lock_pi_to(&futex_var, &timeout);
    }
  }

  gettimeofday(&t, NULL);

  fprintf(stderr, "0x%x : La valeur du futex@%p apres LOCK est : 0x%x\n"
                  "       Le code de retour de l'appel futex() est : %d (errno = %d)\n"
                  "       Date courante %lu s / %lu us\n"
          ,
	  mytid, &futex_var, futex_var, rc, errno, t.tv_sec, t.tv_usec);

  fprintf(stderr, "0x%x : Thread termine\n", mytid);

  return NULL;
}

int main(int ac, char *av[])
{
pthread_t          tid;
int                mytid = gettid();
unsigned int       duree;

  duree = (unsigned int)atoi(av[1]);

  // Le thread courant verrouille le futex
  futex_var = gettid();

  pthread_create(&tid, NULL, thd_main, (void *)&duree);

  pthread_join(tid, NULL);

  return 0;
}
