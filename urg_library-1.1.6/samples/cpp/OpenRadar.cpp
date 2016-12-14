#include "OpenRadar.h"
#include "DFCOM.h"
#define pi 3.141592653



OpenRadar::OpenRadar(void)
{
}


OpenRadar::~OpenRadar(void)
{
}
bool OpenRadar::RadarRead(const vector<long>& data){

	///////////////读取文件方式///////////////////////
	///////参数:char *fileName////////////////////////
    //FILE* fp = NULL;
    //int dis = 0;
    //int totalCnt = 0;
    //fp = fopen(fileName,"r");
    //if (fp == NULL)
    //{
    //    //cout<<"failed to read"<<endl;
    //    return false;
    //}else {
    //    //cout<<"successed to read"<<endl;
    //    RadarRho.clear();
    //    while(!feof(fp))
    //    {

    //        fscanf(fp, "%d, ", &dis);
    //        RadarRho.push_back(dis);
    //        //printf("%d  ", dis);
    //    }
    //    //cout<<"Total Count: "<<RadarRho.size()<<endl;
    //}
    //fclose(fp);
    //return true;

	//////////直接读取数据形式////////////////////
	////////参数：const vector<long>& data////////////
	double theta;
	double deltaTeta;
	theta = (135.0 + 90-45)*pi / 180;
	deltaTeta = -0.25*pi / 180;
	RadarRho.clear();  //必须使用，否则会累积
	RadarTheta.clear();



	for (unsigned int i = 0; i < data.size(); i++)
	{
		RadarRho.push_back(data.at(i));
		RadarTheta.push_back(theta);
		theta += deltaTeta;
	}
	//printf("size=%d,%d\n", data.size(),RadarRho.size());
	return 0;
}





//void OpenRadar::CreateRadarImage(IplImage* RadarImage, const vector<long>& the_data, vector<double>& RadarTheta){
//    //RadarImage = cvCreateImage(cvSize(RadarImageWdith,RadarImageHeight),IPL_DEPTH_8U,1);
//    cvZero(RadarImage);
//    //在中心加上一个圆心
//    cvCircle(RadarImage, cvPoint(RadarImageWdith/2,RadarImageHeight/2),3, CV_RGB(0,255,255), -1, 8,0);
//	cvCircle(RadarImage, cvPoint(RadarImageWdith / 2, RadarImageHeight / 2),400, CV_RGB(255, 255, 0), 1, 8, 0);
//    int x,y;
//    double theta,rho;
//    unsigned char * pPixel = 0;
//    int halfWidth  = RadarImageWdith/2;
//    int halfHeight = RadarImageHeight/2;
//    for (unsigned int i = 0; i < the_data.size();i++)
//    {
//        theta = (i/4.0 - 45)*pi/180;
//        rho = the_data[i];
//		theta = RadarTheta.at(i);
//
//
//
//
//        x = (int)(rho*cos(theta)/25) + halfWidth;
//        y = (int)(-rho*sin(theta)/25)+ halfHeight;
//
//        if (x >= 0 && x < RadarImageWdith && y >= 0 && y < RadarImageHeight)
//        {
//            pPixel = (unsigned char*)RadarImage->imageData + y*RadarImage->widthStep + 3*x+2;
//            *pPixel = 255;
//        }else{
//            //cout<<"x: "<<x<<"  y: "<<y<<endl;
//        }
//    }
//
//}



void OpenRadar::CreateRadarImage(IplImage* RadarImage, vector<int>& RadarRho, vector<double>& RadarTheta) {
	//RadarImage = cvCreateImage(cvSize(RadarImageWdith,RadarImageHeight),IPL_DEPTH_8U,1);
	cvZero(RadarImage);
	//在中心加上一个圆心
	//int halfWidth  = RadarImageWdith/2;
	//int halfHeight = RadarImageHeight/2;
	int dx = RadarImageWdith / 2;
	int dy = RadarImageHeight * 3 / 4;

	cvCircle(RadarImage, cvPoint(RadarImageWdith / 2, RadarImageHeight *3/4), 3, CV_RGB(0, 255, 255), -1, 8, 0);
	//cvCircle(RadarImage, cvPoint(RadarImageWdith / 2, RadarImageHeight *3/ 4), 400, CV_RGB(255, 255, 0), 1, 8, 0);

	int x, y;
	double theta, rho;
	unsigned char * pPixel = 0;



	//颜色
	int colorIndex = 0, colorRGB;
	int R = 255, G = 0, B = 0;
	int pointCnt = 0;                 //这个计数也没用上



	for (unsigned int  i = 0; i < RadarRho.size(); i++)
	{
		//theta = (pointCnt/4.0 - 45)*pi/180;
		theta = RadarTheta.at(i);
		rho = RadarRho.at(i);
		if (rho < 0)
		{
			//雷达数据断点标志
			colorRGB = usualColor[colorIndex];
			R = colorRGB / 65536;
			G = (colorRGB % 65536) / 256;
			B = colorRGB % 256;
			colorIndex = (colorIndex + 1) % 10;        //十色一循环
		}
		else {
			pointCnt++;
		}

		x = (int)(rho*cos(theta) / 10) + dx;
		y = (int)(-rho*sin(theta) / 10) + dy;

		if (x >= 0 && x < RadarImageWdith && y >= 0 && y < RadarImageHeight)
		{
			pPixel = (unsigned char*)RadarImage->imageData + y*RadarImage->widthStep + 3 * x;
			pPixel[0] = B;
			pPixel[1] = G;
			pPixel[2] = R;
		}
		else {
			//cout<<"x: "<<x<<"  y: "<<y<<endl;
		}
	}

}

void OpenRadar::CreateRadarImageself(IplImage* RadarImage, vector<long>& data, vector<double>& the) {
	//RadarImage = cvCreateImage(cvSize(RadarImageWdith,RadarImageHeight),IPL_DEPTH_8U,1);
	cvZero(RadarImage);
	//在中心加上一个圆心
	//int halfWidth  = RadarImageWdith/2;
	//int halfHeight = RadarImageHeight/2;
	int dx = RadarImageWdith / 2;
	int dy = RadarImageHeight * 3 / 4;

	cvCircle(RadarImage, cvPoint(RadarImageWdith / 2, RadarImageHeight * 3 / 4), 3, CV_RGB(0, 255, 255), -1, 8, 0);
	//cvCircle(RadarImage, cvPoint(RadarImageWdith / 2, RadarImageHeight *3/ 4), 400, CV_RGB(255, 255, 0), 1, 8, 0);

	int x, y;
	double theta, rho;
	unsigned char * pPixel = 0;



	//颜色
	int colorIndex = 0, colorRGB;
	int R = 255, G = 0, B = 0;
	int pointCnt = 0;                 //这个计数也没用上



	for (unsigned int i = 0; i < data.size(); i++)
	{
		//theta = (pointCnt/4.0 - 45)*pi/180;
		theta = the.at(i);
		rho = data.at(i);
		if (rho <= 0)
		{
			//雷达数据断点标志
			colorRGB = usualColor[colorIndex];
			R = colorRGB / 65536;
			G = (colorRGB % 65536) / 256;
			B = colorRGB % 256;
			colorIndex = (colorIndex + 1) % 10;        //十色一循环
		}
		else {
			pointCnt++;

			x = (int)(rho*cos(theta) / 10) + dx;
			y = (int)(-rho*sin(theta) / 10) + dy;

			if (x >= 0 && x < RadarImageWdith && y >= 0 && y < RadarImageHeight)
			{
				pPixel = (unsigned char*)RadarImage->imageData + y*RadarImage->widthStep + 3 * x;
				pPixel[0] = B;
				pPixel[1] = G;
				pPixel[2] = R;
			}
			else {
				//cout<<"x: "<<x<<"  y: "<<y<<endl;
			}
		}

	}

}




void OpenRadar::CreateRadarImage2(IplImage* RadarImage, vector<long>& R_x, vector<long>& R_y) {
	//RadarImage = cvCreateImage(cvSize(RadarImageWdith,RadarImageHeight),IPL_DEPTH_8U,1);
	cvZero(RadarImage);
	//在中心加上一个圆心
	//int halfWidth  = RadarImageWdith/2;
	//int halfHeight = RadarImageHeight/2;
	//int dx = RadarImageWdith / 2;
	//int dy = RadarImageHeight * 3 / 4;

	cvCircle(RadarImage, cvPoint(RadarImageWdith / 2, RadarImageHeight / 2), 3, CV_RGB(0, 255, 255), -1, 8, 0);
	cvCircle(RadarImage, cvPoint(RadarImageWdith / 2, RadarImageHeight / 2), 360, CV_RGB(255, 255, 0), 1, 8, 0);

	long x, y;
	//double theta, rho;
	unsigned char * pPixel = 0;



	//颜色
//	int colorIndex = 0, colorRGB;
	int R = 255, G = 0, B = 0;
	int pointCnt = 0;                 //这个计数也没用上


	//printf("%d\n", R_x.size());
	for (unsigned int i = 0; i < R_x.size(); i++)
	{
		//theta = (pointCnt/4.0 - 45)*pi/180;
	//	theta = RadarTheta.at(i);
	//	rho = RadarRho.at(i);

		//if (rho < 0)
		//{
		//	//雷达数据断点标志
		//	colorRGB = usualColor[colorIndex];
		//	R = colorRGB / 65536;
		//	G = (colorRGB % 65536) / 256;
		//	B = colorRGB % 256;
		//	colorIndex = (colorIndex + 1) % 10;        //十色一循环
		//}
		//else {
		//	pointCnt++;
		//}

		x =(int) R_x.at(i)/2 + RadarImageWdith / 2;
		y = (int)R_y.at(i)/2 + RadarImageHeight / 2;

		/*x = R_x.at(i) + RadarImageWdith / 2;
		y = R_y.at(i) + RadarImageHeight / 2;*/

		if (x >= 0 && x < RadarImageWdith && y >= 0 && y < RadarImageHeight)
		{
			pPixel = (unsigned char*)RadarImage->imageData + y*RadarImage->widthStep + 3 * x;
			pPixel[0] = B;
			pPixel[1] = G;
			pPixel[2] = R;
		}
		else {
			//cout<<"x: "<<x<<"  y: "<<y<<endl;
		}
	}

}


int OpenRadar::BreakRadarRho() {                         //区域分割
	int breakCnt = 0;
	int rho = 0;
	//static int capturetimes = 0;
	int lastRho = RadarRho.at(0);
	double theta = RadarTheta.at(0);
	int dis = 0;
	int Dmax = 30;

	BreakedRadarRho.clear();
	BreakedRadarTheta.clear();

	BreakedRadarRho.push_back(lastRho);
	BreakedRadarTheta.push_back(theta);

	for (unsigned int  i = 1; i< RadarRho.size(); i++)
	{
		rho = RadarRho.at(i);
		theta = RadarTheta.at(i);

		BreakedRadarRho.push_back(rho);
		BreakedRadarTheta.push_back(theta);
		dis = abs(rho - lastRho);
		if (dis < Dmax)
		{

		}
		else {
		
			BreakedRadarRho.push_back(-1);
			BreakedRadarTheta.push_back(1000.0);
		/*	printf("%d,%d,%f\n", i - 1, RadarRho[i - 1], RadarTheta[i - 1]);
			printf("%d,%d,%f\n", i, RadarRho[i], RadarTheta[i]);
			printf("%d,%d,%f\n", i + 1, RadarRho[i + 1], RadarTheta[i + 1]);*/
			breakCnt++;
		//	printf("%d,%d\n", capturetimes, breakCnt);
		}

		lastRho = rho;
	}
	BreakedRadarRho.push_back(-1);
	BreakedRadarTheta.push_back(1000.0);
	//capturetimes++;
	//cout<<"breakCnt: "<<breakCnt<<endl;
	return breakCnt;
}






int OpenRadar::FitArc(vector<ArcPara>& FittedArc,
	vector<int>& RadarRho,
	vector<double>& RadarTheta, Pillar_t* pillar)
{
	int rho = 0;
	double theta = 0.0;
	int X[1200] = { 0 };
	int Y[1200] = { 0 };
	int pointCnt = 0;
	ArcPara tmpArcPara;
	FittedArc.clear();
	int ArcCnt = 0;
	
	for (int i = 0; i < (int)(RadarRho.size()); i++)
	{
		rho = RadarRho.at(i);
		theta = RadarTheta.at(i);

		if (rho < 0)
		{
			if (pointCnt > 6 && pointCnt < 40)//点数目足够多的点列才进行直线拟合
			{
				if (HoughArc2(X, Y, pointCnt, 125
					, &tmpArcPara))              //霍夫圆拟合~~~~~~//半径参数
				{
					FittedArc.push_back(tmpArcPara);
					ArcCnt++;
				}
				pointCnt = 0;
				continue;
			}
			else {
				pointCnt = 0;
				continue;
			}
		}
		if (rho > 2500 && rho < 5000)                                   //检测范围~~~~~~~~
		{
			X[pointCnt] = static_cast<int>(rho*cos(theta));
			Y[pointCnt] = static_cast<int>(rho*sin(theta));
			pointCnt++;
		}

	}
	for (unsigned int  i = 0; i < FittedArc.size(); i++)
	{
		//if (i == 0)      //先试试
		//{
			/*cout << " x: " << FittedArc.at(i).center.x
				<< " y: " << FittedArc.at(i).center.y
				<< " r: " << FittedArc.at(i).r << endl;
*/
	//	    DFCOMPush(pillar, FittedArc, i);
	//	int send_size=DFCOMPop(pillar, 16);
		//	cout << send_size << "      "<<sizeof(Pillar_t)<<endl;
		//}
	}
	
	return ArcCnt;
}

void OpenRadar::DrawRadarArc(IplImage* image, vector<ArcPara>& FittedArc)    //画拟合圆
{
	CvPoint center;
	int r;
	int ratio = 10;
	int dx = RadarImageWdith / 2;
	int dy = RadarImageHeight * 3 / 4;
	for (int i = 0; i < (int)(FittedArc.size()); i++)
	{
		center.x = FittedArc.at(i).center.x / ratio + dx;
		center.y = -FittedArc.at(i).center.y / ratio + dy;
		r = FittedArc.at(i).r / ratio;
		cvCircle(image, center, r, CV_RGB(0, 255, 0), 1, 8, 0);
	}

}


//中值滤波 只能对初始的连续数据滤波
//滤波基本不丢弃数据，两端会各自扔掉几个数据
void OpenRadar::MedFilter(vector<int>& RadarRho, vector<double>& RadarTheta) {
	vector<int>rho;
	vector<double>theta;
	int halfWindowSize = 2;
	int *neighbor = new int[2 * halfWindowSize + 1];
	int temp;
	for (int i = halfWindowSize; i< (int)RadarRho.size() - halfWindowSize; i++)
	{
		for (int j = -halfWindowSize; j <= halfWindowSize; j++)
		{
			neighbor[j + halfWindowSize] = RadarRho.at(i + j);
		}
		//排序
		for (int m = 0; m < 2 * halfWindowSize + 1; m++)
		{
			for (int n = m + 1; n < 2 * halfWindowSize + 1; n++)
			{
				if (neighbor[m]> neighbor[n])
				{
					temp = neighbor[m];
					neighbor[m] = neighbor[n];
					neighbor[n] = temp;
				}
			}
		}
		rho.push_back(neighbor[halfWindowSize]);
		theta.push_back(RadarTheta.at(i));
	}

	RadarRho.clear();
	RadarTheta.clear();

	for (int i = 0; i < (int)(rho.size()); i++)
	{
		RadarRho.push_back(rho.at(i));
		RadarTheta.push_back(theta.at(i));
	}
}