#include <stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>

#define MAX 10240

int main(int argc,char * argv[]){
  if(3 != argc){
    printf("error: ./server [ip]  [port]\n");
    return 1;
  }
  char * ip=argv[1];
  int port = atoi(argv[2]);
  int fd = socket(AF_INET,SOCK_STREAM,0);
  if(fd<0){
    perror("socket");
    return 1;
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(ip);
  addr.sin_port = htons(port);
  
  int ret = bind(fd,(struct sockaddr*)&addr,sizeof(addr));
  if(ret<0){
    perror("bind");
    return 1;
  }

  ret = listen(fd,10);
  if(ret<0){
    perror("listen");
    return 1;
  }
  while(1){
    struct sockaddr_in client_addr;
    socklen_t len;
    int client_fd = accept(fd,(struct sockaddr*)&client_addr,&len);
    if(client_fd<0){
      perror("accept");
      continue;
    }
    //用一个足够大的缓冲区，可以直接将数据读完
    char input_buf[MAX]={0};
    ssize_t read_size = read(client_fd,input_buf,sizeof(input_buf)-1);
    if(read_size<0){
      perror("read");
      return 1;
    }
    printf("[Request] %s",input_buf);
    char buf[MAX]={0};
    const char* hello = "<h1>hello world!</h1>";
    sprintf(buf,"HTTP/1.0 200 OK\nContent-Length:%lu\n\n%s",strlen(hello),hello);
    write(client_fd,buf,strlen(buf));
  }
  return 0;
}
