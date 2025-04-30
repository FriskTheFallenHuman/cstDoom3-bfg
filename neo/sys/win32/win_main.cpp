/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "precompiled.h"
#pragma hdrstop

#include <errno.h>
#include <float.h>
#include <fcntl.h>
#include <direct.h>
#include <io.h>
#include <conio.h>
#include <mapi.h>
#include <shellapi.h>
#include <shlobj.h>

#ifndef __MRC__
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include "../sys_local.h"
#include "win_local.h"
#include "../../renderer/tr_local.h"

idCVar Win32Vars_t::sys_arch( "sys_arch", "", CVAR_SYSTEM | CVAR_INIT, "" );
idCVar Win32Vars_t::sys_cpustring( "sys_cpustring", "detect", CVAR_SYSTEM | CVAR_INIT, "" );
idCVar Win32Vars_t::in_mouse( "in_mouse", "1", CVAR_SYSTEM | CVAR_BOOL, "enable mouse input" );
idCVar Win32Vars_t::win_outputEditString( "win_outputEditString", "1", CVAR_SYSTEM | CVAR_BOOL, "" );
idCVar Win32Vars_t::win_viewlog( "win_viewlog", "0", CVAR_SYSTEM | CVAR_INTEGER, "" );
idCVar Win32Vars_t::win_timerUpdate( "win_timerUpdate", "0", CVAR_SYSTEM | CVAR_BOOL, "allows the game to be updated while dragging the window" );
idCVar Win32Vars_t::win_allowMultipleInstances( "win_allowMultipleInstances", "0", CVAR_SYSTEM | CVAR_BOOL, "allow multiple instances running concurrently" );

//#modified-fva; BEGIN
//Win32Vars_t	win32;
Win32Vars_t win32 = {};
//#modified-fva; END

static char		sys_cmdline[MAX_STRING_CHARS];

static HANDLE hProcessMutex;

//#modified-fva; BEGIN
extern "C" {
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

#ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
	#define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002
#endif
class CstTimeHiRes {
	/*
	This class contains code adapted from Blat Blatnik's precise_sleep.c sample
	(retrieved 2024-04-10), which is under the Unlicense license:
	   https://github.com/blat-blatnik/Snippets/blob/main/precise_sleep.c
	   https://github.com/blat-blatnik/Snippets/blob/main/LICENSE
	*/

public:
				CstTimeHiRes();
	void		Init();
	void		Shutdown();
	int64       ClockCount();
	double		ClockCountToMilliseconds(int64 count);
	void		Sleep(double seconds);

private:
	HANDLE Timer;
	int SchedulerPeriodMs;
	INT64 QpcPerSecond;
};

CstTimeHiRes::CstTimeHiRes() {
	Timer = NULL;
	SchedulerPeriodMs = 0;
	QpcPerSecond = 0;
}

void CstTimeHiRes::Init() {
	if (!Timer) {
		Timer = CreateWaitableTimerEx(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
	}

	TIMECAPS timeCaps;
	bool capsObtained = false;
	if (timeGetDevCaps(&timeCaps, sizeof(timeCaps)) == MMSYSERR_NOERROR) {
		SchedulerPeriodMs = (int)timeCaps.wPeriodMin;
		if (SchedulerPeriodMs > 0) {
			timeBeginPeriod((UINT)SchedulerPeriodMs);
			capsObtained = true;
		}
	}
	if (!capsObtained) {
		SchedulerPeriodMs = 0;
	}

	LARGE_INTEGER qpf;
	QueryPerformanceFrequency(&qpf);
	QpcPerSecond = qpf.QuadPart;
}

void CstTimeHiRes::Shutdown() {
	if (Timer) {
		CloseHandle(Timer);
		Timer = NULL;
	}
	if (SchedulerPeriodMs > 0) {
		timeEndPeriod((UINT)SchedulerPeriodMs);
	}
	SchedulerPeriodMs = 0;
}

int64 CstTimeHiRes::ClockCount() {
	if (!idLib::IsMainThread()) {
		return 0;
	}
	LARGE_INTEGER qpc;
	QueryPerformanceCounter(&qpc);
	int64 count = qpc.QuadPart;
	return count;
}

double CstTimeHiRes::ClockCountToMilliseconds(int64 count) {
	if (!idLib::IsMainThread()) {
		return 0.0;
	}
	double msec = ((double)count / (double)QpcPerSecond) * 1000.0;
	return msec;
}

void CstTimeHiRes::Sleep(double milliseconds) {
	if (!idLib::IsMainThread()) {
		return;
	}
	double seconds = milliseconds * 0.001;

	LARGE_INTEGER qpc;
	QueryPerformanceCounter(&qpc);
	INT64 targetQpc = qpc.QuadPart + (INT64)(seconds * (double)QpcPerSecond);

	if (SchedulerPeriodMs > 0)
	{
		if (Timer) // Try using a high resolution timer first.
		{
			const double TOLERANCE = 0.001'02;
			INT64 maxTicks = (INT64)SchedulerPeriodMs * 9'500;
			for (;;) // Break sleep up into parts that are lower than scheduler period.
			{
				double remainingSeconds = (double)(targetQpc - qpc.QuadPart) / (double)QpcPerSecond;
				INT64 sleepTicks = (INT64)((remainingSeconds - TOLERANCE) * 10'000'000.0); // 100ns intervals
				if (sleepTicks <= 0)
					break;

				LARGE_INTEGER due;
				due.QuadPart = -(sleepTicks > maxTicks ? maxTicks : sleepTicks);
				SetWaitableTimerEx(Timer, &due, 0, NULL, NULL, NULL, 0);
				WaitForSingleObject(Timer, INFINITE);
				QueryPerformanceCounter(&qpc);
			}
		}
		else // Fallback to Sleep.
		{
			const double TOLERANCE = 0.000'02;
			double sleepMs = (seconds - TOLERANCE) * 1000.0 - (double)SchedulerPeriodMs; // Sleep for 1 scheduler period less than requested.
			int sleepSlices = (int)(sleepMs / (double)SchedulerPeriodMs);
			if (sleepSlices > 0)
				Sleep((DWORD)sleepSlices * (DWORD)SchedulerPeriodMs);
			QueryPerformanceCounter(&qpc);
		}
	}

	while (qpc.QuadPart < targetQpc) // Spin for any remaining time.
	{
		YieldProcessor();
		QueryPerformanceCounter(&qpc);
	}
}

CstTimeHiRes cstTimeHiRes;
//#modified-fva; END

/*
=============
Sys_Error

Show the early console as an error dialog
=============
*/
void Sys_Error( const char *error, ... ) {
	va_list		argptr;
	char		text[4096];
	MSG        msg;

	va_start( argptr, error );
	vsprintf( text, error, argptr );
	va_end( argptr);

	Conbuf_AppendText( text );
	Conbuf_AppendText( "\n" );

	Win_SetErrorText( text );
	//#modified-fva; BEGIN
	//Sys_ShowConsole( 1, true );
	//#modified-fva; END

	//#modified-fva; BEGIN
	//timeEndPeriod( 1 );
	cstTimeHiRes.Shutdown();
	//#modified-fva; END

	Sys_ShutdownInput();

	//#modified-fva; BEGIN
	//GLimp_Shutdown();
	renderSystem->Shutdown();
	//#modified-fva; END

	//#modified-fva; BEGIN
	Sys_ShowConsole(3, true); // moved from above to be after renderSystem->Shutdown; uses a different visLevel parameter
	//#modified-fva; END

	extern idCVar com_productionMode;
	if ( com_productionMode.GetInteger() == 0 ) {
		// wait for the user to quit
		while ( 1 ) {
			if ( !GetMessage( &msg, NULL, 0, 0 ) ) {
				common->Quit();
			}
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}
	Sys_DestroyConsole();

	exit (1);
}

/*
========================
Sys_Launch
========================
*/
void Sys_Launch( const char * path, idCmdArgs & args,  void * data, unsigned int dataSize ) {

	TCHAR				szPathOrig[_MAX_PATH];
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);

	strcpy( szPathOrig, va( "\"%s\" %s", Sys_EXEPath(), (const char *)data ) );

	if ( !CreateProcess( NULL, szPathOrig, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi ) ) {
		idLib::Error( "Could not start process: '%s' ", szPathOrig );
		return;
	}
	cmdSystem->AppendCommandText( "quit\n" );
}

/*
========================
Sys_GetCmdLine
========================
*/
const char * Sys_GetCmdLine() {
	return sys_cmdline;
}

/*
========================
Sys_ReLaunch
========================
*/
void Sys_ReLaunch( void * data, const unsigned int dataSize ) {
	TCHAR				szPathOrig[MAX_PRINT_MSG];
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);

	strcpy( szPathOrig, va( "\"%s\" %s", Sys_EXEPath(), (const char *)data ) );

	CloseHandle( hProcessMutex );

	if ( !CreateProcess( NULL, szPathOrig, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi ) ) {
		idLib::Error( "Could not start process: '%s' ", szPathOrig );
		return;
	}
	cmdSystem->AppendCommandText( "quit\n" );
}

/*
==============
Sys_Quit
==============
*/
void Sys_Quit() {
	//#modified-fva; BEGIN
	//timeEndPeriod( 1 );
	cstTimeHiRes.Shutdown();
	//#modified-fva; END
	Sys_ShutdownInput();
	Sys_DestroyConsole();
	ExitProcess( 0 );
}


/*
==============
Sys_Printf
==============
*/
#define MAXPRINTMSG 4096
void Sys_Printf( const char *fmt, ... ) {
	char		msg[MAXPRINTMSG];

	va_list argptr;
	va_start(argptr, fmt);
	idStr::vsnPrintf( msg, MAXPRINTMSG-1, fmt, argptr );
	va_end(argptr);
	msg[sizeof(msg)-1] = '\0';

	OutputDebugString( msg );

	if ( win32.win_outputEditString.GetBool() && idLib::IsMainThread() ) {
		Conbuf_AppendText( msg );
	}
}

/*
==============
Sys_DebugPrintf
==============
*/
#define MAXPRINTMSG 4096
void Sys_DebugPrintf( const char *fmt, ... ) {
	char msg[MAXPRINTMSG];

	va_list argptr;
	va_start( argptr, fmt );
	idStr::vsnPrintf( msg, MAXPRINTMSG-1, fmt, argptr );
	msg[ sizeof(msg)-1 ] = '\0';
	va_end( argptr );

	OutputDebugString( msg );
}

/*
==============
Sys_DebugVPrintf
==============
*/
void Sys_DebugVPrintf( const char *fmt, va_list arg ) {
	char msg[MAXPRINTMSG];

	idStr::vsnPrintf( msg, MAXPRINTMSG-1, fmt, arg );
	msg[ sizeof(msg)-1 ] = '\0';

	OutputDebugString( msg );
}

/*
==============
Sys_Sleep
==============
*/
void Sys_Sleep( int msec ) {
	Sleep( msec );
}

//#modified-fva; BEGIN
void Sys_CstEnableThreadAffinity(bool enable) {
	static bool isEnabled = false;
	static DWORD_PTR previousAffinityMask = 0;

	if (!idLib::IsMainThread()) {
		return;
	}
	if (enable) {
		if (!isEnabled) {
			isEnabled = true;
			HANDLE hThread = GetCurrentThread();
			previousAffinityMask = SetThreadAffinityMask(hThread, 0x1); // previousAffinityMask will be 0 if this call fails
			Sleep(0);
		}
	} else {
		if (isEnabled) {
			isEnabled = false;
			if (previousAffinityMask) {
				HANDLE hThread = GetCurrentThread();
				SetThreadAffinityMask(hThread, previousAffinityMask);
				previousAffinityMask = 0;
				Sleep(0);
			}
		}
	}
}

int64 Sys_CstHiResClockCount() {
	return cstTimeHiRes.ClockCount();
}

double Sys_CstHiResClockCountToMilliseconds(int64 count) {
	return cstTimeHiRes.ClockCountToMilliseconds(count);
}

void Sys_CstSleepHiRes(double milliseconds) {
	cstTimeHiRes.Sleep(milliseconds);
}
//#modified-fva; END

/*
==============
Sys_ShowWindow
==============
*/
void Sys_ShowWindow( bool show ) {
	::ShowWindow( win32.hWnd, show ? SW_SHOW : SW_HIDE );
}

/*
==============
Sys_IsWindowVisible
==============
*/
bool Sys_IsWindowVisible() {
	return ( ::IsWindowVisible( win32.hWnd ) != 0 );
}

/*
==============
Sys_Mkdir
==============
*/
void Sys_Mkdir( const char *path ) {
	_mkdir (path);
}

/*
=================
Sys_FileTimeStamp
=================
*/
ID_TIME_T Sys_FileTimeStamp( idFileHandle fp ) {
	FILETIME writeTime;
	GetFileTime( fp, NULL, NULL, &writeTime );

	/*
		FILETIME = number of 100-nanosecond ticks since midnight
		1 Jan 1601 UTC. time_t = number of 1-second ticks since
		midnight 1 Jan 1970 UTC. To translate, we subtract a
		FILETIME representation of midnight, 1 Jan 1970 from the
		time in question and divide by the number of 100-ns ticks
		in one second.
	*/

	SYSTEMTIME base_st = {
		1970,   // wYear
		1,      // wMonth
		0,      // wDayOfWeek
		1,      // wDay
		0,      // wHour
		0,      // wMinute
		0,      // wSecond
		0       // wMilliseconds
	};

	FILETIME base_ft;
	SystemTimeToFileTime( &base_st, &base_ft );

	LARGE_INTEGER itime;
	itime.QuadPart = reinterpret_cast<LARGE_INTEGER&>( writeTime ).QuadPart;
	itime.QuadPart -= reinterpret_cast<LARGE_INTEGER&>( base_ft ).QuadPart;
	itime.QuadPart /= 10000000LL;
	return itime.QuadPart;
}

/*
========================
Sys_Rmdir
========================
*/
bool Sys_Rmdir( const char *path ) {
	return _rmdir( path ) == 0;
}

/*
========================
Sys_IsFileWritable
========================
*/
bool Sys_IsFileWritable( const char *path ) {
	struct _stat st;
	if ( _stat( path, &st ) == -1 ) {
		return true;
	}
	return ( st.st_mode & S_IWRITE ) != 0;
}

/*
========================
Sys_IsFolder
========================
*/
sysFolder_t Sys_IsFolder( const char *path ) {
	struct _stat buffer;
	if ( _stat( path, &buffer ) < 0 ) {
		return FOLDER_ERROR;
	}
	return ( buffer.st_mode & _S_IFDIR ) != 0 ? FOLDER_YES : FOLDER_NO;
}

/*
==============
Sys_Cwd
==============
*/
const char *Sys_Cwd() {
	static char cwd[MAX_OSPATH];

	_getcwd( cwd, sizeof( cwd ) - 1 );
	cwd[MAX_OSPATH-1] = 0;

	return cwd;
}

/*
==============
Sys_DefaultBasePath
==============
*/
const char *Sys_DefaultBasePath() {
	return Sys_Cwd();
}

// Vista shit
typedef HRESULT (WINAPI * SHGetKnownFolderPath_t)( const GUID & rfid, DWORD dwFlags, HANDLE hToken, PWSTR *ppszPath );
// NOTE: FOLIDERID_SavedGames is already exported from in shell32.dll in Windows 7.  We can only detect
// the compiler version, but that doesn't doesn't tell us which version of the OS we're linking against.
// This GUID value should never change, so we name it something other than FOLDERID_SavedGames to get
// around this problem.
const GUID FOLDERID_SavedGames_IdTech5 = { 0x4c5c32ff, 0xbb9d, 0x43b0, { 0xb5, 0xb4, 0x2d, 0x72, 0xe5, 0x4e, 0xaa, 0xa4 } };

/*
==============
Sys_DefaultSavePath
==============
*/
const char *Sys_DefaultSavePath() {
	static char savePath[ MAX_PATH ];
	memset( savePath, 0, MAX_PATH );

	HMODULE hShell = LoadLibrary( "shell32.dll" );
	if ( hShell ) {
		SHGetKnownFolderPath_t SHGetKnownFolderPath = (SHGetKnownFolderPath_t)GetProcAddress( hShell, "SHGetKnownFolderPath" );
		if ( SHGetKnownFolderPath ) {
			wchar_t * path;
			if ( SUCCEEDED( SHGetKnownFolderPath( FOLDERID_SavedGames_IdTech5, CSIDL_FLAG_CREATE | CSIDL_FLAG_PER_USER_INIT, 0, &path ) ) ) {
				if ( wcstombs( savePath, path, MAX_PATH ) > MAX_PATH ) {
					savePath[0] = 0;
				}
				CoTaskMemFree( path );
			}
		}
		FreeLibrary( hShell );
	}

	if ( savePath[0] == 0 ) {
		SHGetFolderPath( NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, savePath );
		strcat( savePath, "\\My Games" );
	}

	strcat( savePath, SAVE_PATH );

	return savePath;
}

/*
==============
Sys_EXEPath
==============
*/
const char *Sys_EXEPath() {
	static char exe[ MAX_OSPATH ];
	GetModuleFileName( NULL, exe, sizeof( exe ) - 1 );
	return exe;
}

/*
==============
Sys_ListFiles
==============
*/
int Sys_ListFiles( const char *directory, const char *extension, idStrList &list ) {
	idStr		search;
	struct _finddata_t findinfo;
	intptr_t	findhandle;
	int			flag;

	if ( !extension) {
		extension = "";
	}

	// passing a slash as extension will find directories
	if ( extension[0] == '/' && extension[1] == 0 ) {
		extension = "";
		flag = 0;
	} else {
		flag = _A_SUBDIR;
	}

	sprintf( search, "%s\\*%s", directory, extension );

	// search
	list.Clear();

	findhandle = _findfirst( search, &findinfo );
	if ( findhandle == -1 ) {
		return -1;
	}

	do {
		if ( flag ^ ( findinfo.attrib & _A_SUBDIR ) ) {
			list.Append( findinfo.name );
		}
	} while ( _findnext( findhandle, &findinfo ) != -1 );

	_findclose( findhandle );

	return list.Num();
}


/*
================
Sys_GetClipboardData
================
*/
char *Sys_GetClipboardData() {
	char *data = NULL;
	char *cliptext;

	if ( OpenClipboard( NULL ) != 0 ) {
		HANDLE hClipboardData;

		if ( ( hClipboardData = GetClipboardData( CF_TEXT ) ) != 0 ) {
			if ( ( cliptext = (char *)GlobalLock( hClipboardData ) ) != 0 ) {
				data = (char *)Mem_Alloc( GlobalSize( hClipboardData ) + 1, TAG_CRAP );
				strcpy( data, cliptext );
				GlobalUnlock( hClipboardData );

				strtok( data, "\n\r\b" );
			}
		}
		CloseClipboard();
	}
	return data;
}

/*
================
Sys_SetClipboardData
================
*/
void Sys_SetClipboardData( const char *string ) {
	HGLOBAL HMem;
	char *PMem;

	// allocate memory block
	HMem = (char *)::GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, strlen( string ) + 1 );
	if ( HMem == NULL ) {
		return;
	}
	// lock allocated memory and obtain a pointer
	PMem = (char *)::GlobalLock( HMem );
	if ( PMem == NULL ) {
		return;
	}
	// copy text into allocated memory block
	lstrcpy( PMem, string );
	// unlock allocated memory
	::GlobalUnlock( HMem );
	// open Clipboard
	if ( !OpenClipboard( 0 ) ) {
		::GlobalFree( HMem );
		return;
	}
	// remove current Clipboard contents
	EmptyClipboard();
	// supply the memory handle to the Clipboard
	SetClipboardData( CF_TEXT, HMem );
	HMem = 0;
	// close Clipboard
	CloseClipboard();
}

/*
========================
ExecOutputFn
========================
*/
static void ExecOutputFn( const char * text ) {
	idLib::Printf( text );
}


/*
========================
Sys_Exec

if waitMsec is INFINITE, completely block until the process exits
If waitMsec is -1, don't wait for the process to exit
Other waitMsec values will allow the workFn to be called at those intervals.
========================
*/
bool Sys_Exec(	const char * appPath, const char * workingPath, const char * args,
	execProcessWorkFunction_t workFn, execOutputFunction_t outputFn, const int waitMS,
	unsigned int & exitCode ) {
		exitCode = 0;
		SECURITY_ATTRIBUTES secAttr;
		secAttr.nLength = sizeof( SECURITY_ATTRIBUTES );
		secAttr.bInheritHandle = TRUE;
		secAttr.lpSecurityDescriptor = NULL;

		HANDLE hStdOutRead;
		HANDLE hStdOutWrite;
		CreatePipe( &hStdOutRead, &hStdOutWrite, &secAttr, 0 );
		SetHandleInformation( hStdOutRead, HANDLE_FLAG_INHERIT, 0 );

		HANDLE hStdInRead;
		HANDLE hStdInWrite;
		CreatePipe( &hStdInRead, &hStdInWrite, &secAttr, 0 );
		SetHandleInformation( hStdInWrite, HANDLE_FLAG_INHERIT, 0 );

		STARTUPINFO si;
		memset( &si, 0, sizeof( si ) );
		si.cb = sizeof( si );
		si.hStdError = hStdOutWrite;
		si.hStdOutput = hStdOutWrite;
		si.hStdInput = hStdInRead;
		si.wShowWindow = FALSE;
		si.dwFlags |= STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

		PROCESS_INFORMATION pi;
		memset ( &pi, 0, sizeof( pi ) );

		if ( outputFn != NULL ) {
			outputFn( va( "^2Executing Process: ^7%s\n^2working path: ^7%s\n^2args: ^7%s\n", appPath, workingPath, args ) );
		} else {
			outputFn = ExecOutputFn;
		}

		// we duplicate args here so we can concatenate the exe name and args into a single command line
		const char * imageName = appPath;
		char * cmdLine = NULL;
		{
			// if we have any args, we need to copy them to a new buffer because CreateProcess modifies
			// the command line buffer.
			if ( args != NULL ) {
				if ( appPath != NULL ) {
					int len = idStr::Length( args ) + idStr::Length( appPath ) + 1 /* for space */ + 1 /* for NULL terminator */ + 2 /* app quotes */;
					cmdLine = (char*)Mem_Alloc( len, TAG_TEMP );
					// note that we're putting quotes around the appPath here because when AAS2.exe gets an app path with spaces
					// in the path "w:/zion/build/win32/Debug with Inlines/AAS2.exe" it gets more than one arg for the app name,
					// which it most certainly should not, so I am assuming this is a side effect of using CreateProcess.
					idStr::snPrintf( cmdLine, len, "\"%s\" %s", appPath, args );
				} else {
					int len = idStr::Length( args ) + 1;
					cmdLine = (char*)Mem_Alloc( len, TAG_TEMP );
					idStr::Copynz( cmdLine, args, len );
				}
				// the image name should always be NULL if we have command line arguments because it is already
				// prefixed to the command line.
				imageName = NULL;
			}
		}

		BOOL result = CreateProcess( imageName, (LPSTR)cmdLine, NULL, NULL, TRUE, 0, NULL, workingPath, &si, &pi );

		if ( result == FALSE ) {
			TCHAR szBuf[1024];
			LPVOID lpMsgBuf;
			DWORD dw = GetLastError();

			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				dw,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR) &lpMsgBuf,
				0, NULL );

			wsprintf( szBuf, "%d: %s", dw, lpMsgBuf );
			if ( outputFn != NULL ) {
				outputFn( szBuf );
			}
			LocalFree( lpMsgBuf );
			if ( cmdLine != NULL ) {
				Mem_Free( cmdLine );
			}
			return false;
		} else if ( waitMS >= 0 ) {	// if waitMS == -1, don't wait for process to exit
			DWORD ec = 0;
			DWORD wait = 0;
			char buffer[ 4096 ];
			for ( ; ; ) {
				wait = WaitForSingleObject( pi.hProcess, waitMS );
				GetExitCodeProcess( pi.hProcess, &ec );

				DWORD bytesRead = 0;
				DWORD bytesAvail = 0;
				DWORD bytesLeft = 0;
				BOOL ok = PeekNamedPipe( hStdOutRead, NULL, 0, NULL, &bytesAvail, &bytesLeft );
				if ( ok && bytesAvail != 0 ) {
					ok = ReadFile( hStdOutRead, buffer, sizeof( buffer ) - 3, &bytesRead, NULL );
					if ( ok && bytesRead > 0 ) {
						buffer[ bytesRead ] = '\0';
						if ( outputFn != NULL ) {
							int length = 0;
							for ( int i = 0; buffer[i] != '\0'; i++ ) {
								if ( buffer[i] != '\r' ) {
									buffer[length++] = buffer[i];
								}
							}
							buffer[length++] = '\0';
							outputFn( buffer );
						}
					}
				}

				if ( ec != STILL_ACTIVE ) {
					exitCode = ec;
					break;
				}

				if ( workFn != NULL ) {
					if ( !workFn() ) {
						TerminateProcess( pi.hProcess, 0 );
						break;
					}
				}
			}
		}

		// this assumes that windows duplicates the command line string into the created process's
		// environment space.
		if ( cmdLine != NULL ) {
			Mem_Free( cmdLine );
		}

		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
		CloseHandle( hStdOutRead );
		CloseHandle( hStdOutWrite );
		CloseHandle( hStdInRead );
		CloseHandle( hStdInWrite );
		return true;
}

/*
========================================================================

DLL Loading

========================================================================
*/

/*
=====================
Sys_DLL_Load
=====================
*/
uintptr_t Sys_DLL_Load( const char *dllName ) {
	HINSTANCE	libHandle;
	libHandle = LoadLibrary( dllName );
	if ( libHandle ) {
		// since we can't have LoadLibrary load only from the specified path, check it did the right thing
		char loadedPath[ MAX_OSPATH ];
		GetModuleFileName( libHandle, loadedPath, sizeof( loadedPath ) - 1 );
		if ( idStr::IcmpPath( dllName, loadedPath ) ) {
			Sys_Printf( "ERROR: LoadLibrary '%s' wants to load '%s'\n", dllName, loadedPath );
			Sys_DLL_Unload( (uintptr_t)libHandle );
			return 0;
		}
	} else {
		DWORD e = GetLastError();

		if ( e ==  0x7E ) {
			// 0x7E is "The specified module could not be found."
			// don't print a warning for that error, it's expected
			// when trying different possible paths for a DLL
			return 0;
		}

		if ( e == 0xC1) {
			// "[193 (0xC1)] is not a valid Win32 application"
			// probably going to be common. Lets try to be less cryptic.
			common->Warning( "LoadLibrary( \"%s\" ) Failed ! [%i (0x%X)]\tprobably the DLL is of the wrong architecture, "
			                 "like x64 instead of x86 (this build expects %s)",
			                 dllName, e, e, CPUSTRING );
			return 0;
		}

		// for all other errors, print whatever FormatMessage() gives us
		LPVOID msgBuf = NULL;

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			e,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&msgBuf,
			0, NULL);

		common->Warning( "LoadLibrary( \"%s\" ) Failed ! [%i (0x%X)]\t%s", dllName, e, e, msgBuf );

		::LocalFree( msgBuf );
	}
	return (uintptr_t)libHandle;
}

/*
=====================
Sys_DLL_GetProcAddress
=====================
*/
void *Sys_DLL_GetProcAddress( uintptr_t dllHandle, const char *procName ) {
	void * adr = (void*)GetProcAddress((HINSTANCE)dllHandle, procName);
	if (!adr)
	{
		DWORD e = GetLastError();
		LPVOID msgBuf = NULL;

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			e,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&msgBuf,
			0, NULL);

		idStr errorStr = va("[%i (0x%X)]\t%s", e, e, msgBuf);

		if (errorStr.Length())
			common->Warning("GetProcAddress( %i %s) Failed ! %s", dllHandle, procName, errorStr.c_str());

		::LocalFree(msgBuf);
	}
	return adr;
}

/*
=====================
Sys_DLL_Unload
=====================
*/
void Sys_DLL_Unload( uintptr_t dllHandle ) {
	if ( !dllHandle ) {
		return;
	}
	if ( FreeLibrary( (HINSTANCE)dllHandle ) == 0 ) {
		int lastError = GetLastError();
		LPVOID lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER,
		    NULL,
			lastError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL
		);
		Sys_Error( "Sys_DLL_Unload: FreeLibrary failed - %s (%d)", lpMsgBuf, lastError );
	}
}

/*
========================================================================

EVENT LOOP

========================================================================
*/

#define	MAX_QUED_EVENTS		256
#define	MASK_QUED_EVENTS	( MAX_QUED_EVENTS - 1 )

sysEvent_t	eventQue[MAX_QUED_EVENTS];
int			eventHead = 0;
int			eventTail = 0;

/*
================
Sys_QueEvent

Ptr should either be null, or point to a block of data that can
be freed by the game later.
================
*/
void Sys_QueEvent( sysEventType_t type, int value, int value2, int ptrLength, void *ptr, int inputDeviceNum ) {
	sysEvent_t * ev = &eventQue[ eventHead & MASK_QUED_EVENTS ];

	if ( eventHead - eventTail >= MAX_QUED_EVENTS ) {
		common->Printf("Sys_QueEvent: overflow\n");
		// we are discarding an event, but don't leak memory
		if ( ev->evPtr ) {
			Mem_Free( ev->evPtr );
		}
		eventTail++;
	}

	eventHead++;

	ev->evType = type;
	ev->evValue = value;
	ev->evValue2 = value2;
	ev->evPtrLength = ptrLength;
	ev->evPtr = ptr;
	ev->inputDevice = inputDeviceNum;
}

/*
=============
Sys_PumpEvents

This allows windows to be moved during renderbump
=============
*/
void Sys_PumpEvents() {
	MSG msg;

	// pump the message loop
	while( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ) {
		if ( !GetMessage( &msg, NULL, 0, 0 ) ) {
			common->Quit();
		}

		// save the msg time, because wndprocs don't have access to the timestamp
		if ( win32.sysMsgTime && win32.sysMsgTime > (int)msg.time ) {
			// don't ever let the event times run backwards
//			common->Printf( "Sys_PumpEvents: win32.sysMsgTime (%i) > msg.time (%i)\n", win32.sysMsgTime, msg.time );
		} else {
			win32.sysMsgTime = msg.time;
		}

		TranslateMessage (&msg);
		DispatchMessage (&msg);
	}
}

/*
================
Sys_GenerateEvents
================
*/
void Sys_GenerateEvents() {
	static int entered = false;
	char *s;

	if ( entered ) {
		return;
	}
	entered = true;

	// pump the message loop
	Sys_PumpEvents();

	// grab or release the mouse cursor if necessary
	IN_Frame();

	// check for console commands
	s = Sys_ConsoleInput();
	if ( s ) {
		char	*b;
		int		len;

		len = strlen( s ) + 1;
		b = (char *)Mem_Alloc( len, TAG_EVENTS );
		strcpy( b, s );
		Sys_QueEvent( SE_CONSOLE, 0, 0, len, b, 0 );
	}

	entered = false;
}

/*
================
Sys_ClearEvents
================
*/
void Sys_ClearEvents() {
	eventHead = eventTail = 0;
}

/*
================
Sys_GetEvent
================
*/
sysEvent_t Sys_GetEvent() {
	sysEvent_t	ev;

	// return if we have data
	if ( eventHead > eventTail ) {
		eventTail++;
		return eventQue[ ( eventTail - 1 ) & MASK_QUED_EVENTS ];
	}

	// return the empty event
	memset( &ev, 0, sizeof( ev ) );

	return ev;
}

//================================================================

/*
=================
Sys_In_Restart_f

Restart the input subsystem
=================
*/
void Sys_In_Restart_f( const idCmdArgs &args ) {
	Sys_ShutdownInput();
	Sys_InitInput();
}

/*
================
Sys_AlreadyRunning

returns true if there is a copy of D3 running already
================
*/
bool Sys_AlreadyRunning() {
#ifndef DEBUG
	if ( !win32.win_allowMultipleInstances.GetBool() ) {
		hProcessMutex = ::CreateMutex( NULL, FALSE, "DOOM3" );
		if ( ::GetLastError() == ERROR_ALREADY_EXISTS || ::GetLastError() == ERROR_ACCESS_DENIED ) {
			return true;
		}
	}
#endif
	return false;
}

/*
================
Sys_Init

The cvar system must already be setup
================
*/

void Sys_Init() {

	CoInitialize( NULL );

	cmdSystem->AddCommand( "in_restart", Sys_In_Restart_f, CMD_FL_SYSTEM, "restarts the input system" );

	//
	// Windows version
	//
	win32.osversion.dwOSVersionInfoSize = sizeof( win32.osversion );

	if ( !GetVersionEx( (LPOSVERSIONINFO)&win32.osversion ) )
		Sys_Error( "Couldn't get OS info" );

	if ( win32.osversion.dwMajorVersion < 7 ) {
		Sys_Error( GAME_NAME " requires Windows 7 SP1 or greater" );
	}

	if ( win32.osversion.dwMajorVersion >= 6 && win32.osversion.dwMinorVersion == 1 && ( win32.osversion.dwBuildNumber == 7600 || win32.osversion.dwBuildNumber == 7601 ) ) {
		common->Printf( "Detected: Windows 7 %u.%u (Build: %u)\n", win32.osversion.dwMajorVersion, win32.osversion.dwMinorVersion, win32.osversion.dwBuildNumber );
	} else if ( win32.osversion.dwMajorVersion >= 6 && win32.osversion.dwMinorVersion == 2 && ( win32.osversion.dwBuildNumber == 9200 ) ) {
		common->Printf( "Detected: Windows 8 %u.%u (Build: %u)\n", win32.osversion.dwMajorVersion, win32.osversion.dwMinorVersion, win32.osversion.dwBuildNumber );
	} else if ( win32.osversion.dwMajorVersion >= 6 && win32.osversion.dwMinorVersion == 3 && ( win32.osversion.dwBuildNumber == 9200 || win32.osversion.dwBuildNumber == 9600 ) ) {
		common->Printf( "Detected: Windows 8.1 %u.%u (Build: %u)\n", win32.osversion.dwMajorVersion, win32.osversion.dwMinorVersion, win32.osversion.dwBuildNumber );
	} else if ( win32.osversion.dwMajorVersion >= 10 && win32.osversion.dwBuildNumber < 21996 ) {
		common->Printf( "Detected: Windows 10 %u.%u (Build: %u)\n", win32.osversion.dwMajorVersion, win32.osversion.dwMinorVersion, win32.osversion.dwBuildNumber );
	} else if ( win32.osversion.dwMajorVersion >= 10 && win32.osversion.dwBuildNumber >= 21996 ) {
		common->Printf( "Detected: Windows 11 %u.%u (Build: %u)\n", win32.osversion.dwMajorVersion, win32.osversion.dwMinorVersion, win32.osversion.dwBuildNumber );
	} else {
		common->Printf( "Detected: Windows (Proton/Wine) %u.%u (Build: %u)", win32.osversion.dwMajorVersion, win32.osversion.dwMinorVersion, win32.osversion.dwBuildNumber );
	}

	//
	// CPU type
	//
	if ( !idStr::Icmp( win32.sys_cpustring.GetString(), "detect" ) ) {
		idStr string;

		common->Printf( "%1.0f MHz ", Sys_ClockTicksPerSecond() / 1000000.0f );

		win32.cpuid = Sys_GetCPUId();

		string.Clear();

		if ( win32.cpuid & CPUID_AMD ) {
			string += "AMD CPU";
		} else if ( win32.cpuid & CPUID_INTEL ) {
			string += "Intel CPU";
		} else {
			string += "generic CPU";
		}

		string += " with ";
		if ( win32.cpuid & CPUID_MMX ) {
			string += "MMX & ";
		}
		if ( win32.cpuid & CPUID_SSE ) {
			string += "SSE & ";
		}
		string.StripTrailing( " & " );
		string.StripTrailing( " with " );
		win32.sys_cpustring.SetString( string );
	} else {
		common->Printf( "forcing CPU type to " );
		idLexer src( win32.sys_cpustring.GetString(), idStr::Length( win32.sys_cpustring.GetString() ), "sys_cpustring" );
		idToken token;

		int id = CPUID_NONE;
		while( src.ReadToken( &token ) ) {
			if ( token.Icmp( "generic" ) == 0 ) {
				id |= CPUID_GENERIC;
			} else if ( token.Icmp( "intel" ) == 0 ) {
				id |= CPUID_INTEL;
			} else if ( token.Icmp( "amd" ) == 0 ) {
				id |= CPUID_AMD;
			} else if ( token.Icmp( "mmx" ) == 0 ) {
				id |= CPUID_MMX;
			} else if ( token.Icmp( "sse" ) == 0 ) {
				id |= CPUID_SSE;
			}
		}
		if ( id == CPUID_NONE ) {
			common->Printf( "WARNING: unknown sys_cpustring '%s'\n", win32.sys_cpustring.GetString() );
			id = CPUID_GENERIC;
		}
		win32.cpuid = (cpuid_t) id;
	}

	common->Printf( "%s\n", win32.sys_cpustring.GetString() );
	common->Printf( "%d MB System Memory\n", Sys_GetSystemRam() );
	common->Printf( "%d MB Video Memory\n", Sys_GetVideoRam() );
	if ( ( win32.cpuid & CPUID_SSE ) == 0 ) {
		common->Error( "SSE not supported!" );
	}

	win32.g_Joystick.Init();
}

/*
================
Sys_Shutdown
================
*/
void Sys_Shutdown() {
	CoUninitialize();
}

/*
================
Sys_GetProcessorId
================
*/
cpuid_t Sys_GetProcessorId() {
	return win32.cpuid;
}

/*
================
Sys_GetProcessorString
================
*/
const char *Sys_GetProcessorString() {
	return win32.sys_cpustring.GetString();
}

//=======================================================================

//#define SET_THREAD_AFFINITY

/*
====================
Win_Frame
====================
*/
void Win_Frame() {
	// if "viewlog" has been modified, show or hide the log console
	if ( win32.win_viewlog.IsModified() ) {
		win32.win_viewlog.ClearModified();
	}
}

/*
==================
WinMain
==================
*/
int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow ) {

	//#modified-fva; BEGIN
	//const HCURSOR hcurSave = ::SetCursor( LoadCursor( 0, IDC_WAIT ) );
	::SetCursor(NULL);
	//#modified-fva; END

	Sys_SetPhysicalWorkMemory( 192 << 20, 1024 << 20 );

	win32.hInstance = hInstance;
	idStr::Copynz( sys_cmdline, lpCmdLine, sizeof( sys_cmdline ) );

	// done before Com/Sys_Init since we need this for error output
	Sys_CreateConsole();

	// no abort/retry/fail errors
	SetErrorMode( SEM_FAILCRITICALERRORS );

	for ( int i = 0; i < MAX_CRITICAL_SECTIONS; i++ ) {
		InitializeCriticalSection( &win32.criticalSections[i] );
	}

	// make sure the timer is high precision, otherwise
	// NT gets 18ms resolution
	//#modified-fva; BEGIN
	//timeBeginPeriod( 1 );
	cstTimeHiRes.Init();
	//#modified-fva; END

	// get the initial time base
	Sys_Milliseconds();

#ifdef DEBUG
	// disable the painfully slow MS heap check every 1024 allocs
	_CrtSetDbgFlag( 0 );
#endif

	common->Init( 0, NULL, lpCmdLine );

	// hide or show the early console as necessary
	if ( win32.win_viewlog.GetInteger() ) {
		Sys_ShowConsole( 1, true );
	} else {
		Sys_ShowConsole( 0, false );
	}

#ifdef SET_THREAD_AFFINITY
	// give the main thread an affinity for the first cpu
	SetThreadAffinityMask( GetCurrentThread(), 1 );
#endif

	//#modified-fva; BEGIN
	//::SetCursor( hcurSave );
	//#modified-fva; END

	::SetFocus( win32.hWnd );

	// main game loop
	while( 1 ) {

		Win_Frame();

#ifdef DEBUG
		Sys_MemFrame();
#endif

		// run the game
		common->Frame();
	}

	// never gets here
	return 0;
}

/*
==================
idSysLocal::OpenURL
==================
*/
void idSysLocal::OpenURL( const char *url, bool doexit ) {
	static bool doexit_spamguard = false;
	HWND wnd;

	if (doexit_spamguard) {
		common->DPrintf( "OpenURL: already in an exit sequence, ignoring %s\n", url );
		return;
	}

	common->Printf("Open URL: %s\n", url);

	if ( !ShellExecute( NULL, "open", url, NULL, NULL, SW_RESTORE ) ) {
		common->Error( "Could not open url: '%s' ", url );
		return;
	}

	wnd = GetForegroundWindow();
	if ( wnd ) {
		ShowWindow( wnd, SW_MAXIMIZE );
	}

	if ( doexit ) {
		doexit_spamguard = true;
		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "quit\n" );
	}
}

/*
==================
idSysLocal::StartProcess
==================
*/
void idSysLocal::StartProcess( const char *exePath, bool doexit ) {
	TCHAR				szPathOrig[_MAX_PATH];
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);

	strncpy( szPathOrig, exePath, _MAX_PATH );

	if( !CreateProcess( NULL, szPathOrig, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi ) ) {
		common->Error( "Could not start process: '%s' ", szPathOrig );
		return;
	}

	if ( doexit ) {
		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "quit\n" );
	}
}

/*
==================
Sys_SetFatalError
==================
*/
void Sys_SetFatalError( const char *error ) {
}

/*
================
Sys_SetLanguageFromSystem
================
*/
extern idCVar sys_lang;
void Sys_SetLanguageFromSystem() {
	sys_lang.SetString( Sys_DefaultLanguage() );
}
