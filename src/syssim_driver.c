/*
 * A sample skeleton for a system simulator that calls DiskSim as
 * a slave.
 *
 * Contributed by Eran Gabber of Lucent Technologies - Bell Laboratories
 *
 * Usage:
 *	syssim <parameters file> <output file> <max. block number>
 * Example:
 *	syssim parv.seagate out 2676846
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>

#include "syssim_driver.h"
#include "disksim_interface.h"
#include "disksim_rand48.h"


#define	BLOCK	4096
#define	SECTOR	512
#define	BLOCK2SECTOR	(BLOCK/SECTOR)

typedef	struct	{
  int n;
  double sum;
  double sqr;
} Stat;


static SysTime now = 0;		/* current time */
static SysTime next_event = -1;	/* next event */
static int completed = 0;	/* last request was completed */
static Stat st;


void
panic(const char *s)
{
  perror(s);
  exit(1);
}


void
add_statistics(Stat *s, double x)
{
  s->n++;
  s->sum += x;
  s->sqr += x*x;
}


void
print_statistics(Stat *s, const char *title)
{
  double avg, std;

  avg = s->sum/s->n;
  std = sqrt((s->sqr - 2*avg*s->sum + s->n*avg*avg) / s->n);
  printf("%s: n=%d average=%f std. deviation=%f\n", title, s->n, avg, std);
}


/*
 * Schedule next callback at time t.
 * Note that there is only *one* outstanding callback at any given time.
 * The callback is for the earliest event.
 */
void
syssim_schedule_callback(disksim_interface_callback_t fn, 
			 SysTime t, 
			 void *ctx)
{
  next_event = t;
}


/*
 * de-scehdule a callback.
 */
void
syssim_deschedule_callback(double t, void *ctx)
{
  next_event = -1;
}

/**
   这个函数是请求完成的回调函数，
   每当一个请求完成时都会调用这个函数，
   所以可以在这个函数里面进行一些请求完成时的业务逻辑操作，
   比如请求的响应时间的计算。
*/
void syssim_report_completion(SysTime t, struct disksim_request *r, void *ctx)
{
  completed = 1;
  now = t;
  add_statistics(&st, t - r->start);
}

/**
	argv[0]:
	argv[1]: parameters file
	argv[2]: output file
	argv[3]: max. block number
*/
int main(int argc, char *argv[])
{
  int i;
  int nsectors;
  struct stat buf;
  struct disksim_request r;
  struct disksim_interface *disksim;

  if (argc != 4 || (nsectors = atoi(argv[3])) <= 0) {
    fprintf(stderr, "usage: %s <param file> <output file> <#sectors>\n",
	    argv[0]);
    exit(1);
  }

  if (stat(argv[1], &buf) < 0)
    panic(argv[1]);

  /**
     实例化一个接口
  */
  disksim = disksim_interface_initialize(argv[1], 
					 argv[2],
					 syssim_report_completion,
					 syssim_schedule_callback,
					 syssim_deschedule_callback,
					 0,
					 0,
					 0);

  /* NOTE: it is bad to use this internal disksim call from external... */
  DISKSIM_srand48(1);

  /**
	  循环1000次，产生随机的1000个请求
	  通过函数 disksim_interface_request_arrive() 发送给 disksim，
	  并通过函数 disksim_interface_internal_event() 处理相应请求的事件。
	  最后调用函数 disksim_interface_shutdown() 关闭接口。
  */
  for (i=0; i < 1000; i++) 
  {
  	  // 一个事件的开始时间，是上一个事件的结束时间
      r.start = now;
      r.flags = DISKSIM_READ;
      r.devno = 0;

    /* NOTE: it is bad to use this internal disksim call from external... */
    //// 随机生成 blkno
	r.blkno = BLOCK2SECTOR*(DISKSIM_lrand48()%(nsectors/BLOCK2SECTOR));
    r.bytecount = BLOCK;
    completed = 0;
	
	//发送请求给disksim
    disksim_interface_request_arrive(disksim, now, &r);

    /* Process events until this I/O is completed */
	//// 当前请求全部完成了才会发送下一个请求
    while(next_event >= 0)
    {
        now = next_event;
        next_event = -1;
	    //处理未完成的事件
        disksim_interface_internal_event(disksim, now, 0);
    }

    if (!completed) {
      fprintf(stderr,
	      "%s: internal error. Last event not completed %d\n",
	      argv[0], i);
      exit(1);
    }
  }

  disksim_interface_shutdown(disksim, now);

  print_statistics(&st, "response time");

  exit(0);
}
