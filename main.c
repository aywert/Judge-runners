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
  start_msg = 10, 
  ready = -1,
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
      runner(i+2, queue_id, &buf);
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
    while(msgrcv(queue_id, &buf, sizeof(int), i+2, IPC_NOWAIT) == 0);
    //printf("Judge: runner %d is ready\n", buf->msg);
  }

  buf->mtype = 1;
  buf->msg = start_msg;
  printf("Jundge: Great! Everyone is ready! Start!\n");
  msgsnd(queue_id, &buf, sizeof(struct msgbuf) - sizeof(long), IPC_NOWAIT);
  
  for (int i = 0; i < N; i++)
  {
    while(msgrcv(queue_id, &buf, sizeof(int), 1, IPC_NOWAIT) == 0); // 1 responds for jundge
    printf("Judge: runner %d finished the race!\n", buf->msg);
  }

  printf("Judge: race is over\n");
  

  return 0;
}

int runner(int runner_n, int id, struct msgbuf* buf)
{
  buf->mtype = 1; // respond for jundge
  buf->msg = runner_n;
  //printf("runner = %d\n", runner_n-1);
  msgsnd(id, &buf, sizeof(struct msgbuf) - sizeof(long), IPC_NOWAIT);

  while(msgrcv(id, &buf, sizeof(int), runner_n, IPC_NOWAIT) == 0);
  
  buf->mtype = 1;
  buf->msg = runner_n-1;
  msgsnd(id, &buf, sizeof(struct msgbuf) - sizeof(long), IPC_NOWAIT);

  buf->mtype = runner_n+1;
  printf("runner: i am %d giving estapheta to next runner\n", runner_n-1);
  msgsnd(id, &buf, sizeof(struct msgbuf) - sizeof(long), IPC_NOWAIT);
  return 0;
}