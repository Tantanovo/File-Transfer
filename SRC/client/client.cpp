#include"client.hpp"
#include <dirent.h>
#include <sys/time.h>
#include <errno.h>

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
        cout<<"操作失败：无法接收服务器响应或服务器已断开连接"<<endl;
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
        cout<<"操作失败：无法接收服务器响应或服务器已断开连接"<<endl;
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
    string fixed_path = "/home/yang/file-transfer-system/files/"; 
    // 列出本地 source 目录中的可选文件
    cout << "本地源目录: " << fixed_path << endl;
    DIR* dir = opendir(fixed_path.c_str());
    if(dir){
        struct dirent* entry;
        int idx = 1;
        while((entry = readdir(dir)) != nullptr){
            if(strcmp(entry->d_name, ".")!=0 && strcmp(entry->d_name, "..")!=0 && entry->d_type==DT_REG){
                cout << idx++ << ". " << entry->d_name << endl;
            }
        }
        closedir(dir);
    } else {
        cout << "无法读取本地目录，请检查路径或权限。" << endl;
    }
    string filename;
    cout << "请输入要上传的文件名：";  
    cin >> filename;
    if(filename.empty()){
        cout << "文件名不能为空" << endl;
        return;
    }
    string full_filename = fixed_path + filename;
    FILE* file = fopen(full_filename.c_str(), "rb");
    if (file == nullptr) {
        cout << "文件不存在或无法打开" << endl;
        return;
    }
    // 3. 获取文件大小
    fseek(file, 0, SEEK_END);//把file这个指针挪到文件末尾
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);//重置指针
    Json::Value val;
    val["type"] = "upload";
    val["filename"] = filename;
    val["filesize"] = (Json::Int64)filesize;
    send(sockfd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str()), 0);
    char buff[256] = {0};
    int n = recv(sockfd, buff, 255, 0);
    if (n <= 0) {
        cout << "服务器无响应" << endl;
        fclose(file);
        return;
    }
    Json::Value srvh;
    Json::Reader read;
    if (!read.parse(buff, srvh)) {
        cout << "服务器返回格式错误" << endl;
        fclose(file);
        return;
    }
    
    string st = srvh["status"].asString();
    if (st.compare("OK") != 0) {
        cout << "上传请求被拒绝" << endl;
        fclose(file);
        return;
    }
    
    cout << "开始上传文件..." << endl;
    char file_buff[4096] = {0};
    size_t read_len = 0;
    size_t total_sent = 0;
    
    while ((read_len = fread(file_buff, 1, 4096, file)) > 0) {
        if (send(sockfd, file_buff, read_len, 0) <= 0) {
            cout << "发送失败" << endl;
            fclose(file);
            return;
        }
        total_sent += read_len;
        cout << "\r已发送: " << total_sent << "/" << filesize << " bytes";
    }
    
    fclose(file);
    cout << "\n文件发送完成" << endl;

    n = recv(sockfd, buff, 255, 0);
    if (n > 0) {
        cout << "上传成功！" << endl;
    }
};

void Client::download_file(){
    cout << "目标服务器目录: /home/yang/file-transfer-system/server_files/" << endl;
    check_file();
    string filename;
    cout << "请输入要下载的文件名：";  
    cin >> filename;
    if(filename.empty()){
        cout << "文件名不能为空" << endl;
        return;
    }
    Json::Value val;
    val["type"] = "download";
    val["filename"] = filename;
    send(sockfd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str()), 0);
    char buff[256] = {0};
    int n = recv(sockfd, buff, 255, 0);
    if (n <= 0) {
        cout << "服务器无响应" << endl;
        return;
    }
    Json::Value resp;
    Json::Reader read;
    if (!read.parse(buff, resp)) {
        cout << "服务器返回格式错误" << endl;
        return;
    }
    string st = resp["status"].asString();
    if (st.compare("OK") != 0) {
        cout << "下载请求被拒绝" << endl;
        return;
    }
    long filesize = resp["filesize"].asInt64();
    cout << "开始下载文件，大小: " << filesize << " bytes" << endl;
    string save_path = "/home/yang/file-transfer-system/downloads/" + filename;
    FILE* file = fopen(save_path.c_str(), "wb");
    if (file == nullptr) {
        cout << "无法创建保存文件" << endl;
        return;
    }
    
    char file_buff[4096] = {0};
    ssize_t recv_len;
    long total_recv = 0;
    
    while ((recv_len = recv(sockfd, file_buff, 4096, 0)) > 0) {
        fwrite(file_buff, 1, recv_len, file);
        total_recv += recv_len;
        cout << "\r已接收: " << total_recv << " bytes";
    }
    
    fclose(file);
    
    if (total_recv == filesize) {
        cout << "\n下载完成！文件保存到: " << save_path << endl;
    } else {
        cout << "\n警告：文件不完整（收到" << total_recv << "/" << filesize << "字节）" << endl;
        remove(save_path.c_str());
    }

};
void Client::delete_file(){
    string filename;
    cout << "目标服务器目录: /home/yang/file-transfer-system/server_files/" << endl;
    check_file();
    cout << "请输入要删除的文件名：";  
    cin >> filename;
    if(filename.empty()){
        cout << "文件名不能为空" << endl;
        return;
    }
    Json::Value val;
    val["type"] = "deletefile";  
    val["filename"] = filename;
    
    send(sockfd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str()), 0);
    char buff[256]={0};
    int n=recv(sockfd, buff, 255, 0);
    if(n<=0){
        cout<<"操作失败：无法接收服务器响应或服务器已断开连接"<<endl;
        return;
    }
    val.clear();
    Json::Reader read;
    if(!read.parse(buff, val)){
        cout<<"解析json失败"<<endl;
        return;
    }
    string st=val["status"].asString();
    if(st.compare("OK")!=0){
        cout<<"删除请求被拒绝（文件不存在或服务端异常）"<<endl;
        return;
    }
    cout << "文件 " << filename << " 删除成功！" << endl;
};

void Client::check_file(){
     Json::Value val;
    val["type"] = "cat";  
    send(sockfd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str()), 0);
    char buff[256]={0};
    int n=recv(sockfd, buff, 255, 0);
    if(n<=0){
        cout<<"操作失败：无法接收服务器响应或服务器已断开连接"<<endl;
        return;
    }
    val.clear();
    Json::Reader read;
    if(!read.parse(buff, val)){
        cout<<"解析json失败"<<endl;
        return;
    }
    string st=val["status"].asString();
    if(st.compare("OK")!=0){
        cout<<"获取文件列表失败"<<endl;
        return;
    }
    cout << "========== 服务端文件列表 ==========" << endl;
    cout << "服务器目录: /home/yang/file-transfer-system/server_files/" << endl;
    Json::Value file_list = val["file_list"];
    if (file_list.empty()) {
        cout << "服务端暂无文件" << endl;
    } else {
        for (int i = 0; i < file_list.size(); ++i) {
            cout << i + 1 << ". " << file_list[i].asString() << endl;
        }
    }
    cout << "====================================" << endl<<endl<<endl;
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