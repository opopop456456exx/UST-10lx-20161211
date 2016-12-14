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


//������ɫ
/*Colour      Red      Green      Blue      ֵ
��ɫ   White    255    255    255    16777215
��ɫ    Red    255    0    0    255
���ɫ    Dark    Red    128    0    0    128
��ɫ    Green    0    255    0    65280
����ɫ    Dark    Green    0    128    0    32768
��ɫ    Blue    0    0    255    16711680
�Ϻ�ɫ    Magenta    255    0    255    16711935
����    Dark    Cyan    0    128    128    8421376
��ɫ    Yellow    255    255    0    65535
��ɫ    Brown    128    128    0    32896
*/
static int usualColor[15] = { 16777215,255,128,65280,32768,
16711680,16711935,8421376,65535,32896 }; /*<10�ֳ��õ���ɫ*/





class OpenRadar
{
public:
    OpenRadar(void);
    ~OpenRadar(void);

    
	vector<int>RadarRho;                       //����ٷ����̣�ȡ�õľ������ݶ���long
	vector<int>BreakedRadarRho;
	vector<long>R_x;
	vector<long>R_y;

	vector<double>RadarTheta;
	vector<double>BreakedRadarTheta;

	vector<ArcPara> FittedArc;//��ϳ���Բ��
    //���ݶ�ȡ
    bool RadarRead(const vector<long>& data);
    void CreateRadarImage(IplImage* RadarImage,  vector<int>& RadarRho, vector<double>& RadarTheta);
	void OpenRadar::CreateRadarImageself(IplImage* RadarImage, vector<long>& data, vector<double>& the);
	void CreateRadarImage2(IplImage* RadarImage, vector<long>& R_x, vector<long>& R_y);
	int BreakRadarRho();//��ֵ���ѡȡ  
	void MedFilter(vector<int>& RadarRho, vector<double>& RadarTheta);//��ֵ�˲�

	int FitArc(vector<ArcPara>& FittedArc, vector<int>& RadarRho, vector<double>& RadarTheta, Pillar_t* pillar);
	void DrawRadarArc(IplImage* image, vector<ArcPara>& FittedArc);
};

