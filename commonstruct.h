#ifndef COMMONSTRUCT_H
#define COMMONSTRUCT_H

#define MsgFlag 0xAA

//#ifndef __TEST
//#pragma option -b
//#endif
//#pragma pack(push,1)
//通讯客户类型
typedef enum {
	CT_STATION = 0,				//站台
	CT_BUS,						//车辆
	CT_COMMUTER,				//行人
	CT_OPERATOR					//操作员
}CustType;	//通讯客户类型
//消息格式定义
typedef enum {
	CM_TEST = 1,				//心跳信号
	CM_STATIONSTATUSREQ,		//请求站台状态信息
	CM_STATIONSTATUS,			//站台状态信息
	CM_BUSSTATUSREQ,			//请求公交车状态信息
	CM_BUSSTATUS,				//公交车状态信息
	CM_STATIONINITREQ,			//站台初始化请求
	CM_STATIONINIT,				//站台初始化信息
	CM_BUSINITREQ,				//公交车初始化请求
	CM_BUSINIT,					//公交车初始化信息
	CM_BUSCOMING,				//公交车到站通告
	CM_BUSCOMINGACK,			//公交车到站通告应答
	CM_BUSSTOPPING,			//公交车到达车位通告
	CM_BUSSTOPPINGACK,		//公交车到达车位通告应答
	CM_BUSLEAVING,				//公交车离站通告
	CM_BUSLEAVINGACK,			//公交车离站通告应答
	CM_BUSRUNNING,				//公交车站间通告
	CM_BUSRUNNINGACK,			//公交车站间通告应答
	CM_LOGON,					//登录
	CM_LOGONACK,				//登录应答
	CM_LOGOUT,					//登出
	CM_LOGOUTACK,				//登出应答
	CM_CHANGEPASSWORD,			//修改登录密码
	CM_CHANGEPASSWORDACK,		//修改登录密码应答
	CM_ALARMMSG,				//异常报警信息
	CM_SYSTIMEREQ,				//请求系统时间
	CM_SYSTIME,					//系统时间消息
	CM_BUSLINEREQ,			    //请求公交线路
	CM_BUSLINE,					//公交线路信息
	CM_UNKNOWN
}MsgType;         //消息类型
typedef union _UNION_BYTE_BIT
{
	BYTE  U_Byte;
	struct{
		unsigned  D0:1;
		unsigned  D1:1;
		unsigned  D2:1;
		unsigned  D3:1;
		unsigned  D4:1;
		unsigned  D5:1;
		unsigned  D6:1;
		unsigned  D7:1;
		unsigned  reserved:24;
	}U_Bit;
}UNION_BYTE_BIT;
typedef union tag_UNION_UNIID
{
	unsigned int UniID;
	struct{
		unsigned short StationID;
		unsigned short BusID;
	}SplitID;
}CombinedID;
typedef struct 
{
	BYTE Flag1;	    //消息帧标志 == MsgFlag
    BYTE Flag2;	    //55
	BYTE Command;	//命令号
	BYTE Len;		//消息长度
}CMsgFrame;	//消息头格式

//#pragma pack(pop)
#endif

