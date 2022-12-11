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
#include <time.h>

#define MAXBUF 1024




int main(int argc, char **argv)
{
    int server_sockfd, client_sockfd;
    int client_len, n, cnt = 0;
    char buf[MAXBUF];
    char msg[MAXBUF];
    struct sockaddr_in clientaddr, serveraddr;
    client_len = sizeof(clientaddr);
    int pid;
    int pid_s;

    if ((server_sockfd = socket (AF_INET, SOCK_STREAM,
                                 IPPROTO_TCP )) == -1)
    {
        perror("socket error : ");
        exit(0);
    }
    memset(&serveraddr, 0x00, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(atoi(argv[1]));

    bind (server_sockfd, (struct sockaddr *)&serveraddr,
            sizeof(serveraddr));
    listen(server_sockfd, 5);

    memset(msg, 0x00, MAXBUF);

    while(1)
    {
        client_sockfd = accept(
                server_sockfd,(struct sockaddr *)&clientaddr,&client_len);
        printf("New Client[%d] Connect: %s port : %d\n",cnt,
               inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port));

        pid = fork();

        if(pid  == 0){
            close(server_sockfd);
            memset(buf, 0x00, MAXBUF);

            //client read
            n = read(client_sockfd, buf, MAXBUF);
            buf[n]= '\0';
            char str[100];
            strcpy(str,buf);

            for(int i = 0 ; str[i] != 0 ; i ++){
                if(str[i]==0){
                    str[i] = 0;
                    break;
                }
            }

            printf("Read Data Client[%d] : %s(%d) : %s \n",cnt,
                   inet_ntoa(clientaddr.sin_addr),
                   ntohs(clientaddr.sin_port),str);

            //공유메모리(Shared Memory)
            int shmid[3], semid[3];
            char *rot_str[3]; //회전 문자열을 담을 배열(공유메모리)
            void *shared_memory[3];
            shared_memory[cnt] = NULL;

            union semun sem_union[3];
            struct sembuf semopen[3] = {{0,-1,SEM_UNDO},
                    {0,-1,SEM_UNDO},{0,-1,SEM_UNDO}};
            struct sembuf semclose[3] = {{0,1,SEM_UNDO},
                    {0,1,SEM_UNDO},{0,1,SEM_UNDO}};

            semid[cnt] = semget((key_t)3477+cnt,1,IPC_CREAT|0666);
            shmid[cnt] = shmget((key_t)1234+cnt,
                                sizeof(200), 0666|IPC_CREAT);
            shared_memory[cnt] = shmat(shmid[cnt],NULL,0);
            rot_str[cnt] = (int*)shared_memory[cnt];

            pid_s = fork();

            if(pid_s == 0){ //소비자
                while(1){
                    //크리티컬 섹션
                    semop(semid[cnt],&semopen[cnt],1);
                    sleep(1);
                    char t[1000];
                    strcpy(str,rot_str[cnt]);
                    strcpy(t,str);
                    strcat(t," ");

                    /*날짜,시간 표현을 위한 tm 구조체 */
                    struct tm newtime;
                    time_t ltime;
                    char time_str[100];
                    ltime=time(&ltime);
                    localtime_r(&ltime, &newtime);
                    asctime_r(&newtime, time_str);
                    strcat(t,time_str);

                    //클라이언트에 전달
                    write(client_sockfd,t,sizeof(t));

                    semop(semid[cnt],&semclose[cnt],1);
                    //크리티컬 섹션
                }

            }else if(pid_s > 0){ //생산자

                sem_union[cnt].val = 100;
                semctl(semid[cnt], 0, SETVAL,sem_union[cnt]);
                strcpy(rot_str[cnt],str);
                while(1){
                    //크리티컬 섹션
                    semop(semid[cnt],&semopen[cnt],1);
                    sleep(1);
                    char tmp = rot_str[cnt][0];
                    for(int j = 0 ; j < strlen(rot_str[cnt])-1 ; j++){
                        rot_str[cnt][j] = rot_str[cnt][j+1];
                    }
                    rot_str[cnt][strlen(rot_str[cnt])-1]=tmp;
                    semop(semid[cnt],&semclose[cnt],1);
                    //크리티컬 섹션
                }
            }

            close(client_sockfd);
            return 0;
        }
        else if(pid > 0 ){
            close(client_sockfd);
            cnt++;
        }
    }
    close(server_sockfd);
    return 0;
}