#include"comm.h"

//success  >0,failed == -1
static int commMsgQueue(int flags){

    //ftok通过目录生成唯一的身份标识key
    key_t _key = ftok(PATHNAME,PROJ_ID);
    if(_key < 0){
        printf("ftok!");
        perror("ftok");
        return -1;
    }
    /********************************************/
    //创建一个消息队列
    int msgid = msgget(_key,flags);
    if(msgid < 0){
        printf("msgget!");
        perror("msgget");
    }

    return msgid;
}

//创建一个消息队列
int createMsgQueue(){
    return commMsgQueue(IPC_CREAT|IPC_EXCL|0666);
}

//访问一个消息队列
int getMsgQueue(){
    return commMsgQueue(IPC_CREAT);
}

//销毁消息队列
int destoryMsgQueue(int msgid){
    if(msgctl(msgid,IPC_RMID,NULL) < 0){
        printf("msgctl!");
        perror("msgctl");
        return -1;
    }
    return 0;
}

//发送消息
int sendMsg(int msgid,int who,char *msg){
    msgbuf buf;
    buf.mtype = who;
    strcpy(buf.mtext,msg);

    if(msgsnd(msgid,(void*)&buf,sizeof(buf.mtext),0) < 0){
        printf("msgsnd!");
        perror("msgsnd");
        return -1;
    }

    return 0;
}
/************************************************/
//接收消息
int recvMsg(int msgid,int recvType,char out[]){
    msgbuf buf;
    buf.mtype = recvType;
    if(msgrcv(msgid,(void *)&buf,sizeof(buf.mtext),recvType,0) < 0){
        printf("msgrcv!");
        perror("msgrcv");
        return -1;
    }
    
    strcpy(out,buf.mtext);
    return 0;
}
