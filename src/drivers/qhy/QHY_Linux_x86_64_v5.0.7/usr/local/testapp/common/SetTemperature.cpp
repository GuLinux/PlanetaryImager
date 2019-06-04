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

int updatecamtemperature(MYSQL connection,char *defaultcamid,double camtemperature)
{
    char sqlcmd[128];

    sprintf(sqlcmd,"UPDATE status set TargetTemperature=%f where CamId='%s'",camtemperature,defaultcamid);
    //LOGFMTD("Updatecamstatus sqlcmd %s",sqlcmd);
  
    return updatecmd(connection,sqlcmd);
}

int main(int argc,char *argv[])
{
    char camid[128] = "90A-Series-M-0017d41072cd594d9";
    char tempstr[128];
    MYSQL connection;
    double targettemperature;

    if(argc <= 1)
    {
        printf("missing parameters argc <= 1,we need target temperature\n");
        return 0;
    }
/*
    if(argv[1] != NULL)
    {
        memcpy(camid,argv[1],strlen(argv[1]));
        camid[strlen(argv[1])] = '\0';
    }    
*/
    if(argv[1] != NULL)
    {
        memcpy(tempstr,argv[1],strlen(argv[1]));
        tempstr[strlen(argv[1])] = '\0';
        targettemperature = atoi(tempstr);
    }

    /*初始化mysql连接*/
    mysql_init(&connection);

    /*建立与数据库的连接*/
    if(mysql_real_connect(&connection, "localhost", "root", "root", "ic8300", 0, NULL, 0))
    {
        updatecamtemperature(connection,camid,targettemperature);

	mysql_close(&connection); 
    }

    mysql_library_end();   
    return 0;
}
