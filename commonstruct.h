#ifndef COMMONSTRUCT_H
#define COMMONSTRUCT_H

#define MsgFlag 0xAA

//#ifndef __TEST
//#pragma option -b
//#endif
//#pragma pack(push,1)
//ͨѶ�ͻ�����
typedef enum {
	CT_STATION = 0,				//վ̨
	CT_BUS,						//����
	CT_COMMUTER,				//����
	CT_OPERATOR					//����Ա
}CustType;	//ͨѶ�ͻ�����
//��Ϣ��ʽ����
typedef enum {
	CM_TEST = 1,				//�����ź�
	CM_STATIONSTATUSREQ,		//����վ̨״̬��Ϣ
	CM_STATIONSTATUS,			//վ̨״̬��Ϣ
	CM_BUSSTATUSREQ,			//���󹫽���״̬��Ϣ
	CM_BUSSTATUS,				//������״̬��Ϣ
	CM_STATIONINITREQ,			//վ̨��ʼ������
	CM_STATIONINIT,				//վ̨��ʼ����Ϣ
	CM_BUSINITREQ,				//��������ʼ������
	CM_BUSINIT,					//��������ʼ����Ϣ
	CM_BUSCOMING,				//��������վͨ��
	CM_BUSCOMINGACK,			//��������վͨ��Ӧ��
	CM_BUSSTOPPING,			//���������ﳵλͨ��
	CM_BUSSTOPPINGACK,		//���������ﳵλͨ��Ӧ��
	CM_BUSLEAVING,				//��������վͨ��
	CM_BUSLEAVINGACK,			//��������վͨ��Ӧ��
	CM_BUSRUNNING,				//������վ��ͨ��
	CM_BUSRUNNINGACK,			//������վ��ͨ��Ӧ��
	CM_LOGON,					//��¼
	CM_LOGONACK,				//��¼Ӧ��
	CM_LOGOUT,					//�ǳ�
	CM_LOGOUTACK,				//�ǳ�Ӧ��
	CM_CHANGEPASSWORD,			//�޸ĵ�¼����
	CM_CHANGEPASSWORDACK,		//�޸ĵ�¼����Ӧ��
	CM_ALARMMSG,				//�쳣������Ϣ
	CM_SYSTIMEREQ,				//����ϵͳʱ��
	CM_SYSTIME,					//ϵͳʱ����Ϣ
	CM_BUSLINEREQ,			    //���󹫽���·
	CM_BUSLINE,					//������·��Ϣ
	CM_UNKNOWN
}MsgType;         //��Ϣ����
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
	BYTE Flag1;	    //��Ϣ֡��־ == MsgFlag
    BYTE Flag2;	    //55
	BYTE Command;	//�����
	BYTE Len;		//��Ϣ����
}CMsgFrame;	//��Ϣͷ��ʽ

//#pragma pack(pop)
#endif

