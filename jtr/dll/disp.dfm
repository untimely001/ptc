object MainForm: TMainForm
  Left = 157
  Top = 145
  BorderStyle = bsDialog
  Caption = '��ǰ·��״̬'
  ClientHeight = 321
  ClientWidth = 568
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesktopCenter
  OnClose = FormClose
  OnDestroy = FormDestroy
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Bevel1: TBevel
    Left = 0
    Top = 40
    Width = 568
    Height = 1
    Align = alTop
    Shape = bsTopLine
  end
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 568
    Height = 40
    Align = alTop
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    TabOrder = 2
    object AllCrossNum: TLabel
      Left = 45
      Top = 13
      Width = 180
      Height = 13
      AutoSize = False
      Caption = 'ϵͳ��·�ڻ�������'
    end
    object CrossOkNum: TLabel
      Left = 307
      Top = 14
      Width = 180
      Height = 13
      AutoSize = False
      Caption = 'ͨѶ������·�ڻ�����'
    end
  end
  object Panel2: TPanel
    Left = 0
    Top = 281
    Width = 568
    Height = 40
    Align = alBottom
    TabOrder = 0
    object BitBtn1: TBitBtn
      Left = 300
      Top = 8
      Width = 75
      Height = 25
      Caption = '�� �� (&C)'
      TabOrder = 0
      OnClick = BitBtn1Click
      Kind = bkClose
    end
  end
  object Panel3: TPanel
    Left = 0
    Top = 41
    Width = 568
    Height = 240
    Align = alClient
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    TabOrder = 1
    OnMouseMove = Panel3MouseMove
    object ChooseLabel: TLabel
      Left = 93
      Top = 205
      Width = 54
      Height = 13
      Caption = '�� ѡ ��'
    end
    object PrePageLabel: TLabel
      Left = 352
      Top = 209
      Width = 60
      Height = 13
      AutoSize = False
      Caption = '��  һ  ҳ'
      OnClick = PrePageLabelClick
      OnMouseMove = PrePageLabelMouseMove
    end
    object NextPageLabel: TLabel
      Left = 507
      Top = 209
      Width = 60
      Height = 13
      AutoSize = False
      Caption = '��  һ  ҳ '
      OnClick = NextPageLabelClick
      OnMouseMove = NextPageLabelMouseMove
    end
    object ComboBox1: TComboBox
      Left = 152
      Top = 203
      Width = 89
      Height = 21
      ItemHeight = 13
      TabOrder = 0
      Text = 'ComboBox1'
      OnChange = ComboBox1Change
    end
  end
  object Timer1: TTimer
    OnTimer = Timer1Timer
    Left = 536
    Top = 288
  end
  object PopupMenu1: TPopupMenu
    Left = 496
    Top = 288
    object N1: TMenuItem
      Caption = 'ͨѶ״̬'
      OnClick = N1Click
    end
    object N2: TMenuItem
      Caption = '�˳�Ӧ��'
      OnClick = N2Click
    end
  end
end
