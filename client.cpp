#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "utils.h"
#include "json/json.h"

using namespace std;
using namespace hzd;
int main(){
    int sockfd = socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET,"127.0.0.1",&serveraddr.sin_addr.s_addr);
    serveraddr.sin_port = htons(9999);
    int ret = connect(sockfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr));
    if(ret==-1){
        perror("connect :");
        exit(-1);
    }

    header h;
    h.type = hzd::header_type::BYTE;
    string s = "i am client";
    h.size = s.size() + 1;
    while(1)
    {
        int x = send(sockfd,&h,HEADER_SIZE,0);
        std::cout << x << std::endl;
        x = send(sockfd,s.c_str(),h.size,0);
        std::cout << x << std::endl;
        sleep(5);
    }

//    h.size = 0;
//    recv(sockfd,&h,HEADER_SIZE,0);
//    char buf[1024] = {0};
//    size_t read_count = 0;
//    while(read_count < h.size)
//    {
//        bzero(buf,1024);
//        read_count += recv(sockfd,buf,1024,0);
//        std::cout << buf;
//    }
    close(sockfd);
}