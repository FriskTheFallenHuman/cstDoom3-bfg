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

#define NBM( x ) (int32)( ( 1LL << x ) - 1 )
int maskForNumBits[33] = {	NBM( 0x00 ), NBM( 0x01 ), NBM( 0x02 ), NBM( 0x03 ),
							NBM( 0x04 ), NBM( 0x05 ), NBM( 0x06 ), NBM( 0x07 ),
							NBM( 0x08 ), NBM( 0x09 ), NBM( 0x0A ), NBM( 0x0B ),
							NBM( 0x0C ), NBM( 0x0D ), NBM( 0x0E ), NBM( 0x0F ),
							NBM( 0x10 ), NBM( 0x11 ), NBM( 0x12 ), NBM( 0x13 ),
							NBM( 0x14 ), NBM( 0x15 ), NBM( 0x16 ), NBM( 0x17 ),
							NBM( 0x18 ), NBM( 0x19 ), NBM( 0x1A ), NBM( 0x1B ),
							NBM( 0x1C ), NBM( 0x1D ), NBM( 0x1E ), NBM( 0x1F ), -1 };

#define NBS( x ) (int32)( (-1) << ( x - 1 ) )
int signForNumBits[33] = {	NBS( 0x01 ), NBS( 0x01 ), NBS( 0x02 ), NBS( 0x03 ),
							NBS( 0x04 ), NBS( 0x05 ), NBS( 0x06 ), NBS( 0x07 ),
							NBS( 0x08 ), NBS( 0x09 ), NBS( 0x0A ), NBS( 0x0B ),
							NBS( 0x0C ), NBS( 0x0D ), NBS( 0x0E ), NBS( 0x0F ),
							NBS( 0x10 ), NBS( 0x11 ), NBS( 0x12 ), NBS( 0x13 ),
							NBS( 0x14 ), NBS( 0x15 ), NBS( 0x16 ), NBS( 0x17 ),
							NBS( 0x18 ), NBS( 0x19 ), NBS( 0x1A ), NBS( 0x1B ),
							NBS( 0x1C ), NBS( 0x1D ), NBS( 0x1E ), NBS( 0x1F ), NBS( 0x20 ) };

#define ID_FORCEINLINE __forceinline

/*
========================
idSWFBitStream::idSWFBitStream
========================
*/
idSWFBitStream::idSWFBitStream() {
	free = false;
	Free();
}

/*
========================
idSWFBitStream::operator=
========================
*/
idSWFBitStream & idSWFBitStream::operator=( idSWFBitStream & other ) {
	Free();
	free = other.free;
	startp = other.startp;
	readp = other.readp;
	endp = other.endp;
	currentBit = other.currentBit;
	currentByte = other.currentByte;
	if ( other.free ) {
		// this is actually quite dangerous, but we need to do this
		// because these things are copied around inside idList
		other.free = false;
	}
	return *this;
}

/*
========================
idSWFBitStream::Free
========================
*/
void idSWFBitStream::Free() {
	if ( free ) {
		Mem_Free( (void *)startp );
	}
	free = false;
	startp = NULL;
	endp = NULL;
	readp = NULL;
	ResetBits();
}

/*
========================
idSWFBitStream::Load
========================
*/
void idSWFBitStream::Load( const byte * data, uint32 len, bool copy ) {
	Free();

	if ( copy ) {
		free = true;
		startp = (const byte *)Mem_Alloc( len, TAG_SWF );
		memcpy( (byte *)startp, data, len );
	} else {
		free = false;
		startp = data;
	}
	endp = startp + len;
	readp = startp;

	ResetBits();
}

/*
========================
idSWFBitStream::ReadEncodedU32
========================
*/
uint32 idSWFBitStream::ReadEncodedU32() {
	uint32 result = 0;
	for ( int i = 0; i < 5; i++ ) {
		byte b = ReadU8();
		result |= ( b & 0x7F ) << ( 7 * i );
		if ( ( b & 0x80 ) == 0 ) {
			return result;
		}
	}
	return result;
}

/*
========================
idSWFBitStream::ReadData
========================
*/
const byte * idSWFBitStream::ReadData( int size ) {
	assert( readp >= startp && readp <= endp );
	ResetBits();
	if ( readp + size > endp ) {
		// buffer overrun
		assert( false );
		readp = endp;
		return startp;
	}
	const byte * buffer = readp;
	readp += size;
	assert( readp >= startp && readp <= endp );
	return buffer;
}

/*
========================
idSWFBitStream::ReadInternalU
========================
*/
ID_FORCEINLINE unsigned int idSWFBitStream::ReadInternalU( uint64 & regCurrentBit, uint64 & regCurrentByte, unsigned int numBits ) {
	assert( numBits <= 32 );

	// read bits with only one microcoded shift instruction (shift with variable) on the consoles
	// this routine never reads more than 7 bits beyond the requested number of bits from the stream
	// such that calling ResetBits() never discards more than 7 bits and aligns with the next byte
	uint64 numExtraBytes = ( numBits - regCurrentBit + 7 ) >> 3;
	regCurrentBit = regCurrentBit + ( numExtraBytes << 3 ) - numBits;
	for ( int i = 0; i < numExtraBytes; i++ ) {
		regCurrentByte = ( regCurrentByte << 8 ) | readp[i];
	}
	readp += numExtraBytes;
	return (unsigned int) ( ( regCurrentByte >> regCurrentBit ) & maskForNumBits[numBits] );
}

/*
========================
idSWFBitStream::ReadInternalS
========================
*/
ID_FORCEINLINE int idSWFBitStream::ReadInternalS( uint64 & regCurrentBit, uint64 & regCurrentByte, unsigned int numBits ) {
	int i = (int)ReadInternalU( regCurrentBit, regCurrentByte, numBits );

	// sign extend without microcoded shift instrunction (shift with variable) on the consoles
	int s = signForNumBits[numBits];
	return ( ( i + s ) ^ s );
}

/*
========================
idSWFBitStream::ReadU
========================
*/
unsigned int idSWFBitStream::ReadU( unsigned int numBits ) {
	return ReadInternalU( currentBit, currentByte, numBits );
}

/*
========================
idSWFBitStream::ReadS
========================
*/
int idSWFBitStream::ReadS( unsigned int numBits ) {
	return ReadInternalS( currentBit, currentByte, numBits );
}

/*
========================
idSWFBitStream::ReadRect
========================
*/
void idSWFBitStream::ReadRect( swfRect_t & rect ) {
	uint64 regCurrentBit = 0;
	uint64 regCurrentByte = 0;

	int nBits = ReadInternalU( regCurrentBit, regCurrentByte, 5 );

	int tl_x = ReadInternalS( regCurrentBit, regCurrentByte, nBits );
	int br_x = ReadInternalS( regCurrentBit, regCurrentByte, nBits );
	int tl_y = ReadInternalS( regCurrentBit, regCurrentByte, nBits );
	int br_y = ReadInternalS( regCurrentBit, regCurrentByte, nBits );

	rect.tl.x = SWFTWIP( tl_x );
	rect.br.x = SWFTWIP( br_x );
	rect.tl.y = SWFTWIP( tl_y );
	rect.br.y = SWFTWIP( br_y );

	currentBit = regCurrentBit;
	currentByte = regCurrentByte;
}

/*
========================
idSWFBitStream::ReadMatrix
========================
*/
void idSWFBitStream::ReadMatrix( swfMatrix_t & matrix ) {
	uint64 regCurrentBit = 0;
	uint64 regCurrentByte = 0;


	unsigned int hasScale = ReadInternalU( regCurrentBit, regCurrentByte, 1 );

	int xx;
	int yy;
	if ( !hasScale ) {
		xx = 65536;
		yy = 65536;
	} else {
		int nBits = ReadInternalU( regCurrentBit, regCurrentByte, 5 );
		xx = ReadInternalS( regCurrentBit, regCurrentByte, nBits );
		yy = ReadInternalS( regCurrentBit, regCurrentByte, nBits );
	}

	unsigned int hasRotate = ReadInternalU( regCurrentBit, regCurrentByte, 1 );

	int yx;
	int xy;
	if ( !hasRotate ) {
		yx = 0;
		xy = 0;
	} else {
		int nBits = ReadInternalU( regCurrentBit, regCurrentByte, 5 );
		yx = ReadInternalS( regCurrentBit, regCurrentByte, nBits );
		xy = ReadInternalS( regCurrentBit, regCurrentByte, nBits );
	}

	int nBits = ReadInternalU( regCurrentBit, regCurrentByte, 5 );
	int tx = ReadInternalS( regCurrentBit, regCurrentByte, nBits );
	int ty = ReadInternalS( regCurrentBit, regCurrentByte, nBits );

	currentBit = regCurrentBit;
	currentByte = regCurrentByte;

	matrix.xx = SWFFIXED16( xx );
	matrix.yy = SWFFIXED16( yy );
	matrix.yx = SWFFIXED16( yx );
	matrix.xy = SWFFIXED16( xy );
	matrix.tx = SWFTWIP( tx );
	matrix.ty = SWFTWIP( ty );

}

/*
========================
idSWFBitStream::ReadColorXFormRGBA
========================
*/
void idSWFBitStream::ReadColorXFormRGBA( swfColorXform_t & cxf ) {
	uint64 regCurrentBit = 0;
	uint64 regCurrentByte = 0;

	unsigned int hasAddTerms = ReadInternalU( regCurrentBit, regCurrentByte, 1 );
	unsigned int hasMulTerms = ReadInternalU( regCurrentBit, regCurrentByte, 1 );
	int nBits = ReadInternalU( regCurrentBit, regCurrentByte, 4 );

	union { int i[4]; } m;
	union { int i[4]; } a;

	if ( !hasMulTerms ) {
		m.i[0] = 256;
		m.i[1] = 256;
		m.i[2] = 256;
		m.i[3] = 256;
	} else {
		m.i[0] = ReadInternalS( regCurrentBit, regCurrentByte, nBits );
		m.i[1] = ReadInternalS( regCurrentBit, regCurrentByte, nBits );
		m.i[2] = ReadInternalS( regCurrentBit, regCurrentByte, nBits );
		m.i[3] = ReadInternalS( regCurrentBit, regCurrentByte, nBits );
	}

	if ( !hasAddTerms ) {
		a.i[0] = 0;
		a.i[1] = 0;
		a.i[2] = 0;
		a.i[3] = 0;
	} else {
		a.i[0] = ReadInternalS( regCurrentBit, regCurrentByte, nBits );
		a.i[1] = ReadInternalS( regCurrentBit, regCurrentByte, nBits );
		a.i[2] = ReadInternalS( regCurrentBit, regCurrentByte, nBits );
		a.i[3] = ReadInternalS( regCurrentBit, regCurrentByte, nBits );
	}

	currentBit = regCurrentBit;
	currentByte = regCurrentByte;

	for ( int i = 0; i < 4; i++ ) {
		cxf.mul[i] = SWFFIXED8( m.i[i] );
		cxf.add[i] = SWFFIXED8( a.i[i] );
	}
}

/*
========================
idSWFBitStream::ReadString
========================
*/
const char * idSWFBitStream::ReadString() {
	return (const char *)ReadData( idStr::Length( (const char *)readp ) + 1 );
}

/*
========================
idSWFBitStream::ReadColorRGB
========================
*/
void idSWFBitStream::ReadColorRGB( swfColorRGB_t & color ) {
	ResetBits();
	color.r = *readp++;
	color.g = *readp++;
	color.b = *readp++;
}

/*
========================
idSWFBitStream::ReadColorRGBA
========================
*/
void idSWFBitStream::ReadColorRGBA( swfColorRGBA_t & color ) {
	ResetBits();
	color.r = *readp++;
	color.g = *readp++;
	color.b = *readp++;
	color.a = *readp++;
}

/*
========================
idSWFBitStream::ReadGradient
========================
*/
void idSWFBitStream::ReadGradient( swfGradient_t & grad, bool rgba ) {
	grad.numGradients = ReadU8() & 0xF;	// the top 4 bits control spread and interpolation mode, but we ignore them
	for ( int i = 0; i < grad.numGradients; i++ ) {
		grad.gradientRecords[i].startRatio = ReadU8();
		if ( rgba ) {
			ReadColorRGBA( grad.gradientRecords[i].startColor );
		} else {
			ReadColorRGB( grad.gradientRecords[i].startColor );
		}
		grad.gradientRecords[i].endRatio = grad.gradientRecords[i].startRatio;
		grad.gradientRecords[i].endColor = grad.gradientRecords[i].startColor;
	}
}

/*
========================
idSWFBitStream::ReadMorphGradient
========================
*/
void idSWFBitStream::ReadMorphGradient( swfGradient_t & grad ) {
	grad.numGradients = ReadU8() & 0xF;	// the top 4 bits control spread and interpolation mode, but we ignore them
	for ( int i = 0; i < grad.numGradients; i++ ) {
		grad.gradientRecords[i].startRatio = ReadU8();
		ReadColorRGBA( grad.gradientRecords[i].startColor );
		grad.gradientRecords[i].endRatio = ReadU8();
		ReadColorRGBA( grad.gradientRecords[i].endColor );
	}
}

//#modified-fva; BEGIN
CstSWFBitstream::CstSWFBitstream() :
	buffer(16384), // granularity
	writePos(0),
	bitIndex(0)
{
}

// ===============
void CstSWFBitstream::WriteU8(uint8 value) {
	WriteByte(value);
}

// ===============
void CstSWFBitstream::WriteU16(uint16 value) {
	WriteByte(value & 0xFF);
	WriteByte((value >> 8) & 0xFF);
}

// ===============
void CstSWFBitstream::WriteU32(uint32 value) {
	WriteByte(value & 0xFF);
	WriteByte((value >> 8) & 0xFF);
	WriteByte((value >> 16) & 0xFF);
	WriteByte((value >> 24) & 0xFF);
}

// ===============
void CstSWFBitstream::WriteS16(int16 value) {
	WriteByte(value & 0xFF);
	WriteByte((value >> 8) & 0xFF);
}

// ===============
void CstSWFBitstream::WriteS32(int32 value) {
	WriteByte(value & 0xFF);
	WriteByte((value >> 8) & 0xFF);
	WriteByte((value >> 16) & 0xFF);
	WriteByte((value >> 24) & 0xFF);
}

// ===============
void CstSWFBitstream::WriteFloat(float value) {
	const uint32 &uintVal = *(uint32 *)&value;
	WriteByte(uintVal & 0xFF);
	WriteByte((uintVal >> 8) & 0xFF);
	WriteByte((uintVal >> 16) & 0xFF);
	WriteByte((uintVal >> 24) & 0xFF);
}

// ===============
void CstSWFBitstream::WriteDouble(double value) {
	// doubles are stored in an unexpected way; see idSWFBitStream::ReadDouble
	const uint64 &uintVal = *(uint64 *)&value;
	WriteByte((uintVal >> 32) & 0xFF);
	WriteByte((uintVal >> 40) & 0xFF);
	WriteByte((uintVal >> 48) & 0xFF);
	WriteByte((uintVal >> 56) & 0xFF);
	WriteByte(uintVal & 0xFF);
	WriteByte((uintVal >> 8) & 0xFF);
	WriteByte((uintVal >> 16) & 0xFF);
	WriteByte((uintVal >> 24) & 0xFF);
}

// ===============
void CstSWFBitstream::WriteString(const char *str) {
	byte value;
	do {
		value = *str++;
		WriteByte(value);
	} while (value != 0);
}

// ===============
void CstSWFBitstream::WriteMatrix(const swfMatrix_t &matrix) {
	// values must be clamped due to the maximum number of bits that can be written
	const int32 max = (1LL << 30) - 1;
	const int32 min = -(1LL << 30);
	{
		int32 xx = idMath::ClampInt(min, max, ToFixed16(matrix.xx));
		int32 yy = idMath::ClampInt(min, max, ToFixed16(matrix.yy));
		if (xx != 65536 || yy != 65536) {
			// has scale
			WriteBits(1, 1);
			int xxNumBits = BitsForSigned(xx);
			int yyNumBits = BitsForSigned(yy);
			int numBits = (xxNumBits > yyNumBits) ? xxNumBits : yyNumBits;
			WriteBits(numBits, 5);
			WriteBits(xx, numBits);
			WriteBits(yy, numBits);
		} else {
			// no scale
			WriteBits(0, 1);
		}
	}
	{
		int32 xy = idMath::ClampInt(min, max, ToFixed16(matrix.xy));
		int32 yx = idMath::ClampInt(min, max, ToFixed16(matrix.yx));
		if (xy || yx) {
			// has rotate
			WriteBits(1, 1);
			int xyNumBits = BitsForSigned(xy);
			int yxNumBits = BitsForSigned(yx);
			int numBits = (xyNumBits > yxNumBits) ? xyNumBits : yxNumBits;
			WriteBits(numBits, 5);
			WriteBits(yx, numBits); // yx goes first; see idSWFBitStream::ReadMatrix
			WriteBits(xy, numBits);
		} else {
			// no rotate
			WriteBits(0, 1);
		}
	}
	{
		// translate
		int32 tx = idMath::ClampInt(min, max, ToTwips(matrix.tx));
		int32 ty = idMath::ClampInt(min, max, ToTwips(matrix.ty));
		int numBits = 0; // 0 bits is for binary compatibility with the original menus
		if (tx || ty) {
			int txNumBits = BitsForSigned(tx);
			int tyNumBits = BitsForSigned(ty);
			numBits = (txNumBits > tyNumBits) ? txNumBits : tyNumBits;
		}
		WriteBits(numBits, 5);
		WriteBits(tx, numBits);
		WriteBits(ty, numBits);
	}
	ByteAlign();
}

// ===============
void CstSWFBitstream::WriteColorXFormRGBA(const swfColorXform_t &cxf) {
	// values must be clamped due to the maximum number of bits that can be written
	const int32 max = (1LL << 14) - 1;
	const int32 min = -(1LL << 14);

	int16 mul[4];
	mul[0] = idMath::ClampInt(min, max, ToFixed8(cxf.mul.x));
	mul[1] = idMath::ClampInt(min, max, ToFixed8(cxf.mul.y));
	mul[2] = idMath::ClampInt(min, max, ToFixed8(cxf.mul.z));
	mul[3] = idMath::ClampInt(min, max, ToFixed8(cxf.mul.w));
	bool hasMul = (mul[0] != 256) || (mul[1] != 256) || (mul[2] != 256) || (mul[3] != 256);

	int16 add[4];
	add[0] = idMath::ClampInt(min, max, ToFixed8(cxf.add.x));
	add[1] = idMath::ClampInt(min, max, ToFixed8(cxf.add.y));
	add[2] = idMath::ClampInt(min, max, ToFixed8(cxf.add.z));
	add[3] = idMath::ClampInt(min, max, ToFixed8(cxf.add.w));
	bool hasAdd = add[0] || add[1] || add[2] || add[3];

	int numBits = 1;
	if (hasMul) {
		for (int i = 0; i < 4; ++i) {
			int bits = BitsForSigned(mul[i]);
			if (bits > numBits) {
				numBits = bits;
			}
		}
	}
	if (hasAdd) {
		for (int i = 0; i < 4; ++i) {
			int bits = BitsForSigned(add[i]);
			if (bits > numBits) {
				numBits = bits;
			}
		}
	}

	WriteBits(hasAdd ? 1 : 0, 1);
	WriteBits(hasMul ? 1 : 0, 1);
	WriteBits(numBits, 4);
	if (hasMul) {
		WriteBits(mul[0], numBits);
		WriteBits(mul[1], numBits);
		WriteBits(mul[2], numBits);
		WriteBits(mul[3], numBits);
	}
	if (hasAdd) {
		WriteBits(add[0], numBits);
		WriteBits(add[1], numBits);
		WriteBits(add[2], numBits);
		WriteBits(add[3], numBits);
	}
	ByteAlign();
}

// ===============
int CstSWFBitstream::Length() {
	return buffer.Num();
}

// ===============
int CstSWFBitstream::Tell() {
	return writePos;
}

// ===============
void CstSWFBitstream::Seek(int byteIndex) {
	if (byteIndex < 0) {
		writePos = 0;
		return;
	}
	if (byteIndex > Length()) {
		writePos = Length();
		return;
	}
	writePos = byteIndex;
}

// ===============
const byte * CstSWFBitstream::Ptr() {
	return buffer.Ptr();
}

// ===============
void CstSWFBitstream::ByteAlign() {
	if (bitIndex > 0) {
		++writePos;
		bitIndex = 0;
	}
}

// ===============
void CstSWFBitstream::WriteBits(uint32 value, unsigned numBits) {
	if (numBits == 0 || numBits > 32) {
		return;
	}

	unsigned bitsAvailable = 8 - bitIndex;
	byte currentByte;
	ReadByte(currentByte);

	if (numBits <= bitsAvailable) {
		value &= (1uLL << numBits) - 1; // apply mask
		currentByte |= value << (bitsAvailable - numBits);
		bitIndex += numBits;
		if (bitIndex == 8) {
			WriteByte(currentByte);
			bitIndex = 0;
		} else {
			WriteByte(currentByte, false);
		}
		return;
	}

	// first byte
	numBits -= bitsAvailable;
	currentByte |= (value >> numBits) & ((1uLL << bitsAvailable) - 1);
	WriteByte(currentByte);

	// middle bytes
	while (numBits >= 8) {
		numBits -= 8;
		currentByte = (value >> numBits) & 0xFF;
		WriteByte(currentByte);
	}

	// last byte
	if (numBits > 0) {
		currentByte = value & ((1uLL << numBits) - 1);
		currentByte <<= 8 - numBits;
		WriteByte(currentByte, false);
		bitIndex = numBits;
	} else {
		bitIndex = 0;
	}
}

// ===============
bool CstSWFBitstream::ReadByte(byte &value, byte _default) {
	if (writePos == Length()) {
		value = _default;
		return false;
	}
	value = buffer[writePos];
	return true;
}

// ===============
void CstSWFBitstream::WriteByte(byte value, bool advance) {
	if (writePos == Length()) {
		buffer.Append(value);
	} else {
		buffer[writePos] = value;
	}
	if (advance) {
		++writePos;
	}
}

// ===============
int CstSWFBitstream::BitsForUnsigned(uint32 value) {
	int bitCount = 0;
	do {
		value >>= 1;
		++bitCount;
	} while (value > 0);
	return bitCount;
}

// ===============
int CstSWFBitstream::BitsForSigned(int32 value) {
	if (value == 0) {
		return 1;
	}
	if (value == -(1LL << 31)) {
		return 32;
	}
	if (value < 0) {
		// This approach was chosen for binary compatibility with the original
		// menus. It isn't accurate, though, as it reports one bit in excess
		// for negative numbers that are powers of two.
		value = -value;
	}
	return BitsForUnsigned((uint32)value) + 1;
}

// ===============
int32 CstSWFBitstream::ToFixed16(float value) {
	const int64 max = (1LL << 31) - 1; // max for int32
	const int64 min = -(1LL << 31); // min for int32
	int64 fixed = (int64)(value * 65536.0f);
	if (fixed > max) {
		return (int32)max;
	}
	if (fixed < min) {
		return (int32)min;
	}
	return (int32)fixed;
}

// ===============
int16 CstSWFBitstream::ToFixed8(float value) {
	const int32 max = (1LL << 15) - 1; // max for int16
	const int32 min = -(1LL << 15); // min for int16
	int32 fixed = (int32)(value * 256.0f);
	if (fixed > max) {
		return (int16)max;
	}
	if (fixed < min) {
		return (int16)min;
	}
	return (int16)fixed;
}

// ===============
int32 CstSWFBitstream::ToTwips(float value) {
	const int64 max = (1LL << 31) - 1; // max for int32
	const int64 min = -(1LL << 31); // min for int32
	int64 twips = (int64)(value * 20.0f);
	if (twips > max) {
		return (int32)max;
	}
	if (twips < min) {
		return (int32)min;
	}
	return (int32)twips;
}
//#modified-fva; END
