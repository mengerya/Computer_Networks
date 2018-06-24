#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/wait.h>

#define MAX_SIZE 10

/*
 *创建socket
 *绑定端口号
 *把socket转换成被动模式(listen)
 *循环的进行accept（父子进程）
 *从accept中读取客户端的请求
 *对接收到的数据进行计算和处理
 *把处理后的结果返回给客户端
 */


void CreateWorker(int client_fd,struct sockaddr_in *client_addr){

    //创建父子进程
    pid_t pid = fork();
    if(pid>0){
        //father
        //非阻塞式等待子进程
        //既要很快的调用到accept,也不能产生僵尸进程
        //所以可以采用捕捉信号的方法
        close(client_fd);
        return;
    }
    else if(0 == pid){
        //child
        //子进程循环处理从客户端接收到的数据
        char buf[1024]={0};
        while(1){
            ssize_t read_size = read(client_fd,buf,sizeof(buf)-1);
            if(read_size < 0){
                perror("read");
                continue;
            }
            if(0 == read_size){
                printf("client:%s  say bye!",inet_ntoa(client_addr->sin_addr));
                close(client_fd);
                break;
            }

            buf[read_size] = '\0';
            printf("client %s :# %s\n",inet_ntoa(client_addr->sin_addr),buf);

            write(client_fd,buf,strlen(buf));
        }
    }
    else{
        perror("fork");
        return;
    }
}

int main(int argc,char *argv[]){

    if(3 != argc){
        printf("usage: ./server [ip] [port]\n");
        return 1;
    }
    //捕捉SIGCHLD信号
    signal(SIGCHLD,SIG_BLOCK);

    int fd = socket(AF_INET,SOCK_STREAM,0);
    if(fd<0){
        perror("socket");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    int ret = 0;
    ret = bind(fd,(struct sockaddr*)&addr,sizeof(addr));
    if(ret<0){
        perror("bind");
        return 2;
    }

    ret = listen(fd,MAX_SIZE);
    if(ret<0){
        perror("listen");
        return 3;
    }

    while(1){

        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int client_fd = accept(fd,(struct sockaddr*)&client_addr,&len);
        if(client_fd < 0){
            perror("accept");
            continue;
        }

        //利用进程完成循环接收客户端的请求
        CreateWorker(client_fd,&client_addr);

    }

    return 0;
}
