#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/msg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

enum message
{
  no_msg = 0,
  start_msg = -1, 
  ready = -2,
};

struct msgbuf {
  long mtype;       /* message type, must be > 0 */
  int msg;    /* message data */
};


int runner(int runner_n, int id, int N);
int judge(int queue_id, int N);
int create_queue(int msgflg);
void init_runners(int queue_id, int N);