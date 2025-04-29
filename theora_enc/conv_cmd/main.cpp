#include "../common/theora_encoder.h"

#include <fcntl.h>
#include <io.h>

#include <chrono>
#include <cwchar>

namespace fs = std::filesystem;

namespace {
	// ===============
	class TheoraEncoderLocal : public TheoraEncoder {
	public:
		TheoraEncoderLocal() : reportedOnce(false), quiet(false), prevPercent(0) {}

		void ReportProgress(unsigned processed, unsigned total) override {
			if (quiet || total == 0) {
				return;
			}
			if (processed == 0) {
				std::wprintf(L"Progress: 0%%");
				reportedOnce = true;
				prevPercent = 0;
				return;
			}
			unsigned percent = (100 * processed) / total;
			if (percent > prevPercent) {
				std::wprintf(L"\rProgress: %d%%", percent);
				prevPercent = percent;
			}
		}

		bool reportedOnce; // manual reset
		bool quiet;

	private:
		unsigned prevPercent;
	};

	// ===============
	template<typename T>
	struct Argument {
		T value;
		bool available;
	};
}

// ===============
int wmain(int argc, wchar_t *argv[]) {
	const wchar_t usage[] =
		L"Create a theora video (in a TH container) from a set of png images.\n"
		L"\n"
		L"Usage:\n"
		L" theora_enc <arguments>\n"
		L"\n"
		L"Required arguments:\n"
		L" -f <value>    Framerate numerator\n"
		L" -i <folder>   Input folder containing the png files\n"
		L" -o <file>     Output file\n"
		L"\n"
		L"Optional arguments:\n"
		L" -F <value>    Framerate denominator (default = 1)\n"
		L" -k <value>    Keyframe period (default = 30)\n"
		L" -v <value>    Video quality: 0 to 63 (default = 63)\n"
		L" -p <format>   Y'CbCr format: 420, 422, 444 (default = 444)\n"
		L" -q <value>    Quiet: 1 = yes; 0 = no (default = 0)\n"
		L"\n"
		L"Examples:\n"
		L" theora_enc -f 30 -i \"C:\\images\" -o \"C:\\out.th\"\n"
		L" theora_enc -f 100 -F 3 -k 100 -p 420 -i \"../../images\" -o \"../out.th\"\n"
		L"\n"
		L"Remarks:\n"
		L" - All images in the input folder must have the same dimensions.\n"
		L" - The 420 format requires the width and the height to be divisible by 2.\n"
		L" - The 422 format requires the width to be divisible by 2.\n"
		L" - The 444 format has no dimension requirements.\n"
		L" - The png images are processed in lexicographical order. To avoid\n"
		L"   surprises, use a simple naming scheme. For example, name each image\n"
		L"   using a common base name and a fixed number of digits:\n"
		L"\n"
		L"     frame0000.png\n"
		L"     frame0001.png\n"
		L"     frame0002.png\n"
		L"     ...\n"
		L"     frame1001.png\n"
		L"     ...\n";

	// allow printing utf16 wide chars with std::wprintf
	if (_setmode(_fileno(stdout), _O_U16TEXT) == -1) {
		std::wprintf(L"Error: Couldn't set stdout to unicode mode.\n");
		return 1;
	}

	// parse the command line arguments
	if (argc < 2 || !(argc & 1)) {
		std::wprintf(usage);
		return 1;
	}
	Argument<unsigned> framerateNumerator = { 0, false };
	Argument<unsigned> framerateDenominator = { 0, false };
	Argument<unsigned> keyframePeriod = { 0, false };
	Argument<unsigned> videoQuality = { 0, false };
	Argument<TheoraEncoderLocal::pixelFormat_t> pixelFormat = { TheoraEncoderLocal::YCBCR_444, false };
	Argument<std::wstring> wInFolderPath = { L"", false };
	Argument<std::wstring> wOutFilePath = { L"", false };
	Argument<bool> quiet = { false, false };

	for (int i = 1; i < argc; i += 2) {
		if (std::wstring(L"-f") == argv[i]) {
			framerateNumerator.value = std::wcstoul(argv[i + 1], nullptr, 10);
			framerateNumerator.available = true;
		} else if (std::wstring(L"-i") == argv[i]) {
			wInFolderPath.value = argv[i + 1];
			wInFolderPath.available = true;
		} else if (std::wstring(L"-o") == argv[i]) {
			wOutFilePath.value = argv[i + 1];
			wOutFilePath.available = true;
		} else if (std::wstring(L"-F") == argv[i]) {
			framerateDenominator.value = std::wcstoul(argv[i + 1], nullptr, 10);
			framerateDenominator.available = true;
		} else if (std::wstring(L"-k") == argv[i]) {
			keyframePeriod.value = std::wcstoul(argv[i + 1], nullptr, 10);
			keyframePeriod.available = true;
		} else if (std::wstring(L"-v") == argv[i]) {
			videoQuality.value = std::wcstoul(argv[i + 1], nullptr, 10);
			videoQuality.available = true;
		} else if (std::wstring(L"-p") == argv[i]) {
			if (std::wstring(L"444") == argv[i + 1]) {
				pixelFormat.value = TheoraEncoderLocal::YCBCR_444;
				pixelFormat.available = true;
			} else if (std::wstring(L"422") == argv[i + 1]) {
				pixelFormat.value = TheoraEncoderLocal::YCBCR_422;
				pixelFormat.available = true;
			} else if (std::wstring(L"420") == argv[i + 1]) {
				pixelFormat.value = TheoraEncoderLocal::YCBCR_420;
				pixelFormat.available = true;
			} else {
				std::wprintf(L"Error: Unknown pixel format \"%s\".\n", argv[i + 1]);
				return 1;
			}
		} else if (std::wstring(L"-q") == argv[i]) {
			quiet.value = (bool)std::wcstoul(argv[i + 1], nullptr, 10);
			quiet.available = true;
		} else {
			std::wprintf(usage);
			return 1;
		}
	}

	// check if the required arguments have been specified
	if (!framerateNumerator.available) {
		std::wprintf(L"Error: The framerate numerator wasn't specified.\n");
		return 1;
	}
	if (!wInFolderPath.available) {
		std::wprintf(L"Error: The input folder wasn't specified.\n");
		return 1;
	}
	if (!wOutFilePath.available) {
		std::wprintf(L"Error: The output file wasn't specified.\n");
		return 1;
	}

	fs::path inFolderPath = wInFolderPath.value;
	inFolderPath.make_preferred();

	fs::path outFilePath = wOutFilePath.value;
	outFilePath.make_preferred();

	// get the list of png files to be processed
	std::list<fs::path> pngFiles;
	if (!ListFilesInDirectory(inFolderPath, ".png", pngFiles)) {
		std::wprintf(L"Error: Couldn't list the png files in the input folder.\n");
		return 1;
	}
	if (pngFiles.empty()) {
		std::wprintf(L"Error: The input folder doesn't contain any png files.\n");
		return 1;
	}
	pngFiles.sort();

	// create an encoder instance and configure it
	TheoraEncoderLocal theoraEnc;

	if (framerateNumerator.available) {
		theoraEnc.framerateNumerator = framerateNumerator.value;
	}
	if (framerateDenominator.available) {
		theoraEnc.framerateDenominator = framerateDenominator.value;
	}
	if (keyframePeriod.available) {
		theoraEnc.keyframePeriod = keyframePeriod.value;
	}
	if (videoQuality.available) {
		theoraEnc.videoQuality = videoQuality.value;
	}
	if (pixelFormat.available) {
		theoraEnc.pixelFormat = pixelFormat.value;
	}
	if (quiet.available) {
		theoraEnc.quiet = quiet.value;
	}

	// run the encoder
	auto startTime = std::chrono::high_resolution_clock::now();

	if (!theoraEnc.Encode(inFolderPath, pngFiles, outFilePath)) {
		if (theoraEnc.reportedOnce) {
			std::wprintf(L"\n");
		}
		utf8_string msg = theoraEnc.GetLastError();
		std::wstring wMsg;
		if (Utf8_To_Utf16(msg, wMsg)) {
			std::wprintf(L"%s\n", wMsg.c_str());
		} else {
			std::wprintf(L"Error: Encoding failed.\n");
		}
		return 1;
	}

	auto endTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> elapsedTime = endTime - startTime;

	if (!theoraEnc.quiet) {
		std::wprintf(L"\n");
		std::wprintf(L"Done. Elapsed time: %.3f seconds\n", elapsedTime.count());
	}
	return 0;
}
