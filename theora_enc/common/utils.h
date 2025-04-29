#ifndef _UTILS_H_
#define _UTILS_H_

#include <cstdio>
#include <filesystem>
#include <list>
#include <string>

// ===============
#if (__cplusplus > 201703L)
	#define _UTILS_H__HAS_U8STRING_
#endif

// ===============
#ifdef _UTILS_H__HAS_U8STRING_
	using utf8_string = std::u8string;
#else
	using utf8_string = std::string;
#endif

// ===============
class FileGuard {
public:
	FileGuard(std::FILE *&_file) : file(_file) {}
	~FileGuard() {
		if (file) {
			fclose(file);
			file = nullptr;
		}
	}
private:
	std::FILE *&file;
};

// ===============
std::FILE *				OpenFileReadBinary(const std::filesystem::path &filePath);
std::FILE *				OpenFileWriteBinary(const std::filesystem::path &filePath);
bool					ListFilesInDirectory(const std::filesystem::path &in_directoryPath, const std::filesystem::path &in_fileExtension, std::list<std::filesystem::path> &out_fileNames);
std::filesystem::path	PathFromUtf8(const utf8_string &path);

bool					Utf16_To_Utf8(const std::wstring &in_utf16, utf8_string &out_utf8);
bool					Utf8_To_Utf16(const utf8_string &in_utf8, std::wstring &out_utf16);

// ===============
template <typename arithmeticType>
utf8_string ToUtf8(arithmeticType value) {
#ifdef _UTILS_H__HAS_U8STRING_
	std::string str = std::to_string(value);
	return utf8_string(str.begin(), str.end());
#else
	return std::to_string(value);
#endif
}

#endif // _UTILS_H_
