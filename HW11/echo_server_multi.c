#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#define MAXLINE 1024
#define PORTNUM 3600

struct send_data{
    char str[MAXLINE];
};
struct rev_data{
    char str[MAXLINE];
};

int main(int argc, char **argv)
{
    int listen_fd, client_fd;
    socklen_t addrlen;
    int fd_num;
    int maxfd = 0;
    int sockfd;
    int i= 0;
    char buf[MAXLINE];
    fd_set readfds, allfds;

    struct rev_data rev_msg;
    //char send_msg[MAXLINE];
    struct send_data send_msg;
    struct timeval tv;

    bool flag = false;

    struct sockaddr_in server_addr, client_addr;

    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket error");
        return 1;
    }
    memset((void *)&server_addr, 0x00, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORTNUM);

    if(bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind error");
        return 1;
    }
    if(listen(listen_fd, 5) == -1)
    {
        perror("listen error");
        return 1;
    }

    FD_ZERO(&readfds);
    FD_SET(listen_fd, &readfds);

    maxfd = listen_fd;
    while(1)
    {
        allfds = readfds;
        if(flag){
            for(i = 4 ; i <= maxfd ; i++){
                if(FD_ISSET(i, &allfds)){
                    write(i,&send_msg,sizeof(send_msg));
                }
            }
        }
        tv.tv_sec = 3;
        tv.tv_usec = 0;

        fd_num = select(maxfd + 1 , &allfds, (fd_set *)0,
                        (fd_set *)0, &tv);

        if (FD_ISSET(listen_fd, &allfds))
        {
            addrlen = sizeof(client_addr);
            client_fd = accept(listen_fd,
                               (struct sockaddr *)&client_addr, &addrlen);

            FD_SET(client_fd,&readfds);
            if (client_fd > maxfd)
                maxfd = client_fd;
            printf("Accept OK : %s (%d) \n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
            continue;
        }

        for (i = 0; i <= maxfd; i++)
        {
            sockfd = i;
            if (FD_ISSET(sockfd, &allfds))
            {
                memset(&rev_msg, 0x00, sizeof(struct rev_data));
                if (read(sockfd, &rev_msg, sizeof(rev_msg)) <= 0)
                {
                    close(sockfd);
                    FD_CLR(sockfd, &readfds);
                }
                else
                {
                    if (strncmp(rev_msg.str, "quit\n", 5) ==0)
                    {
                        close(sockfd);
                        FD_CLR(sockfd, &readfds);
                    }else if(strlen(rev_msg.str)==0){
                        continue;
                    }
                    else
                    {   flag = true;
                        printf("client: %s \n", rev_msg.str);
                        strcpy(send_msg.str, rev_msg.str);
                    }
                }
                if (--fd_num <= 0)
                    break;
            }
        }
    }
}