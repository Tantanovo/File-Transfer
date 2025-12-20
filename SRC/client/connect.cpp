#include"client.hpp"
bool Client::init_client(){
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0){
        perror("socket err");
        return false;
    }
    struct sockaddr_in saddr;
    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family=AF_INET;
    saddr.sin_port=htons(atoi(ser_port.c_str()));
    saddr.sin_addr.s_addr=inet_addr(ser_ip.c_str());
    if(connect(sockfd,(struct sockaddr*)&saddr,sizeof(saddr))<0){
        perror("connect err");
        return false;
    }
    cout<<"client init success!"<<endl;
    return true;
}