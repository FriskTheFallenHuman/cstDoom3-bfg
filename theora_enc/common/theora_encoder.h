#ifndef _THEORA_ENCODER_H_
#define _THEORA_ENCODER_H_

#include "utils.h"
#include <vector>

class TheoraEncoder {
public:
	enum pixelFormat_t {
		YCBCR_420,
		YCBCR_422,
		YCBCR_444
	};

									TheoraEncoder();
	virtual							~TheoraEncoder();

	bool							Encode(const std::filesystem::path &inFolderPath, const std::list<std::filesystem::path> &inFileNames, const std::filesystem::path &outFilePath);
	utf8_string						GetLastError();

	virtual void					ReportProgress(unsigned processed, unsigned total) {}
	virtual bool					CancelRequested() { return false; }

	unsigned						framerateNumerator;
	unsigned						framerateDenominator;
	unsigned						keyframePeriod;
	unsigned						videoQuality;
	pixelFormat_t					pixelFormat;

private:
	utf8_string						errorMsg;

	std::vector<unsigned char>		rgbBuffer;
	std::vector<unsigned char>		yBuffer;
	std::vector<unsigned char>		cbBuffer;
	std::vector<unsigned char>		crBuffer;
};

#endif // _THEORA_ENCODER_H_
