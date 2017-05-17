#include "commonstruct.h"

#ifndef _CTLSCHEME_PROTOCOL_
#define _CTLSCHEME_PROTOCOL_

typedef CMsgFrame Header;

typedef struct tag_DATA_SVR_CONFIRM
{
	BYTE RetCode;//0=success,other=errorcode
//	BYTE TurnOver;
}DATA_SVR_CONFIRM,*PDATA_SVR_CONFIRM;
typedef struct tag_SVR_CONFIRM
{
	Header cmdHeader;
	DATA_SVR_CONFIRM  SvrConfirm;            
}SVR_CONFIRM,*PSVR_CONFIRM;



//#18��·�ڻ�������Ҫִ�е�ʱ�η�����(revised 02,27,2001)
typedef BYTE   DATA_CUR_SCHEME;
typedef struct _CMD_CUR_SCHEME
{
	Header cmdHeader;
	DATA_CUR_SCHEME  CurScheme_Data;   //<127ΪҪִ�еķ����ţ�ʵ��1-16
                                             //>128��Լ���ķ�ʽ���з���ѡ��
}CMD_CUR_SCHEME,*PCMD_CUR_SCHEME;


//#80
//��·�ڻ����Ͳ�ѯ����0x80(revised 02,27,2001)
typedef BYTE IMPL_TYPE;
typedef struct _QRY_ALL_STATE
{
	Header cmdHeader;
	IMPL_TYPE Param;//B1��0��ֻ��һ������,B1��1��FD�����ͼ��������
					//B1��FE��ֹͣ����, B1��FF��������״̬�ı䣩ʱ����
}QRY_ALL_STATE,*PQRY_ALL_STATE;

//��·�ڻ����Ͳ�ѯ����0x82
typedef QRY_ALL_STATE QRY_CHECKERS_NUM;
//��0x82�������Ӧ����16���������������ֵ
typedef struct _DATA_CHECKERS_NUM
{
	BYTE  Checkers[16];
	BYTE  TimeLen[16];
	BYTE  Occupancy[16];  //ռ����
}DATA_CHECKERS_NUM,*PDATA_CHECKERS_NUM;

typedef struct _RESP_CHECKERS_NUM
{
	Header cmdHeader;
	DATA_CHECKERS_NUM       CheckersNum_Data;            
}RESP_CHECKERS_NUM,*PRESP_CHECKERS_NUM;

//��·�ڻ����Ͳ�ѯ����0x83
typedef QRY_ALL_STATE QRY_POLLUTING_VAL;
//��0x83�������Ӧ����������CO��SO2���¶ȵ���ֵ
typedef struct _DATA_POLLUTING_VAL
{
	BYTE  Noise;
	BYTE  CO;
	BYTE  SO2;
	BYTE  Temperature;
}DATA_POLLUTING_VAL,*PDATA_POLLUTING_VAL;
typedef struct _RESP_POLLUTING_VAL
{
	Header cmdHeader;
	DATA_POLLUTING_VAL      PollutingVal_Data;            
}RESP_POLLUTING_VAL,*PRESP_POLLUTING_VAL;

//��·�ڻ����Ͳ�ѯ����0x84(revised 02,27,2001)
typedef QRY_ALL_STATE QRY_OTHER_VAL;
//��0x84�������Ӧ������ͨ������ռ���ʵ�
/*typedef struct _DATA_OTHER_VAL
{
	BYTE     Flux[16];       //��ͨ����(??)
	BYTE     Occupancy[16];  //ռ����(??)
}DATA_OTHER_VAL,*PDATA_OTHER_VAL;
typedef struct _RESP_OTHER_VAL
{
	Header cmdHeader;
	DATA_OTHER_VAL          Other_Data;            
}RESP_OTHER_VAL,*PRESP_OTHER_VAL;
*/
//0x85��TEST event,�ͻ�Ӧ���һ��ʱ��������
typedef QRY_ALL_STATE OPR_TEST_MSG;


//0xf0 Client login(revised 02,27,2001)
typedef struct _DATA_LOGIN_VAL
{
	BYTE  bLogin;//true login,false logout
	BYTE  Name[15];
	BYTE  PassWord[15];
}DATA_LOGIN_VAL,*PDATA_LOGIN_VAL;
typedef struct _DATA_LOGIN
{
	Header cmdHeader;
	DATA_LOGIN_VAL  Login_Data;            
}DATA_LOGIN,*PDATA_LOGIN;

//0xf1 trans perform
typedef QRY_ALL_STATE TRANS_MSG;
//0 trans start,1 trans commit,2 trans rollback

//#pragma pack(pop)
#endif

