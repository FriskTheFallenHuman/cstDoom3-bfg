#include "theora_encoder.h"

#include <png.h>
#include <theora/theoraenc.h>

#include <cstring>

namespace fs = std::filesystem;

namespace {
	// ===============
	class PngImageGuard {
	public:
		PngImageGuard(png_image &_image) : image(_image) {}
		~PngImageGuard() {
			png_image_free(&image);
		}
	private:
		png_image &image;
	};

	// ===============
	class ThEncCtxGuard {
	public:
		ThEncCtxGuard(th_enc_ctx *&_enc) : enc(_enc) {}
		~ThEncCtxGuard() {
			if (enc) {
				th_encode_free(enc);
				enc = nullptr;
			}
		}
	private:
		th_enc_ctx *&enc;
	};

	// ===============
	class ThCommentGuard {
	public:
		ThCommentGuard(th_comment &_comment) : comment(_comment) {}
		~ThCommentGuard() {
			th_comment_clear(&comment);
		}
	private:
		th_comment &comment;
	};

	// ===============
	struct keyframeInfo_t {
		unsigned fileOffset;
		unsigned frameIndex;
	};

	// ===============
	bool WriteByte(std::FILE *file, unsigned char byte) {
		if (std::fwrite(&byte, 1, 1, file) != 1) {
			return false;
		}
		return true;
	}

	// ===============
	bool WriteUnsigned(std::FILE *file, unsigned value) {
		unsigned char bytes[4];
		bytes[0] = value & 0xFF;
		bytes[1] = (value >> 8) & 0xFF;
		bytes[2] = (value >> 16) & 0xFF;
		bytes[3] = (value >> 24) & 0xFF;
		if (std::fwrite(&bytes, 1, 4, file) != 4) {
			return false;
		}
		return true;
	}

	// ===============
	bool WriteUnsigned64(std::FILE *file, unsigned long long value) {
		unsigned char bytes[8];
		bytes[0] = value & 0xFF;
		bytes[1] = (value >> 8) & 0xFF;
		bytes[2] = (value >> 16) & 0xFF;
		bytes[3] = (value >> 24) & 0xFF;
		bytes[4] = (value >> 32) & 0xFF;
		bytes[5] = (value >> 40) & 0xFF;
		bytes[6] = (value >> 48) & 0xFF;
		bytes[7] = (value >> 56) & 0xFF;
		if (std::fwrite(&bytes, 1, 8, file) != 8) {
			return false;
		}
		return true;
	}

	// ===============
	bool WritePacket(std::FILE *file, ogg_packet &packet) {
		unsigned packetSize = packet.bytes + 17; // packet data + additional info
		if (packetSize > (1uLL << 31) - 1) {
			return false;
		}
		unsigned char bos_eos = ((packet.b_o_s & 1) << 1) | (packet.e_o_s & 1);
		if (!WriteUnsigned(file, packetSize)
			|| !WriteByte(file, bos_eos)
			|| !WriteUnsigned64(file, packet.granulepos)
			|| !WriteUnsigned(file, (unsigned)packet.packetno)) {
			return false;
		}
		if (std::fwrite(packet.packet, 1, packet.bytes, file) != packet.bytes) {
			return false;
		}
		return true;
	}
}

// ===============
TheoraEncoder::TheoraEncoder() :
	framerateNumerator(0), // must be specified
	framerateDenominator(1),
	keyframePeriod(30), // libtheora default is 64 (default keyframe_granule_shift is 6)
	videoQuality(63),
	pixelFormat(YCBCR_444) {
}

// ===============
TheoraEncoder::~TheoraEncoder() {
}

// ===============
bool TheoraEncoder::Encode(const fs::path &inFolderPath, const std::list<fs::path> &inFileNames, const fs::path &outFilePath) {
	if (inFolderPath.empty()) {
		errorMsg = u8"Error: Input folder path is an empty string.";
		return false;
	}
	if (inFileNames.empty()) {
		errorMsg = u8"Error: No files to process.";
		return false;
	}
	if (outFilePath.empty()) {
		errorMsg = u8"Error: Output file path is an empty string.";
		return false;
	}

	// validade parameters
	if (framerateNumerator < 1) {
		errorMsg = u8"Error: Invalid framerate numerator.";
		return false;
	}
	if (framerateDenominator < 1) {
		errorMsg = u8"Error: Invalid framerate denominator.";
		return false;
	}
	if (keyframePeriod < 1 || keyframePeriod > 2147483648) {
		errorMsg = u8"Error: Invalid keyframe period.";
		return false;
	}
	if (videoQuality > 63) {
		errorMsg = u8"Error: Invalid video quality.";
		return false;
	}

	// get the picture dimensions to be used in the video
	unsigned picWidth;
	unsigned picHeight;
	{
		png_image pngImage;
		std::memset(&pngImage, 0, sizeof pngImage);
		PngImageGuard pngImageGuard(pngImage);
		pngImage.version = PNG_IMAGE_VERSION;

		std::FILE *file = nullptr;
		FileGuard fileGuard(file);

		// use the dimensions of the first png file
		fs::path inFilePath = inFolderPath / inFileNames.front();
		file = OpenFileReadBinary(inFilePath);
		if (!file) {
			errorMsg = u8"Error: Couldn't open \"" + inFilePath.u8string() + u8"\" for reading.";
			return false;
		}
		if (!png_image_begin_read_from_stdio(&pngImage, file)) {
			errorMsg = u8"Error: Couldn't read the png header from \"" + inFilePath.u8string() + u8"\".";
			return false;
		}
		picWidth = pngImage.width;
		picHeight = pngImage.height;
	}

	// force frame dimensions to be divisible by 16
	unsigned frameWidth = (picWidth + 15) & ~15;
	unsigned frameHeight = (picHeight + 15) & ~15;

	// validade the picture dimensions and the frame dimensions
	if (frameWidth >= 1048576) {
		errorMsg = u8"Error: Frame width is too large.";
		return false;
	}
	if (frameHeight >= 1048576) {
		errorMsg = u8"Error: Frame height is too large.";
		return false;
	}

	if (pixelFormat == YCBCR_420) {
		if (picWidth < 2 || picHeight < 2) {
			errorMsg = u8"Error: The 4:2:0 format requires the width and the height to be at least 2.";
			return false;
		}
		if (picWidth & 1 || picHeight & 1) {
			errorMsg = u8"Error: The 4:2:0 format requires the width and the height to be divisible by 2.";
			return false;
		}
	} else if (pixelFormat == YCBCR_422) {
		if (picWidth < 2) {
			errorMsg = u8"Error: The 4:2:2 format requires the width to be at least 2.";
			return false;
		}
		if (picHeight == 0) {
			errorMsg = u8"Error: Height is zero.";
			return false;
		}
		if (picWidth & 1) {
			errorMsg = u8"Error: The 4:2:2 format requires the width to be divisible by 2.";
			return false;
		}
	} else if (pixelFormat == YCBCR_444) {
		if (picWidth == 0) {
			errorMsg = u8"Error: Width is zero.";
			return false;
		}
		if (picHeight == 0) {
			errorMsg = u8"Error: Height is zero.";
			return false;
		}
	}

	th_enc_ctx *thEncCtx = nullptr;
	ThEncCtxGuard thEncCtxGuard(thEncCtx);
	{
		// fill in th_info
		th_info thInfo;
		th_info_init(&thInfo);
		thInfo.frame_width = frameWidth;
		thInfo.frame_height = frameHeight;
		thInfo.pic_width = picWidth;
		thInfo.pic_height = picHeight;
		thInfo.pic_x = ((frameWidth - picWidth) >> 1) & ~1; // force x offset to be divisible by 2
		thInfo.pic_y = ((frameHeight - picHeight) >> 1) & ~1; // force y offset to be divisible by 2
		thInfo.fps_numerator = framerateNumerator;
		thInfo.fps_denominator = framerateDenominator;
		thInfo.aspect_numerator = 0;
		thInfo.aspect_denominator = 0;
		thInfo.colorspace = TH_CS_UNSPECIFIED;
		if (pixelFormat == YCBCR_420) {
			thInfo.pixel_fmt = TH_PF_420;
		} else if (pixelFormat == YCBCR_422) {
			thInfo.pixel_fmt = TH_PF_422;
		} else { // YCBCR_444
			thInfo.pixel_fmt = TH_PF_444;
		}
		thInfo.target_bitrate = 0;
		thInfo.quality = videoQuality;
		thInfo.keyframe_granule_shift = 0; // adjusted later

		// create a theora encoder context
		thEncCtx = th_encode_alloc(&thInfo);
		th_info_clear(&thInfo);
		if (!thEncCtx) {
			errorMsg = u8"Error: Couldn't create a theora encoder context.";
			return false;
		}
	}

	// set the keyframe frequency; the keyframe_granule_shift will be enlarged by libtheora as needed
	{
		ogg_uint32_t freq = keyframePeriod;
		int ret = th_encode_ctl(thEncCtx, TH_ENCCTL_SET_KEYFRAME_FREQUENCY_FORCE, &freq, sizeof(freq));
		if (ret < 0 || freq != keyframePeriod) {
			errorMsg = u8"Error: Couldn't set the requested keyframe period.";
			return false;
		}
	}

	std::vector<keyframeInfo_t> keyframeTable;

	// open output file
	std::FILE * outFile = nullptr;
	FileGuard outFileGuard(outFile);
	outFile = OpenFileWriteBinary(outFilePath);
	if (!outFile) {
		errorMsg = u8"Error: Couldn't open \"" + outFilePath.u8string() + u8"\" for writing.";
		return false;
	}

	const std::size_t numFrames = inFileNames.size();
	if (numFrames > (1uLL << 31) - 1) {
		errorMsg = u8"Error: Too many frames.";
		return false;
	}
	if (!WriteByte(outFile, 'T') || !WriteByte(outFile, 'H')	// signature
		|| !WriteUnsigned(outFile, 1)							// version
		|| !WriteUnsigned(outFile, numFrames)) {				// number of frames
		errorMsg = u8"Error: Couldn't write to \"" + outFilePath.u8string() + u8"\".";
		return false;
	}

	int posOffsetKeyframeTable = std::ftell(outFile);
	if (posOffsetKeyframeTable == -1) {
		errorMsg = u8"Error: Couldn't retrieve the write position. Too large output file?";
		return false;
	}
	if (!WriteUnsigned(outFile, 0)) { // offset of the keyframe table; to be adjusted later
		errorMsg = u8"Error: Couldn't write to \"" + outFilePath.u8string() + u8"\".";
		return false;
	}

	int posOffsetDataPacket0 = std::ftell(outFile);
	if (posOffsetDataPacket0 == -1) {
		errorMsg = u8"Error: Couldn't retrieve the write position. Too large output file?";
		return false;
	}
	if (!WriteUnsigned(outFile, 0)) { // offset of the first data packet; to be adjusted later
		errorMsg = u8"Error: Couldn't write to \"" + outFilePath.u8string() + u8"\".";
		return false;
	}

	// create the first theora header
	th_comment thComment;
	th_comment_init(&thComment);
	ThCommentGuard thCommentGuard(thComment);
	ogg_packet thPacket;
	if (th_encode_flushheader(thEncCtx, &thComment, &thPacket) <= 0) {
		errorMsg = u8"Error: Internal libtheora error.";
		return false;
	}
	if (!WritePacket(outFile, thPacket)) {
		errorMsg = u8"Error: Couldn't write to \"" + outFilePath.u8string() + u8"\".";
		return false;
	}

	// create the remaining theora headers
	while (true) {
		int ret = th_encode_flushheader(thEncCtx, &thComment, &thPacket);
		if (ret < 0) {
			errorMsg = u8"Error: Internal libtheora error.";
			return false;
		} else if (ret == 0) {
			break; // all headers created
		}
		if (!WritePacket(outFile, thPacket)) {
			errorMsg = u8"Error: Couldn't write to \"" + outFilePath.u8string() + u8"\".";
			return false;
		}
	}

	// adjust the offset of the first data packet
	{
		int posCurrent = std::ftell(outFile);
		if (posCurrent == -1) {
			errorMsg = u8"Error: Couldn't retrieve the write position. Too large output file?";
			return false;
		}
		if (std::fseek(outFile, posOffsetDataPacket0, SEEK_SET) != 0) {
			errorMsg = u8"Error: Couldn't change the write position. Too large output file?";
			return false;
		}
		if (!WriteUnsigned(outFile, posCurrent)) {
			errorMsg = u8"Error: Couldn't write to \"" + outFilePath.u8string() + u8"\".";
			return false;
		}
		if (std::fseek(outFile, posCurrent, SEEK_SET) != 0) {
			errorMsg = u8"Error: Couldn't change the write position. Too large output file?";
			return false;
		}
	}

	// resize rgb and ycbcr buffers as needed
	{
		std::size_t size = picWidth * picHeight;
		rgbBuffer.resize(3 * size); // 3 components: r, g, b
		yBuffer.resize(size);
		if (pixelFormat == YCBCR_420) {
			cbBuffer.resize(size >> 2);
			crBuffer.resize(size >> 2);
		} else if (pixelFormat == YCBCR_422) {
			cbBuffer.resize(size >> 1);
			crBuffer.resize(size >> 1);
		} else { // YCBCR_444
			cbBuffer.resize(size);
			crBuffer.resize(size);
		}
	}

	// create the theora frames
	{
		th_ycbcr_buffer ycbcr;
		ycbcr[0].width = picWidth;
		ycbcr[0].height = picHeight;
		ycbcr[0].stride = picWidth;
		ycbcr[0].data = &yBuffer[0];
		ycbcr[1].width = (pixelFormat == YCBCR_444) ? picWidth : (picWidth >> 1);
		ycbcr[1].height = (pixelFormat != YCBCR_420) ? picHeight : (picHeight >> 1);
		ycbcr[1].stride = ycbcr[1].width;
		ycbcr[1].data = &cbBuffer[0];
		ycbcr[2].width = ycbcr[1].width;
		ycbcr[2].height = ycbcr[1].height;
		ycbcr[2].stride = ycbcr[1].stride;
		ycbcr[2].data = &crBuffer[0];

		auto fileIterator = inFileNames.begin();

		// entering encoding loop
		ReportProgress(0, numFrames);

		for (std::size_t frame = 1; frame <= numFrames; ++frame, ++fileIterator) {
			if (CancelRequested()) {
				errorMsg = u8"Cancel requested.";
				return false;
			}

			// read the png image into the rbg buffer
			{
				png_image pngImage;
				std::memset(&pngImage, 0, sizeof pngImage);
				PngImageGuard pngImageGuard(pngImage);
				pngImage.version = PNG_IMAGE_VERSION;

				std::FILE *file = nullptr;
				FileGuard fileGuard(file);

				fs::path inFilePath = inFolderPath / *fileIterator;
				file = OpenFileReadBinary(inFilePath);
				if (!file) {
					errorMsg = u8"Error: Couldn't open \"" + inFilePath.u8string() + u8"\" for reading.";
					return false;
				}
				if (!png_image_begin_read_from_stdio(&pngImage, file)) {
					errorMsg = u8"Error: Couldn't read the png header from \"" + inFilePath.u8string() + u8"\".";
					return false;
				}
				if (pngImage.width != picWidth || pngImage.height != picHeight) {
					utf8_string errorMsg = u8"Error: Inconsistent input dimensions. The image \"" + inFilePath.u8string() + u8"\" is ";
					errorMsg += ToUtf8(pngImage.width) + u8'x' + ToUtf8(pngImage.height) + u8", but should be ";
					errorMsg += ToUtf8(picWidth) + u8'x' + ToUtf8(picHeight) + u8'.';
					return false;
				}
				pngImage.format = PNG_FORMAT_RGB;
				if (PNG_IMAGE_SIZE(pngImage) != rgbBuffer.size()) {
					// width and height have already been tested, so it should never get here
					errorMsg = u8"Error: The png image size doesn't match the rgb buffer size.";
					return false;
				}
				const png_color black = { 0, 0, 0 };
				if (!png_image_finish_read(&pngImage, &black, &rgbBuffer[0], 0, nullptr)) {
					errorMsg = u8"Error: Couldn't read the png image from \"" + inFilePath.u8string() + u8"\".";
					return false;
				}
			}

			// convert from rgb to the selected ycbcr format
			if (pixelFormat == YCBCR_420) {
				const unsigned char *pRgbRow = &rgbBuffer[0];
				const unsigned char * const pRgbRowBeforeLast = &rgbBuffer[0] + 3 * picWidth * (picHeight - 2);
				unsigned char *pyRow = &yBuffer[0];
				unsigned char *pCb = &cbBuffer[0];
				unsigned char *pCr = &crBuffer[0];

				while (true) {
					const unsigned char *pRgb1 = pRgbRow; // current row
					const unsigned char *pRgb2 = pRgbRow + 3 * picWidth; // the row below the current row
					const unsigned char * const pLast1 = pRgbRow + 3 * picWidth - 6; // last pair of rgb colors in the current row

					unsigned char *pYa = pyRow;
					unsigned char *pYb = pyRow + picWidth;

					while (pRgb1 <= pLast1) {
						unsigned char r1 = *pRgb1++;
						unsigned char g1 = *pRgb1++;
						unsigned char b1 = *pRgb1++;

						unsigned char r2 = *pRgb1++;
						unsigned char g2 = *pRgb1++;
						unsigned char b2 = *pRgb1++;

						unsigned char r3 = *pRgb2++;
						unsigned char g3 = *pRgb2++;
						unsigned char b3 = *pRgb2++;

						unsigned char r4 = *pRgb2++;
						unsigned char g4 = *pRgb2++;
						unsigned char b4 = *pRgb2++;

						int y1 = ((66 * r1 + 129 * g1 + 25 * b1 + 128) >> 8) + 16;
						int y2 = ((66 * r2 + 129 * g2 + 25 * b2 + 128) >> 8) + 16;
						int y3 = ((66 * r3 + 129 * g3 + 25 * b3 + 128) >> 8) + 16;
						int y4 = ((66 * r4 + 129 * g4 + 25 * b4 + 128) >> 8) + 16;

						int cb1 = -38 * r1 - 74 * g1 + 112 * b1;
						int cb2 = -38 * r2 - 74 * g2 + 112 * b2;
						int cb3 = -38 * r3 - 74 * g3 + 112 * b3;
						int cb4 = -38 * r4 - 74 * g4 + 112 * b4;
						int cb = ((cb1 + cb2 + cb3 + cb4 + 512) >> 10) + 128;

						int cr1 = 112 * r1 - 94 * g1 - 18 * b1;
						int cr2 = 112 * r2 - 94 * g2 - 18 * b2;
						int cr3 = 112 * r3 - 94 * g3 - 18 * b3;
						int cr4 = 112 * r4 - 94 * g4 - 18 * b4;
						int cr = ((cr1 + cr2 + cr3 + cr4 + 512) >> 10) + 128;

						*pYa++ = (unsigned char)y1;
						*pYa++ = (unsigned char)y2;

						*pYb++ = (unsigned char)y3;
						*pYb++ = (unsigned char)y4;

						*pCb++ = (unsigned char)cb;
						*pCr++ = (unsigned char)cr;
					}

					if (pRgbRow < pRgbRowBeforeLast) {
						pRgbRow += (2 * 3 * picWidth);
						pyRow += (2 * picWidth);
					} else {
						break; // done
					}
				}
			} else if (pixelFormat == YCBCR_422) {
				const unsigned char *pRgb = &rgbBuffer[0];
				const unsigned char * const pRgbLastPair = &rgbBuffer[0] + rgbBuffer.size() - 6; // last pair of rgb colors
				unsigned char *pY = &yBuffer[0];
				unsigned char *pCb = &cbBuffer[0];
				unsigned char *pCr = &crBuffer[0];

				while (pRgb <= pRgbLastPair) {
					unsigned char r1 = *pRgb++;
					unsigned char g1 = *pRgb++;
					unsigned char b1 = *pRgb++;

					unsigned char r2 = *pRgb++;
					unsigned char g2 = *pRgb++;
					unsigned char b2 = *pRgb++;

					int y1 = ((66 * r1 + 129 * g1 + 25 * b1 + 128) >> 8) + 16;
					int y2 = ((66 * r2 + 129 * g2 + 25 * b2 + 128) >> 8) + 16;

					int cb1 = -38 * r1 - 74 * g1 + 112 * b1;
					int cb2 = -38 * r2 - 74 * g2 + 112 * b2;
					int cb = ((cb1 + cb2 + 256) >> 9) + 128;

					int cr1 = 112 * r1 - 94 * g1 - 18 * b1;
					int cr2 = 112 * r2 - 94 * g2 - 18 * b2;
					int cr = ((cr1 + cr2 + 256) >> 9) + 128;

					*pY++ = (unsigned char)y1;
					*pY++ = (unsigned char)y2;

					*pCb++ = (unsigned char)cb;
					*pCr++ = (unsigned char)cr;
				}
			} else { // YCBCR_444
				const unsigned char *pRgb = &rgbBuffer[0];
				const unsigned char * const pRgbLast = &rgbBuffer[0] + rgbBuffer.size() - 3; // last rgb color
				unsigned char *pY = &yBuffer[0];
				unsigned char *pCb = &cbBuffer[0];
				unsigned char *pCr = &crBuffer[0];

				while (pRgb <= pRgbLast) {
					unsigned char r = *pRgb++;
					unsigned char g = *pRgb++;
					unsigned char b = *pRgb++;

					int y = ((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
					int cb = ((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
					int cr = ((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;

					*pY++ = (unsigned char)y;
					*pCb++ = (unsigned char)cb;
					*pCr++ = (unsigned char)cr;
				}
			}

			// encode frame
			if (th_encode_ycbcr_in(thEncCtx, ycbcr) != 0) {
				errorMsg = u8"Error: Couldn't encode frame.";
				return false;
			}
			while (true) {
				int ret = th_encode_packetout(thEncCtx, frame == numFrames, &thPacket);
				if (ret < 0) {
					errorMsg = u8"Error: Couldn't retrieve encoded packet.";
					return false;
				} else if (ret == 0) {
					break; // no more packets
				}
				if (th_packet_iskeyframe(&thPacket)) {
					int posCurrent = std::ftell(outFile);
					if (posCurrent == -1) {
						errorMsg = u8"Error: Couldn't retrieve the write position. Too large output file?";
						return false;
					}
					keyframeTable.push_back({ (unsigned)posCurrent, frame - 1 });
				}
				if (!WritePacket(outFile, thPacket)) {
					errorMsg = u8"Error: Couldn't write to \"" + outFilePath.u8string() + u8"\".";
					return false;
				}
			}

			// frame processed
			ReportProgress(frame, numFrames);
		}
	}

	// adjust the offset of the keyframe table
	{
		int posCurrent = std::ftell(outFile);
		if (posCurrent == -1) {
			errorMsg = u8"Error: Couldn't retrieve the write position. Too large output file?";
			return false;
		}
		if (std::fseek(outFile, posOffsetKeyframeTable, SEEK_SET) != 0) {
			errorMsg = u8"Error: Couldn't change the write position. Too large output file?";
			return false;
		}
		if (!WriteUnsigned(outFile, posCurrent)) {
			errorMsg = u8"Error: Couldn't write to \"" + outFilePath.u8string() + u8"\".";
			return false;
		}
		if (std::fseek(outFile, posCurrent, SEEK_SET) != 0) {
			errorMsg = u8"Error: Couldn't change the write position. Too large output file?";
			return false;
		}
	}

	// write the keyframe table
	if (!WriteUnsigned(outFile, keyframeTable.size())) {
		errorMsg = u8"Error: Couldn't write to \"" + outFilePath.u8string() + u8"\".";
		return false;
	}
	for (std::size_t i = 0; i < keyframeTable.size(); ++i) {
		if (!WriteUnsigned(outFile, keyframeTable[i].fileOffset)		// keyframe offset
			|| !WriteUnsigned(outFile, keyframeTable[i].frameIndex)) {	// frame index of this keyframe
			errorMsg = u8"Error: Couldn't write to \"" + outFilePath.u8string() + u8"\".";
			return false;
		}
	}
	return true;
}

// ===============
utf8_string TheoraEncoder::GetLastError() {
	return errorMsg;
}
