#include <sys/socket.h>  /* 소켓 관련 함수 */
#include <arpa/inet.h>   /* 소켓 지원을 위한 각종 함수 */
#include <sys/stat.h>
#include <stdio.h>      /* 표준 입출력 관련 */
#include <string.h>     /* 문자열 관련 */
#include <unistd.h>     /* 각종 시스템 함수 */
#include <stdlib.h>
#include "time.h"

#define MAXLINE    1024

struct send_data{
    char str[MAXLINE];
};
struct rev_data{
    char str[MAXLINE];
};

int main(int argc, char **argv)
{
    struct sockaddr_in serveraddr;
    int server_sockfd;
    int client_len;
    char buf[MAXLINE];
    struct rev_data rev_msg;    //서버에 보내는 데이터
    struct send_data send_msg;  //서버에서 받는 데이터
    int maxfd = 0;
    char temp_buf[MAXLINE];

    fd_set temps, reads;
    int result;
    struct timeval tv;

    if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("error :");
        return 1;
    }

    /* 연결요청할 서버의 주소와 포트번호 프로토콜등을 지정한다. */
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serveraddr.sin_port = htons(3600);

    client_len = sizeof(serveraddr);

    /* 서버에 연결을 시도한다. */
    if (connect(server_sockfd, (struct sockaddr *)&serveraddr, client_len)  == -1)
    {
        perror("connect error :");
        return 1;
    }

    FD_ZERO(&reads);
    FD_SET(server_sockfd, &reads);
    FD_SET(0,&reads);
    maxfd = server_sockfd;

    while(1)
    {
        temps = reads;
        tv.tv_sec = 4;
        tv.tv_usec = 0;

        result = select(maxfd + 1, &temps, 0, 0, &tv);

        if(result == -1){
            printf("error \n");
            return 1;
        }else if(result ==0){
            continue;
        }else{

            if(FD_ISSET(0,&temps)){

                memset(buf, 0x00, MAXLINE);
                read(0, buf, MAXLINE);    /* 키보드 입력을 기다린다. */
                if(strncmp(buf, "quit\n",5) == 0)
                    break;
                memset(rev_msg.str, 0x00, MAXLINE);
                memset(&rev_msg, 0x00, sizeof(struct rev_data));
                char *ptr = strtok(buf," ");
                strcpy(rev_msg.str,ptr);

                struct tm newtime;
                time_t ltime;
                char time_str[100];
                ltime=time(&ltime);
                localtime_r(&ltime, &newtime);
                asctime_r(&newtime, time_str);

                for(int i=0; rev_msg.str[i] != 0; i++)
                {
                    if(rev_msg.str[i] == '\n')
                    {
                        rev_msg.str[i] = 0;
                        break;
                    }
                }
                strcat(rev_msg.str," ");
                strcat(rev_msg.str,time_str);

                if (write(server_sockfd, &rev_msg, sizeof(rev_msg)) <= 0) /* 입력 받은 데이터를 서버로 전송한다. */
                {
                    perror("write error : ");
                    return 1;
                }
                memset(buf, 0x00, MAXLINE);
                FD_CLR(0,&temps);
            }

            if(FD_ISSET(server_sockfd,&temps)){
                /* 서버로 부터 데이터를 읽는다. */
                if (read(server_sockfd, &send_msg, sizeof(send_msg)) <= 0)
                {
                    perror("read error : ");
                    return 1;
                }
                printf("read : %s\n", send_msg.str);
                sleep(2);
            }
        }
    }
    close(server_sockfd);
    return 0;
}
