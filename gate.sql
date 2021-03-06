USE [aspdb]
GO
/****** Object:  Table [dbo].[通知表]    Script Date: 08/24/2013 18:24:06 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[通知表](
	[id] [int] IDENTITY(1,1) NOT NULL,
	[QRContent] [nvarchar](255) NULL,
	[type] [int] NULL,
	[Time] [nvarchar](50) NULL,
	[Status] [int] NULL,
 CONSTRAINT [PK_通知表] PRIMARY KEY CLUSTERED 
(
	[id] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[门状态表]    Script Date: 08/24/2013 18:24:06 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[门状态表](
	[id] [int] IDENTITY(1,1) NOT NULL,
	[GateNo] [int] NULL,
	[GateStatus] [int] NULL,
	[SysStatus] [int] NULL,
	[BatteryStatus] [int] NULL,
	[PowerStatus] [int] NULL,
	[GPRSStatus] [int] NULL,
 CONSTRAINT [PK_门状态表] PRIMARY KEY CLUSTERED 
(
	[id] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[警报描述表]    Script Date: 08/24/2013 18:24:06 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[警报描述表](
	[id] [int] IDENTITY(1,1) NOT NULL,
	[AlarmNo] [int] NULL,
	[Discription] [nvarchar](255) NULL,
 CONSTRAINT [PK_警报描述表] PRIMARY KEY CLUSTERED 
(
	[id] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[二维码表]    Script Date: 08/24/2013 18:24:06 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[二维码表](
	[id] [int] IDENTITY(1,1) NOT NULL,
	[QR] [nvarchar](255) NULL,
	[QRPicPos] [nvarchar](255) NULL,
	[Count] [int] NULL,
	[Expiration] [nvarchar](50) NULL,
	[Status] [int] NULL,
 CONSTRAINT [PK_二维码表] PRIMARY KEY CLUSTERED 
(
	[id] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[报警表]    Script Date: 08/24/2013 18:24:06 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[报警表](
	[id] [int] IDENTITY(1,1) NOT NULL,
	[GateNo] [int] NULL,
	[AlarmNo] [int] NULL,
	[Time] [nvarchar](50) NULL,
 CONSTRAINT [PK_报警] PRIMARY KEY CLUSTERED 
(
	[id] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
