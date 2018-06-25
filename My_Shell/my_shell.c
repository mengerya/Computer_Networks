#include<stdio.h>
#include<unistd.h>
#include<wait.h>
#include<stdlib.h>
#include<string.h>

char *argv[8];
int argc = 0;

void do_parse(char* buf){
    int i;
    int status = 0;

    for(argc=i=0;buf[i];i++){
        if(!isspace(buf[i]) && status == 0){
            argv[argc++] = buf + i;
            status = 1;
        }else if(isspace(buf[i])){
            status = 0;
            buf[i] = 0;
        }
    }
    argv[argc] = NULL;
}

void do_execute(){
    pid_t pid = fork();

    if(pid > 0){
        //father
        int st;
        while(wait(&st) != pid)
            ;
    }else if(pid == 0){
        //child
        execvp(argv[0],argv);
        perror("execvp");
        exit(EXIT_FAILURE);
    }else{
        perror("fork!");
        exit(EXIT_FAILURE);
    }
}

int main(){
    /*
     *1、获取命令行
     *
     *2、解析命令行
     *
     *3、建立一个子进程(fork)
     *
     *4、替换子进程(execvp)
     *
     *5、父进程等待子进程退出(wait)
     *
     **/
    char buf[1024] = {};
    while(1){
        printf("myshell> ");
        scanf("%[^\n]%*c",buf);
        do_parse(buf);
        do_execute();
    }

    return 0;
}
