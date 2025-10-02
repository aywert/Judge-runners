#include "runners_jundge.h"
#include <sys/types.h>


static int N = 5;

int main(void)
{

  int queue_id = create_queue(IPC_CREAT|0666);

  init_runners(queue_id, N);

  jundge(queue_id, N);

  struct msqid_ds My_st = {};
  msgctl(queue_id, IPC_RMID, &My_st);

  return 0;
}

