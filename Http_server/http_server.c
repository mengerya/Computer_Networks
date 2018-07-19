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
#include<sys/stat.h>

#define MAX 10240

 pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

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

/*
int isDir(char file_path[]){
  struct stat st;
  int ret = stat(file_path,&st);
  if(ret<0){
    return 0;
  }
  if(S_ISDIR(st.st_mode))
    return 1;
  return 0;
}


void HandlerFilePath(const char* url_path,char file_path[]){
  //给url_path加上前缀（HTTP服务器的根目录）
  //url_path  -->  /index.html 
  //file_path  -->./wwwroot/index.html
  sprintf(file_path,"./wwwroot%s",url_path);
  //例如   
  //url_path    /or    /image/
  //如果输入的网址后面什么都没有写，那么默认追加index.html,静态生成页面
  if(file_path[strlen(file_path)-1] == '/'){
    strcat(file_path,"index.html");
  }
  //url_path  -->  image
  if(isDir(file_path)){
    strcat(file_path,"index.html");
  }
}

int WriteStaticFile(int64_t sock,char file_path[]){

}
*/ 
int ParseCGI(){
  return 404;
}

int ParseStatic(/*int64_t sock,Request * req*/){
  /*
  //1.根据url_path 获取到文件在服务器上的真实路径
  char file_path[MAX] = {0};
  HandlerFilePath(req->url_path,file_path);
  //2.读取文件，把文件的内容直接写到socket之中
  int err_code = WriteStaticFile(sock,file_path);
  return err_code;
  */
  return 404;
}


int split(char * first_line,const char* split_char,char * tok[],int size ){
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


int Parse_url(char *url,char ** url_path,char ** query_string){
  //解析url,区分出 url_path  ,  query_string 
  //在url中，url_path和query_string是以'?'区分的
  //可以继续使用切分函数来实现
  
  char * p = url;
  *url_path = url;
  for(;*p != '\0';++p){
    if(*p == '?'){
      *p = '\0';
      *query_string = p+1;
      return 0;
    }
  }
  *query_string = NULL;
  return 0;
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

int ReadLine(int64_t new_sock,char first_line[],int size){
  //读取首行
  //遇到换行符结束
  char c = '\0';
  int count=0;
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

int ReadHeadler(int64_t new_sock,int * p_content_length){
  //是就读取value,不是就直接丢弃
  //循环的从socket读取一行判定当前行是不是content_length
  //读到空行，循环结束
  
  char buf[MAX]={0};
  while(1){
    ssize_t read_size = ReadLine(new_sock,buf,sizeof(buf));
    if(read_size<=0)
      return -1;
    //处理读完的情况,Headler中没有content_length
    if(strcmp(buf,"\n") == 0){
      return 0;
    }
    //判断当前行是不是content_length
    //是就读取value,不是就直接丢弃
    const char * pCon = "content-Length";
    if(p_content_length != NULL && strncmp(buf,pCon,strlen(pCon)) == 0){
      *p_content_length = atoi(buf+strlen(pCon));
    }
  }
  return 0;
}

void Err_404(int64_t sock){
  //构造一个完整的HTTP响应
  //状态码是404
  //构造一个body部分为404相关的错误页面
  const char* first_line = "HTTP/1.1 404 Not found\n";
  //headler
  const char * type_line = "Content-Type: text/html; charset=utf-8";
  const char * blank_line = "\n";
  //body
  const char * html = "<head><meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\"></head>""<h1>页面无法找到!!</h1>";
  send(sock,first_line,strlen(first_line),0);
  send(sock,type_line,strlen(type_line),0);
  send(sock,blank_line,strlen(blank_line),0);
  send(sock,html,strlen(html),0);
  printf("Err_404 OK\n");
  fflush(stdout);
}

void PrintRequest(Request * req){
  printf("Method:%s\n",req->Method);
  printf("url:%s\n",req->url);
  printf("url_path:%s\n",req->url_path);
  printf("query_string:%s\n",req->query_string);
  printf("Content_Length:%d\n",req->content_length);
}

void HeadlerRequest(int64_t new_sock){
  //反序列化
  printf("HeadlerRequest\n");
  Request req;
  int err_code = 200;//将要返回的状态码
  //读取首行,将读到的内容放入req中的first_line中
  if(ReadLine(new_sock,req.first_line,sizeof(req.first_line))<0){
      printf("ReadLine filed!\n");
      fflush(stdout);
      err_code=404;
      goto END;
      }
  //从首行中读取url   Method
  if(ParseFirstLine(req.first_line,&req.url,&req.Method)<0){
      printf("ParseFirstLine filed!\n");
      fflush(stdout);
      err_code=404;
      goto END;
  }
  //从url中解析出 url_path query_string
  if(Parse_url(req.url,&req.url_path,&req.query_string)<0){

      printf("Parse_url filed!\n");
      fflush(stdout);
      err_code=404;
      goto END;
  }
  //读取Headler中的content_length,其他部分暂时不做考虑
  if(ReadHeadler(new_sock,&req.content_length)<0){
    err_code=404;
    goto END;
  }

  //打印日志
  PrintRequest(&req);

  //根据读到的Method判断方法，并分析该回应动态响应还是静态响应
  if(strcasecmp(req.Method,"GET")== 0 && req.query_string == NULL){
    //返回静态页面
    ParseStatic(/*new_sock,&req*/);
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
  if(200 != err_code){
    Err_404(new_sock);
  }
  close(new_sock);
}

void* CreateWorker(void * arg){
  int64_t new_sock = (int64_t)arg;
  printf("CreateWorker\n");
  HeadlerRequest(new_sock);
  return NULL;
}
 
void tcp_init(const char * ip,short port){
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
    pthread_mutex_lock(&mutex);
    printf("多线程ing\n");
    struct sockaddr_in peer;
    socklen_t len=sizeof(peer);
    int new_sock=accept(sock,(struct sockaddr*)&peer,&len);
    if(new_sock<0){
      perror("accept");
      continue;
    }
    pthread_t tid;
    pthread_create(&tid,NULL,CreateWorker,(void*)new_sock);
    pthread_detach(tid);
    pthread_mutex_unlock(&mutex);
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
