#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<errno.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>

#define _PORT_ 9999
#define _BACKLOG_ 10


int main(){
    //创建socket文件描述符（TCP  客户端 + 服务器）
    int sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock < 0){
        printf("create socket error,error : %d , error string : %s \n",errno,strerror(errno));
    }

    //sockaddr结构（地址类型 端口号 IP地址）
    struct sockaddr_in server_socket;
    struct sockaddr_in client_socket;

    //初始化server_socket
    bzero(&server_socket,sizeof(server_socket));
    server_socket.sin_family = AF_INET;
    //INADDR_ANY就是指定地址为0.0.0.0的地址，这个地址事实上表示不确定地址，或"所有地址"、"任意地址"。
    server_socket.sin_addr.s_addr = htonl(INADDR_ANY);
    server_socket.sin_port = htons(_PORT_);

    //绑定端口号
    if(bind(sock,(struct sockaddr*)&server_socket,sizeof(struct sockaddr_in)) < 0){
        //绑定失败
        printf("bind error,errno: %d , error string: %s \n",errno,strerror(errno));
        close(sock);
        return 1;
    }

    //开始监听socket
    //_BACKLOG_   最多可监听10个
    if(listen(sock,_BACKLOG_) < 0){
        printf("lisen error,errno: %d , error string: %s \n",errno,strerror(errno));
        close(sock);
        return 2;
    }
    
    printf("bind and listen success! wait accept...\n");
    //循环接收客户端的请求
    while(1){
        socklen_t len = 0;
        int client_sock = accept(sock,(struct sockaddr *)&client_socket,&len);
        if(client_sock < 0){
            printf("accept error,client ip: %s , error: %d , error string: %s \n",inet_ntoa(client_socket.sin_addr),errno,strerror(errno));
            close(sock);
            return 3;
        }
        //从accept返回的client_sock中读取客户端的请求
        char buf_ip[INET_ADDRSTRLEN] = {0};
        //将客户端IP地址转换成字符串存入缓冲区buf_ip中
        inet_ntop(AF_INET,&client_socket.sin_addr,buf_ip,sizeof(buf_ip));

        printf("[connect] ip: %s ,port: %d\n",buf_ip,ntohs(client_socket.sin_port));

        while(1){
            //根据读取到的请求进行计算和处理
            //这里实现一个简单的回显服务器，对客户端请求不作任何处理，直接输出
            char buf[1024] = {0};
            read(client_sock,buf,sizeof(buf));
            printf("client :# %s\n",buf);
            printf("server :$ ");
            //把处理后的结果发送给客户端
            memset(buf,'\0',sizeof(buf));
            fgets(buf,sizeof(buf),stdin);
            buf[strlen(buf)-1] = '\0';
            write(client_sock,buf,strlen(buf)+1);
            printf("please wait...\n");
        }
    }
    close(sock);
    return 0;

}
