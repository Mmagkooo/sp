#include<stdio.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<sys/stat.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<sys/select.h>
#include<errno.h>
int main()
{
    int sock=socket(AF_INET,SOCK_DGRAM,0);
    if(sock<0)
    {
        perror("fail\n");
        return -1;
    }
    fd_set rset;
    FD_ZERO(&rset);
    int maxfd = STDIN_FILENO> sock? STDIN_FILENO: sock;
    struct sockaddr_in saddr;
    bzero (& saddr, sizeof (saddr));
    saddr.sin_family		=AF_INET;
    saddr.sin_port			=htons(7979);
    saddr.sin_addr.s_addr		=inet_addr("127.0.0.1");
    char buf[100]="";
    int ilen=0;
    while(1)
    {
        FD_SET(STDIN_FILENO,&rset);
        FD_SET(sock,&rset);
        if(-1==select(maxfd+1,&rset,NULL,NULL,NULL)&&errno!=EINVAL)
        {
            perror("select fail");
            break;
        }
        if (FD_ISSET (STDIN_FILENO, & rset)> 0)
        {
            read(STDIN_FILENO,buf,99);
            sendto(sock,buf,strlen(buf),0,(struct sockaddr*)&saddr,sizeof(saddr));
        }
        if (FD_ISSET (sock, & rset)> 0)
        {
            ilen=recv(sock,buf,99,0);
            if(ilen<=0)
                break;
            buf[ilen]='\0';
            printf ("Получено:% s \n", buf);
        }
    }
    close(sock);
    return 0;
}