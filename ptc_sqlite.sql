--sqlite 不需要用户密码
/*加密 存储的内容，可以使用sqlcipher*/

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
	[Number] INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,   --自动增长的主键必须为INTEGER PRIMARY KEY AUTOINCREMENT
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

/*   必须写成 insert into的形式，结尾加分号（;）   */
INSERT INTO bustypes(BusTypeID,BusTypeName,Power,BusLoad) values(1, '奔驰',200,100);
INSERT INTO bustypes(BusTypeID,BusTypeName,Power,BusLoad) values(2, '奥迪',180,80);
INSERT INTO bustypes(BusTypeID,BusTypeName,Power,BusLoad) values(3,'大宇', 160,60);
INSERT INTO bustypes(BusTypeID,BusTypeName,Power,BusLoad) values(4,'大众 ',130,50);
INSERT INTO bustypes(BusTypeID,BusTypeName,Power,BusLoad) values(5,'丰田 ',120,50);

INSERT INTO buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(1,'苏A00001',1,1,0.9,100);
INSERT INTO buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(2,'苏A00002',1,1,0.9,100);
INSERT INTO buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(3,'苏A00003',1,2,0.8,90);
INSERT INTO buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(4,'苏A00004',1,4,0.7,110);
INSERT INTO buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(5,'苏A00005',1,3,1,80);
INSERT INTO buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(6,'苏A00006',2,1,0.9,111);
INSERT INTO buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(7,'苏A00007',2,5,0.9,120);
INSERT INTO buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(8,'苏A00008',2,2,0.5,122);
INSERT INTO buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(9,'苏A00009',2,3,0.9,79);
INSERT INTO buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(10,'苏A00010',3,1,0.9,111);
INSERT INTO buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(11,'苏A00011',3,2,0.8,100);
INSERT INTO buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(12,'苏A00012',3,5,0.7,90);
INSERT INTO buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(13,'苏A00013',3,4,0.6,130);


INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(1,'大厂',0,0.2,5);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(2,'杨庄',0.1,0.2,3);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(3,'丁解',0.2,0.2,2);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(4,'龙山',0.3,0.2,4);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(5,'高新区',0.4,0.2,2);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(6,'桃园',0.5,0.2,3);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(7,'泰山新村',0.6,0.2,5);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(8,'盘城',0.1,0,3);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(9,'杨庄西',0.1,0.1,4);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(10,'杨庄东',0.1,0.3,3);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(11,'南钢',0.1,0.4,5);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(12,'高新二路',0.4,0,2);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(13,'高新一路',0.4,0.1,3);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(14,'沿江',0.4,0.3,4);
INSERT INTO stations(StationID,StationName,longitude,latitude,BusPits) values(15,'沿江东',0.4,0.4,3);

INSERT INTO buslines(BusLineID,BusLineName,Reserved) values(1,'1路',NULL);
INSERT INTO buslines(BusLineID,BusLineName,Reserved) values(2,'2路',NULL);
INSERT INTO buslines(BusLineID,BusLineName,Reserved) values(3,'3路',NULL);

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

