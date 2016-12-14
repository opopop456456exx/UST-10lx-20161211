/*!
  \example get_distance.c 嫍棧僨乕僞傪庢摼偡傞
                          
  \author Satofumi KAMIMURA

  $Id: get_distance.cpp,v a7acd36ce479 2014/05/19 06:23:12 jun $
*/

#include "Urg_driver.h"
#include "Connection_information.h"
#include "math_utilities.h"
#include <iostream>
#include"OpenRadar.h"
#include <cmath>
#include "io.h"
#include <Windows.h>  
#include"DFCOM.h"

using namespace qrk;
using namespace std;

OpenRadar openRadar;
//ArcPara arcPara;
//union Pillar_u  pillar_u;


static char* ErrorMessage = "no error.";

namespace
{
    void print_data(const Urg_driver& urg,
                    const vector<long>& data, long time_stamp, vector<long>& R_x, vector<long>& R_y)
    {
#if 0
        // 慜曽偺僨乕僞偺傒傪昞帵  
       // int front_index = urg.step2index(0);
		for (int front_index =1; front_index <=1080; front_index++)
        cout << front_index <<","<<data[front_index] << " [mm], ("
             << time_stamp << " [msec])" << endl;

		//for (i = 0; i < data.size(); i++)
		//{    
		//	if (data.at(i) == 0)
		//		cout <<"\n"<<"break!!!!!!!!!!" << "step_f=" << i - 1 << "  " << "data1=" << data.at(i-2) << " " << "data2=" << data.at(i-1) << endl;
		//	else
		//	cout << "distant:" << data.at(i) << " " << "theta=" << theta.at(i) << "  " << endl;
		//}

#else
        // 慡偰偺僨乕僞偺 X-Y 偺埵抲傪昞帵  
        long min_distance = urg.min_distance();
        long max_distance = urg.max_distance();
        size_t data_n = data.size();
		R_x.clear();
		R_y.clear();
        for (size_t i = 0; i < data_n; ++i) {                 // for (size_t i = 0; i < data_n; ++i) {
            long l = data[i];
            if ((l <= min_distance) || (l >= max_distance)) {
                continue;
            }

            double radian = urg.index2rad(i);
            long x = static_cast<long>(l * cos(radian));
            long y = static_cast<long>(l * sin(radian));
			R_x.push_back(x);
			R_y.push_back(y);
         //   cout <<"R="<< radian*(360/(2* M_PI))+90 <<","<< "(" << x << ", " << y << ")" << endl;
        }
      //  cout << endl;
#endif
    }
}

void outputData(const vector<long>& data, int n, size_t Capture_times,char* output_file,int *arg)
{
	//	char output_file[] = "data_xxxxxxxxxx.csv";
	//	_snprintf(output_file, sizeof(output_file), "data_%03d.csv", total_index);
	static int temp2 = 0,temp_allmin=99999,temp_allmax=0;
	int min_index = 0;
	
	FILE* fd = fopen(output_file, "a+");
	if (!fd) {
		perror("fopen");       //perror(s) 用来将上一个函数发生错误的原因输出到标准设备(stderr)。
		return;
	}



	if (Capture_times ==0) {
		fprintf(fd, "\n\n");
		for (int j = 0; j < 1; j++) {

			for (int i = arg[2] - (n - 1) / 2; i <= arg[2] + (n - 1) / 2; ++i) {
				fprintf(fd, "%ld, ", i);    //格式化输出至磁盘文件
				//temp = min(temp, data[i]);
			}
			fprintf(fd, "0, 1,  ,  , ");
		}
		fprintf(fd, "\n");
	}
	

	for (int j = 0; j < 1; j++) {
		long temp = 999999;
		for (int i = arg[2] - (n - 1) / 2; i <= arg[2] + (n - 1) / 2; ++i) {
			fprintf(fd, "%ld, ", data[i]);    //格式化输出至磁盘文件
			temp = min(temp, data[i]); temp2 += data[i];
			temp_allmin = min(temp_allmin, data[i]);
			temp_allmax = max(temp_allmax, data[i]);
			if (temp == data[i])min_index = i;
		}
		fprintf(fd, "%ld, %d,  ,  , ", temp, min_index);    //格式化输出至磁盘文件
	}
	
//	fprintf(fd, "\n");
	fprintf(fd, "\n");
	if (Capture_times == 24) {
		fprintf(fd, "%d, %ld, %ld, ", temp2/25, temp_allmin, temp_allmax);
		fprintf(fd, "\n");
	}
	
	fclose(fd);
}






int main(int argc, char *argv[])
{

	//*************************** TCP连接 ************************************************//
	char output_file[] = "data_test.csv";
	int arg[5] = {120,350,540,720,900};
    Connection_information information(argc, argv);

	char key;
	double fps=0;
	int j = 0;
    // 愙懕   连接
    Urg_driver urg;
    if (!urg.open(information.device_or_ip_name(),
                  information.baudrate_or_port_number(),
                  information.connection_type())) {
        cout << "Urg_driver::open(): "
             << information.device_or_ip_name() << ": " << urg.what() << endl;
        return 1;
    }
//******************************************************************************************//

//******************************  COM连接  *************************************************//
	const char com_port[] = "COM3";                                //设置串口号和波特率
	const long com_baudrate = 115200;
	static char message_buffer[LineLength];
//	int n;

	if (com_connect(com_port, com_baudrate) < 0) {                        //如果波特率、串口设置失败
		_snprintf(message_buffer, LineLength,                     //_snprintf函数将可变个参数(...)按照format格式化成字符串，然后将其复制到str中
			"Cannot connect COM device: %s", com_port);              //写入错误信息到ErrorMessage内
		ErrorMessage = message_buffer;
		printf("urg_connect: %s\n", ErrorMessage);
		getchar();
		return -1;
	}
	else
	{
		printf("open com success!\n");
		printf("set DCB success!\n");
	}

	//n = urg_sendTag("hello world!");
//******************************************************************************************//

    // 僨乕僞庢摼    数据采集
#if 1
    // 僨乕僞偺庢摼斖埻傪曄峏偡傞応崌   可以更改数据的采集范围
   // urg.set_scanning_parameter(urg.deg2step(-135), urg.deg2step(+135), 0);  //这是把正前方当0度方向
	// Pillar_u a;
	//Pillar_t pillar_t;
	
	
	urg.set_scanning_parameter(urg.deg2step(-90), urg.deg2step(+90), 0);
#endif
    enum { Capture_times = 25 };
    urg.start_measurement(Urg_driver::Distance, 0, 0);
	Pillar_t pillar_t;
	IplImage* RadarImage = cvCreateImage(cvSize(RadarImageWdith, RadarImageHeight), IPL_DEPTH_8U, 3);
//	IplImage* RadarImage2 = cvCreateImage(cvSize(RadarImageWdith, RadarImageHeight), IPL_DEPTH_8U, 3);
	cvNamedWindow("Radar", 1); 
	cvNamedWindow("BreakedRadar", 1);         //窗口创建放在循环外,节省循环时间
	cvNamedWindow("ArcRadar", 1);
//	double time00= static_cast<double>(GetTickCount());
	//double time0;
	double time0 = (double)GetTickCount();
    for (int i = 0; ; ++i){

	

        vector<long> data;
		vector<double> theta;
        long time_stamp = 0;

	
		








	//	urg.start_measurement(Urg_driver::Distance, 1, 0);  //损失帧率以进行算法操作
        if (!urg.get_distance(data,theta, &time_stamp)) {
            cout << "Urg_driver::get_distance(): " << urg.what() << endl;
			getchar();
            return 1;
        }
//		printf("************************************************* QQQQQQQQQQQ\n\n");
//for (i = 0; i < data.size(); i++)
//{    
//	if (data.at(i) == 0)
//		cout <<"\n"<<"break!!!!!!!!!!" << "step_f=" << i - 1 << "  " << "data1=" << data.at(i-2) << " " << "data2=" << data.at(i-1) << endl;
//	else
//	cout << "distant:" << data.at(i) << " " << "theta=" << theta.at(i) << "  " << endl;
//}
		//for (i = 0; i < data.size(); i++)
		//{    
		//	if (data.at(i) == 0)
		//		cout <<"\n"<<"break!!!!!!!!!!" << "step_f=" << i - 1 << "  " << "data1=" << data.at(i-2) << " " << "data2=" << data.at(i-1) << endl;
		//	else
		//	cout << "distant:" << data.at(i) << " " << "theta=" << theta.at(i) << "  " << endl;
		//}
    //  print_data(urg, data, time_stamp, openRadar.R_x, openRadar.R_y);
		//outputData(data, 25, i, output_file,arg); //一些显示、输出的
		//outputData(data2, 6, output_file);
	//	openRadar.RadarRead(data);          //从data到RadarRho把变量的范围扩大。其实可以直接用RadarRho代替data，获取距离数据时也一样，可能更省时间
	//	openRadar.CreateRadarImage2(RadarImage2, openRadar.R_x , openRadar.R_y) ;
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
	//	openRadar.MedFilter(openRadar.RadarRho, openRadar.RadarTheta);    //中值滤波
		
	//  openRadar.CreateRadarImage(RadarImage, openRadar.RadarRho, openRadar.RadarTheta);
	//	cvShowImage("Radar", RadarImage);
	
		//openRadar.BreakRadarRho();                    //有返回断点数但是没去用////慢！！！！！！！！！！
		
		//openRadar.CreateRadarImage(RadarImage, openRadar.BreakedRadarRho, openRadar.BreakedRadarTheta);
		openRadar.CreateRadarImageself(RadarImage, data, theta);
		cvShowImage("BreakedRadar", RadarImage);

		////在此处添加必要的算法 
		////未加中值滤波、不知会怎样
		////圆弧拟合成圆，半径参数在霍夫函数里更改
		
		//openRadar.FitArc (openRadar.FittedArc, openRadar.BreakedRadarRho, openRadar.BreakedRadarTheta, &pillar_t);
		
		//openRadar.DrawRadarArc(RadarImage, openRadar.FittedArc);
		//cvShowImage("ArcRadar", RadarImage);

		
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
	key = cvWaitKey(1);                     //是否一定要等待30ms?是否会造成延迟？
	//	if (key == 27)//esc退出
	//	{
	//		break;
	//	}
		if (i == 200)
		{
			double time_all = (double)GetTickCount() - time0;
			fps = 1000 / time0;
			//printf("%d\n",i);
			if ((i % 40) == 0)
				j++;
			cout << "all time:" << time_all  << "    "<<"AVGTIME="<< time_all /200<<endl;
		}


    }
	//cvReleaseImage(&RadarImage);
	//cvDestroyWindow("Radar");
	//cvDestroyWindow("BreakedRadar");
	//cvDestroyWindow("ArcRadar");
//	printf("END\n");
	getchar();
#if defined(URG_MSC)
    getchar();
#endif
    return 0;
}
