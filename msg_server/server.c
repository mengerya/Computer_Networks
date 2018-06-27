#include"comm.h"

int main(){
    //创建消息队列
    int msgid = createMsgQueue();
    char buf[1024];

    while(1){
        buf[0] = 0;
        //将消息队列中CLIENT_TYPE类型的消息取出并输出
        recvMsg(msgid,CLIENT_TYPE,buf);
        printf("[client]:> %s\n",buf);

        printf("Please Enter:>");
        //刷新缓存区
        fflush(stdout);

        ssize_t s = read(0,buf,sizeof(buf));
        if(s > 0){
            //将输出端的数据写入消息管道中，该消息的类型为SERVER_TYPE
            buf[s-1] = 0;
            sendMsg(msgid,SERVER_TYPE,buf);
            printf("send done,wait recv...\n");
        }
    }

    //销毁消息队列
    destoryMsgQueue(msgid);
    return 0;
}
