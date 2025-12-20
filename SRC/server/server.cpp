#include"server.hpp"
void server_con::recv_data(){
    char buff[256];
    memset(buff,0,sizeof(buff));
    int len=recv(cfd,buff,sizeof(buff),0);
    if(len<=0){
        perror("recv err or client closed");
        return;
    }
    cout<<"recv data:"<<buff<<endl;
    Json::Reader reader;
    if(!reader.parse(buff,val)){
        perror("json parse err");
        send_err();
        return;
    }
    int optype=val["optype"].asInt();
    switch(optype){
        case login:
            ser_login();
            break;
        case regist:
            ser_regist();
            break;
        case upload:
            upload_file(); 
            break;
        case download:
            download_file();
            break;
        case deletefile:
            delete_file();
            break;
        case cat:
            check_file();
            break;
        case serexit:
            close(cfd);
            break;
        default:
            perror("unknown optype");
            send_err();
            break;
    }           
};
void server_con::send_err(){
    Json::Value res;
    res["status"]="err";
    send(cfd,res.toStyledString().c_str(),res.toStyledString().length(),0);
};
void server_con::send_ok(){
    Json::Value res;
    res["status"]="OK";
    send(cfd,res.toStyledString().c_str(),res.toStyledString().length(),0);
};
void server_con::ser_login(){
    string tel=val["user_tel"].asString();
    string passwd =val["user_passwd"].asString();
    if(tel.empty()||passwd.empty()){
        send_err();
        return;
    }
    sermysql mysql;
    if(!mysql.mysql_connect()){
        send_err();
        return;
    }
    if(!mysql.mysql_user_login(tel,passwd)){
        send_err();
        return;
    }
    send_ok();
    return;
};
void server_con::ser_regist(){
    string name=val["user_name"].asString();
    string passwd=val["user_passwd"].asString();
    string tel=val["user_tel"].asString();
    if(name.empty()||passwd.empty()||tel.empty()){
        send_err();
        return;
    }
    sermysql mysql;
    if(!mysql.mysql_connect()){
        send_err();
        return;
    }
    if(!mysql.mysql_user_regist(name,passwd,tel)){
        send_err();
        return;
    }
    send_ok();
    return;
};
void server_con::upload_file(){

};
void server_con::download_file(){

};
void server_con::delete_file(){

};
void server_con::check_file(){

};

void SOCK_CON_CALLBACK(int fd,short events,void* arg){
    server_con* sc=(server_con*)arg;
    if(sc==nullptr){
        perror("SOCK_CON_CALLBACK err");
        return;
    }
    if(events&EV_READ){
        sc->recv_data();
    }
}

void SOCK_LIS_CALLBACK(int fd,short events,void* arg){
    Server* ser=(Server*)arg;
    if(ser==nullptr){
        perror("SOCK_LIS_CALLBACK err");
        return;
    }
    if(events&EV_READ){
        int c=ser->accept_client();
        if(c<0){
            perror("accept_client err");
            return;
        }
        cout<<"accept client fd:"<<c<<endl;
        server_con* sc=new server_con(c);
        struct event* ev_con=event_new(ser->get_base(),c,EV_READ|EV_PERSIST,SOCK_CON_CALLBACK,(void*)sc);
        if(!ev_con){
            perror("event_new err");
            delete sc;
            return;
        }
        sc->set_event(ev_con);
        event_add(ev_con,NULL);
    }
}
int main(){
    Server ser;
    if(!ser.init_server()){
        perror("server init err");
        return -1;
    }
    struct event_base* base=event_base_new();
    if(!base){
        perror("event_base_new err");
        return -1;
    }
    ser.set_base(base);
    int sockfd=ser.get_sockfd();
    struct event* ev_listen=event_new(base,sockfd,EV_READ|EV_PERSIST,SOCK_LIS_CALLBACK,(void*)&ser);
    if(!ev_listen){
        perror("event_new err");
        return -1;
    }
    if(event_add(ev_listen,NULL)<0){
        perror("event_add err");
        return -1;
    }
    event_base_dispatch(base);
    event_free(ev_listen);
    event_base_free(base);
    return 0;
}