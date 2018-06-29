#include"comm.h"

int main(){
    
    int msgid = getMsgQueue();

    char buf[1024];

    while(1){
        buf[0] = 0;
        printf("Please Enter:> ");
        fflush(stdout);
        ssize_t s = read(0,buf,sizeof(buf));
        if(s > 0){
            //成功从输出端读取到数据
            buf[s-1] = 0;
            //将读取到的数据放入消息管道中，该消息的类型为CLIENT_TYPE
            sendMsg(msgid,CLIENT_TYPE,buf);
            printf("sent done,wait recv...\n");
        }

        //将消息管道中类型为SERVER_TYPE的消息取出，放入buf中
        recvMsg(msgid,SERVER_TYPE,buf);
        //输出类型为SERVER_TYPE的消息
        printf("[server]:> %s\n",buf);
    }
    return 0;
}
