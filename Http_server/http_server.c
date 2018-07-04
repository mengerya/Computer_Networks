#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<pthread.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/wait.h>

typedef struct Arg {
 int fd;
 struct sockaddr_in addr;
} Arg;

void tcp_inio(char * ip,int port){
        //套接字
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(ip);
        addr.sin_port = htons(port);

        int fd = socket(AF_INET,SOCK_STREAM,0);
        if(fd < 0){
                perror("socket");
                return;
        }

        int ret = bind(fd,(struct sockaddr*)&addr,sizeof(addr));
        if(ret < 0){
                perror("bind");
                return;
        }

        ret = listen(fd,10)；
        if(ret < 0){
                perror("listen");
                return;
        }
 
        while(1){
                struct sockaddr_in new_addr;
                socklen_t len = sizeof(new_addr);
                int new_fd = accept(fd,(struct sockaddr*)&new_addr,&len);
                if(new_fd < 0){
                        perror("accept");
                        continue;
                }
                pthread_t tid = 0;
                Arg* arg = (Arg*)malloc(sizeof(Arg));
                arg->fd = client_fd;
                arg->addr = client_addr;
                pthread_create(&tid,NULL, CreateWorker, (void*)arg)
        }
}

int main(int argc,char * argv[]){
        if(3 != argc){
              printf("error:./server  [ip]   [port]"\n);
              return 1;
        }
        
        tcp_inio(argv[1],atoi(argv[2]));
        return 0;
}
