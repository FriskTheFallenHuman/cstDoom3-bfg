==============================================================================
CstDoom3-BFG v2.0 Source Code
fva <fvarealm@gmail.com>

  This document is licensed under a Creative Commons Attribution 4.0
  International License. See the file "cc-by-4.0.txt" for the license terms.
  Alternatively, see <http://creativecommons.org/licenses/by/4.0/>.

Created: June 4, 2024

This document contains the following sections:

ABOUT
LICENSE
COMPILING

==============================================================================
[ABOUT]

The "CstDoom3-BFG Source Code" is a modified version of the "Doom 3 BFG
Edition GPL Source Code". The original Doom 3 BFG Edition GPL Source Code is
by id Software (https://github.com/id-Software/DOOM-3-BFG) (retrieved: March
26, 2016). The modifications are by fva (last modified: April 28, 2024).

All modifications in the code have been marked with the comment #modified-fva.
The changes in Visual Studio files, however, are not indicated (if needed, a
diff tool can be used to find them). Also, along with each modified file, you
should find a copy of the corresponding original file (with a ".bak"
extension).

To avoid confusions, the "README.txt" file from the original Doom 3 BFG
Edition GPL Source Code has been renamed to "id.README.txt".

==============================================================================
[LICENSE]

The CstDoom3-BFG Source Code is licensed under the GNU GENERAL PUBLIC LICENSE
Version 3 (GPLv3) plus certain ADDITIONAL TERMS (originally added by id
Software). Refer to the file "COPYING.txt" for the complete license (the
ADDITIONAL TERMS are immediately following the terms of the GPLv3).

The remainder of this section is adapted from "id.README.txt".

EXCLUDED CODE: The code described below and contained in the CstDoom3-BFG
Source Code is not part of the Program covered by the GPLv3 and is expressly
excluded from its terms. You are solely responsible for obtaining from the
copyright holder a license for such code and complying with the applicable
license terms.

JPEG library
------------------------------------------------------------------------------
neo/_libs/jpeg-6/*

Copyright (C) 1991-1995, Thomas G. Lane

Permission is hereby granted to use, copy, modify, and distribute this
software (or portions thereof) for any purpose, without fee, subject to these
conditions:
(1) If any part of the source code for this software is distributed, then this
README file must be included, with this copyright and no-warranty notice
unaltered; and any additions, deletions, or changes to the original files
must be clearly indicated in accompanying documentation.
(2) If only executable code is distributed, then the accompanying
documentation must state that "this software is based in part on the work of
the Independent JPEG Group".
(3) Permission for use of this software is granted only if the user accepts
full responsibility for any undesirable consequences; the authors accept
NO LIABILITY for damages of any kind.

These conditions apply to any software derived from or based on the IJG code,
not just to the unmodified library.  If you use our work, you ought to
acknowledge us.

NOTE: The file "id.README.txt" mentions the following:

  "unfortunately the README that came with our copy of the library has
  been lost, so the one from release 6b is included instead. There are a few
  'glue type' modifications to the library to make it easier to use from
  the engine, but otherwise the dependency can be easily cleaned up to a
  better release of the library".

zlib library
------------------------------------------------------------------------------
neo/_libs/zlib/*

Copyright (C) 1995-2005 Jean-loup Gailly and Mark Adler

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
 claim that you wrote the original software. If you use this software
 in a product, an acknowledgment in the product documentation would be
 appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
 misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

Base64 implementation
------------------------------------------------------------------------------
neo/idlib/Base64.cpp

Copyright (c) 1996 Lars Wirzenius.  All rights reserved.

June 14 2003: TTimo <ttimo@idsoftware.com>
	modified + endian bug fixes
	http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=197039

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

IO for uncompress .zip files using zlib
------------------------------------------------------------------------------
neo/framework/Unzip.cpp
neo/framework/Unzip.h

Copyright (C) 1998 Gilles Vollant
zlib is Copyright (C) 1995-1998 Jean-loup Gailly and Mark Adler

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

MD4 Message-Digest Algorithm
------------------------------------------------------------------------------
neo/idlib/hashing/MD4.cpp
Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD4 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD4 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.

MD5 Message-Digest Algorithm
------------------------------------------------------------------------------
neo/idlib/hashing/MD5.cpp
This code implements the MD5 message-digest algorithm.
The algorithm is due to Ron Rivest.  This code was
written by Colin Plumb in 1993, no copyright is claimed.
This code is in the public domain; do with it what you wish.

CRC32 Checksum
------------------------------------------------------------------------------
neo/idlib/hashing/CRC32.cpp
Copyright (C) 1995-1998 Mark Adler

OpenGL headers
------------------------------------------------------------------------------
neo/renderer/OpenGL/glext.h
neo/renderer/OpenGL/wglext.h

Copyright (c) 2007-2012 The Khronos Group Inc.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and/or associated documentation files (the
"Materials"), to deal in the Materials without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Materials, and to
permit persons to whom the Materials are furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Materials.

THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.

Timidity
------------------------------------------------------------------------------
doomclassic/timidity/*

Copyright (c) 1995 Tuukka Toivonen

From http://www.cgs.fi/~tt/discontinued.html :

If you'd like to continue hacking on TiMidity, feel free. I'm
hereby extending the TiMidity license agreement: you can now
select the most convenient license for your needs from (1) the
GNU GPL, (2) the GNU LGPL, or (3) the Perl Artistic License.

libbinkdec
------------------------------------------------------------------------------
neo/_libs/libbinkdec/*

libbinkdec is by Barry Duncan (http://homepage.eircom.net/~duncandsl/avp)
(retrieved: January 28, 2023) and is based on FFmpeg (https://ffmpeg.org).

The version of libbinkdec included with the CstDoom3-BFG Source Code has been
modified by fva to fix a few issues and to make other suitable changes (last
modified: April 28, 2024). All modifications in the code have been marked with
the comment #modified-fva. Also, along with each modified file, you should
find a copy of the corresponding original file (with a ".bak" extension).

libbinkdec is licensed under the GNU LESSER GENERAL PUBLIC LICENSE Version
2.1 (LGPLv2.1), or, at your option, any later version. See the file
"neo/_libs/libbinkdec/COPYING" for the LGPLv2.1 terms.

Note: The project file "neo/_libs/libbinkdec.vcxproj" is by fva and can be
used under the same license terms of those of libbinkdec (see above).

libogg
------------------------------------------------------------------------------
neo/_libs/libogg/*

The file "COPYING" of libogg has the following contents:

  Copyright (c) 2002, Xiph.org Foundation

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  - Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  - Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  - Neither the name of the Xiph.org Foundation nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION
  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Note: The project file "neo/_libs/libogg.vcxproj" is by fva and is based on
the project files available in "neo/_libs/libogg/win32". This project file
can be used and distributed according to the same license terms as those of
libogg (see above).

libtheora
------------------------------------------------------------------------------
neo/_libs/libtheora/*

The file "LICENSE" of libtheora has the following contents:

  Please see the file COPYING for the copyright license for this software.

  In addition to and irrespective of the copyright license associated
  with this software, On2 Technologies, Inc. makes the following statement
  regarding technology used in this software:

    On2 represents and warrants that it shall not assert any rights
    relating to infringement of On2's registered patents, nor initiate
    any litigation asserting such rights, against any person who, or
    entity which utilizes the On2 VP3 Codec Software, including any
    use, distribution, and sale of said Software; which make changes,
    modifications, and improvements in said Software; and to use,
    distribute, and sell said changes as well as applications for other
    fields of use.

  This reference implementation is originally derived from the On2 VP3
  Codec Software, and the Theora video format is essentially compatible
  with the VP3 video format, consisting of a backward-compatible superset.

The file "COPYING" of libtheora has the following contents:

  Copyright (C) 2002-2009 Xiph.org Foundation

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  - Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  - Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  - Neither the name of the Xiph.org Foundation nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION
  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Note: The project file "neo/_libs/libtheora.vcxproj" is by fva and is based on
the project files available in "neo/_libs/libtheora/win32". This project file
can be used and distributed according to the same license terms as those of
libtheora (see above).

precise_sleep
------------------------------------------------------------------------------

The "CstTimeHiRes" class in "neo/sys/win32/win_main.cpp" contains code adapted
from Blat Blatnik's "precise_sleep.c" sample (https://github.com/blat-blatnik/
Snippets/blob/main/precise_sleep.c) (retrieved: April 10, 2024).

The "precise_sleep.c" sample is licensed under the Unlicense license (https://
github.com/blat-blatnik/Snippets/blob/main/LICENSE) (retrieved: April 10,
2024). The terms of the Unlicense are reproduced below:

  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or
  distribute this software, either in source code form or as a compiled
  binary, for any purpose, commercial or non-commercial, and by any
  means.

  In jurisdictions that recognize copyright laws, the author or authors
  of this software dedicate any and all copyright interest in the
  software to the public domain. We make this dedication for the benefit
  of the public at large and to the detriment of our heirs and
  successors. We intend this dedication to be an overt act of
  relinquishment in perpetuity of all present and future rights to this
  software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.

  For more information, please refer to <https://unlicense.org>

rapidjson
------------------------------------------------------------------------------
neo/_libs/rapidjson/*

The file "license.txt" of rapidjson has the following contents:

  Tencent is pleased to support the open source community by making RapidJSON
  available.

  Copyright (C) 2015 THL A29 Limited, a Tencent company, and Milo Yip.
  All rights reserved.

  If you have downloaded a copy of the RapidJSON binary from Tencent, please
  note that the RapidJSON binary is licensed under the MIT License.

  If you have downloaded a copy of the RapidJSON source code from Tencent,
  please note that RapidJSON source code is licensed under the MIT License,
  except for the third-party components listed below which are subject to
  different license terms. Your integration of RapidJSON into your own
  projects may require compliance with the MIT License, as well as the other
  licenses applicable to the third-party components included within RapidJSON.
  To avoid the problematic JSON license in your own projects, it's sufficient
  to exclude the bin/jsonchecker/ directory, as it's the only code under the
  JSON license.

  A copy of the MIT License is included in this file.

  Other dependencies and licenses:

  Open Source Software Licensed Under the BSD License:
  ----------------------------------------------------

  The msinttypes r29
  Copyright (c) 2006-2013 Alexander Chemeris
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  * Neither the name of  copyright holder nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  Open Source Software Licensed Under the JSON License:
  -----------------------------------------------------

  json.org
  Copyright (c) 2002 JSON.org
  All Rights Reserved.

  JSON_checker
  Copyright (c) 2002 JSON.org
  All Rights Reserved.

  Terms of the JSON License:
  --------------------------

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  The Software shall be used for Good, not Evil.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.

  Terms of the MIT License:
  -------------------------

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.

==============================================================================
[COMPILING]

Compiling for Windows
---------------------

Compiling for Windows requires Visual Studio 2019 or 2022 (Community Edition is fine).
The Windows SDK 10.0.16299.0 is also needed (it can be obtained using the
Visual Studio installer).

The solution file is "neo/doom3.sln". The resulting binary is "cstdoom3-bfg.exe".

==============================================================================
