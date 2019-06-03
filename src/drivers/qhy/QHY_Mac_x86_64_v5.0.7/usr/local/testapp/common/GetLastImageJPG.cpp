#include <stdlib.h>
#include <stdio.h>
#include <mysql/mysql.h>
#include <uuid/uuid.h>
#include <libqhy/qhyccd.h>
#include <string.h>
#include <unistd.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <fitsio.h>

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
    char lastimagenamewithfmt[2048];
    char imagenamejpg[2048];
    char sizestr[128];
    char stretchstr[128];
    int size,stretch;

    if(argc <= 2)
    {
        printf("missing parameters argc <= 2,we need size stretch\n");
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
        memcpy(sizestr,argv[1],strlen(argv[1]));
        sizestr[strlen(argv[1])] = '\0';
        size = atoi(sizestr);
    }  

     if(argv[2] != NULL)
    {
        memcpy(stretchstr,argv[2],strlen(argv[2]));
        stretchstr[strlen(argv[2])] = '\0';
        stretch = atoi(stretchstr);
    }  
    /*初始化mysql连接*/
    mysql_init(&connection);

    /*建立与数据库的连接*/
    if(mysql_real_connect(&connection, "localhost", "root", "root", "ic8300", 0, NULL, 0))
    {
        querystatuslastcaptureimagename(connection,camid,lastimagename);

        sprintf(lastimagenamewithfmt,"/var/www/%s-%d.jpg",lastimagename,stretch);

        IplImage *img = cvLoadImage(lastimagenamewithfmt,0);
        int rszwidth,rszheight;
        rszwidth = img->width * size / 100.0;
        rszheight = img->height * size / 100.0;

        IplImage *rszimg = cvCreateImage(cvSize(rszwidth,rszheight),img->depth,img->nChannels);
        cvResize(img,rszimg,CV_INTER_NN);
        sprintf(imagenamejpg,"/var/www/%s-rsz.jpg",lastimagename);
        cvSaveImage(imagenamejpg,rszimg);
        printf("lastimagejpg=%s\n",imagenamejpg);

        if(stretch == 0)
            printf("B,W=0,65535\n");
        else if(stretch == 1)
            printf("B,W=10000,55535\n");
        else if(stretch == 2)
            printf("B,W=20000,45535\n");
        else if(stretch == 3)
            printf("B,W=30000,35535\n");
        cvReleaseImage(&img);
        cvReleaseImage(&rszimg);
	mysql_close(&connection); 
    }

    mysql_library_end();   
    return 0;
}
