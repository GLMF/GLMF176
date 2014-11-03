/*
Le programme suivant est fourni a titre d'exemple pour illustrer un article.
Les instructions superflues telles que le test des codes de retour des
fonctions ont ete volontairement expurgees pour simplifier la lecture.

Rachid Koucha
*/
#include <assert.h>
#include <limits.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <stdio.h>
#include <pthread.h>

#define futex_wait(addr, val) \
           syscall(SYS_futex, addr, FUTEX_WAIT, val, 0)
#define futex_wakeup(addr, nb) \
           syscall(SYS_futex, addr, FUTEX_WAKE, nb)

#define futex_wait_bitset(addr, val, bitset)   \
  syscall(SYS_futex, addr, FUTEX_WAIT_BITSET, val, 0, 0, bitset)
#define futex_wakeup_bitset(addr, nb, bitset)  \
  syscall(SYS_futex, addr, FUTEX_WAKE_BITSET, nb, 0, 0, bitset)

struct
{
  int          lock;
  int          wait;
  unsigned int readers;
  unsigned int waiting_readers;
  unsigned int writer;
  unsigned int waiting_writers;
} rwlock;

int nb_thread;

void LOCK(int *f)
{
int old;

  old = __sync_val_compare_and_swap(f, 0, 1);

  switch(old)
  {
    case 0:
    {
      // Le futex etait libre car il contenait la valeur 0
      // et maintenant il contient la valeur 1 car au moment
      // de l'opération atomique il n'y avait pas de thread
      // en attente
      return;
    }
    break;

    default:
    {
      // Le mutex contient la valeur 1 ou 2 : il est occupe car
      // un autre thread l'a pris

      // On positionne le futex a la valeur 2 pour indiquer qu'il
      // y a des threads en attente
      old = __sync_lock_test_and_set(f, 2);

      while (old != 0)
      {
        // Attente tant que le futex a la valeur 2
        futex_wait(f, 2);

        // Le futex a soit la valeur 0 (libre) ou 1 ou 2
        old = __sync_lock_test_and_set(f, 2);
      }
    }
    break;
  }
}

void UNLOCK(int *f)
{
int old;

  old = __sync_fetch_and_sub(f, 1);

  switch(old)
  {
    case 1:
    {
      // Il n'y avait pas de thread en attente
      // ==> Personne à réveiller
    }
    break;

    case 2:
    {
      // Il y avait des threads en attente
      // Le futex a la valeur 1, on le force à 0
      old = __sync_lock_test_and_set(f, 0);

      // Reveil des threads en attente
      futex_wakeup(f, 1);
    }
    break;

    default:
    {
      // Impossible
    }
  }
}



void RWLOCK_READ(void)
{
  LOCK(&(rwlock.lock));

  while(1)
  {
    // S'il n'y a pas d'ecrivain en cours et pas d'ecrivain en attente
    // ==> Le lecteur prend le rwlock
    if (0 == rwlock.writer && 0 == rwlock.waiting_writers)
    {
      rwlock.wait = 1;
      rwlock.readers ++;

      UNLOCK(&(rwlock.lock));

      break;
    }
    else // Il y a un ecrivain en cours ou en attente
    {
      // Un lecteur supplementaire en attente
      rwlock.waiting_readers ++;

      UNLOCK(&(rwlock.lock));

      // Mise en attente de liberation par l'ecrivain
      futex_wait_bitset(&(rwlock.wait), 1, 0x01);

      LOCK(&(rwlock.lock));

      // Un lecteur en attente de moins
      rwlock.waiting_readers --;
    }
  }
}

void RWLOCK_WRITE(void)
{
  LOCK(&(rwlock.lock));

  while(1)
  {
    // S'il n'y a pas d'ecrivain et de lecteurs en cours
    // ==> L'ecrivain prend le rwlock
    if (0 == rwlock.writer && 0 == rwlock.readers)
    {
      rwlock.wait = 1;
      rwlock.writer = 1;

      UNLOCK(&(rwlock.lock));

      break;
    }
    else // Il y a un ecrivain ou des lecteurs en cours
    {
      // Un ecrivain supplementaire en attente
      rwlock.waiting_writers ++;

      UNLOCK(&(rwlock.lock));

      // Mise en attente de liberation par les lecteurs ou l'ecrivain
      futex_wait_bitset(&(rwlock.wait), 1, 0x02);

      LOCK(&(rwlock.lock));

      // Un ecrivain de moins en attente
      rwlock.waiting_writers --;
    }
  }
}

void RWUNLOCK(void)
{  
  LOCK(&(rwlock.lock));

  // Si c'est l'ecrivain qui deverrouille
  if (rwlock.writer)
  {
    // Il n'y a plus d'ecrivain en cours
    assert(1 == rwlock.writer);
    rwlock.writer = 0;
  }
  else // C'est un lecteur qui deverrouille
  {
    // Decrementation du nombre de lecteurs
    assert(rwlock.readers > 0);
    rwlock.readers --;

    // S'il reste des lecteurs
    if (0 != rwlock.readers)
    {
      // Les lecteurs gardent le rwlock
      UNLOCK(&(rwlock.lock));

      return;
    }
  }

  // Il n'y ni ecrivain, ni lecteur en cours
  // ==> On reveille les eventuels ecrivains ou lecteurs
  // en attente en donnant priorite aux ecrivains
  rwlock.wait = 0;

  // S'il y a au moins un ecrivain en attente, on le reveille
  if (rwlock.waiting_writers)
  {
    UNLOCK(&(rwlock.lock));

    futex_wakeup_bitset(&(rwlock.wait), 1, 0x02);
  }
  else if (rwlock.waiting_readers)
  {
    // On reveille les lecteurs en attente
    UNLOCK(&(rwlock.lock));

    futex_wakeup_bitset(&(rwlock.wait), INT_MAX, 0x01);
  }
  else
  {
    UNLOCK(&(rwlock.lock));
  }
}

static void *thd_main(void *p)
{
int *idx = (int *)p;

  printf("Thread %d demarre...\n", *idx);

  while (1)
  {
    RWLOCK_READ();

    if (*idx >= nb_thread)
    {
      RWUNLOCK();
      break;
    }

    RWUNLOCK();
  }

  printf("Thread %d se termine...\n", *idx);

  return NULL;
}

int main(int ac, char *av[])
{
int       nb;
pthread_t tid[255];
int       param[255];
int       i;
int       nb_thread_old;

  for (i = 0; i < 255; i ++)
  {
    param[i] = i;
  }

  while(1)
  {
    printf("Nombre de threads : ");
    if (EOF == fscanf(stdin, "%d", &nb))
    {
      break;
    }

    if (nb > 255)
    {
      fprintf(stderr, "La valeur max est 255\n");
      continue;
    }

    RWLOCK_WRITE();

    nb_thread_old = nb_thread;
    nb_thread = nb;

    RWUNLOCK();

    if (nb < nb_thread_old)
    {
      // Attente de la fin des threads en trop
      for (i = nb; i < nb_thread_old; i ++)
      {
        pthread_join(tid[i], NULL);
      }
    }
    else if (nb > nb_thread_old)
    {
      // Creation des threads supplementaires
      for (i = nb_thread_old; i < nb; i ++)
      {
        pthread_create(&(tid[i]), NULL, thd_main, &(param[i]));
      }
    }
  }

  return 0;
}

