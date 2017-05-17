//---------------------------------------------------------------------------

#ifndef dispH
#define dispH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include <Buttons.hpp>
#include <Graphics.hpp>

#ifdef __FACEDLL
#define FACEIMP __declspec(dllexport)
#else
#define FACEIMP __declspec(dllimport)
#endif

#include <math.h>
#include "../agentsvr.h"
#include "../wincomm.h"
#include "../jtdb.h"

typedef struct tag_Icon4CrossData
{
   HANDLE hBitMap;
   int CrossNo;
}Icon4CrossData;

typedef struct tag_CManiData
{
   int AllNumber;
   int CurPageNo;
   int FirstCrossNo;
   int LastCrossNo;

   Icon4CrossData Icon4Cross[20];
}CManiData;

//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:	// IDE-managed Components
   TTimer *Timer1;
   TPanel *Panel1;
   TBevel *Bevel1;
   TLabel *AllCrossNum;
   TLabel *CrossOkNum;
   TPopupMenu *PopupMenu1;
   TMenuItem *N1;
   TMenuItem *N2;
   TPanel *Panel2;
   TBitBtn *BitBtn1;
   TPanel *Panel3;
   TLabel *ChooseLabel;
   TComboBox *ComboBox1;
   TLabel *PrePageLabel;
   TLabel *NextPageLabel;
   void __fastcall N1Click(TObject *Sender);
   void __fastcall N2Click(TObject *Sender);
   void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
   void __fastcall BitBtn1Click(TObject *Sender);
   void __fastcall FormShow(TObject *Sender);
   void __fastcall PrePageLabelMouseMove(TObject *Sender,
          TShiftState Shift, int X, int Y);
   void __fastcall NextPageLabelMouseMove(TObject *Sender,
          TShiftState Shift, int X, int Y);
   void __fastcall Panel3MouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
   void __fastcall FormDestroy(TObject *Sender);
//   void __fastcall OnMyDblClick(TObject *Sender);
   int __fastcall findCurPageNo(void);
   int __fastcall GetPageCount(void);
   void __fastcall ComboBox1Change(TObject *Sender);
   void __fastcall DrawPage(int PageNo);
   void __fastcall PrePageLabelClick(TObject *Sender);
   void __fastcall NextPageLabelClick(TObject *Sender);
   void __fastcall Timer1Timer(TObject *Sender);
private:	// User declarations
public:		// User declarations
   __fastcall TMainForm(TComponent* Owner);
   void __fastcall DispFirtPage(void);
   void __fastcall SetInvisible(void);
   void __fastcall SetVisible(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;

extern "C"
{
   FACEIMP __stdcall void GetSysParam(long,char *,char *);
   FACEIMP __stdcall void ShowPopUp(POINT * pMousePos);
//   FACEIMP __stdcall void ShowForm(unsigned char *);
}
//---------------------------------------------------------------------------
#endif
