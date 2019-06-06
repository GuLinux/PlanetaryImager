#include <stdlib.h>
#include <stdio.h>
#include <mysql/mysql.h>
#include <uuid/uuid.h>
#include <libqhy/qhyccd.h>
#include <string.h>
#include <unistd.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

int querystring(MYSQL connection,char *sqlcmd,char *retstr)
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
        //LOGFMTE("select error: %s",mysql_error(&connection));            
    }
    else
    {
        /*将查询的結果给res_ptr*/
        res_ptr = mysql_store_result(&connection);

        if (res_ptr) 
        {
            result_row = mysql_fetch_row(res_ptr);

            //LOGFMTT("retstr: %s",result_row[0]);

            strcpy(retstr,result_row[0]);
            mysql_free_result(res_ptr);
        }
        else
        {
            //LOGFMTE("mysql_store_result error: %s",mysql_error(&connection));             
        }
    }
    return res;
}


int querystatuslastcaptureimagename(MYSQL connection,char *defaultcamid,char *lastcapturimagename)
{
    char sqlcmd[128];
 
    sprintf(sqlcmd,"SELECT LastImageName FROM status WHERE CamId='%s'",defaultcamid);                
    //LOGFMTT("QueryTemperature sqlcmd %s",sqlcmd);

    return querystring(connection,sqlcmd,lastcapturimagename);
}

int main(int argc,char *argv[])
{
    char camid[128] = "90A-Series-M-0017d41072cd594d9";
    MYSQL connection;
    char lastimagename[2048];
/*
    if(argc <= 1)
    {
        printf("missing parameters argc <= 1,we need camid\n");
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
        querystatuslastcaptureimagename(connection,camid,lastimagename);

        printf("lastimagename=/var/www/%s.fit\n",lastimagename);

	mysql_close(&connection); 
    }

    mysql_library_end();   
    return 0;
}
