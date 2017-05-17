//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#define __FACEDLL
#include "CrossStatus.h"
#include "disp.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

#define ICON_LEN       35
#define X_LENGTH       130
#define Y_LENGTH       200
#define X_MARGIN       30
#define Y_MARGIN       80
#define Y_INTERVAL     20


#define MAX_PAGE_NUM   60
#define REFRESHCOUNT   10

TMainForm *MainForm;

COprData *pOpr = NULL;
CrossData_t *pDev = NULL;
HANDLE hMainWnd;
//CManiData ManiData;
int AllCrossNo = 0;
int CrossOk = 0;
int PageCount = 1;
int CurPage = 0;
char pagetext[MAX_PAGE_NUM][15]
       = { {"第 一 页"   },{"第 二 页"    },{"第 三 页"    },{"第 四 页"    },{"第 五 页"    },
           {"第 六 页"    },{"第 七 页"    },{"第 八 页"    },{"第 九 页"    },{"第 十 页"    },
           {"第 十一 页"  },{"第 十二 页"  },{"第 十三 页"  },{"第 十四 页"  },{"第 十五 页"  },
           {"第 十六 页"  },{"第 十七 页"  },{"第 十八 页"  },{"第 十九 页"  },{"第 二十 页"  },
           {"第 二十一 页"},{"第 二十二 页"},{"第 二十三 页"},{"第 二十四 页"},{"第 二十五 页"},
           {"第 二十六 页"},{"第 二十七 页"},{"第 二十八 页"},{"第 二十九 页"},{"第 三十 页"  },
           {"第 三十一 页"},{"第 三十二 页"},{"第 三十三 页"},{"第 三十四 页"},{"第 四十 页"},
           {"第 三十六 页"},{"第 三十七 页"},{"第 三十八 页"},{"第 三十九 页"},{"第 四十 页"  },
           {"第 四十一 页"},{"第 四十二 页"},{"第 四十三 页"},{"第 四十四 页"},{"第 五十 页"},
           {"第 四十六 页"},{"第 四十七 页"},{"第 四十八 页"},{"第 四十九 页"},{"第 三十 页"  },
           {"第 五十一 页"},{"第 五十二 页"},{"第 五十三 页"},{"第 五十四 页"},{"第 五十五 页"},
           {"第 五十六 页"},{"第 五十七 页"},{"第 五十八 页"},{"第 五十九 页"},{"第 六十 页"  }
        };
TCrossForm* CrossHandle[5] = {NULL};
Graphics::TColor PreLabelColor,NextLabelColor;
int RefrshTime = REFRESHCOUNT;
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
   : TForm(Owner)
{
   for(int i = 0;i < 5;i++)
   {
       CrossHandle[i]= new TCrossForm(this);
       CrossHandle[i]->Parent = this;
       CrossHandle[i]->Left = X_MARGIN + i * X_LENGTH;
       CrossHandle[i]->Width = X_LENGTH;
       CrossHandle[i]->Top = Panel1->Height;
       CrossHandle[i]->Height = Y_LENGTH;
//       CrossHandle[i]->Show();
   }
}

void __fastcall TMainForm::FormShow(TObject *Sender)
{
   MainForm->Width = X_MARGIN * 2 + X_LENGTH * 5;
   MainForm->Height = Y_LENGTH + Panel1->Height + Panel2->Height + Y_MARGIN;
/*
   ChooseLabel->Top = Panel1->Height + Y_LENGTH + Y_INTERVAL;
   PrePageLabel->Top = Panel1->Height + Y_LENGTH + Y_INTERVAL;
   NextPageLabel->Top = Panel1->Height + Y_LENGTH + Y_INTERVAL;

   BitBtn1->Left = (Width - BitBtn1->Left) / 2;
*/
//   BitBtn1->SetFocus();

}

//---------------------------------------------------------------------------
void __stdcall ShowPopUp(POINT * pMousePos)
{
   MainForm->PopupMenu1->PopupComponent = MainForm;
   SetForegroundWindow(MainForm->Handle);
   MainForm->PopupMenu1->Popup(pMousePos->x, pMousePos->y);
}

void __stdcall GetSysParam(long hWnd,char * pCross,char *pClient)
{
   hMainWnd = (HANDLE)hWnd;
   pDev = (CrossData_t *)pCross;
   pOpr = (COprData *)pClient;

   MainForm->DispFirtPage();

   PreLabelColor = MainForm->PrePageLabel->Font->Color;
   NextLabelColor = MainForm->NextPageLabel->Font->Color;

   if(IsWindowVisible(hMainWnd))
   {
       if(!IsIconic(hMainWnd))
           ShowWindow(hMainWnd,SW_MINIMIZE	);
   }
}

void __fastcall TMainForm::DispFirtPage(void)
{
   CrossData_t *pDevMap = pDev;
   int count;
   int index;
   int PageNum;

   PageNum = GetPageCount();
   CurPage = 1;

   PrePageLabel->Enabled = false;
   if(PageNum <= 1)
       NextPageLabel->Enabled = false;
   else
       NextPageLabel->Enabled = true;

   SetInvisible();

   count = 0;
   index = 0;
   while(count < 5 && index < MAX_CROSS_NUM)
   {
       if((pDevMap + index)->lkbh != 0)
       {
           CrossHandle[count]->CrossNo = index + 1;
           count++;
       }

       index++;
   }

   ComboBox1->Style = csDropDownList;
   ComboBox1->Items->Clear();
   for(int i = 0;i < PageCount; i++)
       ComboBox1->Items->Add(pagetext[i]);
   ComboBox1->ItemIndex = 0;

   AllCrossNum->Caption = "系统中路口机总数：" + AnsiString(AllCrossNo);
   CrossOkNum->Caption = "通讯正常的路口机数：" + AnsiString(CrossOk);

   SetVisible();
}

int __fastcall TMainForm::GetPageCount(void)
{
   CrossData_t *pDevMap = pDev;
   int i;
   int count = 0;

   AllCrossNo = 0;
   CrossOk = 0;
   for(i = 0;i < MAX_CROSS_NUM;i++)
   {
       if((pDevMap + i)->lkbh != 0)
       {
           AllCrossNo++;
           if((pDevMap + i)->Type == 1)//sanlian cross
           {
                if((pDevMap + i)->Status == CONNECT_STATUS)
                   CrossOk++;
           }
           else if((pDevMap + i)->Type == 2)//jinsan cross
           {
               if((pDevMap + i)->NoSignalCount <= 20)
                   CrossOk++;
           }
       }
   }

   count = (int)ceil((float)AllCrossNo / 5.0);
   PageCount = count;

   return count;
}


void __fastcall TMainForm::N1Click(TObject *Sender)
{
   if(MainForm->N1->Caption == "通讯状态(&Y)")
   {
       if(IsWindowVisible(hMainWnd))
       {
           if(!IsIconic(hMainWnd))
               ShowWindow(hMainWnd,SW_MINIMIZE);
       }
       
       SetForegroundWindow(MainForm->Handle);
       Show();
       //#ifdef TEST
       MainForm->N1->Caption = "关闭显示(&L)";
       //#endif
   }
   else if(MainForm->N1->Caption == "关闭显示(&L)")
   {
       MainForm->N1->Caption = "通讯状态(&Y)";
       if(IsWindowVisible(hMainWnd))
       {
           if(IsIconic(hMainWnd))
           {
               SetForegroundWindow(hMainWnd);
               ShowWindow(hMainWnd,SW_RESTORE);
           }
       }

       Close();
   }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::N2Click(TObject *Sender)
{
   PostMessage(hMainWnd,WM_DESTROY,0,0);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
   if(IsWindowVisible(hMainWnd))
   {
       if(IsIconic(hMainWnd))
       {
           SetForegroundWindow(hMainWnd);
           ShowWindow(hMainWnd,SW_RESTORE);
       }
   }
   
   PostMessage(hMainWnd,WM_FACEUNLOAD,0x5495,0x1688);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::BitBtn1Click(TObject *Sender)
{
   Close();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::PrePageLabelMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
   if(PrePageLabel->Font->Color != clRed)
   {
       PreLabelColor = PrePageLabel->Font->Color;

       PrePageLabel->Cursor = crHandPoint;
       PrePageLabel->Font->Color = clRed;
   }

}
//---------------------------------------------------------------------------

void __fastcall TMainForm::NextPageLabelMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
   if(NextPageLabel->Font->Color != clRed)
   {
       NextLabelColor = NextPageLabel->Font->Color;

       NextPageLabel->Cursor = crHandPoint;
       NextPageLabel->Font->Color = clRed;
   }

}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Panel3MouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
   if(PrePageLabel->Font->Color == clRed)
   {
       PrePageLabel->Cursor = crDefault;
       PrePageLabel->Font->Color = PreLabelColor;
   }
   else if(NextPageLabel->Font->Color == clRed)
   {
       NextPageLabel->Cursor = crDefault;
       NextPageLabel->Font->Color = NextLabelColor;
   }

}
//---------------------------------------------------------------------------


void __fastcall TMainForm::FormDestroy(TObject *Sender)
{
   for(int i = 0;i < 5;i++)
   {
       if(CrossHandle[i] != NULL)
       {
           delete CrossHandle[i];
           CrossHandle[i] = NULL;
       }
   }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ComboBox1Change(TObject *Sender)
{
   int PageNo;

   PageNo = findCurPageNo();
   if(PageNo <= 1 || PageNo > PageCount)
   {
       DispFirtPage();
       return;
   }

   DrawPage(PageNo);
   
   if(PageNo == PageCount)
       NextPageLabel->Enabled = false;
   else
       NextPageLabel->Enabled = true;


   return;
}
//---------------------------------------------------------------------------
int __fastcall TMainForm::findCurPageNo(void)
{
   int i;
   AnsiString text;

   text = ComboBox1->Text;

   for(i = 0;i < PageCount; i++)
       if(strcmp(text.c_str(),pagetext[i]) == 0)
           break;

   if(i >= PageCount)
       return MAX_PAGE_NUM + 1;
   else
       return i + 1;
}

void __fastcall TMainForm::DrawPage(int PageNo)
{
   CrossData_t *pDevMap = pDev;
   int index,count,PastCount;
   int i;

   PageCount = GetPageCount();

   if(PageCount == 0)
   {
       Application->MessageBox("总页数为0","应用异常！",MB_OK);
       return;
   }

   if(PageNo < 1 || PageNo > PageCount)
   {
       DispFirtPage();//default page
       return;
   }

   if(PageNo == 1)
   {
       PrePageLabel->Enabled = false;
       if(PageNo == PageCount)
           NextPageLabel->Enabled = false;
       else
           NextPageLabel->Enabled = true;
   }
   else
   {
       PrePageLabel->Enabled = true;
       if(PageNo == PageCount)
           NextPageLabel->Enabled = false;
       else
           NextPageLabel->Enabled = true;
   }

   PageNo--;

 //Find Cross Index
   index = 0;
   PastCount = 0;
   while(PastCount < PageNo * 5 && index < MAX_CROSS_NUM)
   {
       if((pDevMap + index)->lkbh != 0)
           PastCount++;

       index++;
   }
   if(index >= MAX_CROSS_NUM)
   {
       DispFirtPage();
       Application->MessageBox("无法显示要求的页","应用错误！",MB_OK);
       return;
   }

   SetInvisible();

   count = 0;
   while(count < 5 && index < MAX_CROSS_NUM)
   {
       if((pDevMap + index)->lkbh != 0)
       {
           CrossHandle[count]->CrossNo = index + 1;
           count++;
       }
       index++;
   }

/*
   ComboBox1->Style = csDropDownList;
   ComboBox1->Items->Clear();
   for(int i = 0;i < PageCount; i++)
       ComboBox1->Items->Add(pagetext[i]);
*/
   ComboBox1->ItemIndex = PageNo;

//   AllCrossNum->Caption = "系统中路口机总数：" + AnsiString(AllCrossNo);
   CrossOkNum->Caption = "通讯正常的路口机数：" + AnsiString(CrossOk);
   CurPage = PageNo + 1;

   SetVisible();

   return;
}

void __fastcall TMainForm::SetInvisible(void)
{
   LockWindowUpdate(Handle);
   for(int i = 0;i < 5;i++)
   {
       if(CrossHandle[i] != NULL)
       {
           CrossHandle[i]->Visible = false;
           CrossHandle[i]->CrossNo = 0;
       }
   }
   LockWindowUpdate(NULL);

   return;
}

void __fastcall TMainForm::SetVisible(void)
{
   LockWindowUpdate(Handle);
   for(int i  = 0;i < 5;i++)
   {
       if(CrossHandle[i] != NULL)
           CrossHandle[i]->Visible = true;
   }
   LockWindowUpdate(NULL);

   return;
}

void __fastcall TMainForm::PrePageLabelClick(TObject *Sender)
{
//pre page
   int PageNo;

   PageNo = CurPage;
   if(PageNo < 2 || PageNo > PageCount)
   {
       DispFirtPage();
       Application->MessageBox("无法显示要求的页","应用错误！",MB_OK);
       return;
   }

   ComboBox1->ItemIndex = PageNo - 2;
   DrawPage(PageNo - 1);

   NextPageLabel->Enabled = true;
   //NextPageLabel->Visible = true;

   if(PageNo == 2)
   {
       PrePageLabel->Enabled = false;
       //PrePageLabel->Visible = false;
   }

   return;

}
//---------------------------------------------------------------------------

void __fastcall TMainForm::NextPageLabelClick(TObject *Sender)
{
//next page
   int PageNo;

   PageNo = CurPage;
   if(PageNo < 1 || PageNo >= PageCount)
   {
       DispFirtPage();
       Application->MessageBox("无法显示要求的页","应用错误！",MB_OK);
       return;
   }

   ComboBox1->ItemIndex = PageNo;
   DrawPage(PageNo + 1);

   PrePageLabel->Enabled = true;
   //PrePageLabel->Visible = true;

   if(PageNo == PageCount - 1)
   {
       NextPageLabel->Enabled = false;
       //NextPageLabel->Visible = false;
   }

   return;
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::Timer1Timer(TObject *Sender)
{
   int PageNo;

   if(RefrshTime > 0)
   {
       RefrshTime--;
       return;
   }
   else
      RefrshTime = REFRESHCOUNT;

   PageNo = CurPage;
   if(PageNo < 1 || PageNo > PageCount)
   {
       DispFirtPage();
       Application->MessageBox("无法显示要求的页","应用错误！",MB_OK);
       return;
   }

   DrawPage(PageNo);

   return;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------



