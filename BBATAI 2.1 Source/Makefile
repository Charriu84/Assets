## BUG DLL MAKEFILE
##
## Based on DannyDaemonic's makefile
##
## - Add MODFLAGS macro:				...see below...
## - Add global compiler flags:		$(MODFLAGS)
## - Add project include:			include
## - Add project library path:		lib
## - Add global libraries:			gdi32.lib Ole32.lib
## - Add Debug library:				CxImage_debug.lib
## - Add Release library:			CxImage.lib
##
## Copyright (c) 2010 The BUG Mod


#### Paths ####

TOOLKIT=D:\Program Files (x86)\Microsoft Visual C++ Toolkit 2003
PSDK=E:\WindowsSDK
BIN = G:\BBATAI\bin

CIV = C:\Firaxis Games\Sid Meier's Civilization 4\Beyond the Sword
SDK = G:\BBATAI
BOOST = C:\Firaxis Games\Sid Meier's Civilization 4\Beyond the Sword\CvGameCoreDLL\Boost-1.32.0
PYTHON = C:\Firaxis Games\Sid Meier's Civilization 4\Beyond the Sword\CvGameCoreDLL\Python24

## Copy newly compiled dlls to BTS's Assets directory
COPYTO = $(BIN)

#### Tools ####

CC = "$(TOOLKIT)\bin\cl.exe"
CPP = "$(TOOLKIT)\bin\cl.exe"
LD = "$(TOOLKIT)\bin\link.exe"
RC = "$(PSDK)\bin\rc.exe"
FD = "$(BIN)\fastdep.exe"

#### BLACKLIST ####

BLACKLIST = CvTextScreens


#### Target Files ####

Debug_BIN = Debug\CvGameCoreDLL.dll
Release_BIN = Release\CvGameCoreDLL.dll

!IF [IF NOT EXIST CvGameCoreDLL.rc EXIT 1] == 0
Debug_RESOURCE = Debug\CvGameCoreDLL.res
Release_RESOURCE = Release\CvGameCoreDLL.res
!ENDIF

Debug_STATICLIB = Debug\CvGameCoreDLL.lib
Release_STATICLIB = Release\CvGameCoreDLL.lib

Debug_LIBDEF = Debug\CvGameCoreDLL.def
Release_LIBDEF = Release\CvGameCoreDLL.def

Debug_PCH = Debug\CvGameCoreDLL.pch
Release_PCH = Release\CvGameCoreDLL.pch

Debug_PDB = Debug\CvGameCoreDLL.pdb
Release_PDB = Release\CvGameCoreDLL.pdb

Debug_OTHER = Debug\CvGameCoreDLL.exp Debug\CvGameCoreDLL.ilk
Release_OTHER = Release\CvGameCoreDLL.exp


#### CFLAGS ####

# Optional BULL Components (Mods)
#   _BUFFY
#   _MOD_FRACTRADE
#   _MOD_SENTRY				has extra assets
#   _MOD_GOVWORKERS			has extra assets
#   _MOD_AIAUTOPLAY
#   _MOD_GWARM				has extra assets; effectively eliminates global warming
#   _MOD_SHAM_SPOILER		enables AG modifiers in mod
#	_MOD_SPYNC				Spies return to nearest city, not capital

MODFLAGS = /D_MOD_AIAUTOPLAY 
MODFLAGS = /D_MOD_FRACTRADE /D_MOD_SENTRY /D_MOD_GOVWORKERS /D_MOD_AIAUTOPLAY /D_MOD_GWARM /D_MOD_SPYNC

# /EHsc		catch C++ exceptions only; assume that extern C functions never throw a C++ exception
# /Gd		Use the __cdecl calling convention (x86 only). 
# /GR		enable run-time type information (RTTI)
# /Gy		enable function-level linking
# /G7		optimize for Pentium 4 and above
# /MD		multithreaded normal DLL with MSVCRT.dll
# /MDd		multithreaded debug DLL with MSVCRTD.dll
# /Od		disable optimizations
# /O2		optimize for speed (equal to /Og /Oi /Ot /Oy /Ob2 /Gs /GF /Gy)
# /W3		warning level 3

GLOBAL_CFLAGS = /GR /Gy /W3 /EHsc /Gd /DWIN32 /D_WINDOWS /D_USRDLL /DCVGAMECOREDLL_EXPORTS $(MODFLAGS) /Yu"CvGameCoreDLL.h"
# /Zi 
Debug_CFLAGS = /MDd /Od /D_DEBUG /RTC1 /Fp"$(Debug_PCH)" $(GLOBAL_CFLAGS)
Release_CFLAGS = /MD /O2 /G7 /DNDEBUG /DFINAL_RELEASE /Fp"$(Release_PCH)" $(GLOBAL_CFLAGS)

#### LDFLAGS ####

# /DEBUG	create debugging information
# /OPT:REF	remove unreferenced data/functions
# /OPT:ICF	identical COMDAT folding (trimming identical read-only data)
# /SUB:WIN	exe does not need a console, but this isn't an exe so unnecessary?
# /TLBID:1	override default type library ID (1), so this does nothing

GLOBAL_LDFLAGS = /DLL /NOLOGO /SUBSYSTEM:WINDOWS /LARGEADDRESSAWARE /TLBID:1
Debug_LDFLAGS = /INCREMENTAL /DEBUG /PDB:"$(Debug_PDB)" /IMPLIB:"$(Debug_STATICLIB)" $(GLOBAL_LDFLAGS) /NODEFAULTLIB:msvcprtd.lib
Release_LDFLAGS = /INCREMENTAL:NO /OPT:REF /OPT:ICF /PDB:"$(Release_PDB)" $(GLOBAL_LDFLAGS)

#### INCLUDES ####

GLOBAL_INCS = /I"$(TOOLKIT)/include" /I"$(PSDK)/Include" /I"$(PSDK)/Include/mfc"
PROJECT_INCS = /I"$(BOOST)/include" /I"$(PYTHON)/include" /I"include"
Debug_INCS = $(PROJECT_INCS) $(GLOBAL_INCS)
Release_INCS = $(PROJECT_INCS) $(GLOBAL_INCS)

#### LIBS ####

GLOBAL_LIBS = /LIBPATH:"$(TOOLKIT)/lib" /LIBPATH:"$(PSDK)/Lib" winmm.lib user32.lib
PROJECT_LIBS = /LIBPATH:"$(PYTHON)/libs" /LIBPATH:"$(BOOST)/libs/" /LIBPATH:lib boost_python-vc71-mt-1_32.lib gdi32.lib Ole32.lib
Debug_LIBS = $(PROJECT_LIBS) $(GLOBAL_LIBS) msvcprt.lib CxImage_debug.lib
Release_LIBS = $(PROJECT_LIBS) $(GLOBAL_LIBS) CxImage.lib

#### Objects ####
Debug_LINKOBJS = $(Debug_OBJS)
Release_LINKOBJS = $(Release_OBJS)

#### Auto SOURCES/OBJS ####
!IF [ECHO SOURCES= \> sources.mk] == 0 && \
    [FOR %i IN (*.cpp) DO @ECHO. "%i" \>> sources.mk] == 0 && \
    [ECHO.>> sources.mk] == 0 && \
    [ECHO Debug_OBJS= \>> sources.mk] == 0 && \
    [FOR /F "delims=." %i IN ('dir /b *.cpp') DO @ECHO. Debug\%i.obj \>> sources.mk] == 0 && \
    [ECHO.>> sources.mk] == 0 && \
    [ECHO Release_OBJS= \>> sources.mk] == 0 && \
    [FOR /F "delims=." %i IN ('dir /b *.cpp') DO @ECHO. Release\%i.obj \>> sources.mk] == 0 && \
    [ECHO.>> sources.mk] == 0
!INCLUDE sources.mk
#!IF [DEL sources.mk]
#!ENDIF
!ENDIF

#### Targets ####
#################

.PHONY: all clean Debug_clean Release_clean Debug Release

all: Debug Release

clean: Debug_clean Release_clean 

Debug_clean:
	@FOR %i IN ($(Debug_BIN) $(Debug_STATICLIB) $(Debug_LIBDEF) \
		Debug\*.obj Debug\*.@ $(Debug_RESOURCE) \
		$(Debug_PCH) $(Debug_PDB) $(Debug_OTHER)) DO @IF EXIST "%i" DEL "%i"

Release_clean:
	@FOR %i IN ($(Release_BIN) $(Release_STATICLIB) $(Release_LIBDEF) \
		Release\*.obj Release\*.@ $(Release_RESOURCE) \
		$(Release_PCH) $(Release_PDB) $(Release_OTHER)) DO @IF EXIST "%i" DEL "%i"

Debug: Debug_DIR Debug_unfinished $(Debug_PCH) $(Debug_BIN)
!IFDEF COPYTO
	-COPY "$(Debug_BIN)" "$(COPYTO)\."
!ENDIF

Release: Release_DIR Release_unfinished $(Release_PCH) $(Release_BIN)
!IFDEF COPYTO
	-COPY "$(Release_BIN)" "$(COPYTO)\."
!ENDIF

Debug_DIR:
	-@IF NOT EXIST "Debug\." MKDIR "Debug"

Release_DIR:
	-@IF NOT EXIST "Release\." MKDIR "Release"

Debug_unfinished:
	@ECHO.>Debug\unfinished.@
	@FOR /F "delims=@" %i IN ('dir /b Debug\*.@') DO \
		@IF EXIST "Debug\%i" DEL "Debug\%i"
	@FOR /F %i IN ('dir /b Debug\*.@') DO \
		@IF EXIST "Debug\%i" DEL "Debug\%i"

Release_unfinished:
	@ECHO.>Release\unfinished.@
	@FOR /F "delims=@" %i IN ('dir /b Release\*.@') DO \
		@IF EXIST "Release\%i" DEL "Release\%i"
	@FOR /F %i IN ('dir /b Release\*.@') DO \
		@IF EXIST "Release\%i" DEL "Release\%i"

$(Debug_BIN): $(Debug_LINKOBJS) $(Debug_RESOURCE)
	$(LD) /out:$(Debug_BIN) $(Debug_LDFLAGS) $(Debug_LIBS) $(Debug_LINKOBJS) $(Debug_RESOURCE)

$(Release_BIN): $(Release_LINKOBJS) $(Release_RESOURCE)
	$(LD) /out:$(Release_BIN) $(Release_LDFLAGS) $(Release_LIBS) $(Release_LINKOBJS) $(Release_RESOURCE)

.cpp{Debug}.obj:
	@ECHO.>"$*.obj.@"
    $(CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /Fo$*.obj /c $<
	@DEL "$*.obj.@"

.cpp{Release}.obj:
	@ECHO.>"$*.obj.@"
    $(CPP) /nologo $(Release_CFLAGS) $(Release_INCS) /Fo$*.obj /c $<
	@DEL "$*.obj.@"

$(Debug_PCH) Debug\_precompile.obj:
	@ECHO.>"$(Debug_PCH).@"
	@ECHO.>"Debug\_precompile.obj.@"
	$(CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /YcCvGameCoreDLL.h /Fo"Debug\_precompile.obj" /c _precompile.cpp
	@DEL "$(Debug_PCH).@"
	@DEL "Debug\_precompile.obj.@"

$(Release_PCH) Release\_precompile.obj:
	@ECHO.>"$(Release_PCH).@"
	@ECHO.>"Release\_precompile.obj.@"
    $(CPP) /nologo $(Release_CFLAGS) $(Release_INCS) /YcCvGameCoreDLL.h /Fo"Release\_precompile.obj" /c _precompile.cpp
	@DEL "$(Release_PCH).@"
	@DEL "Release\_precompile.obj.@"

.rc{Debug}.res:
	@ECHO.>"$*.res.@"
	$(RC) /Fo$@ $(Debug_INCS) $<
	@DEL "$*.res.@"

.rc{Release}.res:
	@ECHO.>"$*.res.@"
	$(RC) /Fo$@ $(Release_INCS) $<
	@DEL "$*.res.@"

!IFDEF BLACKLIST

Debug\$(BLACKLIST).obj: $(BLACKLIST).cpp
	@ECHO.>"$*.obj.@"
	@ECHO.>"$*-dummy.cpp"
	$(CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /Y- /Fo$@ /c "$*-dummy.cpp"
	@DEL "$*-dummy.cpp"
	@DEL "$*.obj.@"

Release\$(BLACKLIST).obj: $(BLACKLIST).cpp
	@ECHO.>"$*.obj.@"
	@ECHO.>"$*-dummy.cpp"
	$(CPP) /nologo $(Release_CFLAGS) $(Release_INCS) /Y- /Fo$@ /c "$*-dummy.cpp"
	@DEL "$*-dummy.cpp"
	@DEL "$*.obj.@"

!ENDIF

!IFDEF FD

!IF [IF NOT EXIST $(FD) EXIT 1] == 0
!IF [$(FD) --objectextension=pch -q -O Debug CvGameCoreDLL.cpp > depends] != 0 || \
    [$(FD) --objectextension=obj -q -O Debug $(SOURCES) >> depends] != 0 || \
    [$(FD) --objectextension=pch -q -O Release CvGameCoreDLL.cpp >> depends] != 0 || \
    [$(FD) --objectextension=obj -q -O Release $(SOURCES) >> depends] != 0
!MESSAGE Error running fastdep.
!ENDIF
!ELSE
!IF [ECHO "fastdep.exe" NOT FOUND! && \
     ECHO Please edit Makefile to reflect the correct path of fastdep. && \
     ECHO. ]
!ENDIF
!ENDIF

!ENDIF

!IF EXIST(depends)
!INCLUDE depends
!ENDIF
