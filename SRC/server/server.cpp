#include"server.hpp"
void server_con::recv_data(){
    char buff[256];
    memset(buff,0,sizeof(buff));
    int len=recv(cfd,buff,sizeof(buff),0);
    if(len==0){
        cout<<"client closed"<<endl;
        if(ev){
            event_del(ev);
            event_free(ev);
            ev=nullptr;
        }
        if(cfd>=0){
            close(cfd);
            cfd=-1;
        }
        delete this;
        return;
    }
    if(len<0){
        perror("recv err");
        return;
    }
    cout<<"recv data:"<<buff<<endl;
    Json::Reader reader;
    if(!reader.parse(buff,val)){
        perror("json parse err");
        send_err();
        return;
    }
    int optype=0;
    if(val.isMember("optype") && val["optype"].isInt()){
        optype=val["optype"].asInt();
    }else if(val.isMember("type")){
        // 支持两种形式：数字类型的 type 或 字符串类型的 type
        if(val["type"].isInt()){
            optype = val["type"].asInt();
        } else if(val["type"].isString()){
            string t = val["type"].asString();
            if(t == "login") optype = login;
            else if(t == "regist") optype = regist;
            else if(t == "serexit") optype = serexit;
            else if(t == "upload") optype = upload;
            else if(t == "download") optype = download;
            else if(t == "deletefile") optype = deletefile;
            else if(t == "cat") optype = cat;
            else optype = 0;
        }
    }
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
    string req_type = val["type"].asString();
    string filename = val["filename"].asString();
    long filesize = val["filesize"].asInt64();
    if (req_type != "upload" || filename.empty()) {
        send_err();
        return;
    }
    string full_save_name = "/home/yang/file-transfer-system/server_files/" + filename;
    FILE* file = fopen(full_save_name.c_str(), "wb");
    if (file == nullptr) {
        send_err();
        return;
    }
    send_ok();
    char file_buff[4096] = {0};
    ssize_t recv_len;
    long total_recv = 0;
    while (total_recv < filesize) {
        size_t to_recv = (filesize - total_recv > 4096) ? 4096 : (filesize - total_recv);
        recv_len = recv(cfd, file_buff, to_recv, 0);
        if (recv_len <= 0) {
            fclose(file);
            return;
        }
        fwrite(file_buff, 1, recv_len, file);
        total_recv += recv_len;
    }
    fclose(file);
    send_ok();
    return;
};
void server_con::download_file(){
    string req_type = val["type"].asString();
    string filename = val["filename"].asString();
    if (req_type != "download" || filename.empty()) {
        send_err();
        return;
    }
    string full_file_name = "/home/yang/file-transfer-system/server_files/" + filename;
    FILE* file = fopen(full_file_name.c_str(), "rb");
    if (file == nullptr) {
        send_err();
        return;
    }
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);
    Json::Value res;
    res["status"] = "OK";
    res["filesize"] = (Json::Int64)filesize;
    string res_str = res.toStyledString();
    ssize_t sent = send(cfd, res_str.c_str(), res_str.length(), 0);
    char file_buff[4096] = {0};
    size_t read_len;
    long total_sent = 0;
    while ((read_len = fread(file_buff, 1, 4096, file)) > 0) {
        ssize_t sent_len = send(cfd, file_buff, read_len, 0);
        if (sent_len <= 0) {  
            fclose(file);
            return;
        }   
        total_sent += sent_len;
    }
    if (ferror(file)) {
        cout << "[DEBUG] 文件读取错误: " << strerror(errno) << endl;
    }
    fclose(file);
    shutdown(cfd, SHUT_WR);
    return;
};
void server_con::delete_file(){
   string req_type = val["type"].asString();
    string filename = val["filename"].asString();
    if (req_type != "deletefile" || filename.empty()) {
        send_err();
        return;
    }
    string full_file_name = "/home/yang/file-transfer-system/server_files/" + filename;
    if (remove(full_file_name.c_str()) != 0) {
        send_err();
        return;
    }
    send_ok();
    return;
};
// DT_REG - 普通文件
// DT_DIR - 目录
// DT_LNK - 符号链接
// DIR* opendir(const char* path);        // 打开目录
// struct dirent* readdir(DIR* dir);      // 读取下一个目录项
// int closedir(DIR* dir);                // 关闭目录
void server_con::check_file(){
    string req_type = val["type"].asString();
    if (req_type != "cat") {
        send_err();
        return;
    }
    DIR* dir = opendir("/home/yang/file-transfer-system/server_files/");
    if (dir == nullptr) {
        send_err();
        return;
    }
    Json::Value resp_val;
    Json::Value file_list;
    struct dirent* entry;
    while ((entry = readdir(dir)) ) {
        if (entry->d_type == DT_REG) { // entry->d_type 是文件类型
            file_list.append(entry->d_name);// entry->d_name 是文件名
        }
    }
    closedir(dir);
    resp_val["status"] = "OK";
    resp_val["file_list"] = file_list;
    send(cfd, resp_val.toStyledString().c_str(), resp_val.toStyledString().length(), 0);
    return;
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