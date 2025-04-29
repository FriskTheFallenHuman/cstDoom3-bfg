//#modified-fva; BEGIN
// interface moved from BinkDecoder.h (with changes)

#ifndef __LIBBINKDEC_H__
#define __LIBBINKDEC_H__

#include <stdint.h>

typedef struct BinkHandleTag BinkHandle;

typedef struct AudioInfoTag {
	uint32_t sampleRate;
	uint32_t nChannels;
	uint32_t idealBufferSize;
} AudioInfo;

typedef struct ImagePlaneTag {
	uint32_t widthAllocated;
	uint32_t heightAllocated;
	uint32_t widthDisplay;
	uint32_t heightDisplay;
	uint32_t pitch;
	uint8_t  *data;
} ImagePlane;

typedef ImagePlane YUVbuffer[3];

#ifdef __cplusplus
extern "C" {
#endif

// return values:
//   0 = failed
//   1 = succeded
//  -1 = unhandled exception

int Bink_Init(const uint8_t *fileData, int32_t fileDataSize, BinkHandle **outHandle);
int Bink_Close(BinkHandle *handle);
int Bink_GetNumAudioTracks(BinkHandle *handle, uint32_t *outNumAudioTracks);
int Bink_GetAudioTrackDetails(BinkHandle *handle, uint32_t trackIndex, AudioInfo *outAudioInfo);
int Bink_GetAudioData(BinkHandle *handle, uint32_t trackIndex, int16_t *data, uint32_t *outNumBytesRead);
int Bink_GetFrameRate(BinkHandle *handle, float *outFrameRate);
int Bink_GetNumFrames(BinkHandle *handle, uint32_t *outNumFrames);
int Bink_GetFrameSize(BinkHandle *handle, uint32_t *outWidth, uint32_t *outHeight);
int Bink_GetCurrentFrameNum(BinkHandle *handle, uint32_t *outCurrentFrameNum);

// stick with only one of the GetNextFrame functions below
int Bink_GetNextFrame(BinkHandle *handle, YUVbuffer yuv, uint32_t *outCurrentFrameNum);
int Bink_GetNextFrame_VideoOnly(BinkHandle *handle, YUVbuffer yuv, uint32_t *outCurrentFrameNum);
int Bink_GetNextFrame_AudioOnly(BinkHandle *handle, uint32_t *outCurrentFrameNum);

int Bink_GotoFrame(BinkHandle *handle, uint32_t frameNum);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __LIBBINKDEC_H__
//#modified-fva; END
