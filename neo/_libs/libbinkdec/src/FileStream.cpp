/*
 * libbinkdec - Bink video decoder
 * Copyright (C) 2011 Barry Duncan
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "FileStream.h"
#include <stdlib.h>

namespace BinkCommon {

//#modified-fva; BEGIN
/*
bool FileStream::Open(const std::string &fileName)
{
	file.open(fileName.c_str(), std::ifstream::in | std::ifstream::binary);
	if (!file.is_open())
	{
		// log error
		return false;
	}

	return true;
}

bool FileStream::Is_Open()
{
	return file.is_open();
}

void FileStream::Close()
{
	file.close();
}

int32_t FileStream::ReadBytes(uint8_t *data, uint32_t nBytes)
{
	file.read(reinterpret_cast<char*>(data), nBytes);

	if (file.eof()) {
		return 0;
	}
	else if (file.fail()) {
		return 0;
	}
	else if (file.bad()) {
		return 0;
	}

	return static_cast<int32_t>(file.gcount());
}

uint32_t FileStream::ReadUint32LE()
{
	uint32_t value;
	file.read(reinterpret_cast<char*>(&value), 4);
	return value;
}

uint32_t FileStream::ReadUint32BE()
{
	uint32_t value;
	file.read(reinterpret_cast<char*>(&value), 4);
	return _byteswap_ulong(value);
}

uint16_t FileStream::ReadUint16LE()
{
	uint16_t value;
	file.read(reinterpret_cast<char*>(&value), 2);
	return value;
}

uint16_t FileStream::ReadUint16BE()
{
	uint16_t value;
	file.read(reinterpret_cast<char*>(&value), 2);
	return _byteswap_ushort(value);
}

uint8_t FileStream::ReadByte()
{
	uint8_t value;
	file.read(reinterpret_cast<char*>(&value), 1);
	return value;
}

bool FileStream::Seek(int32_t offset, SeekDirection direction)
{
	if (kSeekStart == direction) {
		file.seekg(offset, std::ios::beg);
	}
	else if (kSeekCurrent == direction) {
		file.seekg(offset, std::ios::cur);
	}

	// TODO - end seek

	if (file.bad())
	{
		// todo
		return false;
	}
	if (file.fail())
	{
		// todo
		return false;
	}

	return true;
}

bool FileStream::Skip(int32_t offset)
{
	return Seek(offset, kSeekCurrent);
}

bool FileStream::Is_Eos()
{
	return file.eof();
}

int32_t FileStream::GetPosition()
{
	return static_cast<int32_t>(file.tellg());
}
*/

FileStream::FileStream() {
	memory = nullptr;
	size = 0;
	readPos = 0;
}

bool FileStream::Init(const uint8_t * fileData, int32_t fileSizeInBytes) {
	if (memory || !fileData || fileSizeInBytes <= 0) {
		return false;
	}
	memory = fileData;
	size = fileSizeInBytes;
	readPos = 0;
	return true;
}

uint32_t FileStream::ReadUint32LE() {
	if (!memory) {
		return 0;
	}
	int32_t nextPos = readPos + sizeof(uint32_t);
	if (nextPos > size) {
		readPos = size;
		return 0;
	}
	uint32_t val = 0;
	const uint8_t * memPtr = &memory[nextPos - 1];
	val |= *memPtr--;
	val = (val << 8) | *memPtr--;
	val = (val << 8) | *memPtr--;
	val = (val << 8) | *memPtr;
	readPos = nextPos;
	return val;
}

uint32_t FileStream::ReadUint32BE() {
	if (!memory) {
		return 0;
	}
	int32_t nextPos = readPos + sizeof(uint32_t);
	if (nextPos > size) {
		readPos = size;
		return 0;
	}
	uint32_t val = 0;
	const uint8_t * memPtr = &memory[readPos];
	val |= *memPtr++;
	val = (val << 8) | *memPtr++;
	val = (val << 8) | *memPtr++;
	val = (val << 8) | *memPtr;
	readPos = nextPos;
	return val;
}

uint16_t FileStream::ReadUint16LE() {
	if (!memory) {
		return 0;
	}
	int32_t nextPos = readPos + sizeof(uint16_t);
	if (nextPos > size) {
		readPos = size;
		return 0;
	}
	uint16_t val = 0;
	const uint8_t * memPtr = &memory[nextPos - 1];
	val |= *memPtr--;
	val = (val << 8) | *memPtr;
	readPos = nextPos;
	return val;
}

uint16_t FileStream::ReadUint16BE() {
	if (!memory) {
		return 0;
	}
	int32_t nextPos = readPos + sizeof(uint16_t);
	if (nextPos > size) {
		readPos = size;
		return 0;
	}
	uint16_t val = 0;
	const uint8_t * memPtr = &memory[readPos];
	val |= *memPtr++;
	val = (val << 8) | *memPtr;
	readPos = nextPos;
	return val;
}

uint8_t FileStream::ReadByte() {
	if (!memory) {
		return 0;
	}
	if (readPos >= size) {
		return 0;
	}
	uint8_t val = memory[readPos++];
	return val;
}

bool FileStream::Seek(int32_t offset, SeekDirection direction) {
	if (!memory) {
		return false;
	}
	int32_t newPos;
	switch (direction) {
		case kSeekStart: newPos = offset; break;
		case kSeekCurrent: newPos = readPos + offset; break;
		case kSeekEnd: newPos = size + offset; break;
		default: newPos = -1; break;
	}
	if (newPos < 0 || newPos > size) {
		return false;
	}
	readPos = newPos;
	return true;
}

bool FileStream::Skip(int32_t offset) {
	bool ret = Seek(offset, kSeekCurrent);
	return ret;
}

int32_t FileStream::GetPosition() {
	return readPos;
}
//#modified-fva; END

} // close namespace BinkCommon

