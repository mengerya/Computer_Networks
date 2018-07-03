#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include <sys/syscall.h>   


void *rout(void *arg){
    (void)arg;
    pid_t pid = syscall(SYS_gettid);
    while(1){
        printf("I am thread:所属进程ID为：%d ,内核级线程ID为： %d,用户态下线程ID为： %lx \n",getpid(),pid,pthread_self());
        sleep(1);
    }
}


int main(){
    pid_t pid = syscall(SYS_gettid); //获得当前进程的内核级线程ID
    int ret;
    pthread_t tid;
    ret=pthread_create(&tid,NULL,rout,NULL);
    if(ret!=0){ 
        perror("pthread_create");
        exit(1);
    }
    while(1){
        printf("I am main: 所属进程ID为： %d 内核级线程ID为： %d 用户态线程ID为： %lx \n",getpid(),pid,pthread_self());
        sleep(1);
    }

    return 0;
}
