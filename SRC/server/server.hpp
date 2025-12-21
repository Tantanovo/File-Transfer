#ifndef SERVER_HPP
#define SERVER_HPP
#include<iostream>
#include<string>
#include<jsoncpp/json/json.h>
#include<event2/event.h>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<mysql/mysql.h>
#include <dirent.h>
using namespace std;
const int offset=3;
enum op_type{
    login=1,
    regist,//2
    serexit,//3
    upload,//上传文件
    download,//下载文件
    deletefile,//删除文件
    cat,//查看文件
    
};
class sermysql{
private:
    MYSQL mysql_con;
    string host="127.0.0.1";
    string user="yzy";
    string passwd="770202";
    string dbname="FTsystem";
    int port=3306;
    bool mysql_user_begin();//目前用不着这三个函数
    bool mysql_user_rollback();
    bool mysql_user_commit();
public:
    sermysql(){}
    //sermysql(string h,string u,string p,string db,int pt):host(h),user(u),passwd(p),dbname(db),port(pt){}//目前没必要改变默认参数

    ~sermysql(){mysql_close(&mysql_con);};
    bool mysql_connect();
    bool mysql_user_regist(string name,string passwd,string tel);
    bool mysql_user_login(string tel,string passwd);
};
class Server{
private:
    string ser_ip="127.0.0.1";
    string ser_port="6000";
    int sockfd;
    struct event_base* base;
public:
    Server(){
        sockfd=-1;
    }
    Server(string ip,string port):ser_ip(ip),ser_port(port){
        sockfd=-1;   
    }
   
    bool init_server();
    int get_sockfd(){
        return sockfd;
    }
    int accept_client();
    void set_base(struct event_base* b){ base = b; }
    struct event_base* get_base(){ return base; }
    

};
class server_con{
private:
    int cfd;
    struct event* ev;
    Json::Value val;
public:
    server_con(int fd):cfd(fd){
        ev=nullptr;
    }
    void set_event(struct event* e){
        ev=e;
    }
    ~server_con(){
        if(ev){
            event_free(ev);
            ev=nullptr;
        }
        if(cfd>=0){
            close(cfd);
            cfd=-1;
        }
    }
    void recv_data();
    void send_err();
    void send_ok();
    void ser_login();
    void ser_regist();
    void upload_file();
    void download_file();
    void delete_file();
    void check_file();
};






#endif