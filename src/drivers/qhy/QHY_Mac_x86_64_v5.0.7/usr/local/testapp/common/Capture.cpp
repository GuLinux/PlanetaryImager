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

int updatecamcmdcamexp(MYSQL connection,char *camid,int exptime)
{
    char sqlcmd[128];

    sprintf(sqlcmd,"UPDATE camcmd set CamExp=%d where CamId='%s'",exptime,camid);
    //LOGFMTT("updatecamcmdcamexp sqlcmd %s",sqlcmd);
   
    return updatecmd(connection,sqlcmd);
}

int updatecamcmdcambin(MYSQL connection,char *camid,int binmode)
{
    char sqlcmd[128];

    sprintf(sqlcmd,"UPDATE camcmd set CamBin=%d where CamId='%s'",binmode,camid);
    //LOGFMTT("updatecamcmdcambin sqlcmd %s",sqlcmd);
   
    return updatecmd(connection,sqlcmd);
}

int updatecamcmdcamspeed(MYSQL connection,char *camid,int speed)
{
    char sqlcmd[128];

    sprintf(sqlcmd,"UPDATE camcmd set CamSpeed=%d where CamId='%s'",speed,camid);
    //LOGFMTT("updatecamcmdcamspeed sqlcmd %s",sqlcmd);
   
    return updatecmd(connection,sqlcmd);
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
    char exptimestr[128];
    char modestr[128];
    char param1str[128];


    int exptime,mode,param1;
    int cambin,camspeed;

    MYSQL connection;
    char lastimagename[2048];

    if(argc <= 2)
    {
        printf("missing parameters argc <= 2,we need exptime mode param1\n");
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
        memcpy(exptimestr,argv[1],strlen(argv[1]));
        exptimestr[strlen(argv[1])] = '\0';
        exptime = atoi(exptimestr);
    }

    if(argv[2] != NULL)
    {
        memcpy(modestr,argv[2],strlen(argv[2]));
        modestr[strlen(argv[2])] = '\0';
        mode = atoi(modestr);
    }

    if(argv[3] != NULL)
    {
        memcpy(param1str,argv[3],strlen(argv[3]));
        param1str[strlen(argv[3])] = '\0';
        param1 = atoi(param1str);    
    }

    /*初始化mysql连接*/
    mysql_init(&connection);

    /*建立与数据库的连接*/
    if(mysql_real_connect(&connection, "localhost", "root", "root", "ic8300", 0, NULL, 0))
    {
        if(mode == 1)
        {
            cambin = 11;
            camspeed = 0;
        }
        else if(mode == 2)
        {
            cambin = 11;
            camspeed = 1;
        }
        else if(mode == 3 || mode == 4)
        {
            cambin = 44;
            camspeed = 1;
        }
        else
        {
            cambin = 44;
            camspeed = 1;
        }

        updatecamcmdcamexp(connection,camid,exptime);
        updatecamcmdcambin(connection,camid,cambin);
        updatecamcmdcamspeed(connection,camid,camspeed);
        
	int rundone = 0;
	updatecamcmdrundone(connection,camid,rundone);


	mysql_close(&connection); 
    }

    mysql_library_end();   
    return 0;
}
