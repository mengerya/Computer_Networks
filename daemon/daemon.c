#include<stdio.h>
#include<signal.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/stat.h>

void mydaemon(void){
  int fd0;
  pid_t pid;
  struct sigaction sa;

  umask(0);
  //1. 调用umask将文件模式创建屏蔽字设置为0
  //2.调用fork,父进程退出（exit）
  //如果该守护进程是作为一条简单的shell命令启动的，那么父进程终止使得shell认为该命令已经执行完了
  //保证子进程不是一个进程组的组长进程
  if((pid = fork())<0){
    perror("fork");
  }else if(pid > 0){
    //父进程
    exit(0);
    //终止父进程
  }else{
    //子进程
    //调用setsid创建一个新会话
    setsid();
    //忽略SIGCHLD信号
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if(sigaction(SIGCHLD,&sa,NULL)<0){
      //注册子进程退出忽略信号
      return;
    }
    //在此fork,终止父进程，保持子进程不是话首进程，从而保证后续不会在和其他终端关联
    //这部分不是必须的
    if((pid = fork())<0){
      printf("fork error!\n");
      return;
    }
    else if(pid != 0){
      //father
      exit(0);
    }
    //将当前工作目录更改为根目录
    if(chdir("/")<0){
      printf("child dir error\n");
      return;
    }
    //关闭不再需要的文件描述符，或者重定向到 /dev/null 
    close(0);
    fd0 = open("/dev/null",O_RDWR);
    dup2(fd0,1);
    dup2(fd0,2);
  }
}

int main(){
  mydaemon();
  while(1){
    sleep(1);
  }
  return 0;
}
