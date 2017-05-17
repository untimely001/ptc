#ifndef _AGENTSVR_H
#define _AGENTSVR_H

#pragma option -b

#pragma pack(push,1)

typedef unsigned char	UINT8;
typedef unsigned short	UINT16;
//typedef unsigned long	UINT32;
typedef unsigned char	BOOLEAN;

//define command Number

//define error messages

#define TRUEE	(UINT8)0xFF
#define FALSEE	(UINT8)0x00

//define error Number
#define ERROR01					1
//System Error
#define ERROR02					2
//System Error
#define ERROR03					3
//System Error
#define ERROR04					4
//System Error
#define ERROR05					5
//System Error
#define ERROR06					6
//System Error
#define ERROR07					7
//System Error
#define ERROR08					8
//System Error
#define ERROR09					9
//System Error
#define ERROR10					10
//Monitor denied
#define ERROR11					11
//Monitor not Present
#define ERROR12					12
//Camera denied
#define ERROR13					13
//Camera not present
#define ERROR14					14
//Gpi denied
#define ERROR15					15
//Gpi not present
#define ERROR16					16
//Macro overload
#define ERROR17					17
//Macro not present
#define ERROR18					18
//Alarm not present
#define ERROR19					19
//Alarm denied
#define ERROR20					20
//Fatal system error
#define ERROR21					21
//Fatal system error
#define ERROR22					22
//Fatal system error
#define ERROR23					23
//Fatal system error
#define ERROR24					24
//Fatal system error
#define ERROR25					25
//Communications error,modem
#define ERROR26					26
//Communications error,no interrupt
#define ERROR27					27
//Communications error,overrun
#define ERROR28					28
//Communications error,parity
#define ERROR29					29
//Communications error,framing(async,byte)
#define ERROR30					30
//Communications error,break
#define ERROR31					31
//Communications error,system error
#define ERROR32					32
//Communications error,maximum retry reached
#define ERROR33					33
//Communications error,IIR
#define ERROR34					34
//Communications error,receive buffer overflow
#define ERROR35					35
//Communications error,Transmit buffer overflow
#define ERROR36					36
//LPT buffer overflow
#define ERROR37					37
//system error,transmit flag lost
#define ERROR38					38
//system error,receive flag lost
#define ERROR39					39
//被抢占
#define ERROR40					40
//rejected by the server
#define ERROR41					41
//time out

//define PIN numbers
#define AlarmInterface			1
#define MatrixPortInput			2
#define NetworkPort				3
#define ExternalPort			4
#define CCDPort					5
#define PCPort					6
#define ModemPort				7
#define MatrixBayPort			8
#define InterceptCCDPort		9
#define MasterDistrAmpPort		10
#define HighLevelPort			11
#define MatrixOutputPort		12
#define KeyboardPort			13

//define Camera Control Constant
#define CameraCtrBase	0x00

#define ProgramPreset1	CameraCtrBase+0x00
#define ProgramPreset2	CameraCtrBase+0x01
#define ProgramPreset3	CameraCtrBase+0x02
#define ProgramPreset4	CameraCtrBase+0x03
#define ProgramPreset5	CameraCtrBase+0x04
#define ProgramPreset6	CameraCtrBase+0x05
#define ProgramPreset7	CameraCtrBase+0x06
#define ProgramPreset8	CameraCtrBase+0x07
#define PanRightStop	CameraCtrBase+0x10
#define PanRightStart	CameraCtrBase+0x11
#define PanLeftStop		CameraCtrBase+0x12
#define PanLeftStart	CameraCtrBase+0x13
#define TiltDownStop	CameraCtrBase+0x14
#define TiltDownStart	CameraCtrBase+0x15
#define TiltUpStop		CameraCtrBase+0x16
#define TiltUpStart		CameraCtrBase+0x17
#define ZoomOutStop		CameraCtrBase+0x24
#define ZoomOutStart	CameraCtrBase+0x25
#define ZoomInStop		CameraCtrBase+0x26
#define ZoomInStart		CameraCtrBase+0x27
#define IrisCloseStop	CameraCtrBase+0x28
#define IrisCloseStart	CameraCtrBase+0x29
#define IrisOpenStop	CameraCtrBase+0x2A
#define IrisOpenStart	CameraCtrBase+0x2B
#define FocusNearStop	CameraCtrBase+0x2C
#define FocusNearStart	CameraCtrBase+0x2D
#define FocusFarStop	CameraCtrBase+0x2E
#define FocusFarStart	CameraCtrBase+0x2F
#define ClearPreset1	CameraCtrBase+0x30
#define ClearPreset2	CameraCtrBase+0x31
#define ClearPreset3	CameraCtrBase+0x32
#define ClearPreset4	CameraCtrBase+0x33
#define ClearPreset5	CameraCtrBase+0x34
#define ClearPreset6	CameraCtrBase+0x35
#define ClearPreset7	CameraCtrBase+0x36
#define ClearPreset8	CameraCtrBase+0x37
#define Auxiliary8Off	CameraCtrBase+0x40
#define Auxiliary8On	CameraCtrBase+0x41
#define Auxiliary7Off	CameraCtrBase+0x42
#define Auxiliary7On	CameraCtrBase+0x43
#define Auxiliary6Off	CameraCtrBase+0x44
#define Auxiliary6On	CameraCtrBase+0x45
#define Auxiliary5Off	CameraCtrBase+0x46
#define Auxiliary5On	CameraCtrBase+0x47
#define Auxiliary4Off	CameraCtrBase+0x48
#define Auxiliary4On	CameraCtrBase+0x49
#define Auxiliary3Off	CameraCtrBase+0x4A
#define Auxiliary3On	CameraCtrBase+0x4B
#define Auxiliary2Off	CameraCtrBase+0x4C
#define Auxiliary2On	CameraCtrBase+0x4D
#define Auxiliary1Off	CameraCtrBase+0x4E
#define Auxiliary1On	CameraCtrBase+0x4F
#define GotoPreset1		CameraCtrBase+0x50
#define GotoPreset2		CameraCtrBase+0x51
#define GotoPreset3		CameraCtrBase+0x52
#define GotoPreset4		CameraCtrBase+0x53
#define GotoPreset5		CameraCtrBase+0x54
#define GotoPreset6		CameraCtrBase+0x55
#define GotoPreset7		CameraCtrBase+0x56
#define GotoPreset8		CameraCtrBase+0x57

//Variable speed camera data
#define FOCUSFAR	(UINT8)1
#define FOCUSNEAR	(UINT8)2
#define IRISOPEN	(UINT8)4
#define IRISCLOSE	(UINT8)8
#define CAMERAON	(UINT8)16
#define AUTOSCAN	(UINT8)32

#define PANRIGHT	(UINT8)2
#define PANLEFT		(UINT8)4
#define TILTUP		(UINT8)8
#define TILTDOWN	(UINT8)16
#define ZOOMIN		(UINT8)32
#define ZOOMOUT		(UINT8)64

//Statistics patterns
#define MACROSTOPPED			(UINT8)2
#define CAMERALOCKED			(UINT8)16
#define CAMERACONTRALLABLE		(UINT8)64
#define ALARMARMED				(UINT8)1
#define ALARMTRIPPED			(UINT8)2
#define NEWALARMSWITCHED		(UINT8)4
#define ALARMSEQUENCESTOPPED	(UINT8)8
#define NEWALARMTRIPPED			(UINT8)32
#define VIDEOLOSSDETECTED		(UINT8)64

//define command class
#define CLIENT_REQUEST			(UINT8)1
#define	SERVER_CONFIRMATIOM		(UINT8)2
#define	SERVER_UNSOLICITED		(UINT8)3
#define RESULT_ERROR			(UINT8)4


#define MAX_MONITOR_NUM			(UINT8)128         /*32*/
#define MAX_CAMERA_NUM			(UINT16)512        /*128*/
#define MAX_OPR_NUM				(UINT8)100         //32 
#define MAX_NAME_LEN			(UINT8)13
#define MAX_WAIT_NUM			(UINT8)72  //F7-B1+ (AA=f8)
#define MAX_ERROR_NUM			(UINT8)56
#define MAX_MSG					(UINT8)20
#define MAX_ECHO_NUM			(UINT8)15
#define MAX_PACKET_LEN			(UINT8)50 //客户发送消息的最大字节数
#define MAX_WAIT_CONF_TIME		(UINT8)10

//define command No.
#define SENDCAMERACTL				0x0b8
#define OPRLOGIN					1
//added 2000,12,05
#define FORCEDRELEASE				2
//added end
#define VARISPEEDCTL				0x0c0
#define SENDCURMONITORASSIGN		0x0b1
#define SWITCH2MONITOR				0x0b2
#define VIDEOLOSSMASK				0x0b3
#define STARTMACRO					0x0b4
#define STOPMACRO					0x0b5
#define NEXTCAMERA					0x0b6
#define PRECAMERA					0x0b7
#define CAMERARELEASE				0x0b9
#define MONITORSTATUS				0x0ba
#define CAMERALOCK					0x0bb
#define CAMERACLEAR					0x0bc
#define CAMERACTLOVERRIDE			0x0be
#define CHECKCAMERA					0x0bf
#define VIDEOLOSS					0x0c4
#define VIDEOLOSSDETECT				0x0c5
#define IODEV						0x0c9
#define TESTPORT					0x0ca
#define CAMERA2MONITORINFO			0x0cc
#define DATATIMEINFO				0x0cd
#define CAMERA2MONITOR				0x0ce
#define REQUESTUPDATES				0x0cf
#define CURALARMNO					0x0d0
#define TRIPPEDALARMNO				0x0d1
#define POSNATTRINFO				0x0d2
#define EQUIPSTATUS					0x0d3
#define SELECTALARM					0x0d6
#define ARMALARM					0x0d7
#define DISARMALARM					0x0d8
#define STEPALARMS					0x0d9
#define GETLASTALARM				0x0da
#define STEPACTIVEALARM				0x0dc
#define RESETALARM					0x0dd
#define ALARMINFO					0x0de
#define STARTALARM					0x0df
#define SENDERROR					0x0e1
#define SENDFATALERR				0x0e2
//#define RESETALARMS				0x0e2
#define MIMICSWITCH					0x0e3
#define GDICTL						0x0e7
#define RELEASEMACRO				0x0e8
#define ARMTABLE					0x0ea
#define STEPMACRO					0x0eb
#define SELECTMACRO					0x0ec
#define UPDATEARMTABLE				0x0ed
//#define STEPMACRO					0x0ee
#define ARMINPUT					0x0ef
#define GDIREPORT					0x0f0
#define MACROREPORT					0x0f1
#define	LASTVIDEOLOSS				0x0f2
#define CHECKPIN					0x0f4
#define RECIEVEALARM				0x0f7
#define SVRNOTIFY					0x01

//define global data status
#define IDLESTATE					(UINT8)0
#define READYSTATE					(UINT8)1//only USED in opr struct
#define DOINGSTATE					(UINT8)2
#define LOCKSTATE					(UINT8)3
#define TROUBLESTATE				(UINT8)4
#define BADSTATE					(UINT8)5

//define server notification event
#define RESULTUNKNOWN				(UINT8)1


//general communication message head
typedef struct tagCommHeader_t
{
	UINT8	CommandClass;	//命令类别
	UINT8	CommandNo;		//命令号,bcd
	UINT8   SendCommand;
//	UINT32	InvokeId;		//调用号
//	UINT32	LogicCameraNo;	//逻辑号
//	UINT32	LogicMonitorNo;	//逻辑号
}CommHeader_t;

//general error message
typedef struct tagErrorMsg_t
{
	UINT8	ErrorNo;		//错误号,39为被抢占;未填
	UINT8	NowOperation;	//正在进行的操作,
	UINT8	RollBackFlag;	//是否回到该操作前的状态,即该操作是否致命
}ErrorMsg_t;

typedef struct tagOprLogin_t
{
	UINT8 LoginFlag; //login/logout true/false
	UINT8 OprName[MAX_NAME_LEN];
	UINT8 password[MAX_NAME_LEN];
//	UINT8 Priority;
}OprLogin_t;

typedef struct tagOprLoginConf_t
{
	UINT8 LoginFlag; //login/logout true/false
	UINT8 Succ;
}OprLoginConf_t;

/*//user defined
typedef struct tagCameraCtlMsg_t
{
	UINT8	CtlCode;		//控制码
	UINT8 	CameraHi;		//相机号in pBCD format
	UINT8 	CameraLo;
	UINT8	MonitorNo;		//监视器号in pBCD format

	UINT16	Angle;			//转动的度数angle to rotate
	UINT16	RoomData;		//room 100 means 100%
	UINT16	IrisData;		//?????
	UINT16	FocusData;		//??????
}CameraCtlMsg_t;

typedef struct tagCameraCtlMsgConf_t
{
	UINT8 	Result;	
}CameraCtlMsgConf_t;
*/
//b8
typedef struct tagSendCameraCtl_t
{
	UINT16	CameraNo;		
	UINT16	MonitorNo;		
	UINT8	CtlCode;		//in hex
}SendCameraCtl_t;
typedef struct tagSendCameraCtlConf_t
{
	UINT16	MonitorNo;
	UINT16 	CameraNo;
	UINT16	MacroNo;	
	UINT16	AuxStatNo;
	UINT8	Statistics1;
	UINT8	Statistics2;
	//added 2000/10/30
	UINT8	Indev;//00 camera/01 vcr/02 multiplexer
	UINT16	Altbase;
	UINT8	Outdev;

	UINT8	Operator;	//if used by another operator
}SendCameraCtlConf_t;

//c0
typedef struct tagVariSpeedCtl_t
{
	UINT16	MonitorNo;		
	UINT16 	CameraNo;
	UINT8	VariSpeed1;
	UINT8	VariSpeed2;
	UINT8	VariSpeed3;
	UINT8	VariSpeed4;
}VariSpeedCtl_t;

typedef struct tagVariSpeedCtlConf_t
{
	UINT16	MonitorNo;
	UINT16 	CameraNo;
	UINT16	MacroNo;	
	UINT16	AuxStatNo;  
	UINT8	Statistics1;
	UINT8	Statistics2;
	//added 2000/10/30
	UINT8	Indev;//00 camera/01 vcr/02 multiplexer
	UINT16	Altbase;
	UINT8	Outdev;
}VariSpeedCtlConf_t;

//b1
typedef struct tagSendCurMonitorAssign_t
{
	UINT16	MonitorNo;
	UINT8	Keybrd;	//00 old/01 new/02 gui,added 2000/10/30 
}SendCurMonitorAssign_t;

typedef struct tagSendCurMonitorAssignConf_t
{
	UINT16	MonitorNo;
	UINT16 	CameraNo;
	UINT16	MacroNo;	
	UINT16	AuxStatNo;  
	UINT8	Statistics1;
	UINT8	Statistics2;
	//added 2000/10/30
	UINT8	Indev;//00 camera/01 vcr/02 multiplexer
	UINT16	Altbase;
	UINT8	Outdev;
}SendCurMonitorAssignConf_t;

//b2
typedef struct tagSwitch2Monitor_t
{
	UINT16	MonitorNo;
	UINT16	CameraNo;
}Switch2Monitor_t;

typedef struct tagSwitch2MonitorConf_t
{
	UINT8	Success;
}Switch2MonitorConf_t;

//b3
typedef struct tagVideoLossMask_t
{
	UINT16	CameraNo;
	UINT8	Data;	//no use
	UINT8	Number; //50 max
	UINT8	Function;	// M/U
}VideoLossMask_t;

typedef struct tagVideoLossMaskConf_t
{						
	UINT16	CameraNo;
	UINT8	Function;	// 00unmask/01masked
	UINT8	EndFlag;	//是否有后续消息
}VideoLossMaskConf_t;

//b4
typedef struct tagStartMacro_t
{
	UINT16	MonitorNo;
}StartMacro_t;

typedef struct tagStartMacroConf_t
{
	UINT16	MonitorNo;
	UINT8	Success;
}StartMacroConf_t;

//b5
typedef struct tagStopMacro_t
{
	UINT16	MonitorNo;
}StopMacro_t;

typedef struct tagStopMacroConf_t
{
	UINT16	MonitorNo;
	UINT8	Success;
}StopMacroConf_t;

//b6
typedef struct tagNextCamera_t
{
	UINT16	MonitorNo;
}NextCamera_t;

typedef struct tagNextCameraConf_t
{
	UINT16	MonitorNo;
	UINT16 	CameraNo;
	UINT16	MacroNo;
	UINT16	AuxStatNo;
	UINT16	AlarmNo;
	UINT8	Statistics1;
	UINT8	Statistics2;
	//added 2000/10/30
	UINT8	Indev;//00 camera/01 vcr/02 multiplexer
	UINT16	Altbase;
	UINT8	Outdev;
	UINT8	EndFlag;	//是否有后续消息
}NextCameraConf_t;

//b7
typedef struct tagPreCamera_t
{
	UINT16	MonitorNo;
}PreCamera_t;

typedef struct tagPreCameraConf_t
{
	UINT16	MonitorNo;
	UINT16 	CameraNo;
	UINT16	MacroNo;
	UINT16	AuxStatNo;
	UINT16	AlarmNo;
	UINT8	Statistics1;
	UINT8	Statistics2;
	//added 2000/10/30
	UINT8	Indev;//00 camera/01 vcr/02 multiplexer
	UINT16	Altbase;
	UINT8	Outdev;
	UINT8	EndFlag;	//是否有后续消息
}PreCameraConf_t;

//b9
typedef struct tagCameraRelease_t
{
	UINT16 	CameraNo;
	UINT16	MonitorNo;
}CameraRelease_t;

typedef struct tagCameraReleaseConf_t
{
	UINT16	CameraNo;
	UINT16	MonitorNo;
	UINT8	Success;//?response
}CameraReleaseConf_t;

//ba
typedef struct tagMonitorStatus_t
{
	UINT16	MonitorNo;
}MonitorStatus_t;

typedef struct tagMonitorStatusConf_t
{
	UINT16	MonitorNo;
	UINT16 	CameraNo;
	UINT16	MacroNo;
	UINT16	AuxStatNo;
	UINT8	Statistics1;
	UINT8	Statistics2;
	//added 2000/10/30
	UINT8	Indev;//00 camera/01 vcr/02 multiplexer
	UINT16	Altbase;
	UINT8	Outdev;
}MonitorStatusConf_t;

//bb
typedef struct tagCameraLock_t
{
	UINT16	MonitorNo;
	UINT16 	CameraNo;
}CameraLock_t;

typedef struct tagCameraLockConf_t
{
	UINT8	Succ;//override by svr
}CameraLockConf_t;

//bc
typedef struct tagCameraClear_t
{
	UINT16	MonitorNo;
	UINT16 	CameraNo;
	UINT8	Function; //01 for unlock,02 camera lock override,03 monitor lock override,02,03未用
}CameraClear_t;

typedef struct tagCameraClearConf_t
{
	UINT8	Success;//override by svr
}CameraClearConf_t;

//be
typedef struct tagCameraCtlOverride_t
{
	UINT16	CameraNo;
	UINT8	CtlCode; 
}CameraCtlOverride_t;

typedef struct tagCameraCtlOverrideConf_t//the same as bb
{
	UINT8	Success;
}CameraCtlOverrideConf_t;

//bf
typedef struct tagCheckCamera_t
{
	UINT16 	CameraNo;
}CheckCamera_t;

typedef struct tagCheckCameraConf_t
{
	UINT8	Result;	// Mask/Unmask
}CheckCameraConf_t;

//c4 ???????????
typedef struct tagVideoLoss_t
{
	UINT16 	CameraNo;
}VideoLoss_t;

typedef struct tagVideoLossConf_t
{
	UINT8 	Succ;
}VideoLossConf_t;

//c5
typedef struct tagVideoLossDetect_t
{
	UINT16 	CameraNo;
	UINT8	Function; //00 for unmask,01 for mask
}VideoLossDetect_t;

typedef struct tagVideoLossDetectConf_t
{
	UINT8 	Succ;
}VideoLossDetectConf_t;

//this message added 2000/10/30
//c9
typedef struct tagTextOnOff_t
{
	UINT8	Stat;//on/off 0,1
	UINT8	Monitor;
}TextOnOff_t;

typedef struct tagNextAltCamera_t
{
	UINT8	TestByte;
}NextAltCamera_t;

typedef struct tagStepDefMonitor_t
{
	UINT8	Dir;
}StepDefMonitor_t;

typedef struct tagMatrixIdOnOff_t
{
	UINT8	Stat;
	UINT8	Monitor;
}MatrixIdOnOff_t;

typedef struct tagOutputDev_t
{
	UINT8	Dev;
	UINT16	Monitor;
	UINT8	Aux;
}OutputDev_t;

typedef struct tagInputDev_t
{
	UINT8	Dev;
	UINT16	Camera;
	UINT8	Aux;
}InputDev_t;

typedef struct tagPreAltCamera_t
{
	UINT8	TestByte;
}PreAltCamera_t;

typedef struct tagIODev_t
{
	UINT8	FuncNo;
	union
	{
		TextOnOff_t			TextOnOff;
		NextAltCamera_t		NextAltCamera;
		StepDefMonitor_t	StepDefMonitor;
		MatrixIdOnOff_t		MatrixIdOnOff;
		OutputDev_t			OutputDev;
		InputDev_t			InputDev;
		PreAltCamera_t		PreAltCamera;
	}u;
}IODev_t;

typedef struct tagIODevConf_t
{
	UINT8	bSucc;
	UINT16	MonitorNo;
	UINT16 	CameraNo;
	UINT16	MacroNo;	
	UINT16	AuxStatNo;  
	UINT8	Statistics1;
	UINT8	Statistics2;
	UINT8	Indev;//00 camera/01 vcr/02 multiplexer
	UINT16	Altbase;
	UINT8	Outdev;
}IODevConf_t;
//added ends

//ca
typedef struct tagTestPort_t
{
	UINT8	TestByte;
}TestPort_t;

typedef struct tagTestPortConf_t 
{
	UINT8	IsOk;
}TestPortConf_t;

//cc
typedef struct tagCamera2MonitorInfo_t
{
	UINT16	MonitorNo;		
	UINT16	CameraNo;
	UINT16	AlarmNo;
}Camera2MonitorInfo_t;

typedef struct tagCamera2MonitorInfoConf_t
{
	UINT8	IsOk;		
}Camera2MonitorInfoConf_t;

//cd
typedef struct tagDataTimeInfo_t
{
	UINT8	Date[12];
	UINT8	Time[12];
}DataTimeInfo_t;

typedef struct tagDataTimeInfoConf_t
{
	UINT8	IsOk;
}DataTimeInfoConf_t;

//ce
typedef struct tagCamera2Monitor_t
{
	UINT16	MonitorNo;
	UINT16	CameraNo;
}Camera2Monitor_t;

typedef struct tagCamera2MonitorConf_t
{
	UINT8	IsOk;
}Camera2MonitorConf_t;

//cf
typedef struct tagRequestUpdates_t
{
	UINT8	RequestCode;//01camera update,02monitor update,04alarm update,08monitor assignment update,10video loss update
}RequestUpdates_t;

typedef struct tagRequestUpdatesConf_t
{
	UINT16	CameraNo;
	UINT16	MonitorNo;
	UINT16	AlarmNo;
	UINT8	RequestCode;
	UINT8	Id[24];
	UINT8	Cstat;
}RequestUpdatesConf_t;

//d0
typedef struct tagCurAlarmNo_t
{
	UINT16	AlarmNo;
}CurAlarmNo_t;

typedef struct tagCurAlarmNoConf_t
{
	UINT8	IsOk;
}CurAlarmNoConf_t;

//d1
typedef struct tagTrippedAlarmNo_t
{
	UINT16	AlarmNo;
}TrippedAlarmNo_t;

typedef struct tagTrippedAlarmNoConf_t
{
	UINT8	IsOk;
}TrippedAlarmNoConf_t;

//d2
typedef struct tagPosnAttrInfo_t
{
	UINT8	Function;//01camera,03monitor,04alarm,08camera No,10time,20special msg update
}PosnAttrInfo_t;

typedef struct tagPosnAttrInfoConf_t
{
	UINT8	Function;
	UINT8	Xpos;
	UINT8	Ypos;
	UINT8	Attr0;//meaning in glossary
	UINT8	Attr1;
}PosnAttrInfoConf_t;

//d3
typedef struct tagEquipStatus_t
{
	UINT8	EquipmentOk;
}EquipStatus_t;

//d6
typedef struct tagSelectAlarm_t
{
	UINT16	AlarmNo;
}SelectAlarm_t;

typedef struct tagSelectAlarmConf_t
{
	UINT16	AlarmNo;
}SelectAlarmConf_t;

//d7
typedef struct tagArmAlarm_t
{
	UINT16	AlarmNo;
}ArmAlarm_t;

typedef struct tagArmAlarmConf_t
{
	UINT16	MonitorNo;
	UINT16 	CameraNo;
	UINT16	MacroNo;	//??
	UINT16	AuxStatNo;  //???
	UINT16	AlarmNo;
	UINT8	Fuction;
	UINT8	Statistics1;
	UINT8	Statistics2;
	//added 2000/10/30
	UINT8	Indev;//00 camera/01 vcr/02 multiplexer
	UINT16	Altbase;
	UINT8	Outdev;
	UINT8	EndFlag;
}ArmAlarmConf_t;

//d8
typedef struct tagDisarmAlarm_t
{
	UINT16	AlarmNo;
}DisarmAlarm_t;

typedef struct tagDisarmAlarmConf_t
{
	UINT16	MonitorNo;
	UINT16 	CameraNo;
	UINT16	MacroNo;
	UINT16	AuxStatNo;
	UINT16	AlarmNo;
	UINT8	Fuction;
	UINT8	Statistics1;
	UINT8	Statistics2;
	//added 2000/10/30
	UINT8	Indev;//00 camera/01 vcr/02 multiplexer
	UINT16	Altbase;
	UINT8	Outdev;
	UINT8	EndFlag;
}DisarmAlarmConf_t;

//d9
typedef struct tagStepAlarms_t
{
	UINT8	Function;//01forward,02backward
}StepAlarms_t;

typedef struct tagStepAlarmsConf_t
{
	UINT16 	AlarmNo;
}StepAlarmsConf_t;

//da
typedef struct tagGetLastAlarm_t
{
	UINT8	TestByte;
}GetLastAlarm_t;

typedef struct tagGetLastAlarmConf_t
{
	UINT16 	AlarmNo;
}GetLastAlarmConf_t;

//dc
typedef struct tagStepActiveAlarm_t
{
	UINT8	Function;//01forward,02backward
}StepActiveAlarm_t;

typedef struct tagStepActiveAlarmConf_t
{
	UINT16 	AlarmNo;
}StepActiveAlarmConf_t;

//dd
typedef struct tagResetAlarm_t
{
	UINT16 	AlarmNo;
}ResetAlarm_t;

typedef struct tagResetAlarmConf_t
{
	UINT16 	AlarmNo;
	UINT8	EndFlag;
}ResetAlarmConf_t;

//de
typedef struct tagAlarmInfo_t
{
	UINT8 	TestByte;
}AlarmInfo_t;

typedef struct tagAlarmInfoConf_t
{
	UINT8 	Function;
}AlarmInfoConf_t;


//df
typedef struct tagStartAlarm_t
{
	UINT8 	Function;//00to start,01 to stop
}StartAlarm_t;

typedef struct tagStartAlarmConf_t
{
	UINT16	MonitorNo;
	UINT16 	CameraNo;
	UINT16	MacroNo;
	UINT16	AuxStatNo;
	UINT8	Statistics1;
	UINT8	Statistics2;
	//added 2000/10/30
	UINT8	Indev;//00 camera/01 vcr/02 multiplexer
	UINT16	Altbase;
	UINT8	Outdev;
}StartAlarmConf_t;

//e1
typedef struct tagSendError_t
{
	UINT16 	ErrorNo;
}SendError_t;

typedef struct tagSendErrorConf_t
{
	UINT16 	ErrorNo;
}SendErrorConf_t;

//e2
typedef struct tagSendFatalErr_t
{
	UINT16 	ErrorNo;
}SendFatalErr_t;

typedef struct tagSendFatalErrConf_t
{
	UINT16 	ErrorNo;
}SendFatalErrConf_t;

//e2
/*typedef struct tagResetAlarms_t
{
	UINT8 	MonitorNo;
}ResetAlarms_t;

typedef struct tagResetAlarmsConf_t
{
	UINT8	MonitorNo;
	UINT8	Statistics1;
	UINT8	Statistics2;
	UINT8 	CameraHi;
	UINT8 	CameraLo;
	UINT8	MacroHi;
	UINT8	MacroLo;
	UINT8	AuxStatLo;
	UINT8	AuxStatHi;
	//added 2000/10/30
	UINT8	Indev;//00 camera/01 vcr/02 multiplexer
	UINT16	Altbase;
	UINT8	Outdev;
}ResetAlarmsConf_t;
*/

//e3
typedef struct tagMimicSwitch_t
{
	UINT8 	Data;// = 00
	UINT8	ButtonNo;	//pBCD
}MimicSwitch_t;

typedef struct tagMimicSwitchConf_t
{
	UINT8 	IsOk;
}MimicSwitchConf_t;

//e7
typedef struct tagGdiCtl_t
{
	UINT8 	GdiNo;  //pbcd
	UINT8 	GdiAux;
}GdiCtl_t;

typedef struct tagGdiCtlConf_t
{
	UINT8 	GdiNo;
	UINT8 	GdiBit;
	UINT8	Opr;
}GdiCtlConf_t;

//e8
typedef struct tagReleaseMacro_t
{
	UINT16 	MonitorNo;
}ReleaseMacro_t;

typedef struct tagReleaseMacroConf_t
{
	UINT16	MonitorNo;
	UINT16 	CameraNo;
	UINT16	MacroNo;
	UINT16	AuxStatNo;
	UINT8	Statistics1;
	UINT8	Statistics2;
	//added 2000/10/30
	UINT8	Indev;//00 camera/01 vcr/02 multiplexer
	UINT16	Altbase;
	UINT8	Outdev;
}ReleaseMacroConf_t;

//ea
typedef struct tagArmTable_t
{
	UINT8 	Unit;
	UINT8 	Arm;
	UINT8	EndFlag;
}ArmTable_t;

typedef struct tagArmTableConf_t
{
	UINT8 	IsOk;
}ArmTableConf_t;

//eb
typedef struct tagStepMacro_t
{
	UINT8 	Function;//01forward,02backward
}StepMacro_t;

typedef struct tagStepMacroConf_t
{
	UINT8 	Macro;
}StepMacroConf_t;

//ec
typedef struct tagSelectMacro_t
{
	UINT16 	MonitorNo;
	UINT8	Macro;
}SelectMacro_t;

typedef struct tagSelectMacroConf_t
{
	UINT16	MonitorNo;
	UINT16 	CameraNo;
	UINT16	MacroNo;	
	UINT16	AuxStatNo;  
	UINT8	Statistics1;
	UINT8	Statistics2;
	//added 2000/10/30
	UINT8	Indev;//00 camera/01 vcr/02 multiplexer
	UINT16	Altbase;
	UINT8	Outdev;
}SelectMacroConf_t;

//ed
typedef struct tagUpdateArmTable_t
{
	UINT8 	Unit;
}UpdateArmTable_t;

typedef struct tagUpdateArmTableConf_t
{
	UINT8	HaveAlarm;
	UINT8 	Unit;
	UINT8 	Arm;
	UINT8	EndFlag;
}UpdateArmTableConf_t;

//ee
/*typedef struct tagStepMacro_t
{
	UINT8 	Macro;
}StepMacro_t;
*/
//ef
typedef struct tagArmInput_t
{
	UINT16 	AlarmNo;
	UINT8	Function;	//00 to arm,02 to disarm
}ArmInput_t;

//f0
typedef struct tagGdiReport_t
{
	UINT8 	Operation;
}GdiReport_t;

//f1
typedef struct tagMacroReport_t
{
	UINT8 	Operator;
}MacroReport_t;

//f2
typedef struct tagLastVideoLoss_t
{
	UINT8 	TestByte;//no use 
}LastVideoLoss_t;

typedef struct tagLastVideoLossConf_t
{
	UINT16 	CameraNo;
}LastVideoLossConf_t;

//f4
typedef struct tagCheckPin_t
{
	UINT8 	Data; // = 01
	UINT8	PinHi;//pbcd
	UINT8	PinLo;
}CheckPin_t;

typedef struct tagCheckPinConf_t
{
	UINT8	EquipmentOk;
}CheckPinConf_t;

//f7
typedef struct tagRecieveAlarm_t
{
	UINT16	AlarmNo;
}RecieveAlarm_t;

typedef struct tagRecieveAlarmConf_t
{
	UINT16 	AlarmNo;
	UINT8	Function;	//00 to arm,02 to disarm
}RecieveAlarmConf_t;


//global data structures
typedef struct tagMonitor_t
{
	UINT16 CameraNo;//关联的CAMERA号
	UINT8 Status;
	UINT8 NoReplyCount;//未应答次数
	UINT8 ExcptCount;
	UINT8 NowOpr;//操作者工号
}Monitor_t;

typedef struct tagCamera_t
{
	UINT16 MonitorNo;
	UINT8 Status;
	UINT8 NoReplyCount;
	UINT8 ExcptCount;
	UINT8 NowOpr;
}Camera_t;

typedef struct tagOperator_t
{
	UINT16 MonitorNo;//正在操作的MONITOR
	UINT16 CameraNo;//正在操作的CameraNo
	UINT8 Priority;//
	UINT8 CommIndex;//通讯用
	UINT8 Status;
	UINT8 NoReplyCount;
	UINT8 OprName[MAX_NAME_LEN];
/*	UINT8 PassWord[MAX_NAME_LEN];*/
	UINT8 mPermit[MAX_MONITOR_NUM];
	UINT8 cPermit[MAX_CAMERA_NUM];
	UINT8 CommandNo;
	UINT8 ReplyHead;
	UINT8 ErrorHead;
	UINT8 TimeLeft;
}Operator_t;

//added 2000,12,5
typedef struct tagCamOper_t
{
	UINT8	Used;
//	UINT16	CameraNo;	 
	UINT8	OprIndex;
	UINT8	Priority;
}CamOper_t;

typedef struct tagMonOper_t
{
	UINT8	Used;
//	UINT16	MonitorNo;	 
	UINT8	OprIndex;
	UINT8	Priority;
}MonOper_t;

/*typedef struct tagHistory_t
{
	UINT8	OperIndex;
	UINT16	MonitorNo;	 
	UINT16	CameraNo;	 
	UINT8	Priority;
}History_t;
*/

typedef struct tagForcedRelease_t
{
	UINT8	DevType;//1=camera, 2=monitor
	UINT16	Number;
}ForcedRelease_t;

typedef struct tagForcedReleaseConf_t
{
	UINT8 Result;
}ForcedReleaseConf_t;

//added end

typedef struct tagWaitConf_t
{
	UINT8 NowOpr;
	UINT8 NextWait;
	UINT8 EndFlag;
	UINT8 MaxMsg;
}WaitConf_t;

typedef struct tagErrorConf_t
{
	UINT8 NowOpr;
	UINT8 NextError;
	UINT8 EndFlag;
	UINT8 MaxMsg;
}ErrorConf_t;

typedef struct tagResTable_t
{
	UINT8 WaitQue[MAX_ECHO_NUM];
	UINT8 WaitNum;
	UINT8 ErrorQue[MAX_ECHO_NUM];
	UINT8 ErrorNum;
}ResTable_t;

typedef struct tagDevAckConf_t
{
	UINT8 Reserved;

}DevAckConf_t;


/*typedef struct tagSvrNotify_t
{
	UINT8 Cause;

}SvrNotify_t;
*/
typedef struct tagClientRequest_t
{
	union
	{
		OprLogin_t OprLoginEvent;
//		CameraCtlMsg_t CameraCtlEvent;
		SendCameraCtl_t SendCameraCtlEvent; //b8
		VariSpeedCtl_t VariSpeedCtlEvent; //c0
		SendCurMonitorAssign_t SendCurMonitorAssignEvent;//b1
		Switch2Monitor_t Switch2MonitorEvent;//b2
		VideoLossMask_t VideoLossMaskEvent;//b3
		StartMacro_t StartMacroEvent; //b4
		StopMacro_t StopMacroEvent; //b5
		NextCamera_t NextCameraEvent;//b6
		PreCamera_t PreCameraEvent;//b7
		CameraRelease_t CameraReleaseEvent;//b9
		MonitorStatus_t MonitorStatusEvent;//ba
		CameraLock_t CameraLockEvent;//bb
		CameraClear_t CameraClearEvent;//bc
		CameraCtlOverride_t CameraCtlOverrideEvent;//be
		CheckCamera_t CheckCameraEvent;//bf
		VideoLoss_t VideoLossEvent;//c4
		VideoLossDetect_t VideoLossDetectEvent;//c5
		IODev_t	IODev;//added 2000/10/30       //c9
		TestPort_t TestPortEvent;//ca
		Camera2MonitorInfo_t Camera2MonitorInfoEvent;//cc
		DataTimeInfo_t DataTimeInfoEvent;//cd
		Camera2Monitor_t Camera2MonitorEvent;//ce
		RequestUpdates_t RequestUpdatesEvent;//cf
		CurAlarmNo_t CurAlarmNoEvent;//d0
		TrippedAlarmNo_t TrippedAlarmNoEvent;//d1	
		PosnAttrInfo_t PosnAttrInfoEvent;//d2	
		EquipStatus_t EquipStatusEvent;//d3	
		SelectAlarm_t SelectAlarmEvent;//d6
		ArmAlarm_t ArmAlarmEvent;//d7
		DisarmAlarm_t DisarmAlarmEvent;//d8
		StepAlarms_t StepAlarmsEvent;//d9
		GetLastAlarm_t GetLastAlarmEvent;//da
		StepActiveAlarm_t StepActiveAlarmEvent;//dc
		ResetAlarm_t ResetAlarmEvent;//dd
		AlarmInfo_t AlarmInfoEvent;//de
		StartAlarm_t StartAlarmEvent;//df
		SendError_t SendErrorEvent;//e1
		SendFatalErr_t SendFatalErrEvent;//e2
//		ResetAlarms_t ResetAlarmsEvent; //e2
		MimicSwitch_t MimicSwitchEvent;//e3
		GdiCtl_t GdiCtlEvent;//e7
		ReleaseMacro_t ReleaseMacroEvent;//e8
		ArmTable_t ArmTableEvent;//ea
		StepMacro_t StepMacroEvent;//eb
		SelectMacro_t SelectMacroEvent;//ec
		UpdateArmTable_t UpdateArmTableEvent;//ed
//		StepMacro_t StepMacroEvent;//ee
		ArmInput_t ArmInputEvent;//ef
		GdiReport_t GdiReportEvent;//f0
		MacroReport_t MacroReportEvent;//f1
		LastVideoLoss_t LastVideoLossEvent;//f2
		CheckPin_t CheckPinEvent;//f4
		RecieveAlarm_t RecieveAlarmEvent;//f7
//added 2000,12,05
		ForcedRelease_t ForceReleaseEvent; //2
//added end
	}u;
}ClientRequest_t;

typedef struct tagConfirmation_t
{
	union
	{
		DevAckConf_t DevAckConfEvent;
		OprLoginConf_t OprLoginConfEvent;
//		CameraCtlMsgConf_t CameraCtlconfEvent;
		SendCameraCtlConf_t SendCameraCtlConfEvent;//b8
		VariSpeedCtlConf_t VariSpeedCtlConfEvent; //c0
		SendCurMonitorAssignConf_t SendCurMonitorAssignConfEvent;//b1
		Switch2MonitorConf_t Switch2MonitorConfEvent;//b2
		VideoLossMaskConf_t VideoLossMaskConfEvent;//b3
		StartMacroConf_t StartMacroConfEvent;//b4
		StopMacroConf_t StopMacroConfEvent;//b5
		NextCameraConf_t NextCameraConfEvent;//b6
		PreCameraConf_t PreCameraConfEvent;//b7
		CameraReleaseConf_t CameraReleaseConfEvent;//b9
		MonitorStatusConf_t MonitorStatusConfEvent;//ba
		CameraLockConf_t CameraLockConfEvent;//bb
		CameraClearConf_t CameraClearConfEvent;//bc
		CameraCtlOverrideConf_t CameraCtlOverrideConfEvent;//be
		CheckCameraConf_t CheckCameraConfEvent;//bf
		VideoLossConf_t  VideoLossConfEvent;//c4
		VideoLossDetectConf_t VideoLossDetectConfEvent;//c5
		IODevConf_t IODevConf;//added 2000/10/30      //c9
		TestPortConf_t TestPortConfEvent;//ca
		Camera2MonitorInfoConf_t Camera2MonitorInfoConfEvent;//cc
		DataTimeInfoConf_t DataTimeInfoConfEvent;//cd
		Camera2MonitorConf_t Camera2MonitorConfEvent;//ce
		RequestUpdatesConf_t RequestUpdatesConfEvent;//cf
		CurAlarmNoConf_t CurAlarmNoConfEvent;//d0
		TrippedAlarmNoConf_t TrippedAlarmNoConfEvent;//d1
		PosnAttrInfoConf_t PosnAttrInfoConfEvent;//d2	
		SelectAlarmConf_t SelectAlarmConfEvent;//d6	
		ArmAlarmConf_t ArmAlarmConfEvent;//d7	
		DisarmAlarmConf_t DisarmAlarmConfEvent;//d8
		StepAlarmsConf_t StepAlarmsConfEvent;//d9
		GetLastAlarmConf_t GetLastAlarmConfEvent;//da
		StepActiveAlarmConf_t StepActiveAlarmConfEvent;//dc
		ResetAlarmConf_t ResetAlarmConfEvent;//dd
		AlarmInfoConf_t AlarmInfoConfEvent;//de
		StartAlarmConf_t StartAlarmConfEvent;//df
		SendErrorConf_t SendErrorConfEvent;//e1
		SendFatalErrConf_t SendFatalErrConfEvent;//e2
		MimicSwitchConf_t MimicSwitchConfEvent;//e3
//		ResetAlarmsConf_t ResetAlarmsConfEvent; //e2
		GdiCtlConf_t GdiCtlConfEvent;//e7
		ReleaseMacroConf_t ReleaseMacroConfEvent;//e8
		ArmTableConf_t ArmTableConfEvent;//ea
		StepMacroConf_t StepMacroConfEvent;//eb
		SelectMacroConf_t SelectMacroConfEvent;//ec
		UpdateArmTableConf_t UpdateArmTableConfEvent;//ed
		LastVideoLossConf_t LastVideoLossConfEvent;//f2
		CheckPinConf_t CheckPinConfEvent;//f4
		RecieveAlarmConf_t RecieveAlarmConfEvent;//f7
//		SvrNotify_t SvrNotifyEvent;
//added 2000,12,05
		ForcedReleaseConf_t ForcedReleaseConfEvent;//2
//added end
	}u;

}Confirmation_t;

typedef struct tagMonitorEvent_t
{
	CommHeader_t CommHeader;
	union
	{
		ClientRequest_t RequestEvent;
		Confirmation_t  ConfEvent;
		ErrorMsg_t      ErrorEvent;
	}u;

}MonitorEvent_t;

#pragma pack(pop)
#endif

