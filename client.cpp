#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <memory.h>
#include <unistd.h>
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

    char data[64] = {'A','B','C','D','E'};
    send(sockfd,data,sizeof(data),0);

    json pack;
    while(1){
        bzero(data,64);
        send(sockfd,"i am client",12,0);
        if(recv(sockfd,data,64,MSG_PEEK)<=0)
        {
            exit(-1);
        }
        string s(data);
        int size = s.size() + 1;
        std::cout << s << std::endl;
        if(pack.load(s))
        {
            if(pack["type"] == "heart_beat")
            {
                pack["type"] = "heart_beat_ret";
                recv(sockfd,data,64,0);
                string a = pack.dump();
                send(sockfd,a.c_str(),a.size()+1,0);
            }
        }
        pack.clear();
    }
    close(sockfd);
}