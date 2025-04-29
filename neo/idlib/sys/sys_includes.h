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
#ifndef SYS_INCLUDES_H
#define SYS_INCLUDES_H

// Include the various platform specific header files (windows.h, etc)

/*
================================================================================================

	Windows

================================================================================================
*/

#include <sdkddkver.h>

#ifndef _D3SDK
#ifndef GAME_DLL

#include <winsock2.h>
#include <mmsystem.h>
#include <mmreg.h>

#endif /* !GAME_DLL */
#endif /* !_D3SDK */

#include <intrin.h>			// needed for intrinsics like _mm_setzero_si28
#include <malloc.h>			// no malloc.h on mac or unix

#include <windows.h>		// for qgl.h

// stupid namespace polluting Microsoft monkeys
#undef FindText
#undef IsMinimized
#undef DrawText
#undef CopyFile
#undef LoadImage
#undef AddJob
#undef PlaySound

/*
================================================================================================

	Common Include Files

================================================================================================
*/

#if !defined( _DEBUG ) && !defined( NDEBUG )
	// don't generate asserts
	#define NDEBUG
#endif

// STD
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <cassert>
#include <ctime>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <climits>

// STL
#include <typeinfo>
#include <memory>

//-----------------------------------------------------

// Hacked stuff we may want to consider implementing later
class idScopedGlobalHeap {
};

#endif // SYS_INCLUDES_H
