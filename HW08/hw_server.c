#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define MAXLINE 1024
#define PORTNUM 3600

int main(int argc, char **argv)
{
    int listen_fd;
    int client_fd[3]; //클라이언트가 담길 배열 3개
    pid_t pid;
    socklen_t addrlen;
    int readn;
    int p_readn;
    char buf[MAXLINE] = {};
    struct sockaddr_in client_addr, server_addr;

    char arr[MAXLINE] = {}; //합쳐질 문자가 담길 문자열 초기화
    int cnt = 0; //클라이언트의 순서를 나타냄

    int pp[2]; //pipe 통신
    pipe(pp);


    if( (listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return 1;
    }
    memset((void *)&server_addr, 0x00, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORTNUM);

    if(bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==-1)
    {
        perror("bind error");
        return 1;
    }
    if(listen(listen_fd, 5) == -1)
    {
        perror("listen error");
        return 1;
    }

    signal(SIGCHLD, SIG_IGN);
    while(1)
    {
        addrlen = sizeof(client_addr);
        client_fd[cnt] = accept(listen_fd,
                                (struct sockaddr *)&client_addr, &addrlen);
        if(client_fd[cnt] == -1)
        {
            printf("accept error\n");
            break;
        }
        pid = fork();

        //자식 프로세스
        if(pid == 0)
        {
            close( listen_fd );
            memset(buf, 0x00, MAXLINE);

            //클라이언트로 부터 문자열(buf) 수신
            while((readn = read(client_fd[cnt], buf, MAXLINE)) > 0)
            {
                printf("Read Data %s(%d) : %s",
                       inet_ntoa(client_addr.sin_addr),
                       client_addr.sin_port,
                       buf);

                //부모 프로세스에 문자열(buf) 전송
                write(pp[1], buf, strlen(buf));
                memset(buf, 0x00, MAXLINE);
            }
            return 0;
        }

            //부모 프로세스
        else if( pid > 0)
        {
            //자식 프로세스로 부터 전송받은 값 수신
            p_readn = read(pp[0], buf, MAXLINE);

            //개행문자 제거
            for(int i = 0 ; buf[i]!=0 ; i ++){
                if(buf[i]=='\n')
                {
                    buf[i] = 0;
                    break;
                }
            }

        }
        //띄어쓰기 삽입 & 문자열 합치기
        strcat(arr," ");
        strcat(arr, buf);

        //클라이언트에게 전송
        write(client_fd[cnt], arr, sizeof(arr));
        close(client_fd[cnt]);

        //다음 클라이언트로 순서로 넘어가기 위해 cnt값 증가
        cnt++;
    }
    close(listen_fd);
    return 0;
}