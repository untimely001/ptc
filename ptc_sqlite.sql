--sqlite ����Ҫ�û�����
/*���� �洢�����ݣ�����ʹ��sqlcipher*/

CREATE TABLE [Buses](
	[BusID] [int] NOT NULL,
	[NumberPlate] [nchar](10) NULL,
	[BusLineID] [int] NULL,
	[BusType] [tinyint] NULL,
	[DepRate] [real] NULL,
	[Price] [real] NULL,
        CONSTRAINT [PK_Buses] PRIMARY KEY 
	(
		[BusID] ASC
	)
);

CREATE TABLE [BusTypes](
	[BusTypeID] [int] NOT NULL,
	[BusTypeName] [char](20) NULL,
	[Power] [real] NULL,
	[BusLoad] [smallint] NULL,
        CONSTRAINT [PK_BusTypes] PRIMARY KEY  
        (
		[BusTypeID] ASC
        ) 
); 

CREATE TABLE [Users](
	[UserID] [int] NOT NULL,
	[UserType] [tinyint] NULL,
	[EquipID] [smallint] NULL,
	[UserName] [char](20) NULL,
	[Password] [char](20) NULL,
	[Status] [tinyint] NULL,
        CONSTRAINT [PK_Users] PRIMARY KEY  
        (
		[UserID] ASC
        ) 
); 

CREATE TABLE [StationStatus](
	[Number] INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,   --�Զ���������������ΪINTEGER PRIMARY KEY AUTOINCREMENT
	[StationID] [int] NULL,
	[BusPits] [tinyint] NULL,
	[BusWaiting] [tinyint] NULL,
	[BusID1] [smallint] NULL,
	[BusID2] [smallint] NULL,
	[BusID3] [smallint] NULL,
	[BusID4] [smallint] NULL,
	[BusID5] [smallint] NULL,
	[UTC] [char](12) NULL
); 

CREATE TABLE [Alarms](
	[AlarmSeq] INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	[UserID] [int] NULL,
	[UserType] [tinyint] NULL,
	[EquipID] [smallint] NULL,
	[Status] [int] NULL,
	[UTC] [char](12) NULL,
	[Reserved] [int] NULL
); 


CREATE TABLE [BusInservices](
	[Seq] INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	[BusID] [int] NULL,
	[Longitude] [float] NULL,
	[Latitude] [float] NULL,
	[Direction] [tinyint] NULL,
	[Velocity] [real] NULL,
	[Passengers] [smallint] NULL,
	[UTC] [char](12) NULL,
	[RecordType] [tinyint] NULL
); 


CREATE TABLE [BusLines](
	[BusLineID] [int] NOT NULL,
	[BusLineName] [char](30) NULL,
	[Reserved] [int] NULL,
        CONSTRAINT [PK_BusLines] PRIMARY KEY  
        (
		[BusLineID] ASC
        ) 
); 


CREATE TABLE [BusLineStations](
	[BusLineID] [int] NOT NULL,
	[StopSeq] [tinyint] NOT NULL,
	[StationID] [int] NULL,
        CONSTRAINT [PK_BusLineStations] PRIMARY KEY  
        (
		[BusLineID] ASC,
		[StopSeq] ASC
        ) 
); 


CREATE TABLE [Stations](
	[StationID] [int] NOT NULL,
	[StationName] [char](40) NULL,
	[longitude] [float] NULL,
	[latitude] [float] NULL,
	[BusPits] [tinyint] NULL,
        CONSTRAINT [PK_Stations] PRIMARY KEY  
        (
		[StationID] ASC
        ) 
); 

CREATE TABLE [Notification](
	[Id] INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,   
	[DevId] [int] NULL,
	[Cmdno] [int] NULL,
	[Params] [nchar](120) NULL,
	[Type] [int] NULL,
	[Status] [int] NULL,
	[Time] [nchar](12) NULL
); 

/*   ����д�� insert into����ʽ����β�ӷֺţ�;��   */
INSERT INTO bustypes(BusTypeID,BusTypeName,Power,BusLoad) values(1, '����',200,100);
INSERT INTO bustypes(BusTypeID,BusTypeName,Power,BusLoad) values(2, '�µ�',180,80);
INSERT INTO bustypes(BusTypeID,BusTypeName,Power,BusLoad) values(3,'����', 160,60);
INSERT INTO bustypes(BusTypeID,BusTypeName,Power,BusLoad) values(4,'���� ',130,50);
INSERT INTO bustypes(BusTypeID,BusTypeName,Power,BusLoad) values(5,'���� ',120,50);

INSERT INTO buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(1,'��A00001',1,1,0.9,100);
INSERT INTO buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(2,'��A00002',1,1,0.9,100);
INSERT INTO buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(3,'��A00003',1,2,0.8,90);
INSERT INTO buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(4,'��A00004',1,4,0.7,110);
INSERT INTO buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(5,'��A00005',1,3,1,80);
INSERT INTO buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(6,'��A00006',2,1,0.9,111);
INSERT INTO buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(7,'��A00007',2,5,0.9,120);
INSERT INTO buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(8,'��A00008',2,2,0.5,122);
INSERT INTO buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(9,'��A00009',2,3,0.9,79);
INSERT INTO buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(10,'��A00010',3,1,0.9,111);
INSERT INTO buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(11,'��A00011',3,2,0.8,100);
INSERT INTO buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(12,'��A00012',3,5,0.7,90);
INSERT INTO buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(13,'��A00013',3,4,0.6,130);


INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(1,'��',0,0.2,5);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(2,'��ׯ',0.1,0.2,3);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(3,'����',0.2,0.2,2);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(4,'��ɽ',0.3,0.2,4);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(5,'������',0.4,0.2,2);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(6,'��԰',0.5,0.2,3);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(7,'̩ɽ�´�',0.6,0.2,5);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(8,'�̳�',0.1,0,3);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(9,'��ׯ��',0.1,0.1,4);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(10,'��ׯ��',0.1,0.3,3);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(11,'�ϸ�',0.1,0.4,5);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(12,'���¶�·',0.4,0,2);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(13,'����һ·',0.4,0.1,3);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(14,'�ؽ�',0.4,0.3,4);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(15,'�ؽ���',0.4,0.4,3);

INSERT INTO buslines(BusLineID,BusLineName,Reserved) values(1,'1·',NULL);
INSERT INTO buslines(BusLineID,BusLineName,Reserved) values(2,'2·',NULL);
INSERT INTO buslines(BusLineID,BusLineName,Reserved) values(3,'3·',NULL);

INSERT INTO buslinestations(BusLineID,StopSeq,StationID) values(1,1,1);
INSERT INTO buslinestations(BusLineID,StopSeq,StationID) values(1,2,2);
INSERT INTO buslinestations(BusLineID,StopSeq,StationID) values(1,3,3);
INSERT INTO buslinestations(BusLineID,StopSeq,StationID) values(1,4,4);
INSERT INTO buslinestations(BusLineID,StopSeq,StationID) values(1,5,5);
INSERT INTO buslinestations(BusLineID,StopSeq,StationID) values(1,6,6);
INSERT INTO buslinestations(BusLineID,StopSeq,StationID) values(1,7,7);
INSERT INTO buslinestations(BusLineID,StopSeq,StationID) values(2,1,8);
INSERT INTO buslinestations(BusLineID,StopSeq,StationID) values(2,2,9);
INSERT INTO buslinestations(BusLineID,StopSeq,StationID) values(2,3,2);
INSERT INTO buslinestations(BusLineID,StopSeq,StationID) values(2,4,10);
INSERT INTO buslinestations(BusLineID,StopSeq,StationID) values(2,5,11);
INSERT INTO buslinestations(BusLineID,StopSeq,StationID) values(3,1,12);
INSERT INTO buslinestations(BusLineID,StopSeq,StationID) values(3,2,13);
INSERT INTO buslinestations(BusLineID,StopSeq,StationID) values(3,3,5);
INSERT INTO buslinestations(BusLineID,StopSeq,StationID) values(3,4,14);
INSERT INTO buslinestations(BusLineID,StopSeq,StationID) values(3,5,15);


INSERT INTO users(UserID,UserType,EquipID,UserName,Password) values(1,1,1,'yangzhuang','yangzhuang');
INSERT INTO users(UserID,UserType,EquipID,UserName,Password) values(2,1,2,'yanjiang','yanjiang');
INSERT INTO users(UserID,UserType,EquipID,UserName,Password) values(3,2,3,'daben','daben');
INSERT INTO users(UserID,UserType,EquipID,UserName,Password) values(4,2,4,'dayu','dayu');
INSERT INTO users(UserID,UserType,EquipID,UserName,Password) values(5,3,5,'xiaoli','xiaoli');
INSERT INTO users(UserID,UserType,EquipID,UserName,Password) values(6,3,6,'xiaoliu','xiaoliu');

