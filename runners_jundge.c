#include "runners_jundge.h"                                                                        \

int judge(int queue_id, int N)
{
  struct msgbuf buf = {0, no_msg};

  for (int i = 0; i < N; i++)
  {
    if (msgrcv(queue_id, &buf, sizeof(int), N+1, 0) == -1)
    {
      fprintf(stderr, "msgrcv4: %s\n", strerror(errno));
      exit(1);
    }

    printf("Judge: runner %d is ready\n", buf.msg);
  }

  printf("Judge: Great! Everyone is ready! Start!\n");

  buf.mtype = 1;
  buf.msg = start_msg;
  msgsnd(queue_id, &buf, sizeof(int), 0);
  
  if (msgrcv(queue_id, &buf, sizeof(int), N+1, 0) == -1)
  {
    fprintf(stderr, "msgrcv3: %s\n", strerror(errno));
    exit(1);
  }

  printf("Judge: race is over!\n");
  

  return 0;
}

int runner(int runner_n, int queue_id, int N)
{
  struct msgbuf buf = {0, no_msg};
  buf.mtype = N+1; // N+1 responds for jundge
  buf.msg = runner_n;
  
  printf("runner %d: I'm ready!\n", runner_n);
  msgsnd(queue_id, &buf, sizeof(int), 0);

  if (msgrcv(queue_id, &buf, sizeof(int), runner_n, 0) == -1) 
  {
    fprintf(stderr, "msgrcv1: %s\n", strerror(errno));
    exit(1); 
  }

  if (runner_n < N)
  {
    buf.mtype = runner_n+1;
    buf.msg = start_msg;
    for (int i = 0; i < N; i++) printf("runner_n = %d\n", runner_n);
    printf("runner %d: giving estapheta to next runner!\n", runner_n);
    msgsnd(queue_id, &buf, sizeof(int), 0);
  }

  else 
  {
    buf.mtype = N+1;
    buf.msg = start_msg;
    printf("runner %d: Hey jundge, my team is finished!\n", runner_n);
    msgsnd(queue_id, &buf, sizeof(int), 0);
  }

  exit(1);
  return 0;
}

int create_queue(int msgflg)
{
  int queue_id = msgget(ftok("main.c", 1), msgflg);

  if(queue_id == -1) {
    fprintf(stderr, "msgrcv2: %s\n", strerror(errno));
    exit(1); 
  }

  return queue_id; 
}

void init_runners(int queue_id, int N)
{
  for (int i = 0; i < N; i++) {
    pid_t pid = fork();

    if (pid == -1) {
      fprintf(stderr, "fork: %s\n", strerror(errno));
      exit(1);
    }

    if (pid == 0) {
      runner(i+1, queue_id, N);
      return;
    }
  } 
  
  return;
}
