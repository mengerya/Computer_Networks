#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<pthread.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/wait.h>
#include<strings.h>

#define MAX 10240



typedef struct Request{
  char first_line[MAX];//首行
  //这里忽略不管首行中的版本
  char * Method;//首行中的方法
  char * url;
  char *url_path;
  char *query_string;
  int content_length;//Headler中的content_length,
  // 为了简单，这里不展开其他headler,如果要展开的话，可以考虑哈希表
}Request;

void ParseCGI(){

}

void ParseStatic(){

}

int Parse_url(char *url,char ** url_path,char ** query_string){

}

int split(char * first_line,const char * split_char,char * tok[],int size ){
  //根据空格切分字符first_line
  //将空格换成'\0'
  //将切割后的字符串，每一个子串的首地址存入指针数组tok中
  //使用strtok函数进行切割，
  //但是strtok使用静态变量实现的，所以再多线程环境下，
  //他是不安全的，有可能会导致程序崩溃
  //在这里要使用线程安全函数  strtok_r
  char * ptr;
  int i=0;
  char * temp = NULL;
  ptr=strtok_r(first_line,split_char,&temp);
  while(ptr != NULL){
    if(i >= size){
      return i;
    }
    tok[i++] = ptr;
    ptr=strtok_r(NULL,split_char,&temp);
  }
  return i;
}

int ParseFirstLine(char * first_line,char ** url,char ** method){
  //从首行中解析出url,method
  //忽略首行中的版本号
  //它们之间是用空格分割的
  //把首行按照空格进行切割
  char * tok[10];
  int tok_size = split(first_line," ",tok,10);
  if(tok_size != 3){
    printf("split failed! tok_size = %d\n",tok_size);
    return -1;
  }
  *method = tok[0];
  *url = tok[1];
  return tok_size;
}

int ReadFirstLine(int new_sock,char* first_line,int size){
  //读取首行
  //遇到换行符结束
  char c = '\0';
  int count=0;
  //考虑到版本兼容问题，可能遇到的换行符有  \n   \r    \r\n
  //解决办法： 如果是  \r  \r\n，将它们都转换成\n
  while(count<size-1 && c != '\n'){
    ssize_t read_size = recv(new_sock,&c,1,0);//一次只读取一个字符
    if(read_size<0)
      return -1;//读取失败
    else if(read_size == 0)
      return -1;//读到EOF
    if(c == '\r'){
      recv(new_sock,&c,1,MSG_PEEK);
        //MSG_PEEK 查看当前数据。数据将被复制到缓冲区中，但并不从输入队列中删除
      if(c == '\n'){
        //当前换行字符为 \r\n
        //直接将 \n 从new_sock中取出来
        recv(new_sock,&c,1,0);
      }
      else{
        //当前换行字符为 \r,不用把当前字符从缓冲区中取出，直接把 \r  转换成 \n
        c = '\n';
      }
    }
    first_line[count++] = c;
  }
  first_line[count]='\0';
  return count;
}

void Err_404(){

}

void HeadlerRequest(int64_t new_sock){
  //反序列化
  Request req;
  int err_code = 200;//将要返回的状态码
  
  //读取首行,将读到的内容放入req中的first_line中
  printf("first_line len:%ld\n",sizeof(req.first_line));
  if(ReadFirstLine(new_sock,req.first_line,sizeof(req.first_line)<0)){
      err_code=404;
      goto END;
      }
  //从首行中读取url   Method
  if(ParseFirstLine(req.first_line,&req.url,&req.Method)<0){
      err_code=404;
      goto END;
  }
  //从url中解析出 url_path query_string
  if(Parse_url(req.url,&req.url_path,&req.query_string)<0){

      err_code=404;
      goto END;
  }
  //根据读到的Method判断方法，并分析该回应动态响应还是静态响应
  if(strcasecmp(req.Method,"GET")== 0 && req.query_string == NULL){
    //返回静态页面
    ParseStatic();
  }
  else if(strcasecmp(req.Method,"GET") == 0 && req.query_string != NULL){
    //返回动态页面
    ParseCGI();
  }else if(strcasecmp(req.Method,"POST") == 0){
    ParseCGI();
  }
  else{
    
      err_code=404;
      goto END;
  }
END:
if(200 != err_code)
  Err_404();

  close(new_sock);

}

void* CreateWorker(void * arg){
  int64_t new_sock = (int64_t)arg;
  HeadlerRequest(new_sock);
  return NULL;
}

void tcp_init(char * ip,short port){
  int sock = socket(AF_INET,SOCK_STREAM,0);
  if(sock<0){
    perror("sock");
    return;
  }
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(ip);
  addr.sin_port = htons(port);

  int ret = bind(sock,(struct sockaddr*)&addr,sizeof(addr));
  if(ret<0){
    perror("bind");
    return;
  }

  ret = listen(sock,10);
  if(ret<0){
    perror("listen");
    return;
  }

  printf("server init OK!\n");

  //多线程处理请求
  while(1){
    struct sockaddr_in peer;
    socklen_t len;
    int new_sock=accept(sock,(struct sockaddr*)&peer,&len);
    if(new_sock<0){
      perror("accept");
      continue;
    }
    pthread_t tid = 0;
    pthread_create(&tid,NULL,CreateWorker,(void*)new_sock);
    pthread_detach(tid);
  }
}

int main(int argc,char * argv[]){
  if(3 != argc){
    printf("error: ./server  [ip] [port]\n");
    return 1;
  }
  tcp_init(argv[1],atoi(argv[2]));
  return 0;
}
