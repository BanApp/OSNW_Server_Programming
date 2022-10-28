#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define PORT 3600

struct cal_data
{
        int left_num;
        int right_num;
        char op;
        int result;
        int min_result;
        int max_result;
        u_long min_add,max_add; //각 최대,최소값의 ip주소를 담을 변수
        u_short min_pt,max_pt; //각 최대,최소값의 포트번호를 담을 변수
        short int error;
};

int main(int argc, char **argv)
{
        //클라이언트 3개를 받아야 하기 때문에 구조체 배열을 선언
        struct sockaddr_in client_addr[3];
        struct sockaddr_in sock_addr;
        int listen_sockfd;
        //클라이언트 3개를 받아야 하기 때문에 구조체 배열을 선언
        int client_sockfd[3];
        int addr_len;
        struct cal_data rdata;
        int left_num, right_num, cal_result;
        short int cal_error;

        int min = 2147483647; //최소값을 임시로 담을 변수
        int max = -2147483648; //최대값을 임시로 담을 변수

        u_long min_address; //min값의 ip주소를 임시로 담을 변수
        u_long max_address; //max값의 ip주소를 임시로 담을 변수
        u_short max_port; //max값의 포트주소를 임시로 담을 변수
        u_short min_port; //min값의 포트주소를 임시로 담을 변수

        if( (listen_sockfd  = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
                perror("Error ");
                return 1;
        }

        memset((void *)&sock_addr, 0x00, sizeof(sock_addr));
        sock_addr.sin_family = AF_INET;
        sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        sock_addr.sin_port = htons(PORT);

        if( bind(listen_sockfd, (struct sockaddr *)&sock_addr,
                sizeof(sock_addr)) == -1)
        {
                perror("Error ");
                return 1;
        }

        if(listen(listen_sockfd, 5) == -1)
        {
                perror("Error ");
                return 1;
        }

        //3개의 클라이언트를 받기위해 반복문으로 3번 진행한다.
        for(int i=0; i<3; i++)
        {
                addr_len = sizeof(client_addr[i]);
                client_sockfd[i] = accept(listen_sockfd,
                        (struct sockaddr *)&client_addr[i], &addr_len);
                if(client_sockfd[i] == -1)
                {
                        perror("Error ");
                        return 1;
                }
                printf("New Client Connect : %s\n",
                       inet_ntoa(client_addr[i].sin_addr));

                read(client_sockfd[i], (void *)&rdata, sizeof(rdata));

                char *ptr = (char *) &rdata;

                for(int i=0; i< sizeof(rdata); i++)
                    printf("%02x ", *(ptr+i) & 0xFF);
                printf("\n");

                cal_result = 0;
                cal_error = 0;

                left_num = ntohl(rdata.left_num);
                right_num = ntohl(rdata.right_num);

                switch(rdata.op)
                {
                        case '+':
                                cal_result = left_num + right_num;
                                break;
                        case '-':
                                cal_result = left_num  - right_num;
                                break;
                        case '*':
                                cal_result = left_num * right_num;
                                break;
                        case '/':
                                if(right_num == 0)
                                {
                                        cal_error = 2;
                                        break;
                                }
                                cal_result = left_num / right_num;
                                break;
                        default:
                                cal_error = 1;

                }

                //계산결과가 max 값보다 작다면
                if(cal_result > max)
                {
                    //max값을 갱신해주고 max_port,max_address를 갱신한다.
                    max = cal_result;
                    max_address = client_addr[i].sin_addr.s_addr;
                    max_port = client_addr[i].sin_port;

                }

                //계산결과가 min값 보다 작다면
                if(cal_result < min)
                {
                    //min값을 갱신해주고 max_port,max_address를 갱신한다.
                    min = cal_result;
                    min_address = client_addr[i].sin_addr.s_addr;
                    min_port = client_addr[i].sin_port;

                }

                rdata.result = htonl(cal_result);
                rdata.error = htons(cal_error);

                //결과값과 ip주소는 htonl() - "Host to Network Long"로 변환
                rdata.max_result = htonl(max);
                rdata.max_add = htonl(max_address);

                //포트넘버는 htons() - "Host to Network Short"로 변환
                rdata.max_pt = htons(max_port);

                //결과값과 ip주소는 htonl() - "Host to Network Long"로 변환
                rdata.min_result = htonl(min);
                rdata.min_add = htonl(min_address);

                //포트넘버는 htons() - "Host to Network Short"로 변환
                rdata.min_pt = htons(min_port);

                printf("%d %c %d = %d \n", left_num, rdata.op,
                       right_num, cal_result);

                ptr = (char *) &rdata;

                for(int i=0; i< sizeof(rdata); i++)
                    printf("%02x ", *(ptr+i) & 0xFF);
                printf("\n");
        }

        for(int i =0; i<3; i++)
        {
            write(client_sockfd[i], (void *)&rdata,
                  sizeof(rdata));
            close(client_sockfd[i]);
        }

        close(listen_sockfd);
        return 0;
}

