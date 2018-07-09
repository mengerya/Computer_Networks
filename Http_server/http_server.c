
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<pthread.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/wait.h>
#define SIZE 10240


typedef struct Request{

	char first_line[SIZE];
	char *method;
	char *url_path;
	char *query_string;
	//char *version;
	//接下来是header部分，如果要完整的解析下来，此处需要hash表，或二叉搜索树
	//偷懒：不要其他header,只保留一个content_length
	//content_length是一个整数，用int就可以了
	int content_length;
}Request;

int ReadLine(int sock,char buf[],ssize_t  size){
  //一次从socket中读取一行字符
  //把数据放到缓冲区中
  //如果读取失败，就返回-1
  //可能遇到的换行符有   \n    \r     \r\n
  //1.从socket中一次读取一个字符
  char c = '\0';
  ssize_t count = 0;//当前读了多少个字符
  //结束条件：读的长度太长，达到了缓冲区长度的上线
  //读到了'\n',要考虑兼容问题（将 \r  \r\n  转换成  \n）
  while(count < (size-1) && c != '\n'){
    ssize_t read_size=recv(sock,&c,1,0);
    if(read_size<0)
      return -1;
    else if(read_size == 0)
      return -1;
    if(c == '\r'){
      //MSG_PEEK选项从内核的缓冲区中读取出字符时，并不会从缓冲区中删除掉该字符
      //如果遇到\r   再看下一字符是不是 \n   
      recv(sock,&c,1,MSG_PEEK);
      if(c == '\n'){
        //此时的分隔符为  \r\n
        read_size = recv(sock,&c,1,0);
      }
      else{
        //此时的分隔符为 \r  把分割符转换成  \n
        c = '\n';
      }
    }
    //   \r\n   \r  都转换成功
    buf[count++]=c;
  }
  buf[count]='\0';
  return count;//返回成功读到首行的字符数
}

void* HttpServer(void* ptr){
  
	int err_code = 200;
	//对字符进行反序列化
		Request req;
		memset(&req,0,sizeof(req));
	//从socket中解析首行
    if(ReadLine(fd,req.first_line,sizeof(req.first_line))<0){
      printf("ReadLine\n");
      fflush(stdout);
      goto END;
    }
	
	//对首行进行解析（解析出方法，url,url_path,query_string）
	
	
	//对header进行解析（只保留了Content_Length）
	//对于静态页面，根据url_path,打开对应的文件，根据文件内容构造HTTP响应就可以了
	//对于动态页面，按照CGI的规则来生成动态页面
END:
    if(err_code != 200){
      ERR_404(fd);
    }
    close(fd);
}

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

	ret = listen(fd,SOMAXCONN);
	if(ret < 0){
		perror("listen");
		return;
	}

	printf("Server Inio OK\n");

	while(1){
		struct sockaddr_in new_addr;
		socklen_t len = sizeof(new_addr);
		int new_fd = accept(fd,(struct sockaddr*)&new_addr,&len);
		if(new_fd < 0){
			perror("accept");
			continue;
		}
		pthread_t tid = 0;
		pthread_create(&tid,NULL,HttpServer,(void*)new_fd);
		pthread_detach(tid);
	}
}
int main(int argc,char *argv[]){
	if(3 != argc){
		printf("errno:[./server][IP][port]\n");
		return 1;
	}
	tcp_inio(argv[1],atoi(argv[2]));
}

