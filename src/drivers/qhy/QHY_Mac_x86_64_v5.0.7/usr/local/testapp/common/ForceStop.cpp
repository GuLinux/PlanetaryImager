#include <stdlib.h>
#include <stdio.h>
#include <mysql/mysql.h>
#include <uuid/uuid.h>
#include <libqhy/qhyccd.h>
#include <string.h>
#include <unistd.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>


int updatecmd(MYSQL connection,char *sqlcmd)
{    
    int res = mysql_query(&connection,sqlcmd);    
    if(res)
    {
        //LOGFMTD("updatecmd error: %s",mysql_error(&connection));
    }
    return res;
}


int updatecamcmdrundone(MYSQL connection,char *camid,int rundone)
{
    char sqlcmd[128];

    sprintf(sqlcmd,"UPDATE camcmd set RunDone=%d where CamId='%s'",rundone,camid);
    //LOGFMTT("Updatecamcmdrundone sqlcmd %s",sqlcmd);
   
    return updatecmd(connection,sqlcmd);
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
        int rundone = 4;
        updatecamcmdrundone(connection,camid,rundone);

	mysql_close(&connection); 
    }

    mysql_library_end();   
    return 0;
}
