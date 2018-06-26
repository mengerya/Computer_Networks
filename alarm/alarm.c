#include<stdio.h>
#include<unistd.h>

size_t count = 0;
int main(){
    alarm(1);
    while(1){
        ++count;
        printf("%lu\n",count);
    }
    return 0;
}
