//---------------------------------------------------------------------------

#ifndef CrossStatusH
#define CrossStatusH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TCrossForm : public TForm
{
__published:	// IDE-managed Components
   TImage *Image1;
   TLabel *Label1;
   TLabel *Label2;
   TLabel *Label3;
   TLabel *Label4;
   TLabel *CommType;
   void __fastcall FormShow(TObject *Sender);
private:	// User declarations
public:		// User declarations
   int CrossNo;
   __fastcall TCrossForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TCrossForm *CrossForm;
//---------------------------------------------------------------------------
#endif
