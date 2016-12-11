#pragma once



#include "Urg_driver.h"
#include "OpenRadar.h"
//#include "Connection_information.h"
#include "math_utilities.h"
#include<vector>
#include <iostream>
#include <cmath>
#include "io.h"
#include <Windows.h>  
#include "WeightedFit.h"

using namespace std;

#define MSG_LEN 21
#define MSG_HEADER 0xdd
#define MSG_ID 0xff
//#define MSG_CRC16 0xee                 //
//#define BUF_LEN 20



//typedef struct
//{
//	union FRAME_HEAD_u
//	{
//		uint8_t  FRAME_HEAD = MSG_HEADER;  //1+1+1+4+4+4+1=16
//		char samespace[1];
//
//	}FRAME_HEAD_u;
//	union DEVICE_ID_u
//	{
//		uint8_t  DEVICE_ID = MSG_ID;
//		char samespace;
//	}DEVICE_ID_u;
//	union SCANF_FLAG_u
//	{
//		uint8_t  SCANF_FLAG;
//		char samespace;
//	}SCANF_FLAG_u;
//	union CHECKSUM_u
//	{
//		uint8_t     CHECKSUM;
//		char samespace;
//	}CHECKSUM_u;
//
//	union CC_X_u
//	{
//		int      CC_X;
//		char samespace[4];
//	}CC_X_u;
//	union CC_Y_u
//	{
//		int      CC_Y;
//		char samespace[4];
//	}CC_Y_u;
//	union CC_Tho_u
//	{
//		float      CC_Tho;
//		char samespace[4];
//	}CC_Tho_u;
//
//
//	
//}Pillar_t;
//typedef struct
//{
//
//	uint8_t      FRAME_HEAD = MSG_HEADER;
//	uint8_t      DEVICE_ID = MSG_ID;
//	uint8_t      SCANF_FLAG;
//	uint8_t      CHECKSUM;
//	int          CC_X;
//	int          CC_Y;
//	float        CC_Tho;
//
//}Pillar_t;


//union Pillar_u
//{
//
//	struct Pillar_t
//	{
//
//		uint8_t      FRAME_HEAD = MSG_HEADER;
//		uint8_t      DEVICE_ID = MSG_ID;
//		uint8_t      SCANF_FLAG;
//		uint8_t      CHECKSUM;
//		int          CC_X;
//		int          CC_Y;
//		float        CC_Tho;
//
//	}pillar;
//
//	char samespace[16];
//
//
//}pillar_u;


//Բ��Բ����Ϣ���ڴ��ݽṹ�塣һ֡16byte // �ṹ�����λ�ò������Ե������漰��λ��
	struct Pillar_t
	{

		uint8_t      FRAME_HEAD = MSG_HEADER;    //֡ͷ
		uint8_t      DEVICE_ID = MSG_ID;         //ID
		uint8_t      SCANF_FLAG;                 //��ɨ��֡��־λ
		uint8_t      CHECKSUM;                   //У���
		int          CC_X;                       //Բ��X����
		int          CC_Y;                       //Բ��Y����
		float        CC_Tho;                     //Բ�ļ�����Ƕ�ֵ
		uint8_t      pillar_sendchar[16] = {0};  //��֡����
	};

	




enum {
	Timeout = 1000,               // [msec]
	EachTimeout = 2,              // [msec]
	LineLength =1+1+1+1+4+4+4,
};

 int com_changeBaudrate(long baudrate);
 int com_connect(const char* device, long baudrate);
bool SetupTimeout(DWORD ReadInterval, DWORD ReadTotalMultiplier, DWORD
	ReadTotalConstant, DWORD WriteTotalMultiplier, DWORD WriteTotalConstant);
 int com_send(const char* data, int size);
 int dfcom_send(uint8_t * data, int size);
 int urg_sendTag( char* tag);
 int com_recv(char* data, int max_size, int timeout);      //���ڶ�ȡ
 int COMREAD(char *data);

 void DFCOMPush(Pillar_t *pillar, vector<ArcPara>& FittedArc, int i);

 //int DFCOMPop(Pillar_t &pillar, size_t len);
 int DFCOMPop(Pillar_t *pillar, size_t len);