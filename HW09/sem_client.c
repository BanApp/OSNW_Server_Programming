#include <sys/socket.h>  /* 소켓 관련 함수 */
#include <arpa/inet.h>   /* 소켓 지원을 위한 각종 함수 */
#include <sys/stat.h>
#include <stdio.h>      /* 표준 입출력 관련 */
#include <string.h>     /* 문자열 관련 */
#include <unistd.h>     /* 각종 시스템 함수 */
#include <stdlib.h>

#define MAXLINE    1024

int main(int argc, char **argv)
{
    struct sockaddr_in serveraddr;
    int server_sockfd;
    int client_len;
    char buf[MAXLINE];
    char msg[MAXLINE];
    int n;

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
    if (connect(server_sockfd,
                (struct sockaddr *)&serveraddr, client_len)  == -1)
    {
        perror("connect error :");
        return 1;
    }
    memset(msg, 0x00, MAXLINE);
    memset(buf, 0x00, MAXLINE);

    /* 키보드 입력을 기다린다. */
    scanf("%s", buf);

    //문자열의 마지막을 표시하기 위함
    for(int i = 0 ; buf[i] != 0 ; i ++){
        if(buf[i] == '\n'){
            buf[i] = 0;
            break;
        }
    }
    strcpy(msg,buf);

    /* 입력 받은 데이터를 서버로 전송한다. */
    if (write(server_sockfd, msg, MAXLINE) <= 0)
    {
        perror("write error : ");
        return 1;
    }
    memset(buf, 0x00, MAXLINE);

    /* 서버로 부터 데이터를 읽는다. */
    while(1){
        if (n == read(server_sockfd, buf, MAXLINE) <= 0)
        {
            perror("read error : ");
            return 1;
        }

        //문자열의 마지막을 표시하기 위함
        for(int i = 0 ;  buf[i] != 0 ; i ++){
            if(buf[i]==0){
                buf[i] = 0;
                break;
            }
        }
        char str[1000];
        strcpy(str,buf);

        for(int i = 0 ; str[i] != 0 ; i++){	//문자열의 마지막을 표시하기 위함
            if(str[i]==0){
                str[i]=0;
                break;
            }
        }

        printf("read : %s \n", str);
        str[0] = '\n';
        memset(buf,0x00,MAXLINE);
    }
    close(server_sockfd);
    return 0;
}