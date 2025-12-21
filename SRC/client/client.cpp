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

bool send_all(int sock, const char* data, size_t len) {
    size_t total_sent = 0;
    while (total_sent < len) {
        ssize_t sent = send(sock, data + total_sent, len - total_sent, 0);
        if (sent <= 0) {
            return false;  
        }
        total_sent += sent;
    }
    return true;
}

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
    FILE* file = fopen(full_filename.c_str(), "rb");  // 二进制模式打开，支持所有文件类型
    if (file == nullptr) {
        cout << "文件不存在或无法打开，请检查文件名和权限。" << endl;
        return;
    }

    Json::Value val;
    val["type"] = "upload";
    val["filename"] = filename;
        // 使用临时短连接上传：保持主控制连接不变（登录状态）
        cout << "目标服务器目录: /home/yang/file-transfer-system/server_files/" << endl;
        int tsock = socket(AF_INET, SOCK_STREAM, 0);
        if (tsock < 0) {
            perror("socket");
            fclose(file);
            return;
        }
        struct sockaddr_in saddr;
        memset(&saddr,0,sizeof(saddr));
        saddr.sin_family = AF_INET;
        saddr.sin_port = htons(atoi(ser_port.c_str()));
        saddr.sin_addr.s_addr = inet_addr(ser_ip.c_str());
        if (connect(tsock, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) {
            perror("connect");
            close(tsock);
            fclose(file);
            return;
        }

        // 发送 header（type+filename）；保留原样（无需 filesize 改动按你的要求）
        send(tsock, val.toStyledString().c_str(), strlen(val.toStyledString().c_str()), 0);
        char buff[256] = {0};
        int n = recv(tsock, buff, 255, 0);
        if (n <= 0) {
            cout << "操作失败：无法接收服务器响应或服务器已断开连接（短连接）" << endl;
            close(tsock);
            fclose(file);
            return;
        }
        Json::Value srvh;
        Json::Reader read;
        if (!read.parse(buff, srvh)) {
            cout << "解析服务器响应失败（短连接）" << endl;
            close(tsock);
            fclose(file);
            return;
        }
        string st = srvh["status"].asString();
        if (st.compare("OK") != 0) {
            cout << "上传请求被拒绝" << endl;
            close(tsock);
            fclose(file);
            return;
        }
        cout << "上传请求成功，开始上传文件" << endl;
    const size_t BUFFER_SIZE = 4096;  // 文件读取缓冲区，大小适中提升效率
    char file_buff[BUFFER_SIZE] = {0};
    size_t read_len = 0;
    bool upload_success = true;

    size_t total_sent = 0;
    while ((read_len = fread(file_buff, 1, BUFFER_SIZE, file)) > 0) {
            if (!send_all(tsock, file_buff, read_len)) {
                cout << "文件数据传输失败，短连接发送异常。" << endl;
            upload_success = false;
            break;
        }
        total_sent += read_len;
    }

    // 校验文件读取是否正常结束
    if (upload_success && ferror(file)) {
        cout << "读取本地文件失败。" << endl;
        upload_success = false;
    }
    if (upload_success) {
        cout << "文件 " << filename << " 上传完成！" << endl;
    } else {
        cout << "文件 " << filename << " 上传失败！" << endl;
    }
    fclose(file);

    // 等待服务端确认接收完成，使用 5 秒超时，避免长时间阻塞
    // 关闭短连接发送端，等待服务端确认
    shutdown(tsock, SHUT_WR);
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(tsock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

    char resp[256] = {0};
    int rn = recv(tsock, resp, 255, 0);
    if (rn > 0) {
        Json::Value rj;
        Json::Reader rdr;
        if (rdr.parse(resp, rj)) {
            string st2 = rj["status"].asString();
            if (st2.compare("OK") == 0) {
                cout << "服务器确认已接收文件（短连接）。" << endl<<endl<<endl;
            } else {
                cout << "服务器返回错误（短连接）：" << st2 << endl<<endl<<endl;
            }
        }
    } else if (rn == 0) {
        cout << "短连接被服务器关闭" << endl;
    } else {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            cout << "等待服务器确认超时5s，继续下一步。" << endl<<endl<<endl;
        } else {
            cout << "短连接接收确认时出错 errno=" << errno << endl;
        }
    }
    close(tsock);
    // 清除短连接接收超时设置（恢复默认）
    tv.tv_sec = 0; tv.tv_usec = 0;
    setsockopt(tsock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
};

void Client::download_file(){
    string save_path = "/home/yang/file-transfer-system/downloads/"; 
    string filename;
    // 先请求并显示服务器端文件列表，帮助用户选择可下载的文件
    cout << "目标服务器目录: /home/yang/file-transfer-system/server_files/" << endl;
    check_file();

    cout << "请输入要下载的文件名：";  
    cin >> filename;
    if(filename.empty()){
        cout << "文件名不能为空" << endl;
        return;
    }
    string full_save_name = save_path + filename;

   
    Json::Value val;
    val["type"] = "download";  
    val["filename"] = filename;
    cout << "目标服务器目录: /home/yang/file-transfer-system/server_files/" << endl;
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
        cout<<"下载请求被拒绝"<<endl;
        return;
    }
    cout<<"下载请求成功，开始接收文件"<<endl;

   
    FILE* file = fopen(full_save_name.c_str(), "wb");
    if (file == nullptr) {
        cout << "无法创建/打开保存文件，请检查目录是否存在和权限。" << endl;
        return;
    }
    const size_t BUFFER_SIZE = 4096;
    char file_buff[BUFFER_SIZE] = {0};
    ssize_t recv_len = 0;
    bool download_success = true;

    while ((recv_len = recv(sockfd, file_buff, BUFFER_SIZE, 0)) > 0) {        
        size_t write_len = fwrite(file_buff, 1, (size_t)recv_len, file);
        if (write_len != (size_t)recv_len) {
            cout << "写入下载文件失败，可能磁盘已满。" << endl;
            download_success = false;
            break;
        }
    }

    if (download_success && recv_len < 0) {
        cout << "接收文件数据失败，网络异常。" << endl;
        download_success = false;
    }
    if (download_success && ferror(file)) {
        cout << "写入本地下载文件失败。" << endl;
        download_success = false;
    }

    if (download_success) cout << "文件 " << filename << " 下载完成！" << endl;
    else {
        cout << "文件 " << filename << " 下载失败！" << endl;
        remove(full_save_name.c_str());
    }
    fclose(file);
};
void Client::delete_file(){
    string filename;
    // 先获取并显示服务端文件列表，便于选择要删除的文件
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