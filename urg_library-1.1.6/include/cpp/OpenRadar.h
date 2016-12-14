#pragma once
#include <iostream>
#include <cmath>
#include<cv.h>
#include<highgui.h>
#include <vector>
#include "DFCOM.h"
#include "WeightedFit.h"
#include <fstream>

using namespace std;
static int RadarImageWdith  = 720;
static int RadarImageHeight = 720;


//常用颜色
/*Colour      Red      Green      Blue      值
白色   White    255    255    255    16777215
红色    Red    255    0    0    255
深红色    Dark    Red    128    0    0    128
绿色    Green    0    255    0    65280
深绿色    Dark    Green    0    128    0    32768
蓝色    Blue    0    0    255    16711680
紫红色    Magenta    255    0    255    16711935
深紫    Dark    Cyan    0    128    128    8421376
黄色    Yellow    255    255    0    65535
棕色    Brown    128    128    0    32896
*/
static int usualColor[15] = { 16777215,255,128,65280,32768,
16711680,16711935,8421376,65535,32896 }; /*<10种常用的颜色*/





class OpenRadar
{
public:
    OpenRadar(void);
    ~OpenRadar(void);

    
	vector<int>RadarRho;                       //跟随官方例程，取得的距离数据都用long
	vector<int>BreakedRadarRho;
	vector<long>R_x;
	vector<long>R_y;

	vector<double>RadarTheta;
	vector<double>BreakedRadarTheta;

	vector<ArcPara> FittedArc;//拟合出的圆弧
    //数据读取
    bool RadarRead(const vector<long>& data);
    void CreateRadarImage(IplImage* RadarImage,  vector<int>& RadarRho, vector<double>& RadarTheta);
	void OpenRadar::CreateRadarImageself(IplImage* RadarImage, vector<long>& data, vector<double>& the);
	void CreateRadarImage2(IplImage* RadarImage, vector<long>& R_x, vector<long>& R_y);
	int BreakRadarRho();//阈值如何选取  
	void MedFilter(vector<int>& RadarRho, vector<double>& RadarTheta);//中值滤波

	int FitArc(vector<ArcPara>& FittedArc, vector<int>& RadarRho, vector<double>& RadarTheta, Pillar_t* pillar);
	void DrawRadarArc(IplImage* image, vector<ArcPara>& FittedArc);
};

