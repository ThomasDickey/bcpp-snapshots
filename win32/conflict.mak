# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=conflict - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to conflict - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "conflict - Win32 Release" && "$(CFG)" !=\
 "conflict - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "conflict.mak" CFG="conflict - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "conflict - Win32 Release" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "conflict - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "conflict - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\conflict.exe"

CLEAN : 
	-@erase ".\Release\conflict.exe"
	-@erase ".\Release\conflict.obj"
	-@erase ".\Release\dirent.obj"
	-@erase ".\Release\msdos.obj"
	-@erase ".\Release\txtalloc.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /I "." /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /I "." /D "WIN32" /D "NDEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/conflict.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/conflict.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/conflict.pdb" /machine:I386 /out:"$(OUTDIR)/conflict.exe" 
LINK32_OBJS= \
	"$(INTDIR)/conflict.obj" \
	"$(INTDIR)/dirent.obj" \
	"$(INTDIR)/msdos.obj" \
	"$(INTDIR)/txtalloc.obj"

"$(OUTDIR)\conflict.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "conflict - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\conflict.exe"

CLEAN : 
	-@erase ".\Debug\conflict.exe"
	-@erase ".\Debug\conflict.obj"
	-@erase ".\Debug\dirent.obj"
	-@erase ".\Debug\msdos.obj"
	-@erase ".\Debug\txtalloc.obj"
	-@erase ".\Debug\conflict.ilk"
	-@erase ".\Debug\conflict.pdb"
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "." /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I "." /D "WIN32" /D "_DEBUG" /D\
 "_CONSOLE" /Fp"$(INTDIR)/conflict.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/conflict.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/conflict.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/conflict.exe" 
LINK32_OBJS= \
	"$(INTDIR)/conflict.obj" \
	"$(INTDIR)/dirent.obj" \
	"$(INTDIR)/msdos.obj" \
	"$(INTDIR)/txtalloc.obj"

"$(OUTDIR)\conflict.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "conflict - Win32 Release"
# Name "conflict - Win32 Debug"

!IF  "$(CFG)" == "conflict - Win32 Release"

!ELSEIF  "$(CFG)" == "conflict - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\conflict.c
DEP_CPP_CONFL=\
	".\conflict.h"\
	{$(INCLUDE)}"\sys\types.h"\
	{$(INCLUDE)}"\sys\stat.h"\
	".\dirent.h"\
	
NODEP_CPP_CONFL=\
	".\config.h"\
	

"$(INTDIR)\conflict.obj" : $(SOURCE) $(DEP_CPP_CONFL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\dirent.c
DEP_CPP_DIREN=\
	".\dirent.h"\
	

"$(INTDIR)\dirent.obj" : $(SOURCE) $(DEP_CPP_DIREN) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\msdos.c
DEP_CPP_MSDOS=\
	".\conflict.h"\
	{$(INCLUDE)}"\sys\types.h"\
	{$(INCLUDE)}"\sys\stat.h"\
	".\dirent.h"\
	
NODEP_CPP_MSDOS=\
	".\config.h"\
	

"$(INTDIR)\msdos.obj" : $(SOURCE) $(DEP_CPP_MSDOS) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\txtalloc.c
DEP_CPP_TXTAL=\
	".\conflict.h"\
	{$(INCLUDE)}"\sys\types.h"\
	{$(INCLUDE)}"\sys\stat.h"\
	".\dirent.h"\
	
NODEP_CPP_TXTAL=\
	".\config.h"\
	

"$(INTDIR)\txtalloc.obj" : $(SOURCE) $(DEP_CPP_TXTAL) "$(INTDIR)"


# End Source File
# End Target
# End Project
################################################################################
