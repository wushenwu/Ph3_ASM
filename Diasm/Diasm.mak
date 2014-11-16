# Microsoft Developer Studio Generated NMAKE File, Based on Diasm.dsp
!IF "$(CFG)" == ""
CFG=Diasm - Win32 Debug
!MESSAGE No configuration specified. Defaulting to Diasm - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Diasm - Win32 Release" && "$(CFG)" != "Diasm - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Diasm.mak" CFG="Diasm - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Diasm - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Diasm - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Diasm - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\Diasm.exe"


CLEAN :
	-@erase "$(INTDIR)\Diasm.obj"
	-@erase "$(INTDIR)\Diasm.pch"
	-@erase "$(INTDIR)\Diasm.res"
	-@erase "$(INTDIR)\DiasmDlg.obj"
	-@erase "$(INTDIR)\DiasmEngine.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\Diasm.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\Diasm.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x804 /fo"$(INTDIR)\Diasm.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Diasm.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\Diasm.pdb" /machine:I386 /out:"$(OUTDIR)\Diasm.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Diasm.obj" \
	"$(INTDIR)\DiasmDlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\Diasm.res" \
	"$(INTDIR)\DiasmEngine.obj"

"$(OUTDIR)\Diasm.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Diasm - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\Diasm.exe" "$(OUTDIR)\Diasm.pch"


CLEAN :
	-@erase "$(INTDIR)\Diasm.obj"
	-@erase "$(INTDIR)\Diasm.pch"
	-@erase "$(INTDIR)\Diasm.res"
	-@erase "$(INTDIR)\DiasmDlg.obj"
	-@erase "$(INTDIR)\DiasmEngine.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\Diasm.exe"
	-@erase "$(OUTDIR)\Diasm.ilk"
	-@erase "$(OUTDIR)\Diasm.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\Diasm.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ   /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x804 /fo"$(INTDIR)\Diasm.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Diasm.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\Diasm.pdb" /debug /machine:I386 /out:"$(OUTDIR)\Diasm.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\Diasm.obj" \
	"$(INTDIR)\DiasmDlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\Diasm.res" \
	"$(INTDIR)\DiasmEngine.obj"

"$(OUTDIR)\Diasm.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("Diasm.dep")
!INCLUDE "Diasm.dep"
!ELSE 
!MESSAGE Warning: cannot find "Diasm.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "Diasm - Win32 Release" || "$(CFG)" == "Diasm - Win32 Debug"
SOURCE=.\Diasm.cpp

"$(INTDIR)\Diasm.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Diasm.pch"


SOURCE=.\Diasm.rc

"$(INTDIR)\Diasm.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\DiasmDlg.cpp

"$(INTDIR)\DiasmDlg.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Diasm.pch"


SOURCE=.\DiasmEngine.cpp

"$(INTDIR)\DiasmEngine.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Diasm.pch"


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "Diasm - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\Diasm.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\Diasm.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Diasm - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\Diasm.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ   /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\Diasm.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

