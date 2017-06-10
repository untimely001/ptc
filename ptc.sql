IF NOT EXISTS (SELECT * FROM sys.database_principals WHERE name = N'ptcuser000')
CREATE USER [ptcuser000] FOR LOGIN [ptcuser000] WITH DEFAULT_SCHEMA=[dbo]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Buses]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[Buses](
	[BusID] [int] NOT NULL,
	[NumberPlate] [nchar](10) NULL,
	[BusLineID] [int] NULL,
	[BusType] [tinyint] NULL,
	[DepRate] [real] NULL,
	[Price] [real] NULL,
 CONSTRAINT [PK_Buses] PRIMARY KEY CLUSTERED 
(
	[BusID] ASC
)WITH (IGNORE_DUP_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
END
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[BusTypes]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[BusTypes](
	[BusTypeID] [int] NOT NULL,
	[BusTypeName] [char](20) NULL,
	[Power] [real] NULL,
	[BusLoad] [smallint] NULL,
 CONSTRAINT [PK_BusTypes] PRIMARY KEY CLUSTERED 
(
	[BusTypeID] ASC
)WITH (IGNORE_DUP_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
END
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Users]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[Users](
	[UserID] [int] NOT NULL,
	[UserType] [tinyint] NULL,
	[EquipID] [smallint] NULL,
	[UserName] [char](20) NULL,
	[Password] [char](20) NULL,
	[Status] [tinyint] NULL,
 CONSTRAINT [PK_Users] PRIMARY KEY CLUSTERED 
(
	[UserID] ASC
)WITH (IGNORE_DUP_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
END
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[StationStatus]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[StationStatus](
	[Number] [int] IDENTITY(1,1) NOT NULL,
	[StationID] [int] NULL,
	[BusPits] [tinyint] NULL,
	[BusWaiting] [tinyint] NULL,
	[BusID1] [smallint] NULL,
	[BusID2] [smallint] NULL,
	[BusID3] [smallint] NULL,
	[BusID4] [smallint] NULL,
	[BusID5] [smallint] NULL,
	[UTC] [char](12) NULL,
 CONSTRAINT [PK_StationStatus] PRIMARY KEY CLUSTERED 
(
	[Number] ASC
)WITH (IGNORE_DUP_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
END
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Alarms]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[Alarms](
	[AlarmSeq] [int] IDENTITY(1,1) NOT NULL,
	[UserID] [int] NULL,
	[UserType] [tinyint] NULL,
	[EquipID] [smallint] NULL,
	[Status] [int] NULL,
	[UTC] [char](12) NULL,
	[Reserved] [int] NULL,
 CONSTRAINT [PK_Alarms_1] PRIMARY KEY CLUSTERED 
(
	[AlarmSeq] ASC
)WITH (IGNORE_DUP_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
END
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[BusInservices]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[BusInservices](
	[Seq] [int] IDENTITY(1,1) NOT NULL,
	[BusID] [int] NULL,
	[Longitude] [float] NULL,
	[Latitude] [float] NULL,
	[Direction] [tinyint] NULL,
	[Velocity] [real] NULL,
	[Passengers] [smallint] NULL,
	[UTC] [char](12) NULL,
	[RecordType] [tinyint] NULL,
 CONSTRAINT [PK_BusInservices_1] PRIMARY KEY CLUSTERED 
(
	[Seq] ASC
)WITH (IGNORE_DUP_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
END
GO

SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[BusLines]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[BusLines](
	[BusLineID] [int] NOT NULL,
	[BusLineName] [char](30) NULL,
	[Reserved] [int] NULL,
 CONSTRAINT [PK_BusLines] PRIMARY KEY CLUSTERED 
(
	[BusLineID] ASC
)WITH (IGNORE_DUP_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
END
GO

SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[BusLineStations]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[BusLineStations](
	[BusLineID] [int] NOT NULL,
	[StopSeq] [tinyint] NOT NULL,
	[StationID] [int] NULL,
 CONSTRAINT [PK_BusLineStations] PRIMARY KEY CLUSTERED 
(
	[BusLineID] ASC,
	[StopSeq] ASC
)WITH (IGNORE_DUP_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
END
GO

IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[BusLineStations]') AND name = N'IX_BusLineStations')
CREATE NONCLUSTERED INDEX [IX_BusLineStations] ON [dbo].[BusLineStations] 
(
	[BusLineID] ASC
)WITH (IGNORE_DUP_KEY = OFF) ON [PRIMARY]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Stations]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[Stations](
	[StationID] [int] NOT NULL,
	[StationName] [char](40) NULL,
	[longitude] [float] NULL,
	[latitude] [float] NULL,
	[BusPits] [tinyint] NULL,
 CONSTRAINT [PK_Stations] PRIMARY KEY CLUSTERED 
(
	[StationID] ASC
)WITH (IGNORE_DUP_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
END
GO

SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Notification](
	[Id] [int] IDENTITY(1,1) NOT NULL,
	[DevId] [int] NULL,
	[Cmdno] [int] NULL,
	[Params] [nchar](120) COLLATE Chinese_PRC_CI_AS NULL,
	[Type] [int] NULL,
	[Status] [int] NULL,
	[Time] [nchar](12) COLLATE Chinese_PRC_CI_AS NULL
) ON [PRIMARY]
GO


/* SQL Server中,表中的数据导出为insert语句 的存储过程
   使用方法：   exec UspOutputData 表名   */
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[UspOutputData]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[UspOutputData]

@tablename sysname

AS

declare @column varchar(1000)

declare @columndata varchar(1000)

declare @sql varchar(4000)

declare @xtype tinyint

declare @name sysname

declare @objectId int

declare @objectname sysname

declare @ident int

set nocount on

set @objectId=object_id(@tablename)

if @objectId is null -- ????????

begin

print ''The object not exists''

return

end

set @objectname=rtrim(object_name(@objectId))

if @objectname is null or charindex(@objectname,@tablename)=0 --??????

begin

print ''object not in current database''

return

end

if OBJECTPROPERTY(@objectId,''IsTable'') < > 1 -- ???????table

begin

print ''The object is not table''

return

end

select @ident=status&0x80 from syscolumns where id=@objectid and status&0x80=0x80

if @ident is not null

print ''SET IDENTITY_INSERT ''+@TableName+'' ON''

declare syscolumns_cursor cursor

for select c.name,c.xtype from syscolumns c where c.id=@objectid order by c.colid

open syscolumns_cursor

set @column=''''

set @columndata=''''

fetch next from syscolumns_cursor into @name,@xtype

while @@fetch_status < >-1

begin

if @@fetch_status < >-2

begin

if @xtype not in(189,34,35,99,98) --timestamp?????image,text,ntext,sql_variant ?????

begin

set @column=@column+case when len(@column)=0 then'''' else '',''end+@name

set @columndata=@columndata+case when len(@columndata)=0 then '''' else '','''','''',''

end

+case when @xtype in(167,175) then ''''''''''''''''''+''+@name+''+'''''''''''''''''' --varchar,char

when @xtype in(231,239) then ''''''N''''''''''''+''+@name+''+'''''''''''''''''' --nvarchar,nchar

when @xtype=61 then ''''''''''''''''''+convert(char(23),''+@name+'',121)+'''''''''''''''''' --datetime

when @xtype=58 then ''''''''''''''''''+convert(char(16),''+@name+'',120)+'''''''''''''''''' --smalldatetime

when @xtype=36 then ''''''''''''''''''+convert(char(36),''+@name+'')+'''''''''''''''''' --uniqueidentifier

else @name end

end

end

fetch next from syscolumns_cursor into @name,@xtype

end

close syscolumns_cursor

deallocate syscolumns_cursor

set @sql=''set nocount on select ''''insert ''+@tablename+''(''+@column+'') values(''''as ''''--'''',''+@columndata+'','''')'''' from ''+@tablename

print ''--''+@sql

exec(@sql)

if @ident is not null

print ''SET IDENTITY_INSERT ''+@TableName+'' OFF''

' 
END
GO


insert bustypes(BusTypeID,BusTypeName,Power,BusLoad) values(1, '奔驰',200,100)
insert bustypes(BusTypeID,BusTypeName,Power,BusLoad) values(2, '奥迪',180,80)
insert bustypes(BusTypeID,BusTypeName,Power,BusLoad) values(3,'大宇', 160,60)
insert bustypes(BusTypeID,BusTypeName,Power,BusLoad) values(4,'大众 ',130,50)
insert bustypes(BusTypeID,BusTypeName,Power,BusLoad) values(5,'丰田 ',120,50)

insert buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(	1,N'苏A00001',1,1,0.9,100)
insert buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(	2,N'苏A00002',1,1,0.9,100)
insert buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(	3,N'苏A00003',1,2,0.8,90)
insert buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(	4,N'苏A00004',1,4,0.7,110)
insert buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(	5,N'苏A00005',1,3,1,80	)
insert buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(	6,N'苏A00006',2,1,0.9,111)
insert buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(	7,N'苏A00007',2,5,0.9,120)
insert buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(	8,N'苏A00008',2,2,0.5,122)
insert buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(	9,N'苏A00009',2,3,0.9,79)
insert buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(	10,N'苏A00010',3,1,0.9,111)
insert buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(	11,N'苏A00011',3,2,0.8,100)
insert buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(	12,N'苏A00012',3,5,0.7,90)
insert buses(BusID,NumberPlate,BusLineID,BusType,DepRate,Price) values(	13,N'苏A00013',3,4,0.6,130)


insert stations(StationID,StationName,longitude,latitude,BusPits) values(1,'大厂',0,0.2,5)
insert stations(StationID,StationName,longitude,latitude,BusPits) values(2,'杨庄',0.1,0.2,3)
insert stations(StationID,StationName,longitude,latitude,BusPits) values(3,'丁解',0.2,0.2,2)
insert stations(StationID,StationName,longitude,latitude,BusPits) values(4,'龙山',0.3,0.2,4)
insert stations(StationID,StationName,longitude,latitude,BusPits) values(5,'高新区',0.4,0.2,2)
insert stations(StationID,StationName,longitude,latitude,BusPits) values(6,'桃园',0.5,0.2,3)
insert stations(StationID,StationName,longitude,latitude,BusPits) values(7,'泰山新村',0.6,0.2,5)
insert stations(StationID,StationName,longitude,latitude,BusPits) values(8,'盘城',0.1,0	,3)
insert stations(StationID,StationName,longitude,latitude,BusPits) values(9,'杨庄西',0.1,0.1,4)
insert stations(StationID,StationName,longitude,latitude,BusPits) values(10,'杨庄东',0.1,0.3,3)
insert stations(StationID,StationName,longitude,latitude,BusPits) values(11,'南钢',0.1,0.4,5)
insert stations(StationID,StationName,longitude,latitude,BusPits) values(12,'高新二路',0.4,0,2)
insert stations(StationID,StationName,longitude,latitude,BusPits) values(13,'高新一路',0.4,0.1,3)
insert stations(StationID,StationName,longitude,latitude,BusPits) values(14,'沿江',0.4,0.3,4)
insert stations(StationID,StationName,longitude,latitude,BusPits) values(15,'沿江东',0.4,0.4,3)

insert buslines(BusLineID,BusLineName,Reserved) values(	1,'1路',NULL)
insert buslines(BusLineID,BusLineName,Reserved) values(	2,'2路',NULL)
insert buslines(BusLineID,BusLineName,Reserved) values(	3,'3路',NULL)

insert buslinestations(BusLineID,StopSeq,StationID) values(1,1,1)
insert buslinestations(BusLineID,StopSeq,StationID) values(1,2,2)
insert buslinestations(BusLineID,StopSeq,StationID) values(1,3,3)
insert buslinestations(BusLineID,StopSeq,StationID) values(1,4,4)
insert buslinestations(BusLineID,StopSeq,StationID) values(1,5,5)
insert buslinestations(BusLineID,StopSeq,StationID) values(1,6,6)
insert buslinestations(BusLineID,StopSeq,StationID) values(1,7,7)
insert buslinestations(BusLineID,StopSeq,StationID) values(2,1,8)
insert buslinestations(BusLineID,StopSeq,StationID) values(2,2,9)
insert buslinestations(BusLineID,StopSeq,StationID) values(2,3,2)
insert buslinestations(BusLineID,StopSeq,StationID) values(2,4,10)
insert buslinestations(BusLineID,StopSeq,StationID) values(2,5,11)
insert buslinestations(BusLineID,StopSeq,StationID) values(3,1,12)
insert buslinestations(BusLineID,StopSeq,StationID) values(3,2,13)
insert buslinestations(BusLineID,StopSeq,StationID) values(3,3,5)
insert buslinestations(BusLineID,StopSeq,StationID) values(3,4,14)
insert buslinestations(BusLineID,StopSeq,StationID) values(3,5,15)


insert users(UserID,UserType,EquipID,UserName,Password) values(1,1,1,'yangzhuang','yangzhuang')
insert users(UserID,UserType,EquipID,UserName,Password) values(2,1,2,'yanjiang','yanjiang')
insert users(UserID,UserType,EquipID,UserName,Password) values(	3,2,3,'daben','daben')
insert users(UserID,UserType,EquipID,UserName,Password) values(	4,2,4,'dayu','dayu')
insert users(UserID,UserType,EquipID,UserName,Password) values(	5,3,5,'xiaoli','xiaoli')
insert users(UserID,UserType,EquipID,UserName,Password) values(	6,3,6,'xiaoliu','xiaoliu')

