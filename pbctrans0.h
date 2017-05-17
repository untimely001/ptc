
typedef struct tag_Test
{
	BYTE cmdNo;        //CM_TEST心跳
	BYTE CustType;     //客户类型
	UINT16 EquipID;    //设备编号
}TestMsg;
typedef struct tag_StationStatusReq
{
	BYTE cmdNo;				//CM_STATIONSTATUSReq请求站台状态信息
	UINT16 StationNo;
}StationStatusReq;
typedef struct tag_StationStatus
{
	BYTE cmdNo;				//CM_STATIONSTATUS 站台状态信息
	UINT16 StationNo;
	BYTE BusPits;			//停车位数
	BYTE BusWaiting;		//等待车辆数
	UINT16 BusId1;			//车位1上的公交车ID 
	UINT16 BusId2;			//车位2上的公交车ID
	UINT16 BusId3;			//车位3上的公交车ID
	UINT16 BusId4;			//车位4上的公交车ID
	UINT16 BusId5;			//车位5上的公交车ID
}StationStatus;
typedef struct tag_BusStatusReq
{
	BYTE cmdNo;				//CM_BUSSTATUSREQ,			//请求公交车状态信息
	UINT16 BusNo;
}BusStatusReq;
typedef struct tag_BusStatus
{
	BYTE cmdNo;				//CM_BUSSTATUS,			//公交车状态信息
	UINT16 BusNo;
	double Longitude;		//经度
	double Latitude;		//纬度
	BYTE Direction;			//方向（上行/下行）
	float Velocity;			//即时行驶速度
	UINT32 UTC;				//时间
	UINT16 Passengers;		//乘客人数
}BusStatus;
typedef struct tag_StationInitReq
{
	BYTE cmdNo;				//CM_STATIONINITREQ		//站台初始化请求
	UINT16 StationNo;
}StationInitReq;
typedef struct tag_StationInit
{
	BYTE cmdNo;				//CM_STATIONINIT			//站台初始化信息
	UINT16 StationNo;
	BYTE BusPits;			//停车位数
	BYTE BusLines;			//停靠的公交线路数
	double Longitude;		//经度
	double Latitude;		//纬度
	UINT16 BusLineIDs[20];	//停靠的公交线路
	char Name[40];			//车站名
	int  Reserved;			//保留
}StationInit;
typedef struct tag_BusInitReq
{
	BYTE cmdNo;				//CM_BUSINITREQ,				//公交车初始化请求
	BYTE Auxiliary;       //意义待定
	UINT16 BusNo;
}BusInitReq;
typedef struct tag_BusInit
{
	BYTE cmdNo;				//CM_BUSINITREQ,				//公交车初始化
	UINT16 BusNo;
	char NumberPlate[10];     //车牌号
	UINT16 BusLineID;		//公交线号
	UINT16 CarryCap;		//最大载人数
	char BusLineName[10];	//公交线路名
	int  Reserved;			//保留
}BusInit;
typedef struct tag_BusComing
{
	BYTE cmdNo;			//CM_BUSCOMING,				//公交车到站通告
	UINT16 StationNo;
	UINT16 BusID;		//车辆号
	BYTE Direction;		//方向（上行/下行）
	UINT32 UTC;			//时间
	UINT16 Passengers;	//乘客人数
}BusComing;
typedef struct tag_BusStopping
{
	BYTE cmdNo;			//CM_BUSSTOPPing,			公交车到达车位通告
	UINT16 StationNo;
	UINT16 BusID;		//车辆号
	BYTE Direction;		//方向（上行/下行）
	BYTE BusPit;		//车位号
	UINT32 UTC;			//时间
	UINT16 Passengers;	//乘客人数
}BusStopping;
typedef struct tag_BusLeaving
{
	BYTE cmdNo;			//CM_BUSLEAVING,				//公交车离站通告
	UINT16 StationNo;
	UINT16 BusID;		//车辆号
	BYTE Direction;		//方向（上行/下行）
	BYTE BusPit;		//车位号
	UINT32 UTC;			//时间
	UINT16 Passengers;	//乘客人数
}BusLeaving;
typedef struct tag_BusRunning
{
	BYTE cmdNo;			//CM_BUSRUNNING,		//公交车站间通告
	UINT16 BusID;		//车辆号
	BYTE Direction;		//方向（上行/下行）
	double Longitude;	//经度
	double Latitude;	//纬度
	float Velocity;		//即时行驶速度
	UINT32 UTC;			//时间
	UINT16 Passengers;	//乘客人数
}BusRunning;
typedef struct tag_Login
{
	BYTE cmdNo;			//CM_LOGON,					//登录
	BYTE CustType;		//客户类型	0=站台, 1=BUS, 2=其它
	UINT16 EquipID;		//设备编号
	char UserID[20];
	char Password[20];
}Login;
typedef struct tag_Logout
{
	BYTE cmdNo;			//CM_LOGOUT,					//登出
	BYTE CustType;		//客户类型	0=站台, 1=BUS, 2=其它
	UINT16 EquipID;		//设备编号
	char UserID[20];
	char Password[20];
}Logout;
typedef struct tag_ChangePassword
{
	BYTE cmdNo;			//CM_CHANGEPASSWORD,			//修改登录密码
	BYTE CustType;		//客户类型	0=站台, 1=BUS, 2=其它
	UINT16 EquipID;		//设备编号
	char UserID[20];
	char NewPassword[20];
}ChangePassword;
typedef struct tag_AlarmMsg
{
	BYTE cmdNo;         //CM_ALARMMSG,		//异常报警信息
	BYTE CustType;		//客户类型	0=站台, 1=BUS, 2=其它
	UINT16 EquipID;		//设备编号
	UINT32 aux;			//辅助字节，具体待定义
	UINT32 Status;		//异常状态字，如电池、电源、通讯口等各种设备状态，具体待定义
}AlarmMsg;
typedef struct tag_SysTimeReq
{
	BYTE cmdNo;			//CM_SYSTIMEREQ    请求系统时间
	BYTE CustType;		//客户类型	0=站台, 1=BUS, 2=其它
	UINT16 EquipID;		//设备编号
	UINT32 aux;			//辅助字节，具体待定义
}SysTimeReq;
typedef struct tag_SysTime
{
	BYTE cmdNo;				//CM_SYSTIME    系统时间
	BYTE CustType;			//客户类型	0=站台, 1=BUS, 2=其它
	UINT16 EquipID;			//设备编号
	UINT32 aux;				//辅助字节，具体待定义
	UINT32 UTC;				//时间
}SysTime;
typedef struct tag_BusLineReq
{
	BYTE cmdNo;				//CM_BUSLINEREQ    请求公交线路
	BYTE CustType;			//客户类型	0=站台, 1=BUS, 2=其它
	UINT16 EquipID;			//设备编号
	UINT16 BusLineID;		//公交线号
}BusLineReq;
typedef struct tag_BusLine
{
	BYTE cmdNo;				//CM_BUSLINE    公交线路信息
	UINT16 BusLineID;		//公交线号
	char BusLineName[10];	//公交线路名
	UINT16 Stations[50];	//经过的站台号 
	UINT32 Reserved;
}BusLine;

