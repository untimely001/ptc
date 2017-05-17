object CrossForm: TCrossForm
  Left = 366
  Top = 242
  BorderStyle = bsNone
  Caption = 'CrossForm'
  ClientHeight = 173
  ClientWidth = 122
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Image1: TImage
    Left = 33
    Top = 12
    Width = 50
    Height = 50
    ParentShowHint = False
    ShowHint = True
  end
  object Label1: TLabel
    Left = 0
    Top = 73
    Width = 130
    Height = 13
    AutoSize = False
    Caption = '路 口 编 号: '
  end
  object Label2: TLabel
    Left = 0
    Top = 93
    Width = 130
    Height = 13
    AutoSize = False
    Caption = '控制机类型: '
  end
  object Label3: TLabel
    Left = 0
    Top = 134
    Width = 130
    Height = 13
    AutoSize = False
    Caption = '通讯端口号: '
  end
  object Label4: TLabel
    Left = 0
    Top = 154
    Width = 130
    Height = 13
    AutoSize = False
    Caption = '通 讯 状 态: '
  end
  object CommType: TLabel
    Left = 0
    Top = 114
    Width = 130
    Height = 13
    AutoSize = False
    Caption = '通 讯 方 式: '
  end
end
