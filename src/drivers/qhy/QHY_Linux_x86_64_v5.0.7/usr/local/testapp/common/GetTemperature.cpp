#include <stdlib.h>
#include <stdio.h>
#include <mysql/mysql.h>
#include <uuid/uuid.h>
#include <libqhy/qhyccd.h>
#include <string.h>
#include <unistd.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

int querydouble(MYSQL connection,char *sqlcmd,double *retdouble)
{
    /*指向查询结果的指针*/
    MYSQL_RES *res_ptr;

    /*字段结构指针*/
    MYSQL_FIELD *field; 

    /*按行返回的查询信息*/
    MYSQL_ROW result_row; 

    /*查询返回的行数和列数*/
    int row, column; 

    int res = mysql_query(&connection,sqlcmd);
    if(res)
    {
        //LOGFMTD("select error: %s",mysql_error(&connection));            
    }
    else
    {
        /*将查询的結果给res_ptr*/
        res_ptr = mysql_store_result(&connection);

        if (res_ptr) 
        {
            result_row = mysql_fetch_row(res_ptr);
            if(result_row)
            {                  
                *retdouble = atoi(result_row[0]);
            }
            mysql_free_result(res_ptr);
        }
        else
        {
            //LOGFMTD("mysql_store_result error: %s",mysql_error(&connection));            
            return 1;  
        }
    }
    return 0;  
}

int querycamtemperature(MYSQL connection,char *defaultcamid,double *camtemperature)
{
    char sqlcmd[128];
 
    sprintf(sqlcmd,"SELECT Temperature FROM status WHERE CamId='%s'",defaultcamid);                
    //LOGFMTT("QueryTemperature sqlcmd %s",sqlcmd);

    return querydouble(connection,sqlcmd,camtemperature);
}

int main(int argc,char *argv[])
{
    char camid[128] = "90A-Series-M-0017d41072cd594d9";
    MYSQL connection;
    double temperature;
/*
    if(argc <= 1)
    {
        printf("missing parameters argc <= 1,we need camid\n");
        return 0;
    }

    if(argv[1] != NULL)
    {
        memcpy(camid,argv[1],strlen(argv[1]));
        camid[strlen(argv[1])] = '\0';
    } 
*/   
    /*初始化mysql连接*/
    mysql_init(&connection);

    /*建立与数据库的连接*/
    if(mysql_real_connect(&connection, "localhost", "root", "root", "ic8300", 0, NULL, 0))
    {
        querycamtemperature(connection,camid,&temperature);

        printf("temperature=%f\n",temperature);

	mysql_close(&connection); 
    }

    mysql_library_end();   
    return 0;
}
