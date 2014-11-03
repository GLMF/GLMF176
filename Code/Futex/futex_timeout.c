/*
Le programme suivant est fourni a titre d'exemple pour illustrer un article.
Les instructions superflues telles que le test des codes de retour des
fonctions ont ete volontairement expurgees pour simplifier la lecture.

Rachid Koucha
*/
#include <errno.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

#define futex_wait_to(addr, val, to) \
           syscall(SYS_futex, addr, FUTEX_WAIT_PRIVATE, val, to)

void mysleep(struct timespec *timeout)
{
int rc;
int futex_var = 0;

  rc = futex_wait_to(&futex_var, 0, timeout);
  printf("rc=%d, errno='%m' (%d)\n", rc, errno);
}

int main(int ac, char *av[])
{
struct timespec timeout;

  timeout.tv_sec = atoi(av[1]);
  timeout.tv_nsec = 0;
  mysleep(&timeout);
} // main
