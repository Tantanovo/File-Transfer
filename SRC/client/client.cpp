#include"client.hpp"

void Client::cli_login(){
    string tel,passwd;
    cout<<"请输入手机号码"<<endl;
    cin>>tel;
    cout<<"请输入密码"<<endl;
    cin>>passwd;
    if(tel.empty()||passwd.empty()){
        cout<<"帐号或密码不能为空"<<endl;
        return;
    }
    Json::Value val;
    val["type"]=login;
    val["user_tel"]=tel;
    val["user_passwd"]=passwd;
    send(sockfd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0);
    char buff[256]={0};
    int n=recv(sockfd,buff,255,0);
    if(n<=0){
        cout<<"ser close"<<endl;
        return;
    }
    val.clear();
    Json::Reader read;
    if(!read.parse(buff,val)){
        cout<<"解析json失败"<<endl;
        return;
    }
    string st=val["status"].asString();
    if(st.compare("OK")!=0){
        cout<<"登陆失败"<<endl;
        return;
    }

    op_flag=true;
    username=val["user_name"].asString();
    usertel=tel;
    cout<<"登陆成功"<<endl;
    op_flag=false;
    return;

};
void Client::cli_regist(){
    string name,tel,passwd,temp;
    cout<<"请输入用户名"<<endl;
    cin>>name;
    cout<<"请输入手机号码"<<endl;
    cin>>tel;
    cout<<"请输入密码"<<endl;
    cin>>temp;
    cout<<"请再次输入密码"<<endl;
    cin>>passwd;
    if(temp!=passwd){
        cout<<"两次输入密码不一致"<<endl;
        return;
    }
    if(name.empty()||tel.empty()||passwd.empty()){
        cout<<"用户名、帐号或密码不能为空"<<endl;
        return;
    }
    Json::Value val;
    val["type"]=regist;
    val["user_name"]=name;
    val["user_tel"]=tel;
    val["user_passwd"]=passwd;
    send(sockfd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0);
    char buff[256]={0};
    int n=recv(sockfd,buff,255,0);
    if(n<=0){
        cout<<"ser close"<<endl;
        return;
    }
    val.clear();
    Json::Reader read;
    if(!read.parse(buff,val)){
        cout<<"解析json失败"<<endl;
        return;
    }
    string st=val["status"].asString();
    if(st.compare("OK")!=0){
        cout<<"注册失败"<<endl;
        return;
    }
    cout<<"注册成功"<<endl;
    return;
};
void Client::upload_file(){
    
};
void Client::download_file(){

};
void Client::delete_file(){

};
void Client::check_file(){

};
void Client::print_info(){
    if(op_flag){
        cout<<"======欢迎使用文件传输系统======\n";
        cout<<"1.登录\n";
        cout<<"2.注册\n";
        cout<<"3.退出\n";
        cout<<"请输入选项数字：";
        cin>>user_op;
        if(user_op>3)user_op=9;
    }else{//已登录状态
        cout<<"======文件传输系统======\n";
        cout<<"1.上传文件\n";
        cout<<"2.下载文件\n";
        cout<<"3.删除文件\n";
        cout<<"4.查看文件列表\n";
        cout<<"5.退出登录\n";
        cout<<"请输入选项数字：";
        cin>>user_op;
        user_op+=offset;
        if(user_op==8){
            op_flag=true;
        }
    }
}
void Client::run(){
    while(running){
        print_info();
        switch(user_op){
            case login:
            cli_login();
            break;
        case regist:
            cli_regist();
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
            close(sockfd);
            running=false;
            cout<<"已退出系统！\n";
            break;
        case 8:
            cout<<"已退出登录！\n";
            print_info();
            break;
        default:
            cout<<"输入了无效选项"<<endl;
            break;
        }
    }
}

int main(){
    Client cli;
    if(!cli.init_client()){
        perror("client init err");
        return -1;
    }
    cli.run();

    return 0;
}