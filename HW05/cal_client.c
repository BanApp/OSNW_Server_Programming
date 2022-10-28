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
#define IP "127.0.0.1"

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
    struct sockaddr_in addr;

    //ip주소를 할당받을 sockaddr_in의 구조체 변수 선언
    struct sockaddr_in min_addr,max_addr;
    int s;
    int len;
    int sbyte, rbyte;
    struct cal_data sdata;
    if (argc < 4)
    {
   	 printf("Usage : %s [num1] [num2] [op]\n", argv[0]);
   	 return 1;
    }

    memset((void *)&sdata, 0x00, sizeof(sdata));
    sdata.left_num = atoi(argv[1]);
    sdata.right_num = atoi(argv[2]);

    //입력시 문자 '*'를 문자 'c'로 인식하는 경우가 발생해서 수정함.
    if(argv[3][0] == 'c')
        sdata.op = '*';
    else
        sdata.op = argv[3][0];

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == -1)
    {
   	 return 1;
    }
   
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);

    if ( connect(s, (struct sockaddr *)&addr, sizeof(addr)) == -1 )
    {
   	 printf("fail to connect\n");
   	 close(s);
   	 return 1;
    }

    len = sizeof(sdata);
    sdata.left_num = htonl(sdata.left_num);
    sdata.right_num = htonl(sdata.right_num);

    sbyte = write(s, (char *)&sdata, len);
    if(sbyte != len)
    {
   	 return 1;
    }

    rbyte = read(s, (char *)&sdata, len);
    if(rbyte != len)
    {
   	 return 1;
    }
    if (ntohs(sdata.error != 0))
    {
   	 printf("CALC Error %d\n", ntohs(sdata.error));
    }


    //서버로 부터 받은 각 최대,최소의 ip주소값을 ntohl을 통해서
    // 변환해서 구조체 변수에 할당
    min_addr.sin_addr.s_addr = ntohl(sdata.min_add);
    max_addr.sin_addr.s_addr = ntohl(sdata.max_add);

    printf("min = %d from %s:%d\n",ntohl(sdata.min_result),
           inet_ntoa(min_addr.sin_addr),ntohs(sdata.min_pt));

    printf("max = %d from %s:%d\n",ntohl(sdata.max_result),
           inet_ntoa(max_addr.sin_addr),ntohs(sdata.max_pt));

    close(s);
    return 0;
}
