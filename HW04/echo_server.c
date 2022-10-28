#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXBUF 1024

int main(int argc, char **argv)
{
	int server_sockfd, client_sockfd;
	int client_len, n;
	char buf[MAXBUF];
	struct sockaddr_in clientaddr[3]; //구조체 배열로 clientaddr을 크기 3으로 선언, 각 클라이언트의 정보가 담긴다.
    struct sockaddr_in serveraddr;
	client_len = sizeof(clientaddr[0]);
	if ((server_sockfd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP )) == -1)
	{
		perror("socket error : ");
		exit(0);
	}
	memset(&serveraddr, 0x00, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(atoi(argv[1]));

	bind (server_sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	listen(server_sockfd, 5);

    char arr[4000] = {}; //buf로 입력받은 값을 strcat으로 합쳐서 클라이언트에 돌려줄 문자열 선언

    for(int i = 0; i<3; i++) //for문으로 3명의 클라이언트를 받도록 설정, 한번의 for문에 한명의 클라이언트
    {
        while(1) //클라이언트 접속까지 대기
        {
            client_sockfd = accept(server_sockfd, (struct sockaddr *)&clientaddr[0],
                                   &client_len);
            printf("New Client Connect: %s\n", inet_ntoa(clientaddr[0].sin_addr));

            memset(buf, 0x00, MAXBUF);
            if ((n = read(client_sockfd, buf, MAXBUF)) <= 0)
            {
                close(client_sockfd);
                continue;
            }

            strcat(arr,buf); //arr 문자열에 client로 부터 입력받은 buf를 strcat으로 이어 붙여준다.

            for(int j=0; j<4000; j++) //한줄에 출력하기 위해 줄바꿈 문자를 띄어쓰기로 바꿔준다.
                if(arr[j]=='\n')
                    arr[j]=' ';

            if (write(client_sockfd, arr, MAXBUF) <=0)
            {
                perror("write error : ");
                close(client_sockfd);
            }
            close(client_sockfd);

        }

    }
	close(server_sockfd);
	return 0;
}
