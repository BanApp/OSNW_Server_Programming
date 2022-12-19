#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <time.h>

#define MAXBUF 1024

pthread_mutex_t mutex;
pthread_cond_t cond;
char send_msg[MAXBUF];

int cnt = 0;

void * producer_thread(void *data){
    int sockfd = *((int *)data);
    int readn;

    socklen_t addrlen;
    char buf[MAXBUF];
    struct sockaddr_in client_addr;
    memset(buf, 0x00, MAXBUF);
    addrlen = sizeof(client_addr);
    getpeername(sockfd, (struct sockaddr *)&client_addr, &addrlen);

    //데이터 읽어오기
    readn = read(sockfd, buf, MAXBUF);
    buf[readn] = '\0';

    char temp_str[100];
    int temp_num;

    strcat(temp_str, buf);

    for(int i = 0 ; temp_str[i] != 0 ; i++){
        if(temp_str[i] == 0){
            temp_str[i] = 0;
            break;
        }
    }

    int flag = cnt;
    cnt += 1;

    //읽어온 데이터 나누기
    while(1){
        pthread_mutex_lock(&mutex);
        pthread_cond_signal(&cond);
        //str데이터 회전하기
        char imsi=temp_str[strlen(temp_str)-1];
        for(int i = strlen(temp_str)-1 ; i >= 0 ; i--){
            temp_str[i] = temp_str[i-1];
        }
        temp_str[0] = imsi;

        //num 데이터 증가하기
        struct tm newtime;
        time_t ltime;
        char time_str[100];
        ltime=time(&ltime);
        localtime_r(&ltime, &newtime);
        asctime_r(&newtime, time_str);

        //클라이언트에게 보낼 메시지
        char t[1000];
        strcat(send_msg,temp_str);
        strcat(send_msg,"/");
        strcat(send_msg,time_str);
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
}

void * consumer_thread(void *data){
    int sockfd = *((int *)data);
    int readn;
    socklen_t addrlen;
    char buf[MAXBUF];
    struct sockaddr_in client_addr;
    memset(buf, 0x00, MAXBUF);
    getpeername(sockfd, (struct sockaddr *)&client_addr, &addrlen);
    int flag = cnt;
    while(1){
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&cond, &mutex);
        write(sockfd,send_msg,MAXBUF);
        //sleep(1);
        send_msg[0] = '\0';
        pthread_mutex_unlock(&mutex);
    }
}

int main(int argc, char **argv)
{
    int server_sockfd, client_sockfd;
    int client_temp[3];
    int client_len, n, cnt = 0;
    char buf[MAXBUF];
    char msg[MAXBUF];
    struct sockaddr_in clientaddr, serveraddr;
    client_len = sizeof(clientaddr);
    pthread_t thread_id, thread_id1;

    if ((server_sockfd = socket (AF_INET,
                                 SOCK_STREAM, IPPROTO_TCP )) == -1)
    {
        perror("socket error : ");
        exit(0);
    }
    memset(&serveraddr, 0x00, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(atoi(argv[1]));

    bind (server_sockfd,
          (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    listen(server_sockfd, 5);

    memset(msg, 0x00, MAXBUF);

    while(1)
    {
        client_sockfd = accept(server_sockfd,
                               (struct sockaddr *)&clientaddr,
                                       &client_len);

        if(client_sockfd == -1){
            printf("accept error \n");
        }else{
            printf("before server cnt : %d \n",cnt);
            pthread_create(&thread_id, NULL,
                           producer_thread, (void *)&client_sockfd);
            pthread_create(&thread_id1, NULL,
                           consumer_thread, (void *)&client_sockfd);
            printf("after server cnt : %d \n",cnt);
            pthread_detach(thread_id);
            pthread_detach(thread_id1);
            cnt += 1;
        }
    }
    close(server_sockfd);
    return 0;
}