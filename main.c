#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

struct msgbuf {
  long mtype;       /* message type, must be > 0 */
  int msg;    /* message data */
};

int runner(int runner_n, int id, struct msgbuf* buf);
int jundge(struct msqid_ds* My_st, int queue_id, struct msgbuf* buf, int N);

static int N = 5;

enum message
{
  no_msg = 0,
  start_msg = -1, 
  ready = -2,
};


int main(void)
{

  int queue_id = msgget(ftok("main.c", 1), IPC_CREAT|0666);

  struct msqid_ds My_st = {};
  struct msgbuf buf = {1, no_msg};

  for (int i = 0; i < N; i++)
  {
    pid_t pid = fork();
    if (pid == -1)
    {
      fprintf(stderr, "fork: %s\n", strerror(errno));
      exit(-1);
    }

    if (pid == 0)
    {
      runner(i+1, queue_id, &buf);
      return 0;
    }
  }

  jundge(&My_st, queue_id, &buf, N);

  msgctl(queue_id, IPC_RMID, &My_st);

  return 0;
}

int jundge(struct msqid_ds* My_st, int queue_id, struct msgbuf* buf, int N)
{
  for (int i = 0; i < N; i++)
  {
    int n = msgrcv(queue_id, buf, sizeof(int), N+1, 0);
    if (n == -1)
    {
      fprintf(stderr, "msgrcv: %s\n", strerror(errno));
      exit(1);
    }

    printf("Judge: runner %d is ready\n", buf->msg);
  }

  buf->mtype = 1;
  buf->msg = start_msg;
  printf("Jundge: Great! Everyone is ready! Start!\n");
  msgsnd(queue_id, buf, sizeof(int), 0);
  
  if (msgrcv(queue_id, buf, sizeof(int), N+1, 0) == -1)
  {
    fprintf(stderr, "msgrcv: %s\n", strerror(errno));
    exit(1);
  }

  printf("Race is over!\n");
  

  return 0;
}

int runner(int runner_n, int id, struct msgbuf* buf)
{
  buf->mtype = N+1; // Т+1 responds for jundge
  buf->msg = runner_n;
  
  printf("i am runner number %d sending msg with N+1 type\n", runner_n);
  msgsnd(id, buf, sizeof(int), 0);

  if (msgrcv(id, buf, sizeof(int), runner_n, 0) == -1) 
  {
    fprintf(stderr, "msgrcv: %s\n", strerror(errno));
    exit(1); 
  }

  if (runner_n < N)
  {
    buf->mtype = runner_n+1;
    printf("runner: i am %d giving estapheta to next runner!\n", runner_n);
    for (int i = 0; i < N; i++) printf("runner_n = %d\n", runner_n);
    msgsnd(id, buf, sizeof(int), 0);
  }

  else 
  {
    buf->mtype = N+1;
    printf("runner: i am %d. Hey jundge, my team is finished!\n", runner_n);
    msgsnd(id, buf, sizeof(int), 0);
  }

  // buf->mtype = runner_n+1;
  // printf("runner: i am %d giving estapheta to next runner\n", runner_n-1);
  // msgsnd(id, &buf, sizeof(struct msgbuf) - sizeof(long), IPC_NOWAIT);
  return 0;
}