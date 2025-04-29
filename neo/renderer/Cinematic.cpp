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


extern idCVar s_noSound;

#include "tr_local.h"
//#modified-fva; BEGIN
#include <theora/theoradec.h>
#if CST_LIBBINKDEC
#include "_libs/libbinkdec/include/libbinkdec.h"
#endif

const int CST_LOOPING_THRESHOLD_MSEC = 10000;

/*
==================
CstCinematic
==================
*/
class CstCinematic {
public:
									CstCinematic();
	virtual							~CstCinematic() {}

	static void						InitCinematic();
	static void						ShutdownCinematic();
	static bool						Enabled() { return image[0] != NULL; }

protected:
	struct UploadData {
		void SetData (unsigned char * _bytes, int _width, int _height, int _stride) {
			bytes = _bytes;
			width = _width;
			height = _height;
			stride = _stride;
		}
		unsigned char *				bytes;
		int							width;
		int							height;
		int							stride;
	};
	void							UploadToGPU(UploadData data[3]);

	static idImage *				image[3];
	static unsigned					cin_count;
	static unsigned					lastUploaded_cinId;

	const unsigned int				cinId;
};

// ===============
CstCinematic::CstCinematic() : cinId(++cin_count) {
	assert(idLib::IsMainThread());
}

// ===============
idImage *CstCinematic::image[3] = { NULL, NULL, NULL };
unsigned CstCinematic::cin_count = 0;
unsigned CstCinematic::lastUploaded_cinId = 0;

// ===============
void CstCinematic::InitCinematic() {
	if (!R_IsInitialized()) {
		return;
	}

	image[0] = globalImages->AllocStandaloneImage("_cinematic_Y");
	image[1] = globalImages->AllocStandaloneImage("_cinematic_Cb");
	image[2] = globalImages->AllocStandaloneImage("_cinematic_Cr");

	if (!image[0] || !image[1] || !image[2]) {
		ShutdownCinematic();
	}

	for (int i = 0; i < 3; ++i) {
		image[i]->opts.textureType = TT_2D;
		image[i]->opts.format = FMT_INT8;
		image[i]->opts.width = 16;
		image[i]->opts.height = 16;
		image[i]->opts.numLevels = 1;

		image[i]->filter = TF_LINEAR;
		image[i]->repeat = TR_REPEAT;
		image[i]->usage = TD_LOOKUP_TABLE_MONO;

		image[i]->AllocImage();
	}

	cin_count = 0;
	lastUploaded_cinId = 0;
}

// ===============
void CstCinematic::ShutdownCinematic() {
	if (!R_IsInitialized()) {
		return;
	}
	for (int i = 0; i < 3; ++i) {
		if (image[i]) {
			image[i]->PurgeImage();
			delete image[i];
			image[i] = NULL;
		}
	}
}

// ===============
void CstCinematic::UploadToGPU(UploadData data[3]) {
	lastUploaded_cinId = cinId;

	for (int i = 0; i < 3; ++i) {
		UploadData & inData = data[i];
		idImage * img = image[i];

		if (img->opts.width != inData.width || img->opts.height != inData.height) {
			img->opts.width = inData.width;
			img->opts.height = inData.height;
			img->AllocImage();
		}

		img->SetSamplerState(TF_LINEAR, TR_REPEAT);
		img->SubImageUpload(0, 0, 0, 0, inData.width, inData.height, inData.bytes, inData.stride);
	}
}

/*
==================
CstCinematicBink
==================
*/
#if CST_LIBBINKDEC
class CstCinematicBink : public CstCinematic {
public:
									CstCinematicBink(const idStr & _fileName);
	virtual							~CstCinematicBink();

	bool							InitFromFile(const char * _fileData, int _fileLength, bool _looping);
	cinData_t						ImageForTime(int thisTime);
	void							ResetTime(int time);
	void							CloseNoLock();

	int								AnimationLength() { return animationLengthMsec; }
	int								GetStartTime() { return startTime; }
	float							GetFrameRate() const { return frameRate; }

	bool							CstIsPlaying() { return status == FMV_PLAY; }
	void							CstUpdateNoLock(int thisTime, bool forceLoopingUpdate = false);

private:
	void							UploadImages();
	bool							Reset();

	YUVbuffer						yuv;

	const idStr &					fileName;
	const char *					iFileData;
	int								iFileLength;
	BinkHandle *					pBinkHandle;

	int								frameWidth;
	int								frameHeight;
	int								numFrames;
	int								currentFrame;
	cinStatus_t						status;
	int								animationLengthMsec;
	float							frameRate;
	int								startTime;
	bool							looping;
	int								updatedTime;
	int								lastSeenTime;
	bool							uploadPending;
};

// ===============
CstCinematicBink::CstCinematicBink(const idStr & _fileName) : fileName(_fileName) {
	memset(&yuv, 0, sizeof(yuv));
	iFileData = NULL;
	iFileLength = 0;
	pBinkHandle = NULL;
	animationLengthMsec = 0;
	frameRate = 0.0f;
	startTime = -1;
	status = FMV_EOF;
}

// ===============
CstCinematicBink::~CstCinematicBink() {
	CloseNoLock();
}

// ===============
void CstCinematicBink::CloseNoLock() {
	if (pBinkHandle) {
		Bink_Close(pBinkHandle);
		pBinkHandle = NULL;
	}
	iFileData = NULL;
	iFileLength = 0;
	memset(&yuv, 0, sizeof(yuv));
	status = FMV_EOF;
}

// ===============
void CstCinematicBink::UploadImages() {
	if (!yuv[0].data || !yuv[1].data || !yuv[2].data) {
		return;
	}

	UploadData data[3];
	for (int i = 0; i < 3; ++i) {
		data[i].SetData(yuv[i].data, yuv[i].widthDisplay, yuv[i].heightDisplay, yuv[i].pitch);
	}

	UploadToGPU(data);
	uploadPending = false;
}

// ===============
bool CstCinematicBink::InitFromFile(const char * _fileData, int _fileLength, bool _looping) {
	if (!_fileData || _fileLength <= 0) {
		return false;
	}
	CloseNoLock();

	iFileData = _fileData;
	iFileLength = _fileLength;

	if (Bink_Init((const uint8_t *)iFileData, iFileLength, &pBinkHandle) < 1) {
		CloseNoLock();
		return false;
	}
	uint32_t _numFrames;
	if (Bink_GetNumFrames(pBinkHandle, &_numFrames) < 1 || _numFrames == 0) {
		CloseNoLock();
		return false;
	}

	if (Bink_GotoFrame(pBinkHandle, 0) < 1) {
		CloseNoLock();
		return false;
	}
	uint32_t _currentFrame;
	if (Bink_GetNextFrame_VideoOnly(pBinkHandle, yuv, &_currentFrame) < 1) { // decode first frame
		CloseNoLock();
		return false;
	}

	uint32_t _frameWidth;
	uint32_t _frameHeight;
	if (Bink_GetFrameSize(pBinkHandle, &_frameWidth, &_frameHeight) < 1) {
		CloseNoLock();
		return false;
	}

	float _frameRate;
	if (Bink_GetFrameRate(pBinkHandle, &_frameRate) < 1) {
		CloseNoLock();
		return false;
	}

	frameWidth = (int)_frameWidth;
	frameHeight = (int)_frameHeight;
	numFrames = (int)_numFrames;
	currentFrame = (int)_currentFrame;
	frameRate = _frameRate;
	animationLengthMsec = (1000.0f * _numFrames) / _frameRate;
	startTime = -1;
	looping = _looping;
	updatedTime = -1;
	lastSeenTime = -1;
	uploadPending = false;

	status = (_looping) ? FMV_PLAY : FMV_IDLE;

	if (lastUploaded_cinId == cinId) {
		lastUploaded_cinId = 0;
	}

	return true;
}

// ===============
bool CstCinematicBink::Reset() {
	if (Bink_GotoFrame(pBinkHandle, 0) < 1) {
		return false;
	}
	uint32_t _currentFrame;
	if (Bink_GetNextFrame_VideoOnly(pBinkHandle, yuv, &_currentFrame) < 1) { // decode first frame
		return false;
	}
	currentFrame = (int)_currentFrame;
	return true;
}

// ===============
void CstCinematicBink::CstUpdateNoLock(int thisTime, bool forceLoopingUpdate) {
	if (status == FMV_EOF || status == FMV_IDLE) {
		return;
	}

	if (looping && (lastSeenTime < 0 || thisTime - lastSeenTime > CST_LOOPING_THRESHOLD_MSEC)) {
		if (!forceLoopingUpdate) {
			return;
		}
		if (currentFrame != 0) {
			if (!Reset()) {
				CloseNoLock();
				return;
			}
		}
		uploadPending = true;
		startTime = thisTime;
		updatedTime = thisTime;
		return;
	}

	updatedTime = thisTime;

	if (startTime == -1) {
		if (currentFrame != 0) {
			if (!Reset()) {
				CloseNoLock();
				return;
			}
		}
		uploadPending = true; // force 1st upload
		startTime = thisTime;
	}

	int targetFrame = (int)((float)(thisTime - startTime) * frameRate * 0.001f);

	if (targetFrame >= numFrames) {
		// video finished
		if (looping) {
			if (currentFrame != 0) {
				if (!Reset()) {
					CloseNoLock();
					return;
				}
			}
			uploadPending = true;
			startTime = thisTime;
			return;
		} else {
			status = FMV_IDLE;
			return;
		}
	}

	if (targetFrame < 0) {
		targetFrame = 0;
	}

	if (targetFrame < currentFrame) {
		if (!Reset()) {
			CloseNoLock();
			return;
		}
		uploadPending = true;
	}

	if (targetFrame == currentFrame) {
		return; // frame already computed
	}

	while (currentFrame < targetFrame) {
		uint32_t _currentFrame;
		if (Bink_GetNextFrame_VideoOnly(pBinkHandle, yuv, &_currentFrame) < 1) {
			CloseNoLock();
			return;
		}
		currentFrame = (int)_currentFrame;
	}

	uploadPending = true;
}

// ===============
cinData_t CstCinematicBink::ImageForTime(int thisTime) {
	if (updatedTime != thisTime) {
		CstUpdateNoLock(thisTime, true);
		if (status == FMV_EOF || status == FMV_IDLE) {
			cinData_t cinData;
			memset(&cinData, 0, sizeof(cinData));
			return cinData;
		}
	}

	if (lastUploaded_cinId != cinId || uploadPending) {
		UploadImages();
	}
	cinData_t cinData;
	cinData.imageY = image[0];
	cinData.imageCb = image[1];
	cinData.imageCr = image[2];
	cinData.imageWidth = frameWidth;
	cinData.imageHeight = frameHeight;
	cinData.status = status;

	lastSeenTime = thisTime;

	return cinData;
}

// ===============
void CstCinematicBink::ResetTime(int time) {
	if (status == FMV_EOF) {
		// FMV_EOF means "in error" in this implementation
		return;
	}
	startTime = time;
	status = FMV_PLAY;
}

/*
==================
CstCinematicBinkComposite
==================
*/
class CstCinematicBinkComposite : public idCinematic {
public:
									CstCinematicBinkComposite();
	virtual							~CstCinematicBinkComposite();

	virtual bool					InitFromFile(const char *qpath, bool _looping);
	virtual cinData_t				ImageForTime(int thisTime);
	virtual void					CstResetTime_Game(int time);
	virtual void					CstResetTime_Sys(int time);
	virtual void					Close() { idScopedCriticalSection cs(stateMutex); CloseNoLock(); }

	virtual int						AnimationLength() { idScopedCriticalSection cs(stateMutex); return decoderLocal.AnimationLength(); }
	virtual int						CstGetStartTime_Game();
	virtual int						CstGetStartTime_Sys();
	virtual float					GetFrameRate() const { idScopedCriticalSection cs(stateMutex); return decoderLocal.GetFrameRate(); }

	virtual bool					CstIsPlaying_Game();
	virtual bool					CstIsPlaying_Sys();
	virtual void					CstUpdate(int thisTime);

private:
	void							CloseNoLock();
	CstCinematicBink *				NewDecoder();

	mutable idSysMutex				stateMutex;

	CstCinematicBink *				decoderGame;		// decoderGame: idStaticEntity videos
	CstCinematicBink *				decoderSys;			// decoderSys: menus, pda, and testVideo videos

	CstCinematicBink				decoderLocal;		// assigned to the first of decoderGame or decoderSys that is used (the other one uses decoderDynamic if needed)
	bool							decoderLocalInUse;
	CstCinematicBink *				decoderDynamic;		// allocated on-the-fly when decoderLocal is already in use

	idStr							fileName;
	idFile_Memory *					iFile;
	bool							looping;

	int								deferredStartTime_Game;
	int								deferredStartTime_Sys;
	bool							ready;
};

// ===============
CstCinematicBinkComposite::CstCinematicBinkComposite() : decoderLocal(fileName) {
	decoderGame = NULL;
	decoderSys = NULL;
	decoderLocalInUse = false;
	decoderDynamic = NULL;
	iFile = NULL;
	deferredStartTime_Game = -1;
	deferredStartTime_Sys = -1;
	ready = false;
}

// ===============
CstCinematicBinkComposite::~CstCinematicBinkComposite() {
	CloseNoLock();
}

// ===============
void CstCinematicBinkComposite::CloseNoLock() {
	decoderGame = NULL;
	decoderSys = NULL;
	decoderLocal.CloseNoLock();
	decoderLocalInUse = false;
	if (decoderDynamic) {
		delete decoderDynamic;
		decoderDynamic = NULL;
	}
	if (iFile) {
		fileSystem->CloseFile(iFile);
		iFile = NULL;
	}
	deferredStartTime_Game = -1;
	deferredStartTime_Sys = -1;
	ready = false;
}

// ===============
CstCinematicBink * CstCinematicBinkComposite::NewDecoder() {
	if (!decoderLocalInUse) {
		decoderLocalInUse = true;
		return &decoderLocal;
	} else if (!decoderDynamic) {
		CstCinematicBink * newDecoder = new CstCinematicBink(fileName);
		if (!newDecoder) {
			return NULL;
		}
		if (!newDecoder->InitFromFile(iFile->GetDataPtr(), iFile->Length(), looping)) {
			delete newDecoder;
			return NULL;
		}
		decoderDynamic = newDecoder;
		return decoderDynamic;
	}
	return NULL;
}

// ===============
int CstCinematicBinkComposite::CstGetStartTime_Game() {

	idScopedCriticalSection cs(stateMutex);

	if (deferredStartTime_Game >= 0) {
		return deferredStartTime_Game;
	}
	if (decoderGame) {
		return decoderGame->GetStartTime();
	}
	return -1;
}

// ===============
int CstCinematicBinkComposite::CstGetStartTime_Sys() {

	idScopedCriticalSection cs(stateMutex);

	if (deferredStartTime_Sys >= 0) {
		return deferredStartTime_Sys;
	}
	if (decoderSys) {
		return decoderSys->GetStartTime();
	}
	return -1;
}

// ===============
bool CstCinematicBinkComposite::CstIsPlaying_Game() {

	idScopedCriticalSection cs(stateMutex);

	if (deferredStartTime_Game >= 0) {
		return true;
	}
	if (decoderGame) {
		return decoderGame->CstIsPlaying();
	}
	return false;
}

// ===============
bool CstCinematicBinkComposite::CstIsPlaying_Sys() {

	idScopedCriticalSection cs(stateMutex);

	if (deferredStartTime_Sys >= 0) {
		return true;
	}
	if (decoderSys) {
		return decoderSys->CstIsPlaying();
	}
	return false;
}

// ===============
bool CstCinematicBinkComposite::InitFromFile(const char *qpath, bool _looping) {
	assert(idLib::IsMainThread());

	if (!CstCinematicBink::Enabled()) {
		return false;
	}

	idScopedCriticalSection cs(stateMutex);

	CloseNoLock();

	if (strstr(qpath, "/") == NULL && strstr(qpath, "\\") == NULL) {
		sprintf(fileName, "video/%s", qpath);
	} else {
		sprintf(fileName, "%s", qpath);
	}
	fileName.SetFileExtension(".bik");

	iFile = (idFile_Memory *)fileSystem->CstOpenFileRead_FileMemory(fileName.c_str());
	if (!iFile) {
		CloseNoLock();
		return false;
	}

	if (!decoderLocal.InitFromFile(iFile->GetDataPtr(), iFile->Length(), _looping)) {
		CloseNoLock();
		return false;
	}

	looping = _looping;
	ready = true;

	return true;
}

// ===============
void CstCinematicBinkComposite::CstUpdate(int thisTime) {
	assert(idLib::IsMainThread());

	idScopedCriticalSection cs(stateMutex);

	if (!ready) {
		return;
	}

	// only decoderGame should receive updates
	if (!decoderGame) {
		decoderGame = NewDecoder();
		if (!decoderGame) {
			deferredStartTime_Game = -1;
			return;
		}
	}
	if (deferredStartTime_Game >= 0) {
		decoderGame->ResetTime(deferredStartTime_Game);
		deferredStartTime_Game = -1;
	}
	decoderGame->CstUpdateNoLock(thisTime);
}

// ===============
cinData_t CstCinematicBinkComposite::ImageForTime(int thisTime) {
	assert(idLib::IsMainThread());

	idScopedCriticalSection cs(stateMutex);

	if (!ready) {
		cinData_t data;
		memset(&data, 0, sizeof(data));
		return data;
	}

	bool useDecoderGame = true;
	if (thisTime <= 0 || tr.testVideo == this) {
		useDecoderGame = false;
	}
	if (useDecoderGame) {
		if (!decoderGame) {
			decoderGame = NewDecoder();
			if (!decoderGame) {
				deferredStartTime_Game = -1;
				cinData_t data;
				memset(&data, 0, sizeof(data));
				return data;
			}
		}
		if (deferredStartTime_Game >= 0) {
			decoderGame->ResetTime(deferredStartTime_Game);
			deferredStartTime_Game = -1;
		}
	} else {
		if (!decoderSys) {
			decoderSys = NewDecoder();
			if (!decoderSys) {
				deferredStartTime_Sys = -1;
				cinData_t data;
				memset(&data, 0, sizeof(data));
				return data;
			}
		}
		if (deferredStartTime_Sys >= 0) {
			decoderSys->ResetTime(deferredStartTime_Sys);
			deferredStartTime_Sys = -1;
		}
	}

	cinData_t data;
	if (useDecoderGame) {
		data = decoderGame->ImageForTime(thisTime);
	} else {
		int time = Sys_Milliseconds();
		data = decoderSys->ImageForTime(time);
	}
	return data;
}

// ===============
void CstCinematicBinkComposite::CstResetTime_Game(int time) {
	if (time < 0) {
		return;
	}

	idScopedCriticalSection cs(stateMutex);

	if (!ready) {
		return;
	}
	deferredStartTime_Game = time;
}

// ===============
void CstCinematicBinkComposite::CstResetTime_Sys(int time) {
	if (time < 0) {
		return;
	}

	idScopedCriticalSection cs(stateMutex);

	if (!ready) {
		return;
	}
	deferredStartTime_Sys = time;
}
#endif

/*
==================
CstCinematicTheora
==================
*/
class CstCinematicTheora : public CstCinematic {
public:
	struct keyframeInfo_t {
		int fileOffset;
		int frameIndex;
	};
	typedef idList<keyframeInfo_t>	KeyframeTable;

									CstCinematicTheora(const idStr & _fileName, KeyframeTable & _keyframeTable);
	virtual							~CstCinematicTheora();

	bool							InitFromFile(const char * _fileData, int _fileLength, bool _looping, bool _initKeyframeTable);
	cinData_t						ImageForTime(int thisTime);
	void							ResetTime(int time);
	void							CloseNoLock();

	int								AnimationLength() { return animationLengthMsec; }
	int								GetStartTime() { return startTime; }
	float							GetFrameRate() const { return frameRate; }

	bool							CstIsPlaying() { return status == FMV_PLAY; }
	void							CstUpdateNoLock(int thisTime, bool forceLoopingUpdate = false);

private:
	void							UploadImages();
	int64							ReadInt64();
	void							ReadPacket();
	int								DecodeFrame();
	bool							Reset();
	int								SeekKeyframe(int targetFrame);

	th_ycbcr_buffer					ycbcr;

	const idStr &					fileName;
	idFile_Memory					iFile;

	th_info							thInfo;
	th_dec_ctx *					thDecCtx;
	ogg_packet						thPacket;

	KeyframeTable &					keyframeTable;
	int								firstDataPacketOffset;
	int								numFrames;

	int								currentFrame;
	int								currentKeyframe;
	cinStatus_t						status;
	int								animationLengthMsec;
	float							frameRate;
	int								startTime;
	bool							looping;
	int								updatedTime;
	int								lastSeenTime;
	bool							uploadPending;
};

// ===============
CstCinematicTheora::CstCinematicTheora(const idStr & _fileName, KeyframeTable & _keyframeTable) : fileName(_fileName), keyframeTable(_keyframeTable) {
	iFile.SetData(NULL, 0);
	memset(&thInfo, 0, sizeof(thInfo));
	thDecCtx = NULL;
	memset(&ycbcr, 0, sizeof(ycbcr));
	animationLengthMsec = 0;
	frameRate = 0.0f;
	startTime = -1;
	status = FMV_EOF;
}

// ===============
CstCinematicTheora::~CstCinematicTheora() {
	CloseNoLock();
}

// ===============
void CstCinematicTheora::CloseNoLock() {
	th_info_clear(&thInfo);
	if (thDecCtx) {
		th_decode_free(thDecCtx);
		thDecCtx = NULL;
	}
	iFile.SetData(NULL, 0);
	memset(&ycbcr, 0, sizeof(ycbcr));
	status = FMV_EOF;
}

// ===============
void CstCinematicTheora::UploadImages() {
	if (!ycbcr[0].data || !ycbcr[1].data || !ycbcr[2].data) {
		return;
	}

	UploadData data[3];
	for (int i = 0; i < 3; ++i) {
		int hdec = i != 0 && !(thInfo.pixel_fmt & 1);
		int vdec = i != 0 && !(thInfo.pixel_fmt & 2);

		int pic_x = thInfo.pic_x >> hdec;
		int pic_y = thInfo.pic_y >> vdec;
		int pic_width = thInfo.pic_width >> hdec;
		int pic_height = thInfo.pic_height >> vdec;

		unsigned char * pic_data = ycbcr[i].data + pic_x + ycbcr[i].stride * pic_y;

		data[i].SetData(pic_data, pic_width, pic_height, ycbcr[i].stride);
	}

	UploadToGPU(data);
	uploadPending = false;
}

// ===============
int64 CstCinematicTheora::ReadInt64() {
	int64 ret = 0;
	byte bytes[8];
	iFile.Read(&bytes, 8);
	ret |= bytes[7];
	ret = (ret << 8) | bytes[6];
	ret = (ret << 8) | bytes[5];
	ret = (ret << 8) | bytes[4];
	ret = (ret << 8) | bytes[3];
	ret = (ret << 8) | bytes[2];
	ret = (ret << 8) | bytes[1];
	ret = (ret << 8) | bytes[0];
	return ret;
}

// ===============
void CstCinematicTheora::ReadPacket() {
	int packetSize;
	iFile.ReadInt(packetSize);
	byte bos_eos;
	iFile.ReadUnsignedChar(bos_eos);
	int64 granulepos = ReadInt64();
	unsigned packetno;
	iFile.ReadUnsignedInt(packetno);

	int dataSize = packetSize - 17;
	assert(dataSize >= 0);
	unsigned char *data = (unsigned char *)iFile.GetDataPtr() + iFile.Tell();
	iFile.Seek(dataSize, FS_SEEK_CUR);

	thPacket.packet = data;
	thPacket.bytes = dataSize;
	thPacket.b_o_s = ((bos_eos & 2) != 0) ? 1 : 0;
	thPacket.e_o_s = ((bos_eos & 1) != 0) ? 1 : 0;
	thPacket.granulepos = granulepos;
	thPacket.packetno = packetno;
}

// ===============
int CstCinematicTheora::DecodeFrame() {
	// return values:
	//  0 = new frame available; contents changed
	//  1 = new frame is a dropped frame; contents didn't change
	// -1 = error

	int ret = th_decode_packetin(thDecCtx, &thPacket, NULL);
	if (ret == 0) {
		th_decode_ycbcr_out(thDecCtx, ycbcr);
		return 0;
	} else if (ret == TH_DUPFRAME) {
		return 1;
	}
	return -1;
}

// ===============
bool CstCinematicTheora::InitFromFile(const char * _fileData, int _fileLength, bool _looping, bool _initKeyframeTable) {
	if (!_fileData || _fileLength <= 0) {
		return false;
	}
	CloseNoLock();

	iFile.SetData(_fileData, _fileLength);
	iFile.MakeReadOnly();

	// check signature and version
	byte signature[2];
	unsigned version;
	if (iFile.ReadUnsignedChar(signature[0]) != 1
		|| signature[0] != 'T'
		|| iFile.ReadUnsignedChar(signature[1]) != 1
		|| signature[1] != 'H'
		|| iFile.ReadUnsignedInt(version) != 4
		|| version != 1) { // expecting version 1
		CloseNoLock();
		return false;
	}

	// read the info preceding the headers
	iFile.ReadInt(numFrames);
	if (numFrames == 0) {
		CloseNoLock();
		return false;
	}
	int keyframeTableOffset;
	iFile.ReadInt(keyframeTableOffset);
	iFile.ReadInt(firstDataPacketOffset);

	// read the keyframeTable
	if (_initKeyframeTable) {
		int firstHeaderOffset = iFile.Tell();
		iFile.Seek(keyframeTableOffset, FS_SEEK_SET);
		int numKeyframes = 0;
		iFile.ReadInt(numKeyframes);
		if (numKeyframes == 0) {
			// at least the 1st frame should be a keyframe
			CloseNoLock();
			return false;
		}
		keyframeTable.Clear();
		keyframeTable.SetNum(numKeyframes);
		for (int i = 0; i < numKeyframes; ++i) {
			iFile.ReadInt(keyframeTable[i].fileOffset);
			iFile.ReadInt(keyframeTable[i].frameIndex);
		}
		iFile.Seek(firstHeaderOffset, FS_SEEK_SET);
	}

	// read the first theora header
	th_comment thComment;
	th_setup_info *thSetup = NULL;

	th_comment_init(&thComment);
	th_info_init(&thInfo);

	ReadPacket();
	if (th_decode_headerin(&thInfo, &thComment, &thSetup, &thPacket) <= 0) {
		CloseNoLock();
		return false;
	}

	// read the remaining headers
	while (true) {
		ReadPacket();
		int ret = th_decode_headerin(&thInfo, &thComment, &thSetup, &thPacket);
		if (ret == 0) {
			break; // done
		} else if (ret < 0) {
			CloseNoLock();
			return false;
		}
	}
	// at this point, thPacket contains the first data packet ready to be decoded

	// init theora decoder
	thDecCtx = th_decode_alloc(&thInfo, thSetup);

	// comment info and setup info are no longer needed
	th_comment_clear(&thComment);
	th_setup_free(thSetup);
	thSetup = NULL;

	if (!thDecCtx) {
		CloseNoLock();
		return false;
	}

	if (thInfo.pixel_fmt >= TH_PF_NFORMATS || thInfo.pixel_fmt == TH_PF_RSVD) {
		// unknown pixel format
		CloseNoLock();
		return false;
	}

	bool needsCropping = (thInfo.pic_width != thInfo.frame_width) || (thInfo.pic_height != thInfo.frame_height);
	if (needsCropping) {
		int hdec = !(thInfo.pixel_fmt & 1);
		int vdec = !(thInfo.pixel_fmt & 2);
		if ((thInfo.pic_x & hdec) || (thInfo.pic_width & hdec) || (thInfo.pic_y & vdec) || (thInfo.pic_height & vdec)) {
			// cropping 4:2:2 requires pic_x and pic_width to be even (divisible by 2)
			// cropping 4:2:0 requires all offsets/sizes to be even (divisible by 2)
			CloseNoLock();
			return false;
		}
	}

	// get first frame
	if (DecodeFrame() < 0) {
		CloseNoLock();
		return false;
	}
	currentFrame = 0;
	currentKeyframe = 0;

	frameRate = ((float)(thInfo.fps_numerator)) / ((float)(thInfo.fps_denominator));
	animationLengthMsec = (1000.0f * numFrames) / frameRate;
	startTime = -1;
	looping = _looping;
	updatedTime = -1;
	lastSeenTime = -1;
	uploadPending = false;

	status = (looping) ? FMV_PLAY : FMV_IDLE;

	if (lastUploaded_cinId == cinId) {
		lastUploaded_cinId = 0;
	}

	return true;
}

// ===============
bool CstCinematicTheora::Reset() {
	// returns false on error

	iFile.Seek(firstDataPacketOffset, FS_SEEK_SET);
	ReadPacket();
	if (DecodeFrame() < 0) {
		return false;
	}
	currentFrame = 0;
	currentKeyframe = 0;
	return true;
}

// ===============
int CstCinematicTheora::SeekKeyframe(int targetFrame) {
	// return values:
	//  0 = the current keyframe and the current frame changed
	//	1 = no changes
	// -1 = error

	int tableIndex;
	if (targetFrame < currentFrame) {
		tableIndex = 0;
		for (int i = currentKeyframe; i > 0; --i) {
			if (keyframeTable[i].frameIndex < targetFrame) {
				tableIndex = i;
				break;
			}
		}
	} else {
		tableIndex = keyframeTable.Num() - 1;
		for (int i = currentKeyframe + 1; i < keyframeTable.Num(); ++i) {
			if (keyframeTable[i].frameIndex > targetFrame) {
				tableIndex = i - 1;
				break;
			}
		}
	}

	if (tableIndex == currentKeyframe) {
		return 1;
	}
	iFile.Seek(keyframeTable[tableIndex].fileOffset, FS_SEEK_SET);
	ReadPacket();
	if (DecodeFrame() < 0) {
		return -1;
	}
	currentFrame = keyframeTable[tableIndex].frameIndex;
	currentKeyframe = tableIndex;
	return 0;
}

// ===============
void CstCinematicTheora::CstUpdateNoLock(int thisTime, bool forceLoopingUpdate) {
	if (status == FMV_EOF || status == FMV_IDLE) {
		return;
	}

	if (looping && (lastSeenTime < 0 || thisTime - lastSeenTime > CST_LOOPING_THRESHOLD_MSEC)) {
		if (!forceLoopingUpdate) {
			return;
		}
		if (currentFrame != 0) {
			if (!Reset()) {
				CloseNoLock();
				return;
			}
		}
		uploadPending = true;
		startTime = thisTime;
		updatedTime = thisTime;
		return;
	}

	updatedTime = thisTime;

	if (startTime == -1) {
		if (currentFrame != 0) {
			if (!Reset()) {
				CloseNoLock();
				return;
			}
		}
		uploadPending = true; // force 1st upload
		startTime = thisTime;
		return;
	}

	int targetFrame = (int)((float)(thisTime - startTime) * frameRate * 0.001f);

	if (targetFrame >= numFrames) {
		// video finished
		if (looping) {
			if (currentFrame != 0) {
				if (!Reset()) {
					CloseNoLock();
					return;
				}
			}
			uploadPending = true;
			startTime = thisTime;
			return;
		} else {
			status = FMV_IDLE;
			return;
		}
	}

	if (targetFrame < 0) {
		targetFrame = 0;
	}

	if (targetFrame == currentFrame) {
		return; // frame already computed
	}

	if (targetFrame > currentFrame + 2 || targetFrame < currentFrame) {
		int ret = SeekKeyframe(targetFrame);
		if (ret == 0) {
			// current keyframe and current frame changed
			uploadPending = true;
		} else if (ret < 0) {
			// error
			CloseNoLock();
			return;
		}
	}

	while (currentFrame < targetFrame) {
		ReadPacket();
		int ret = DecodeFrame();
		if (ret == 0) {
			// contents changed
			uploadPending = true;
		} else if (ret < 0) {
			// error
			CloseNoLock();
			return;
		}
		++currentFrame;
		if (th_packet_iskeyframe(&thPacket)) {
			++currentKeyframe;
		}
	}
}

// ===============
cinData_t CstCinematicTheora::ImageForTime(int thisTime) {
	if (updatedTime != thisTime) {
		CstUpdateNoLock(thisTime, true);
		if (status == FMV_EOF || status == FMV_IDLE) {
			cinData_t cinData;
			memset(&cinData, 0, sizeof(cinData));
			return cinData;
		}
	}

	if (lastUploaded_cinId != cinId || uploadPending) {
		UploadImages();
	}

	cinData_t cinData;
	cinData.imageY = image[0];
	cinData.imageCb = image[1];
	cinData.imageCr = image[2];
	cinData.imageWidth = thInfo.pic_width;
	cinData.imageHeight = thInfo.pic_height;
	cinData.status = status;

	lastSeenTime = thisTime;

	return cinData;
}

// ===============
void CstCinematicTheora::ResetTime(int time) {
	if (status == FMV_EOF) {
		// FMV_EOF means "in error" in this implementation
		return;
	}
	startTime = time;
	status = FMV_PLAY;
}

/*
==================
CstCinematicTheoraComposite
==================
*/
class CstCinematicTheoraComposite : public idCinematic {
public:
									CstCinematicTheoraComposite();
	virtual							~CstCinematicTheoraComposite();

	virtual bool					InitFromFile(const char *qpath, bool _looping);
	virtual cinData_t				ImageForTime(int thisTime);
	virtual void					CstResetTime_Game(int time);
	virtual void					CstResetTime_Sys(int time);
	virtual void					Close() { idScopedCriticalSection cs(stateMutex); CloseNoLock(); }

	virtual int						AnimationLength() { idScopedCriticalSection cs(stateMutex); return decoderLocal.AnimationLength(); }
	virtual int						CstGetStartTime_Game();
	virtual int						CstGetStartTime_Sys();
	virtual float					GetFrameRate() const { idScopedCriticalSection cs(stateMutex); return decoderLocal.GetFrameRate(); }

	virtual bool					CstIsPlaying_Game();
	virtual bool					CstIsPlaying_Sys();
	virtual bool					CstIsTheora() { return true; }
	virtual void					CstUpdate(int thisTime);

private:
	typedef CstCinematicTheora::KeyframeTable KeyframeTable;

	void							CloseNoLock();
	CstCinematicTheora *			NewDecoder();

	mutable idSysMutex				stateMutex;

	CstCinematicTheora *			decoderGame;		// decoderGame: idStaticEntity videos
	CstCinematicTheora *			decoderSys;			// decoderSys: menus, pda, and testVideo videos

	CstCinematicTheora				decoderLocal;		// assigned to the first of decoderGame or decoderSys that is used (the other one uses decoderDynamic if needed)
	bool							decoderLocalInUse;
	CstCinematicTheora *			decoderDynamic;		// allocated on-the-fly when decoderLocal is already in use

	idStr							fileName;
	idFile_Memory *					iFile;
	KeyframeTable					keyframeTable;
	bool							looping;

	int								deferredStartTime_Game;
	int								deferredStartTime_Sys;
	bool							ready;
};

// ===============
CstCinematicTheoraComposite::CstCinematicTheoraComposite() : decoderLocal(fileName, keyframeTable) {
	decoderGame = NULL;
	decoderSys = NULL;
	decoderLocalInUse = false;
	decoderDynamic = NULL;
	iFile = NULL;
	deferredStartTime_Game = -1;
	deferredStartTime_Sys = -1;
	ready = false;
}

// ===============
CstCinematicTheoraComposite::~CstCinematicTheoraComposite() {
	CloseNoLock();
}

// ===============
void CstCinematicTheoraComposite::CloseNoLock() {
	decoderGame = NULL;
	decoderSys = NULL;
	decoderLocal.CloseNoLock();
	decoderLocalInUse = false;
	if (decoderDynamic) {
		delete decoderDynamic;
		decoderDynamic = NULL;
	}
	if (iFile) {
		fileSystem->CloseFile(iFile);
		iFile = NULL;
	}
	keyframeTable.Clear();
	deferredStartTime_Game = -1;
	deferredStartTime_Sys = -1;
	ready = false;
}

// ===============
CstCinematicTheora * CstCinematicTheoraComposite::NewDecoder() {
	if (!decoderLocalInUse) {
		decoderLocalInUse = true;
		return &decoderLocal;
	} else if (!decoderDynamic) {
		CstCinematicTheora * newDecoder = new CstCinematicTheora(fileName, keyframeTable);
		if (!newDecoder) {
			return NULL;
		}
		if (!newDecoder->InitFromFile(iFile->GetDataPtr(), iFile->Length(), looping, false)) {
			delete newDecoder;
			return NULL;
		}
		decoderDynamic = newDecoder;
		return decoderDynamic;
	}
	return NULL;
}

// ===============
int CstCinematicTheoraComposite::CstGetStartTime_Game() {

	idScopedCriticalSection cs(stateMutex);

	if (deferredStartTime_Game >= 0) {
		return deferredStartTime_Game;
	}
	if (decoderGame) {
		return decoderGame->GetStartTime();
	}
	return -1;
}

// ===============
int CstCinematicTheoraComposite::CstGetStartTime_Sys() {

	idScopedCriticalSection cs(stateMutex);

	if (deferredStartTime_Sys >= 0) {
		return deferredStartTime_Sys;
	}
	if (decoderSys) {
		return decoderSys->GetStartTime();
	}
	return -1;
}

// ===============
bool CstCinematicTheoraComposite::CstIsPlaying_Game() {

	idScopedCriticalSection cs(stateMutex);

	if (deferredStartTime_Game >= 0) {
		return true;
	}
	if (decoderGame) {
		return decoderGame->CstIsPlaying();
	}
	return false;
}

// ===============
bool CstCinematicTheoraComposite::CstIsPlaying_Sys() {

	idScopedCriticalSection cs(stateMutex);

	if (deferredStartTime_Sys >= 0) {
		return true;
	}
	if (decoderSys) {
		return decoderSys->CstIsPlaying();
	}
	return false;
}

// ===============
bool CstCinematicTheoraComposite::InitFromFile(const char *qpath, bool _looping) {
	assert(idLib::IsMainThread());

	if (!CstCinematicTheora::Enabled()) {
		return false;
	}

	idScopedCriticalSection cs(stateMutex);

	CloseNoLock();

	if (strstr(qpath, "/") == NULL && strstr(qpath, "\\") == NULL) {
		sprintf(fileName, "video/%s", qpath);
	} else {
		sprintf(fileName, "%s", qpath);
	}
	fileName.SetFileExtension(".th");

	iFile = (idFile_Memory *)fileSystem->CstOpenFileRead_FileMemory(fileName.c_str());
	if (!iFile) {
		CloseNoLock();
		return false;
	}

	if (!decoderLocal.InitFromFile(iFile->GetDataPtr(), iFile->Length(), _looping, true)) {
		CloseNoLock();
		return false;
	}

	looping = _looping;
	ready = true;

	return true;
}

// ===============
void CstCinematicTheoraComposite::CstUpdate(int thisTime) {
	assert(idLib::IsMainThread());

	idScopedCriticalSection cs(stateMutex);

	if (!ready) {
		return;
	}

	// only decoderGame should receive updates
	if (!decoderGame) {
		decoderGame = NewDecoder();
		if (!decoderGame) {
			deferredStartTime_Game = -1;
			return;
		}
	}
	if (deferredStartTime_Game >= 0) {
		decoderGame->ResetTime(deferredStartTime_Game);
		deferredStartTime_Game = -1;
	}
	decoderGame->CstUpdateNoLock(thisTime);
}

// ===============
cinData_t CstCinematicTheoraComposite::ImageForTime(int thisTime) {
	assert(idLib::IsMainThread());

	idScopedCriticalSection cs(stateMutex);

	if (!ready) {
		cinData_t data;
		memset(&data, 0, sizeof(data));
		return data;
	}

	bool useDecoderGame = true;
	if (thisTime <= 0 || tr.testVideo == this) {
		useDecoderGame = false;
	}
	if (useDecoderGame) {
		if (!decoderGame) {
			decoderGame = NewDecoder();
			if (!decoderGame) {
				deferredStartTime_Game = -1;
				cinData_t data;
				memset(&data, 0, sizeof(data));
				return data;
			}
		}
		if (deferredStartTime_Game >= 0) {
			decoderGame->ResetTime(deferredStartTime_Game);
			deferredStartTime_Game = -1;
		}
	} else {
		if (!decoderSys) {
			decoderSys = NewDecoder();
			if (!decoderSys) {
				deferredStartTime_Sys = -1;
				cinData_t data;
				memset(&data, 0, sizeof(data));
				return data;
			}
		}
		if (deferredStartTime_Sys >= 0) {
			decoderSys->ResetTime(deferredStartTime_Sys);
			deferredStartTime_Sys = -1;
		}
	}

	cinData_t data;
	if (useDecoderGame) {
		data = decoderGame->ImageForTime(thisTime);
	} else {
		int time = Sys_Milliseconds();
		data = decoderSys->ImageForTime(time);
	}
	return data;
}

// ===============
void CstCinematicTheoraComposite::CstResetTime_Game(int time) {
	if (time < 0) {
		return;
	}

	idScopedCriticalSection cs(stateMutex);

	if (!ready) {
		return;
	}
	deferredStartTime_Game = time;
}

// ===============
void CstCinematicTheoraComposite::CstResetTime_Sys(int time) {
	if (time < 0) {
		return;
	}

	idScopedCriticalSection cs(stateMutex);

	if (!ready) {
		return;
	}
	deferredStartTime_Sys = time;
}
//#modified-fva; END

//===========================================

/*
==============
idCinematic::InitCinematic
==============
*/
void idCinematic::InitCinematic( void ) {
	//#modified-fva; BEGIN
	CstCinematic::InitCinematic();
	//#modified-fva; END
}

/*
==============
idCinematic::ShutdownCinematic
==============
*/
void idCinematic::ShutdownCinematic( void ) {
	//#modified-fva; BEGIN
	CstCinematic::ShutdownCinematic();
	//#modified-fva; END
}

/*
==============
idCinematic::Alloc
==============
*/
//#modified-fva; BEGIN
/*
idCinematic * idCinematic::Alloc() {
	return new idCinematic;
}
*/
idCinematic * idCinematic::CstAlloc(idStr & fileName) {
#if CST_LIBBINKDEC
	if (fileName.Find('/') < 0 && fileName.Find('\\') < 0) {
		fileName.Insert("video/", 0);
	}
	fileName.SetFileExtension(".th");
	if (fileSystem->FindFile(fileName.c_str()) == FIND_YES) {
		return new CstCinematicTheoraComposite();
	}
	fileName.SetFileExtension(".bik");
	return new CstCinematicBinkComposite();
#else
	return new CstCinematicTheoraComposite();
#endif
}
//#modified-fva; END

/*
==============
idCinematic::~idCinematic
==============
*/
idCinematic::~idCinematic( ) {
	Close();
}

/*
==============
idCinematic::InitFromFile
==============
*/
bool idCinematic::InitFromFile( const char *qpath, bool looping ) {
	return false;
}

/*
==============
idCinematic::AnimationLength
==============
*/
int idCinematic::AnimationLength() {
	return 0;
}

/*
==============
idCinematic::GetStartTime
==============
*/
//#modified-fva; BEGIN
/*
int idCinematic::GetStartTime() {
	return -1;
}
*/
int idCinematic::CstGetStartTime_Game() {
	return -1;
}

int idCinematic::CstGetStartTime_Sys() {
	return -1;
}
//#modified-fva; END

/*
==============
idCinematic::ResetTime
==============
*/
//#modified-fva; BEGIN
/*
void idCinematic::ResetTime(int milliseconds) {
}
*/
void idCinematic::CstResetTime_Game(int milliseconds) {
}

void idCinematic::CstResetTime_Sys(int milliseconds) {
}
//#modified-fva; END

/*
==============
idCinematic::ImageForTime
==============
*/
cinData_t idCinematic::ImageForTime( int milliseconds ) {
	cinData_t c;
	memset( &c, 0, sizeof( c ) );
	return c;
}

/*
==============
idCinematic::ExportToTGA
==============
*/
void idCinematic::ExportToTGA( bool skipExisting ) {
}

/*
==============
idCinematic::GetFrameRate
==============
*/
float idCinematic::GetFrameRate() const {
	return 30.0f;
}

/*
==============
idCinematic::Close
==============
*/
void idCinematic::Close() {
}

//#modified-fva; BEGIN
bool idCinematic::CstIsPlaying_Game() {
	return false;
}

bool idCinematic::CstIsPlaying_Sys() {
	return false;
}

void idCinematic::CstUpdate(int milliseconds) {
}

bool idCinematic::CstIsTheora() {
	return false;
}
//#modified-fva; END

/*
==============
idSndWindow::InitFromFile
==============
*/
bool idSndWindow::InitFromFile( const char *qpath, bool looping ) {
	idStr fname = qpath;

	fname.ToLower();
	if ( !fname.Icmp( "waveform" ) ) {
		showWaveform = true;
	} else {
		showWaveform = false;
	}
	return true;
}

/*
==============
idSndWindow::ImageForTime
==============
*/
cinData_t idSndWindow::ImageForTime( int milliseconds ) {
	return soundSystem->ImageForTime( milliseconds, showWaveform );
}

/*
==============
idSndWindow::AnimationLength
==============
*/
int idSndWindow::AnimationLength() {
	return -1;
}
