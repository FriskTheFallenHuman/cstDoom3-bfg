#include "utils.h"

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <ShlObj.h>

#include <cstring>
#include <system_error>
#include <utility>

namespace fs = std::filesystem;

namespace {
	// ===============
	std::FILE * OpenFile(const fs::path &filePath, const std::wstring &mode) {
		if (filePath.empty() || mode.empty()) {
			return nullptr;
		}
		std::FILE * file;
		if (_wfopen_s(&file, filePath.c_str(), mode.c_str()) != 0) {
			return nullptr;
		}
		return file;
	}
}

// ===============
std::FILE * OpenFileReadBinary(const fs::path &filePath) {
	return OpenFile(filePath, L"rb");
}

// ===============
std::FILE * OpenFileWriteBinary(const fs::path &filePath) {
	return OpenFile(filePath, L"wb");
}

// ===============
bool ListFilesInDirectory(const fs::path &in_directoryPath, const fs::path &in_fileExtension, std::list<fs::path> &out_fileNames) {
	// returns true if successful or when no files matching the specified extension have been found

	std::list<fs::path> fileNames;
	bool listAllFiles = in_fileExtension == fs::path(".*");

	std::error_code errorCode;
	fs::directory_iterator dirIterator(in_directoryPath, errorCode);
	if (errorCode) {
		return false;
	}
	for (const fs::directory_entry &dirEntry : dirIterator) {
		bool isRegularFile = dirEntry.is_regular_file(errorCode);
		if (errorCode) {
			return false;
		}
		if (isRegularFile && (listAllFiles || dirEntry.path().extension() == in_fileExtension)) {
			fileNames.push_back(dirEntry.path().filename());
		}
	}
	out_fileNames = std::move(fileNames);
	return true;
}

// ===============
std::filesystem::path PathFromUtf8(const utf8_string &path) {
#ifdef _UTILS_H__HAS_U8STRING_
	return fs::path(path);
#else
	return fs::u8path(path);
#endif
}

// ===============
bool Utf16_To_Utf8(const std::wstring &in_utf16, utf8_string &out_utf8) {
	if (in_utf16.empty()) {
		out_utf8 = u8"";
		return true;
	}
	// get the required size
	int size = WideCharToMultiByte(CP_UTF8, 0, &in_utf16[0], in_utf16.size(), nullptr, 0, nullptr, nullptr);
	if (size == 0) {
		return false;
	}
	// convert
	std::string str;
	str.resize(size);
	size = WideCharToMultiByte(CP_UTF8, 0, &in_utf16[0], in_utf16.size(), &str[0], size, nullptr, nullptr);
	if (size == 0) {
		return false;
	}
#ifdef _UTILS_H__HAS_U8STRING_
	out_utf8 = utf8_string(str.begin(), str.end());
#else
	out_utf8 = std::move(str);
#endif
	return true;
}

// ===============
bool Utf8_To_Utf16(const utf8_string &in_utf8, std::wstring &out_utf16) {
	if (in_utf8.empty()) {
		out_utf16 = L"";
		return true;
	}
	// get the required size
	int size = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char *>(&in_utf8[0]), in_utf8.size(), nullptr, 0);
	if (size == 0) {
		return false;
	}
	// convert
	std::wstring wStr;
	wStr.resize(size);
	size = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char *>(&in_utf8[0]), in_utf8.size(), &wStr[0], size);
	if (size == 0) {
		return false;
	}
	out_utf16 = std::move(wStr);
	return true;
}
