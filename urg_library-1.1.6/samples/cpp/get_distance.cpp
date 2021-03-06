/*!
  \example get_distance.c 距離データを取得する
                          
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
        // 前方のデータのみを表示  
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
        // 全てのデータの X-Y の位置を表示  
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
		perror("fopen");       //perror(s) ﾓﾃﾀｴｽｫﾉﾏﾒｻｸ�ｺｯﾊ�ｷ｢ﾉ�ｴ�ﾎ�ｵﾄﾔｭﾒ�ﾊ莎�ｵｽｱ�ﾗｼﾉ雎ｸ(stderr)｡｣
		return;
	}



	if (Capture_times ==0) {
		fprintf(fd, "\n\n");
		for (int j = 0; j < 1; j++) {

			for (int i = arg[2] - (n - 1) / 2; i <= arg[2] + (n - 1) / 2; ++i) {
				fprintf(fd, "%ld, ", i);    //ｸ�ﾊｽｻｯﾊ莎�ﾖﾁｴﾅﾅﾌﾎﾄｼ�
				//temp = min(temp, data[i]);
			}
			fprintf(fd, "0, 1,  ,  , ");
		}
		fprintf(fd, "\n");
	}
	

	for (int j = 0; j < 1; j++) {
		long temp = 999999;
		for (int i = arg[2] - (n - 1) / 2; i <= arg[2] + (n - 1) / 2; ++i) {
			fprintf(fd, "%ld, ", data[i]);    //ｸ�ﾊｽｻｯﾊ莎�ﾖﾁｴﾅﾅﾌﾎﾄｼ�
			temp = min(temp, data[i]); temp2 += data[i];
			temp_allmin = min(temp_allmin, data[i]);
			temp_allmax = max(temp_allmax, data[i]);
			if (temp == data[i])min_index = i;
		}
		fprintf(fd, "%ld, %d,  ,  , ", temp, min_index);    //ｸ�ﾊｽｻｯﾊ莎�ﾖﾁｴﾅﾅﾌﾎﾄｼ�
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

	//*************************** TCPﾁｬｽﾓ ************************************************//
	char output_file[] = "data_test.csv";
	int arg[5] = {120,350,540,720,900};
    Connection_information information(argc, argv);

	char key;
	double fps=0;
	int j = 0;
    // 接続   ﾁｬｽﾓ
    Urg_driver urg;
    if (!urg.open(information.device_or_ip_name(),
                  information.baudrate_or_port_number(),
                  information.connection_type())) {
        cout << "Urg_driver::open(): "
             << information.device_or_ip_name() << ": " << urg.what() << endl;
        return 1;
    }
//******************************************************************************************//

//******************************  COMﾁｬｽﾓ  *************************************************//
	const char com_port[] = "COM3";                                //ﾉ靹ﾃｴｮｿﾚｺﾅｺﾍｲｨﾌﾘﾂﾊ
	const long com_baudrate = 115200;
	static char message_buffer[LineLength];
//	int n;

	if (com_connect(com_port, com_baudrate) < 0) {                        //ﾈ郢�ｲｨﾌﾘﾂﾊ｡｢ｴｮｿﾚﾉ靹ﾃﾊｧｰﾜ
		_snprintf(message_buffer, LineLength,                     //_snprintfｺｯﾊ�ｽｫｿﾉｱ荳�ｲﾎﾊ�(...)ｰｴﾕﾕformatｸ�ﾊｽｻｯｳﾉﾗﾖｷ�ｴｮ｣ｬﾈｻｺ�ｽｫﾆ荳ｴﾖﾆｵｽstrﾖﾐ
			"Cannot connect COM device: %s", com_port);              //ﾐｴﾈ�ｴ�ﾎ�ﾐﾅﾏ｢ｵｽErrorMessageﾄﾚ
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

    // データ取得    ﾊ�ｾﾝｲﾉｼｯ
#if 1
    // データの取得範囲を変更する場合   ｿﾉﾒﾔｸ�ｸﾄﾊ�ｾﾝｵﾄｲﾉｼｯｷｶﾎｧ
   // urg.set_scanning_parameter(urg.deg2step(-135), urg.deg2step(+135), 0);  //ﾕ簗ﾇｰﾑﾕ�ﾇｰｷｽｵｱ0ｶﾈｷｽﾏ�
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
	cvNamedWindow("BreakedRadar", 1);         //ｴｰｿﾚｴｴｽｨｷﾅﾔﾚﾑｭｻｷﾍ�,ｽﾚﾊ｡ﾑｭｻｷﾊｱｼ�
	cvNamedWindow("ArcRadar", 1);
//	double time00= static_cast<double>(GetTickCount());
	//double time0;
	double time0 = (double)GetTickCount();
    for (int i = 0; ; ++i){

	

        vector<long> data;
		vector<double> theta;
        long time_stamp = 0;

	
		








	//	urg.start_measurement(Urg_driver::Distance, 1, 0);  //ﾋ�ﾊｧﾖ｡ﾂﾊﾒﾔｽ�ﾐﾐﾋ羚ｨｲﾙﾗ�
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
		//outputData(data, 25, i, output_file,arg); //ﾒｻﾐｩﾏﾔﾊｾ｡｢ﾊ莎�ｵﾄ
		//outputData(data2, 6, output_file);
	//	openRadar.RadarRead(data);          //ｴﾓdataｵｽRadarRhoｰﾑｱ菽ｿｵﾄｷｶﾎｧﾀｩｴ�｡｣ﾆ萍ｵｿﾉﾒﾔﾖｱｽﾓﾓﾃRadarRhoｴ�ﾌ訶ata｣ｬｻ�ﾈ｡ｾ狢�ﾊ�ｾﾝﾊｱﾒｲﾒｻﾑ�｣ｬｿﾉﾄﾜｸ�ﾊ｡ﾊｱｼ�
	//	openRadar.CreateRadarImage2(RadarImage2, openRadar.R_x , openRadar.R_y) ;
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
	//	openRadar.MedFilter(openRadar.RadarRho, openRadar.RadarTheta);    //ﾖﾐﾖｵﾂﾋｲｨ
		
	//  openRadar.CreateRadarImage(RadarImage, openRadar.RadarRho, openRadar.RadarTheta);
	//	cvShowImage("Radar", RadarImage);
	
		//openRadar.BreakRadarRho();                    //ﾓﾐｷｵｻﾘｶﾏｵ飜�ｵｫﾊﾇﾃｻﾈ･ﾓﾃ////ﾂ�｣｡｣｡｣｡｣｡｣｡｣｡｣｡｣｡｣｡｣｡
		
		//openRadar.CreateRadarImage(RadarImage, openRadar.BreakedRadarRho, openRadar.BreakedRadarTheta);
		openRadar.CreateRadarImageself(RadarImage, data, theta);
		cvShowImage("BreakedRadar", RadarImage);

		////ﾔﾚｴﾋｴｦﾌ�ｼﾓｱﾘﾒｪｵﾄﾋ羚ｨ 
		////ﾎｴｼﾓﾖﾐﾖｵﾂﾋｲｨ｡｢ｲｻﾖｪｻ瞞�ﾑ�
		////ﾔｲｻ｡ﾄ篌ﾏｳﾉﾔｲ｣ｬｰ�ｾｶｲﾎﾊ�ﾔﾚｻ�ｷ�ｺｯﾊ�ﾀ�ｸ�ｸﾄ
		
		//openRadar.FitArc (openRadar.FittedArc, openRadar.BreakedRadarRho, openRadar.BreakedRadarTheta, &pillar_t);
		
		//openRadar.DrawRadarArc(RadarImage, openRadar.FittedArc);
		//cvShowImage("ArcRadar", RadarImage);

		
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
	key = cvWaitKey(1);                     //ﾊﾇｷ�ﾒｻｶｨﾒｪｵﾈｴ�30ms?ﾊﾇｷ�ｻ瞞�ｳﾉﾑﾓｳﾙ｣ｿ
	//	if (key == 27)//escﾍﾋｳ�
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
