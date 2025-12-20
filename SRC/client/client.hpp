#ifndef CLIENT_HPP
#define CLIENT_HPP
#include<iostream>
#include<string>
#include<jsoncpp/json/json.h>
#include<event2/event.h>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<sys/stat.h>
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
class Client{
private:
    string ser_ip="127.0.0.1";
    string ser_port="6000";
    int sockfd;
    struct event_base* base;
    string username;
    string usertel;
    Json::Value val;
    int user_op;//用户操作选项
    bool op_flag=true;//true表示未登录状态，false表示已登录状态
    bool running=true;
public:
    Client(){
        sockfd=-1;
    }
    Client(string ip,string port):ser_ip(ip),ser_port(port){
        sockfd=-1;   
    }
    bool init_client();
    int get_sockfd(){
        return sockfd;
    }
    void cli_login();
    void cli_regist();
    void upload_file();
    void download_file();
    void delete_file();
    void check_file();
    void print_info();//呈现界面
    void run();
};
#endif


