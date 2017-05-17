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



//#18给路口机发现在要执行的时段方案号(revised 02,27,2001)
typedef BYTE   DATA_CUR_SCHEME;
typedef struct _CMD_CUR_SCHEME
{
	Header cmdHeader;
	DATA_CUR_SCHEME  CurScheme_Data;   //<127为要执行的方案号，实际1-16
                                             //>128按约定的方式进行方案选择
}CMD_CUR_SCHEME,*PCMD_CUR_SCHEME;


//#80
//向路口机发送查询命令0x80(revised 02,27,2001)
typedef BYTE IMPL_TYPE;
typedef struct _QRY_ALL_STATE
{
	Header cmdHeader;
	IMPL_TYPE Param;//B1＝0：只发一次数据,B1＝1－FD：发送间隔秒数；
					//B1＝FE：停止发送, B1＝FF：参数（状态改变）时发送
}QRY_ALL_STATE,*PQRY_ALL_STATE;

//向路口机发送查询命令0x82
typedef QRY_ALL_STATE QRY_CHECKERS_NUM;
//对0x82命令的响应，发16个车辆检测器的数值
typedef struct _DATA_CHECKERS_NUM
{
	BYTE  Checkers[16];
	BYTE  TimeLen[16];
	BYTE  Occupancy[16];  //占有率
}DATA_CHECKERS_NUM,*PDATA_CHECKERS_NUM;

typedef struct _RESP_CHECKERS_NUM
{
	Header cmdHeader;
	DATA_CHECKERS_NUM       CheckersNum_Data;            
}RESP_CHECKERS_NUM,*PRESP_CHECKERS_NUM;

//向路口机发送查询命令0x83
typedef QRY_ALL_STATE QRY_POLLUTING_VAL;
//对0x83命令的响应，发噪音、CO、SO2、温度的数值
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

//向路口机发送查询命令0x84(revised 02,27,2001)
typedef QRY_ALL_STATE QRY_OTHER_VAL;
//对0x84命令的响应，发交通流量、占有率等
/*typedef struct _DATA_OTHER_VAL
{
	BYTE     Flux[16];       //交通流量(??)
	BYTE     Occupancy[16];  //占有率(??)
}DATA_OTHER_VAL,*PDATA_OTHER_VAL;
typedef struct _RESP_OTHER_VAL
{
	Header cmdHeader;
	DATA_OTHER_VAL          Other_Data;            
}RESP_OTHER_VAL,*PRESP_OTHER_VAL;
*/
//0x85发TEST event,客户应间隔一段时间主动发
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

