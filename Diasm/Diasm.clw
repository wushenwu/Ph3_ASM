; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CDiasmDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "Diasm.h"

ClassCount=3
Class1=CDiasmApp
Class2=CDiasmDlg
Class3=CAboutDlg

ResourceCount=3
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Resource3=IDD_DIASM_DIALOG

[CLS:CDiasmApp]
Type=0
HeaderFile=Diasm.h
ImplementationFile=Diasm.cpp
Filter=N

[CLS:CDiasmDlg]
Type=0
HeaderFile=DiasmDlg.h
ImplementationFile=DiasmDlg.cpp
Filter=W
LastObject=IDC_OUTPUT
BaseClass=CDialog
VirtualFilter=dWC

[CLS:CAboutDlg]
Type=0
HeaderFile=DiasmDlg.h
ImplementationFile=DiasmDlg.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_DIASM_DIALOG]
Type=1
Class=CDiasmDlg
ControlCount=10
Control1=IDC_STATIC,static,1342308352
Control2=IDC_DIASM,edit,1350631552
Control3=IDC_BTN_DIASM,button,1342242816
Control4=IDC_STATIC,static,1342308352
Control5=IDC_ASM,edit,1353777348
Control6=IDC_STATIC,static,1342308352
Control7=IDC_FILE,edit,1350631552
Control8=IDC_BTN_OPENFILE,button,1342242816
Control9=IDC_OUTPUT,edit,1353777348
Control10=IDC_BTN_DIASM_FILE,button,1342242816

