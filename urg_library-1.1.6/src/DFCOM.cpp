

#include "OpenRadar.h"
#include"DFCOM.h"



HANDLE HCom = INVALID_HANDLE_VALUE;
int ReadableSize = 0;

char* ErrorMessage = "no error.";

 int com_changeBaudrate(long baudrate)
{
	DCB dcb;

	GetCommState(HCom, &dcb);
	dcb.BaudRate = baudrate;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.fParity = FALSE;
	dcb.StopBits = ONESTOPBIT;
	SetCommState(HCom, &dcb);

	return 0;
}



 int com_connect(const char* device, long baudrate)
{
#if defined(RAW_OUTPUT)
	Raw_fd_ = fopen("raw_output.txt", "w");
#endif

	char adjust_device[16];
	_snprintf(adjust_device, 16, "\\\\.\\%s", device);           //第二种打开串口方式，可以打开COM10以上串口
	HCom = CreateFileA(adjust_device, GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (HCom == INVALID_HANDLE_VALUE) {
		return -1;
	}

	// Baud rate setting
	return com_changeBaudrate(baudrate);
}



bool SetupTimeout(DWORD ReadInterval, DWORD ReadTotalMultiplier, DWORD
	ReadTotalConstant, DWORD WriteTotalMultiplier, DWORD WriteTotalConstant)
{
	COMMTIMEOUTS timeouts;
	timeouts.ReadIntervalTimeout = ReadInterval;
	timeouts.ReadTotalTimeoutConstant = ReadTotalConstant;
	timeouts.ReadTotalTimeoutMultiplier = ReadTotalMultiplier;
	timeouts.WriteTotalTimeoutConstant = WriteTotalConstant;
	timeouts.WriteTotalTimeoutMultiplier = WriteTotalMultiplier;
	if (!SetCommTimeouts(HCom, &timeouts))
	{
		return false;
	}
	else
		return true;
}


 int com_send( const char* data, int size)
{
	DWORD n;
	WriteFile(HCom, data, size, &n, NULL);   //WriteFile函数将数据写入一个文件。
	return n;
}

 int dfcom_send(uint8_t * data, int size)
 {
	 DWORD n;
	 WriteFile(HCom, data, size, &n, NULL);   //WriteFile函数将数据写入一个文件。
	 return n;
 }

// The command is transmitted to URG
 int urg_sendTag( char * tag)
{
	char send_message[LineLength];
	
	_snprintf(send_message, LineLength, "%s\n", tag);
	int send_size = (int)strlen(send_message);
	com_send(send_message, send_size);

	return send_size;
}









 int com_recv(char* data, int max_size, int timeout)      //串口读取
{
	if (max_size <= 0) {
		return 0;
	}

	if (ReadableSize < max_size) {
		DWORD dwErrors;
		COMSTAT ComStat;
		ClearCommError(HCom, &dwErrors, &ComStat);
		ReadableSize = ComStat.cbInQue;              //当前串口中存有的数据个数
	}

	if (max_size > ReadableSize) {
		COMMTIMEOUTS pcto;
		int each_timeout = 2;

		if (timeout == 0) {
			max_size = ReadableSize;

		}
		else {
			if (timeout < 0) {
				/* If timeout is 0, this function wait data infinity */   //如果timeout为0，此函数无限制等待数据
				timeout = 0;
				each_timeout = 0;
			}

			/* set timeout */
			GetCommTimeouts(HCom, &pcto);
			pcto.ReadIntervalTimeout = timeout;
			pcto.ReadTotalTimeoutMultiplier = each_timeout;
			pcto.ReadTotalTimeoutConstant = timeout;
			SetCommTimeouts(HCom, &pcto);
		}
	}

	DWORD n;
	ReadFile(HCom, data, (DWORD)max_size, &n, NULL);
#if defined(RAW_OUTPUT)
	if (Raw_fd_) {
		for (int i = 0; i < n; ++i) {
			fprintf(Raw_fd_, "%c", data[i]);
		}
		fflush(Raw_fd_);
	}
#endif
	if (n > 0) {
		ReadableSize -= n;
	}

	return n;
}


 int COMREAD(char *data)
{

	//	char str[100];

	DWORD dwErrors;
	COMSTAT ComStat;

	DWORD wCount;//读取的字节数  
	BOOL bReadStat;

	ClearCommError(HCom, &dwErrors, &ComStat);
	ReadableSize = ComStat.cbInQue;              //当前串口中存有的数据个数
												 //	printf("ReadableSize=%d", ReadableSize);
	bReadStat = ReadFile(HCom, data, 1, &wCount, NULL);
	//data  = str;
	if (!bReadStat)
	{
		printf("Fail to read the COM!\n");
		return FALSE;
	}

	return wCount;

}

 void DFCOMPush(Pillar_t*  pillar, vector<ArcPara>& FittedArc,int i)
 {
	 pillar->CHECKSUM = 0;
	 pillar->CC_X = FittedArc.at(i).center.x;
	 pillar->CC_Y = FittedArc.at(i).center.y;
	 pillar->CC_Tho = (float)(pillar->CC_X / pillar->CC_Y);
	
	/* pillar->CHECKSUM = 0;
	 pillar->CC_X = 0x11223344;
	 pillar->CC_Y = 0x55667788;
	 pillar->CC_Tho = (float)154.654;*/
	
	 if (i == 0)
		 pillar->SCANF_FLAG = 1;
	 else
		 pillar->SCANF_FLAG = 0;

	 memcpy(pillar->pillar_sendchar , pillar, 16);
	 for (size_t j = 4; j < 16;j++)
	 {
		pillar->CHECKSUM += pillar->pillar_sendchar[j];
	 }
	 pillar->CHECKSUM = pillar->CHECKSUM & 0x3f;

	 memcpy(pillar->pillar_sendchar, pillar, 16);
	/* memcpy(pillar, pillar->pillar_sendchar, 16);
	 cout << " x: " << pillar->CC_X
		 << " y: " << pillar->CC_Y
		 << " r: " << pillar->CC_Tho << endl;*/
	
	
 }


 int DFCOMPop(Pillar_t *pillar,size_t len)
 {
	
	 int send_size = dfcom_send(pillar->pillar_sendchar, len);
	 /*char buf[16] = { 0 };

	 int send_size = urg_sendTag(buf);*/
	 return send_size;
 }