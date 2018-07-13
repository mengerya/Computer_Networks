#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<pthread.h>

#define CONSUMERS_COUNT 2
#define PRODUCERS_COUNT 2

typedef struct msg{
  struct msg *next;
  int num;
}msg;

msg *head = NULL;
pthread_cond_t cond; //条件变量
pthread_mutex_t mutex; //互斥量
pthread_t threads[CONSUMERS_COUNT+PRODUCERS_COUNT];//生产者消费者模型中的缓冲池

void *consumer(void *p){
  int num = *(int *)p;
  free(p);//p是在main函数里malloc的

  msg *mp;
  while(1){
    //给互斥量mutex加锁
    pthread_mutex_lock(&mutex);
    while(head == NULL){
      printf("%d begin wait a condition...\n",num);
      pthread_cond_wait(&cond,&mutex);
    }

    printf("%d end wait a condition...\n",num);
    printf("%d begin consume product...\n",num);

    mp = head;
    head = mp->next;

    pthread_mutex_unlock(&mutex);
    printf("Consume %d\n",mp->num);
    free(mp);
    printf("%d end consume product...\n",num);
    sleep(rand()%5);
  }

}

void *producer(void *p){
  msg *mp;
  int num = *(int *)p;
  free(p);

  while(1){
    printf("%d begin produce product...\n",num);
    mp = (msg*)malloc(sizeof(msg));
    mp->num = rand()%1000 + 1;
    printf("produce %d\n",mp->num);
    pthread_mutex_lock(&mutex);
    mp->next = head;
    head = mp;
    printf("%d end produce product...\n",num);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    sleep(rand()%5);
  }
}

int main(void){
  srand(time(NULL));

  //初始化条件变量cond，互斥量mutex
  pthread_cond_init(&cond,NULL);
  pthread_mutex_init(&mutex,NULL);

  int i;
  for(i=0;i<CONSUMERS_COUNT;i++){
    int *p = (int *)malloc(sizeof(int));
    if(NULL == p){
      perror("malloc");
      continue;
    }
    *p=i;
    pthread_create(&threads[i],NULL,consumer,(void*)p);
  }

  for(i=0;i<PRODUCERS_COUNT;i++){
    int *p = (int *)malloc(sizeof(int));
    *p = i;
    if(NULL == p){
      perror("malloc");
      continue;
    }
    pthread_create(&threads[CONSUMERS_COUNT+i],NULL,producer,(void*)p);
  }
  for(i=0;i<CONSUMERS_COUNT+PRODUCERS_COUNT;i++){
    pthread_join(threads[i],NULL);
  }

  //销毁互斥量mutex，条件变量cond
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&cond);
}
