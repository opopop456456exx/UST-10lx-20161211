
#include"OpenRadar.h"
#include <iostream>
#include <cmath>
#include "io.h"
using namespace std;
const int MAX_POINT_COUNT = 1200;
int Rho[MAX_POINT_COUNT] = {0}; 

int main(){
    OpenRadar openRadar;
    char fileName[32] = "csv\\data_0.csv";
    int frameCnt = 0;
    char key;
    IplImage* RadarImage = cvCreateImage(cvSize(RadarImageWdith,RadarImageHeight),IPL_DEPTH_8U,3);
    while (access(fileName,0) == 0)
    {
        sprintf(fileName,"csv\\data_%d.csv",frameCnt);
        openRadar.RadarRead(fileName);
        
        openRadar.CreateRadarImage(RadarImage);
        cvNamedWindow("Radar",1);
        cvShowImage("Radar",RadarImage);
        key = cvWaitKey(30);
        if (key == 27)//escÍË³ö
        {
            break;
        }
        frameCnt++;
    }
    cvReleaseImage(&RadarImage);
    cvDestroyWindow("Radar");
    return 0;
}
