#include "disksim_global.h"

/**
	disksim的主函数
*/
int main (int argc, char **argv)
{
  int len;


#ifndef _WIN32
  setlinebuf(stdout);
  setlinebuf(stderr);
#endif

  if(argc == 2) {
     disksim_restore_from_checkpoint (argv[1]);
  } 
  else 
  {
  	//给全局变量 disksim 分配内存
    disksim = calloc(1, sizeof(struct disksim));
	//初始化 disksim 结构体
    disksim_initialize_disksim_structure(disksim);
	// 建立 disksim
	/*
	这里调用的是 disksim_setup_disksim (argc, argv) 这个函数，
	这个函数比较复杂，主要是对 disksim 做一些初始化操作。
	大致功能如下：设置对齐方式(大端对齐还是小端对齐)，设置输出文件，
	设置 trace 的格式，设置输入的 trace 数据的文件，判断是否启用 synthgen，
	设置重写的参数，根据配置文件设置参数，iosim_info 的初始化，
	最后就是为开始模拟磁盘做准备。
	*/
	disksim_setup_disksim (argc, argv);
  }
  //开始模拟
  /*
  这个函数并没有传入任何参数，可见模拟的是全局的 disksim 
  */
  disksim_run_simulation ();
  //到这一步，磁盘的模拟工作已经结束了，接下主要是释放程序占用的内存，
  //关闭相应的文件，然后打印输出程序的运行结果。
  disksim_cleanup_and_printstats ();
  exit(0);
}
