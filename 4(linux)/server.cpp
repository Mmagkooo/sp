#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include <unistd.h>
#include <memory.h>
int main()
{
    int sock=socket(AF_INET,SOCK_DGRAM,0);
    if(sock<0)
    {
        perror("socket fail");
        return -1;
    }
    struct sockaddr_in myaddr;
    myaddr.sin_family		=AF_INET;
    myaddr.sin_port			=htons(7979);
    myaddr.sin_addr.s_addr		=INADDR_ANY;
    if(bind(sock,(struct sockaddr*)&myaddr,sizeof(myaddr))<0)
    {
        perror("socket fail");
        return -1;
    }
    int ilen=0;
    char buf[100]="";
    struct sockaddr_in caddr;
    socklen_t addrlen=sizeof(caddr);
    while(1)
    {
        ilen=recvfrom(sock,buf,99,0,(struct sockaddr*)&caddr,&addrlen);
        if(ilen<=0)
            break;
        buf[ilen]='\0';
        printf("%s\n",buf);
        sendto(sock,"ok",2,0,(struct sockaddr*)&caddr,sizeof(caddr));
        memset(buf, 0, 100);
    }
    close(sock);
    return 0;
}