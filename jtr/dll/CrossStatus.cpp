//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "disp.h"
#include "CrossStatus.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TCrossForm *CrossForm;

extern COprData *pOpr;
extern CrossData_t *pDev;

//---------------------------------------------------------------------------
__fastcall TCrossForm::TCrossForm(TComponent* Owner)
   : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TCrossForm::FormShow(TObject *Sender)
{
   CrossData_t *pDevMap = pDev;
   int Flag = 10;
   int index = CrossNo - 1;

   if(index >= 0 && index < MAX_CROSS_NUM)
   {
      if((pDevMap + index)->Type == 1)//sanlian cross
       {
           if((pDevMap + index)->Status == CONNECT_STATUS)
               Flag = 1;//Green
           else
               Flag = 0;//Red,disconnect
      }
      else if((pDevMap + index)->Type == 2)//jinsan cross
      {
           if((pDevMap + index)->NoSignalCount <= 20)
               Flag = 1;//Green
           else
               Flag = 0;//Red,disconnect
      }

      Label1->Caption = "路 口 编 号: " + AnsiString((pDevMap + index)->lkbh);
      if((pDevMap + index)->Type == 1)
         Label2->Caption = "控制机类型: " + AnsiString("三  联");
      else if((pDevMap + index)->Type == 2)
         Label2->Caption = "控制机类型: " + AnsiString("京  三");
      else
         Label2->Caption = "控制机类型: " + AnsiString("未  知");

      if((pDevMap + index)->CardNo == 0)
         CommType->Caption = "通 讯 方 式: " + AnsiString("串口通讯");
      else if((pDevMap + index)->CardNo == 1 || (pDevMap + index)->CardNo == 2)
         CommType->Caption = "通 讯 方 式: " + AnsiString("LCU通讯");
      else
         CommType->Caption = "通 讯 方 式: " + AnsiString("未知方式");

      Label3->Caption = "通讯端口号: " + AnsiString(CrossNo);

      if(Flag == 1)
         Label4->Caption = "通 讯 状 态: " + AnsiString("正常");
      else if(Flag == 0)
         Label4->Caption = "通 讯 状 态: " + AnsiString("异常");
      else
         Label4->Caption = "通 讯 状 态: " + AnsiString("未知");
   }
   else
   {
       Label1->Caption = "路 口 编 号: " + AnsiString("未知");
       Label2->Caption = "控制机类型: " + AnsiString("未知");
       CommType->Caption = "通 讯 方 式: " + AnsiString("未知方式");
       Label3->Caption = "通讯端口号: " + AnsiString("未知");
       Label4->Caption = "通 讯 状 态: " + AnsiString("未知");
   }

   if(Flag == 0)
       Image1->Canvas->Brush->Color = clRed;
   else if(Flag == 1)
       Image1->Canvas->Brush->Color = clGreen;
   else
       Image1->Canvas->Brush->Color = clYellow;

   Image1->Canvas->Brush->Style = bsSolid;
   Image1->Transparent = true;

   Image1->Canvas->Ellipse(0, 0,Image1->Width,Image1->Height);

   //display reconnect times
   Image1->Hint = "";
   if(/*(pDevMap + index)->CardNo == 0 &&*/(pDevMap + index)->Type == 1 \
                                         || (pDevMap + index)->Type == 2)
   {
       if((pDevMap + index)->CardNo == 0)
          Image1->Hint = "连接次数: " + AnsiString( (pDevMap + index)->OpenTimes );
       else
          Image1->Hint = "连接次数: " + AnsiString(1);

       if((pDevMap + index)->LastDisconnectTime.wYear > 2000)
       {
           Image1->Hint = Image1->Hint + "\n断开时间:"
               + AnsiString(" ") + Format("%2.2d",ARRAYOFCONST(( (pDevMap + index)->LastDisconnectTime.wHour)) )    \
               + AnsiString(":") + Format("%2.2d",ARRAYOFCONST(( (pDevMap + index)->LastDisconnectTime.wMinute)) )  \
               + AnsiString(":") + Format("%2.2d",ARRAYOFCONST(( (pDevMap + index)->LastDisconnectTime.wSecond)) )  \
               + AnsiString(" ") + Format("%4.2d",ARRAYOFCONST(( (pDevMap + index)->LastDisconnectTime.wYear)) )    \
               + AnsiString(",") + Format("%2.2d",ARRAYOFCONST(( (pDevMap + index)->LastDisconnectTime.wMonth)) )   \
               + AnsiString(",") + Format("%2.2d",ARRAYOFCONST(( (pDevMap + index)->LastDisconnectTime.wDay)) );
       }

       if((pDevMap + index)->LastConnectTime.wYear > 2000)
       {
           Image1->Hint = Image1->Hint + "\n连接时间:"
               + AnsiString(" ") + Format("%2.2d",ARRAYOFCONST(( (pDevMap + index)->LastConnectTime.wHour)) )    \
               + AnsiString(":") + Format("%2.2d",ARRAYOFCONST(( (pDevMap + index)->LastConnectTime.wMinute)) )  \
               + AnsiString(":") + Format("%2.2d",ARRAYOFCONST(( (pDevMap + index)->LastConnectTime.wSecond)) )  \
               + AnsiString(" ") + Format("%4.4d",ARRAYOFCONST(( (pDevMap + index)->LastConnectTime.wYear)) )    \
               + AnsiString(",") + Format("%2.2d",ARRAYOFCONST(( (pDevMap + index)->LastConnectTime.wMonth)) )   \
               + AnsiString(",") + Format("%2.2d",ARRAYOFCONST(( (pDevMap + index)->LastConnectTime.wDay)) );
       }
   }
}
//---------------------------------------------------------------------------


