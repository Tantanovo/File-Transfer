#include"server.hpp"

bool sermysql::mysql_connect(){
    mysql_init(&mysql_con);
    if(!mysql_real_connect(&mysql_con,host.c_str(),user.c_str(),passwd.c_str(),dbname.c_str(),port,NULL,0)){
        cerr<<"mysql_real_connect err:"<<mysql_error(&mysql_con)<<endl;
        return false;
    }
    return true;
}
bool sermysql::mysql_user_begin(){
    if(mysql_query(&mysql_con,"begin")){
        cerr<<"mysql_user_begin err:"<<mysql_error(&mysql_con)<<endl;
        return false;
    }
    return true;
}
bool sermysql::mysql_user_rollback(){
    if(mysql_query(&mysql_con,"rollback")){
        cerr<<"mysql_user_rollback err:"<<mysql_error(&mysql_con)<<endl;
        return false;
    }
    return true;
}
bool sermysql::mysql_user_commit(){
    if(mysql_query(&mysql_con,"commit")){
        cerr<<"mysql_user_commit err:"<<mysql_error(&mysql_con)<<endl;
        return false;
    }
    return true;
}

bool sermysql::mysql_user_regist(string name,string passwd,string tel){
    string query="insert into user_info (username,passwd,tel) values ('"+name+"','"+passwd+"','"+tel+"')";
    if(mysql_query(&mysql_con,query.c_str())){
        cerr<<"mysql_user_regist err:"<<mysql_error(&mysql_con)<<endl;
        return false;
    }
    return true;
}


bool sermysql::mysql_user_login(string tel,string passwd){
    string query="select * from user_info where tel='"+tel+"' and passwd='"+passwd+"'";
    if(mysql_query(&mysql_con,query.c_str())){
        cerr<<"mysql_user_login err:"<<mysql_error(&mysql_con)<<endl;
        return false;
    }
    MYSQL_RES* res=mysql_store_result(&mysql_con);
    if(res==nullptr){
        cerr<<"mysql_store_result err:"<<mysql_error(&mysql_con)<<endl;
        return false;
    }
    int num_rows=mysql_num_rows(res);
    mysql_free_result(res);
    if(num_rows==0){
        return false;
    }
    return true;
}