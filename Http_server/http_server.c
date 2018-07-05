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

typedef struct Arg{
	int fd;
	sockaddr_in addr;
}Arg;

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

void* HttpServer(void* ptr){
	Arg* arg = (Arg*)ptr;
	int fd = arg->fd;
	sockaddr_in addr=arg->addr;
	//解析首行
	//对首行进行解析（解析出方法，url,url_path,query_string）
	
	
	//对header进行解析（只保留了Content_Length）
	//对于静态页面，根据url_path,打开对应的文件，根据文件内容构造HTTP响应就可以了
	//对于动态页面，按照CGI的规则来生成动态页面
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
		Arg* arg = (Arg*)malloc(sizeof(Arg));
		arg->fd = new_fd;
		arg->addr = new_addr;
		pthread_create(&tid,NULL,HttpServer,(void*)arg);
		pthread_detach(tid);
	}
}
int main(int argv,char *argv[]){
	if(3 != argc){
		printf("errno:[./server][IP][port]\n");
		return 1;
	}
	tcp_inio(argv[1],atoi(argv[2]));
}

