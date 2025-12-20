#include"server.hpp"
bool Server::init_server(){
    int sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0){
        perror("socket err");
        return false;
    }
    struct sockaddr_in saddr;
    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family=AF_INET;
    saddr.sin_port=htons(atoi(ser_port.c_str()));
    saddr.sin_addr.s_addr=inet_addr(ser_ip.c_str());
    int reuse=1;//端口复用
    if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse))<0){
        perror("setsockopt err");
        return false;
    };
    if(bind(sockfd,(struct sockaddr*)&saddr,sizeof(saddr))<0){
        perror("bind err");
        return false;
    }
    if(listen(sockfd,5)<0){
        perror("listen err");
        return false;
    }
    cout<<"server init success!"<<endl;
    this->sockfd = sockfd;
    return true;
}

int Server::accept_client(){
    int c=accept(sockfd,NULL,NULL);
    if(c<0){
        perror("accept err");
        return -1;
    }
    return c;
}
