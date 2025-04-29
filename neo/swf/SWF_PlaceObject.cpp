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

int c_PlaceObject2;
int c_PlaceObject3;

#define PlaceFlagHasClipActions		BIT( 7 )
#define PlaceFlagHasClipDepth		BIT( 6 )
#define PlaceFlagHasName			BIT( 5 )
#define PlaceFlagHasRatio			BIT( 4 )
#define PlaceFlagHasColorTransform	BIT( 3 )
#define PlaceFlagHasMatrix			BIT( 2 )
#define PlaceFlagHasCharacter		BIT( 1 )
#define PlaceFlagMove				BIT( 0 )

#define PlaceFlagPad0				BIT( 7 )
#define PlaceFlagPad1				BIT( 6 )
#define PlaceFlagPad2				BIT( 5 )
#define PlaceFlagHasImage			BIT( 4 )
#define PlaceFlagHasClassName		BIT( 3 )
#define PlaceFlagCacheAsBitmap		BIT( 2 )
#define PlaceFlagHasBlendMode		BIT( 1 )
#define PlaceFlagHasFilterList		BIT( 0 )

/*
========================
idSWFSpriteInstance::PlaceObject2
========================
*/
void idSWFSpriteInstance::PlaceObject2( idSWFBitStream & bitstream ) {
	c_PlaceObject2++;

	uint64 flags = bitstream.ReadU8();
	int depth = bitstream.ReadU16();

	int characterID = -1;
	if ( ( flags & PlaceFlagHasCharacter ) != 0 ) {
		characterID = bitstream.ReadU16();
	}

	swfDisplayEntry_t * display = NULL;

	if ( ( flags & PlaceFlagMove ) != 0 ) {
		// modify an existing entry
		display = FindDisplayEntry( depth );
		if ( display == NULL ) {
			idLib::Warning( "PlaceObject2: trying to modify entry %d, which doesn't exist", depth );
			return;
		}
		if ( characterID >= 0 ) {
			// We are very picky about what kind of objects can change characters
			// Shapes can become other shapes, but sprites can never change
			if ( display->spriteInstance || display->textInstance ) {
				idLib::Warning( "PlaceObject2: Trying to change the character of a sprite after it's been created" );
				return;
			}
			idSWFDictionaryEntry * dictEntry = sprite->swf->FindDictionaryEntry( characterID );
			if ( dictEntry != NULL ) {
				if ( dictEntry->type == SWF_DICT_SPRITE || dictEntry->type == SWF_DICT_EDITTEXT ) {
					idLib::Warning( "PlaceObject2: Trying to change the character of a shape to a sprite" );
					return;
				}
			}
			display->characterID = characterID;
		}
	} else {
		if ( characterID < 0 ) {
			idLib::Warning( "PlaceObject2: Trying to create a new object without a character" );
			return;
		}
		// create a new entry
		display = AddDisplayEntry( depth, characterID );
		if ( display == NULL ) {
			idLib::Warning( "PlaceObject2: trying to create a new entry at %d, but an item already exists there", depth );
			return;
		}
	}
	if ( ( flags & PlaceFlagHasMatrix ) != 0 ) {
		bitstream.ReadMatrix( display->matrix );
	}
	if ( ( flags & PlaceFlagHasColorTransform ) != 0 ) {
		bitstream.ReadColorXFormRGBA( display->cxf );
	}
	if ( ( flags & PlaceFlagHasRatio ) != 0 ) {
		display->ratio = bitstream.ReadU16() * ( 1.0f / 65535.0f );
	}
	if ( ( flags & PlaceFlagHasName ) != 0 ) {
		idStr name = bitstream.ReadString();
		if ( display->spriteInstance ) {
			display->spriteInstance->name = name;
			scriptObject->Set( name, display->spriteInstance->GetScriptObject() );
		} else if ( display->textInstance ) {
			scriptObject->Set( name, display->textInstance->GetScriptObject() );
		}
	}
	if ( ( flags & PlaceFlagHasClipDepth ) != 0 ) {
		display->clipDepth = bitstream.ReadU16();
	}
	if ( ( flags & PlaceFlagHasClipActions ) != 0 ) {
		// FIXME: clip actions
	}
}

//#modified-fva; BEGIN
void idSWFSpriteInstance::CstWriteJson_PlaceObject2(JsonPrettyWriter &writer, idSWFBitStream &bitstream) {
	// see idSWFSpriteInstance::PlaceObject2
	writer.StartObject();
	{
		writer.Key("tag");
		writer.String("PlaceObject2");

		uint64 flags = bitstream.ReadU8();
		unsigned depth = bitstream.ReadU16();

		writer.Key("depth");
		writer.Uint(depth);

		writer.Key("move");
		writer.Bool((flags & PlaceFlagMove) != 0);

		if ((flags & PlaceFlagHasCharacter) != 0) {
			unsigned characterID = bitstream.ReadU16();
			writer.Key("characterID");
			writer.String(idStr(characterID).c_str());
		}
		if ((flags & PlaceFlagHasMatrix) != 0) {
			swfMatrix_t matrix;
			bitstream.ReadMatrix(matrix);
			writer.Key("matrix");
			writer.StartArray();
			writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
			{
				writer.Double(matrix.xx);
				writer.Double(matrix.yy);
				writer.Double(matrix.xy);
				writer.Double(matrix.yx);
				writer.Double(matrix.tx);
				writer.Double(matrix.ty);
			}
			writer.EndArray();
			writer.SetFormatOptions(rapidjson::kFormatDefault);
		}
		if ((flags & PlaceFlagHasColorTransform) != 0) {
			swfColorXform_t cxf;
			bitstream.ReadColorXFormRGBA(cxf);
			writer.Key("colorTransform");
			writer.StartObject();
			{
				writer.Key("mul");
				writer.StartArray();
				writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
				{
					writer.Double(cxf.mul.x);
					writer.Double(cxf.mul.y);
					writer.Double(cxf.mul.z);
					writer.Double(cxf.mul.w);
				}
				writer.EndArray();
				writer.SetFormatOptions(rapidjson::kFormatDefault);
				writer.Key("add");
				writer.StartArray();
				writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
				{
					writer.Double(cxf.add.x);
					writer.Double(cxf.add.y);
					writer.Double(cxf.add.z);
					writer.Double(cxf.add.w);
				}
				writer.EndArray();
				writer.SetFormatOptions(rapidjson::kFormatDefault);
			}
			writer.EndObject();
		}
		if ((flags & PlaceFlagHasRatio) != 0) {
			unsigned ratio = bitstream.ReadU16();
			writer.Key("ratio");
			writer.Uint(ratio);
		}
		if ((flags & PlaceFlagHasName) != 0) {
			const char *name = bitstream.ReadString();
			writer.Key("name");
			writer.String(name);
		}
		if ((flags & PlaceFlagHasClipDepth) != 0) {
			unsigned clipDepth = bitstream.ReadU16();
			writer.Key("clipDepth");
			writer.Uint(clipDepth);
		}
	}
	writer.EndObject();
}

// ===============
void idSWFSpriteInstance::CstLoadJson_PlaceObject2(JsonValue &objPlace, CstSWFBitstream &cstBitstream, const idList<idStr> &characterIDs, idList<idStr> &errorMsg) {
	// may throw CstJson::Except
	assert(objPlace.IsObject());
	uint64 flags = 0;

	uint16 depth = CstJson::Object_GetUint16(objPlace, "depth", errorMsg);

	const char keyMove[] = "move";
	bool move = CstJson::Object_GetBool(objPlace, keyMove, errorMsg);
	if (move) {
		flags |= PlaceFlagMove;
	}

	const char keyCharacterID[] = "characterID";
	uint16 characterID = 0;
	if (objPlace.HasMember(keyCharacterID)) {
		idStr oldCharacterID = CstJson::Object_GetString(objPlace, keyCharacterID, errorMsg);
		int newCharacterID = characterIDs.FindIndex(oldCharacterID);
		if (newCharacterID == -1) {
			errorMsg.Append("No dictionary entry matches the specified value.");
			errorMsg.Append(keyCharacterID);
			throw CstJson::Except();
		}
		characterID = (uint16)newCharacterID; // adjusted characterID
		flags |= PlaceFlagHasCharacter;
	}

	if ((flags & PlaceFlagMove) == 0 && (flags & PlaceFlagHasCharacter) == 0) {
		idStr msg; msg.Format("'%s' must be specified when '%s' is false", keyCharacterID, keyMove); errorMsg.Append(msg);
		throw CstJson::Except();
	}

	const char keyMatrix[] = "matrix";
	swfMatrix_t matrix;
	if (objPlace.HasMember(keyMatrix)) {
		JsonValue &arrayMatrix = CstJson::Object_GetArray(objPlace, keyMatrix, errorMsg, 6);
		JsonSizeType index;
		try {
			matrix.xx = CstJson::Array_GetFloat(arrayMatrix, index = 0, errorMsg);
			matrix.yy = CstJson::Array_GetFloat(arrayMatrix, index = 1, errorMsg);
			matrix.xy = CstJson::Array_GetFloat(arrayMatrix, index = 2, errorMsg);
			matrix.yx = CstJson::Array_GetFloat(arrayMatrix, index = 3, errorMsg);
			matrix.tx = CstJson::Array_GetFloat(arrayMatrix, index = 4, errorMsg);
			matrix.ty = CstJson::Array_GetFloat(arrayMatrix, index = 5, errorMsg);
		}
		catch (CstJson::Except) {
			idStr msg; msg.Format("%s (%u)", keyMatrix, index); errorMsg.Append(msg);
			throw;
		}
		flags |= PlaceFlagHasMatrix;
	}

	const char keyColorTransform[] = "colorTransform";
	swfColorXform_t cxf;
	if (objPlace.HasMember(keyColorTransform)) {
		JsonValue &objColorTransform = CstJson::Object_GetObject(objPlace, keyColorTransform, errorMsg);
		try {
			const char keyMul[] = "mul";
			JsonValue &arrayMul = CstJson::Object_GetArray(objColorTransform, keyMul, errorMsg, 4);
			JsonSizeType index;
			try {
				cxf.mul.x = CstJson::Array_GetFloat(arrayMul, index = 0, errorMsg);
				cxf.mul.y = CstJson::Array_GetFloat(arrayMul, index = 1, errorMsg);
				cxf.mul.z = CstJson::Array_GetFloat(arrayMul, index = 2, errorMsg);
				cxf.mul.w = CstJson::Array_GetFloat(arrayMul, index = 3, errorMsg);
			}
			catch (CstJson::Except) {
				idStr msg; msg.Format("%s (%u)", keyMul, index); errorMsg.Append(msg);
				throw;
			}
			const char keyAdd[] = "add";
			JsonValue &arrayAdd = CstJson::Object_GetArray(objColorTransform, keyAdd, errorMsg, 4);
			try {
				cxf.add.x = CstJson::Array_GetFloat(arrayAdd, index = 0, errorMsg);
				cxf.add.y = CstJson::Array_GetFloat(arrayAdd, index = 1, errorMsg);
				cxf.add.z = CstJson::Array_GetFloat(arrayAdd, index = 2, errorMsg);
				cxf.add.w = CstJson::Array_GetFloat(arrayAdd, index = 3, errorMsg);
			}
			catch (CstJson::Except) {
				idStr msg; msg.Format("%s (%u)", keyAdd, index); errorMsg.Append(msg);
				throw;
			}
		}
		catch (CstJson::Except) {
			errorMsg.Append(keyColorTransform);
			throw;
		}
		flags |= PlaceFlagHasColorTransform;
	}

	const char keyRatio[] = "ratio";
	uint16 ratio = 0;
	if (objPlace.HasMember(keyRatio)) {
		ratio = CstJson::Object_GetUint16(objPlace, keyRatio, errorMsg);
		flags |= PlaceFlagHasRatio;
	}

	const char keyName[] = "name";
	const char *name = NULL;
	if (objPlace.HasMember(keyName)) {
		name = CstJson::Object_GetString(objPlace, keyName, errorMsg);
		flags |= PlaceFlagHasName;
	}

	const char keyClipDepth[] = "clipDepth";
	uint16 clipDepth = 0;
	if (objPlace.HasMember(keyClipDepth)) {
		clipDepth = CstJson::Object_GetUint16(objPlace, keyClipDepth, errorMsg);
		flags |= PlaceFlagHasClipDepth;
	}

	cstBitstream.WriteU8(flags);
	cstBitstream.WriteU16(depth);
	if (flags & PlaceFlagHasCharacter) {
		cstBitstream.WriteU16(characterID);
	}
	if (flags & PlaceFlagHasMatrix) {
		cstBitstream.WriteMatrix(matrix);
	}
	if (flags & PlaceFlagHasColorTransform) {
		cstBitstream.WriteColorXFormRGBA(cxf);
	}
	if (flags & PlaceFlagHasRatio) {
		cstBitstream.WriteU16(ratio);
	}
	if (flags & PlaceFlagHasName) {
		cstBitstream.WriteString(name);
	}
	if (flags & PlaceFlagHasClipDepth) {
		cstBitstream.WriteU16(clipDepth);
	}
}
//#modified-fva; END

/*
========================
idSWFSpriteInstance::PlaceObject3
========================
*/
void idSWFSpriteInstance::PlaceObject3( idSWFBitStream & bitstream ) {
	c_PlaceObject3++;

	uint64 flags1 = bitstream.ReadU8();
	uint64 flags2 = bitstream.ReadU8();
	uint16 depth = bitstream.ReadU16();

	if ( ( flags2 & PlaceFlagHasClassName ) != 0 || ( ( ( flags2 & PlaceFlagHasImage ) != 0 ) && ( ( flags1 & PlaceFlagHasCharacter ) != 0 ) ) ) {
		bitstream.ReadString(); // ignored
	}

	int characterID = -1;
	if ( ( flags1 & PlaceFlagHasCharacter ) != 0 ) {
		characterID = bitstream.ReadU16();
	}

	swfDisplayEntry_t * display = NULL;

	if ( ( flags1 & PlaceFlagMove ) != 0 ) {
		// modify an existing entry
		display = FindDisplayEntry( depth );
		if ( display == NULL ) {
			idLib::Warning( "PlaceObject3: trying to modify entry %d, which doesn't exist", depth );
			return;
		}
		if ( characterID >= 0 ) {
			// We are very picky about what kind of objects can change characters
			// Shapes can become other shapes, but sprites can never change
			if ( display->spriteInstance || display->textInstance ) {
				idLib::Warning( "PlaceObject3: Trying to change the character of a sprite after it's been created" );
				return;
			}
			idSWFDictionaryEntry * dictEntry = sprite->swf->FindDictionaryEntry( characterID );
			if ( dictEntry != NULL ) {
				if ( dictEntry->type == SWF_DICT_SPRITE || dictEntry->type == SWF_DICT_EDITTEXT ) {
					idLib::Warning( "PlaceObject3: Trying to change the character of a shape to a sprite" );
					return;
				}
			}
			display->characterID = characterID;
		}
	} else {
		if ( characterID < 0 ) {
			idLib::Warning( "PlaceObject3: Trying to create a new object without a character" );
			return;
		}
		// create a new entry
		display = AddDisplayEntry( depth, characterID );
		if ( display == NULL ) {
			idLib::Warning( "PlaceObject3: trying to create a new entry at %d, but an item already exists there", depth );
			return;
		}
	}
	if ( ( flags1 & PlaceFlagHasMatrix ) != 0 ) {
		bitstream.ReadMatrix( display->matrix );
	}
	if ( ( flags1 & PlaceFlagHasColorTransform ) != 0 ) {
		bitstream.ReadColorXFormRGBA( display->cxf );
	}
	if ( ( flags1 & PlaceFlagHasRatio ) != 0 ) {
		display->ratio = bitstream.ReadU16() * ( 1.0f / 65535.0f );
	}
	if ( ( flags1 & PlaceFlagHasName ) != 0 ) {
		idStr name = bitstream.ReadString();
		if ( display->spriteInstance ) {
			display->spriteInstance->name = name;
			scriptObject->Set( name, display->spriteInstance->GetScriptObject() );
		} else if ( display->textInstance ) {
			scriptObject->Set( name, display->textInstance->GetScriptObject() );
		}
	}
	if ( ( flags1 & PlaceFlagHasClipDepth ) != 0 ) {
		display->clipDepth = bitstream.ReadU16();
	}
	if ( ( flags2 & PlaceFlagHasFilterList ) != 0 ) {
		// we don't support filters and because the filter list is variable length we
		// can't support anything after the filter list either (blend modes and clip actions)
		idLib::Warning( "PlaceObject3: has filters" );
		return;
	}
	if ( ( flags2 & PlaceFlagHasBlendMode ) != 0 ) {
		display->blendMode = bitstream.ReadU8();
	}
	if ( ( flags1 & PlaceFlagHasClipActions ) != 0 ) {
		// FIXME:
	}
}

//#modified-fva; BEGIN
void idSWFSpriteInstance::CstWriteJson_PlaceObject3(JsonPrettyWriter &writer, idSWFBitStream &bitstream) {
	// see idSWFSpriteInstance::PlaceObject3
	writer.StartObject();
	{
		writer.Key("tag");
		writer.String("PlaceObject3");

		uint64 flags1 = bitstream.ReadU8();
		uint64 flags2 = bitstream.ReadU8();
		unsigned depth = bitstream.ReadU16();

		writer.Key("depth");
		writer.Uint(depth);

		writer.Key("move");
		writer.Bool((flags1 & PlaceFlagMove) != 0);

		if ((flags2 & PlaceFlagHasClassName) != 0 || (((flags2 & PlaceFlagHasImage) != 0) && ((flags1 & PlaceFlagHasCharacter) != 0))) {
			bitstream.ReadString(); // ignored
		}
		if ((flags1 & PlaceFlagHasCharacter) != 0) {
			unsigned characterID = bitstream.ReadU16();
			writer.Key("characterID");
			writer.String(idStr(characterID).c_str());
		}
		if ((flags1 & PlaceFlagHasMatrix) != 0) {
			swfMatrix_t matrix;
			bitstream.ReadMatrix(matrix);
			writer.Key("matrix");
			writer.StartArray();
			writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
			{
				writer.Double(matrix.xx);
				writer.Double(matrix.yy);
				writer.Double(matrix.xy);
				writer.Double(matrix.yx);
				writer.Double(matrix.tx);
				writer.Double(matrix.ty);
			}
			writer.EndArray();
			writer.SetFormatOptions(rapidjson::kFormatDefault);
		}
		if ((flags1 & PlaceFlagHasColorTransform) != 0) {
			swfColorXform_t cxf;
			bitstream.ReadColorXFormRGBA(cxf);
			writer.Key("colorTransform");
			writer.StartObject();
			{
				writer.Key("mul");
				writer.StartArray();
				writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
				{
					writer.Double(cxf.mul.x);
					writer.Double(cxf.mul.y);
					writer.Double(cxf.mul.z);
					writer.Double(cxf.mul.w);
				}
				writer.EndArray();
				writer.SetFormatOptions(rapidjson::kFormatDefault);
				writer.Key("add");
				writer.StartArray();
				writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
				{
					writer.Double(cxf.add.x);
					writer.Double(cxf.add.y);
					writer.Double(cxf.add.z);
					writer.Double(cxf.add.w);
				}
				writer.EndArray();
				writer.SetFormatOptions(rapidjson::kFormatDefault);
			}
			writer.EndObject();
		}
		if ((flags1 & PlaceFlagHasRatio) != 0) {
			unsigned ratio = bitstream.ReadU16();
			writer.Key("ratio");
			writer.Uint(ratio);
		}
		if ((flags1 & PlaceFlagHasName) != 0) {
			idStr name = bitstream.ReadString();
			writer.Key("name");
			writer.String(name.c_str());
		}
		if ((flags1 & PlaceFlagHasClipDepth) != 0) {
			unsigned clipDepth = bitstream.ReadU16();
			writer.Key("clipDepth");
			writer.Uint(clipDepth);
		}
		if ((flags2 & PlaceFlagHasFilterList) == 0 && (flags2 & PlaceFlagHasBlendMode) != 0) {
			// see idSWF::GLStateForRenderState
			unsigned blendMode = bitstream.ReadU8();
			writer.Key("blendMode");
			writer.Uint(blendMode);
		}
	}
	writer.EndObject();
}

// ===============
void idSWFSpriteInstance::CstLoadJson_PlaceObject3(JsonValue &objPlace, CstSWFBitstream &cstBitstream, const idList<idStr> &characterIDs, idList<idStr> &errorMsg) {
	// may throw CstJson::Except
	assert(objPlace.IsObject());
	uint64 flags1 = 0;
	uint64 flags2 = 0;

	uint16 depth = CstJson::Object_GetUint16(objPlace, "depth", errorMsg);

	const char keyMove[] = "move";
	bool move = CstJson::Object_GetBool(objPlace, keyMove, errorMsg);
	if (move) {
		flags1 |= PlaceFlagMove;
	}

	const char keyCharacterID[] = "characterID";
	uint16 characterID = 0;
	if (objPlace.HasMember(keyCharacterID)) {
		idStr oldCharacterID = CstJson::Object_GetString(objPlace, keyCharacterID, errorMsg);
		int newCharacterID = characterIDs.FindIndex(oldCharacterID);
		if (newCharacterID == -1) {
			errorMsg.Append("No dictionary entry matches the specified value.");
			errorMsg.Append(keyCharacterID);
			throw CstJson::Except();
		}
		characterID = (uint16)newCharacterID; // adjusted characterID value
		flags1 |= PlaceFlagHasCharacter;
	}

	if ((flags1 & PlaceFlagMove) == 0 && (flags1 & PlaceFlagHasCharacter) == 0) {
		idStr msg; msg.Format("'%s' must be specified when '%s' is false", keyCharacterID, keyMove); errorMsg.Append(msg);
		throw CstJson::Except();
	}

	const char keyMatrix[] = "matrix";
	swfMatrix_t matrix;
	if (objPlace.HasMember(keyMatrix)) {
		JsonValue &arrayMatrix = CstJson::Object_GetArray(objPlace, keyMatrix, errorMsg, 6);
		JsonSizeType index;
		try {
			matrix.xx = CstJson::Array_GetFloat(arrayMatrix, index = 0, errorMsg);
			matrix.yy = CstJson::Array_GetFloat(arrayMatrix, index = 1, errorMsg);
			matrix.xy = CstJson::Array_GetFloat(arrayMatrix, index = 2, errorMsg);
			matrix.yx = CstJson::Array_GetFloat(arrayMatrix, index = 3, errorMsg);
			matrix.tx = CstJson::Array_GetFloat(arrayMatrix, index = 4, errorMsg);
			matrix.ty = CstJson::Array_GetFloat(arrayMatrix, index = 5, errorMsg);
		}
		catch (CstJson::Except) {
			idStr msg; msg.Format("%s (%u)", keyMatrix, index); errorMsg.Append(msg);
			throw;
		}
		flags1 |= PlaceFlagHasMatrix;
	}

	const char keyColorTransform[] = "colorTransform";
	swfColorXform_t cxf;
	if (objPlace.HasMember(keyColorTransform)) {
		JsonValue &objColorTransform = CstJson::Object_GetObject(objPlace, keyColorTransform, errorMsg);
		try {
			const char keyMul[] = "mul";
			JsonValue &arrayMul = CstJson::Object_GetArray(objColorTransform, keyMul, errorMsg, 4);
			JsonSizeType index;
			try {
				cxf.mul.x = CstJson::Array_GetFloat(arrayMul, index = 0, errorMsg);
				cxf.mul.y = CstJson::Array_GetFloat(arrayMul, index = 1, errorMsg);
				cxf.mul.z = CstJson::Array_GetFloat(arrayMul, index = 2, errorMsg);
				cxf.mul.w = CstJson::Array_GetFloat(arrayMul, index = 3, errorMsg);
			}
			catch (CstJson::Except) {
				idStr msg; msg.Format("%s (%u)", keyMul, index); errorMsg.Append(msg);
				throw;
			}
			const char keyAdd[] = "add";
			JsonValue &arrayAdd = CstJson::Object_GetArray(objColorTransform, keyAdd, errorMsg, 4);
			try {
				cxf.add.x = CstJson::Array_GetFloat(arrayAdd, index = 0, errorMsg);
				cxf.add.y = CstJson::Array_GetFloat(arrayAdd, index = 1, errorMsg);
				cxf.add.z = CstJson::Array_GetFloat(arrayAdd, index = 2, errorMsg);
				cxf.add.w = CstJson::Array_GetFloat(arrayAdd, index = 3, errorMsg);
			}
			catch (CstJson::Except) {
				idStr msg; msg.Format("%s (%u)", keyAdd, index); errorMsg.Append(msg);
				throw;
			}
		}
		catch (CstJson::Except) {
			errorMsg.Append(keyColorTransform);
			throw;
		}
		flags1 |= PlaceFlagHasColorTransform;
	}

	const char keyRatio[] = "ratio";
	uint16 ratio = 0;
	if (objPlace.HasMember(keyRatio)) {
		ratio = CstJson::Object_GetUint16(objPlace, keyRatio, errorMsg);
		flags1 |= PlaceFlagHasRatio;
	}

	const char keyName[] = "name";
	const char *name = NULL;
	if (objPlace.HasMember(keyName)) {
		name = CstJson::Object_GetString(objPlace, keyName, errorMsg);
		flags1 |= PlaceFlagHasName;
	}

	const char keyClipDepth[] = "clipDepth";
	uint16 clipDepth = 0;
	if (objPlace.HasMember(keyClipDepth)) {
		clipDepth = CstJson::Object_GetUint16(objPlace, keyClipDepth, errorMsg);
		flags1 |= PlaceFlagHasClipDepth;
	}

	const char keyBlendMode[] = "blendMode";
	uint8 blendMode = 0;
	if (objPlace.HasMember(keyBlendMode)) {
		// see idSWF::GLStateForRenderState
		blendMode = CstJson::Object_GetUint(objPlace, keyBlendMode, errorMsg, 0, 14);
		flags2 |= PlaceFlagHasBlendMode;
	}

	cstBitstream.WriteU8(flags1);
	cstBitstream.WriteU8(flags2);
	cstBitstream.WriteU16(depth);
	if (flags1 & PlaceFlagHasCharacter) {
		cstBitstream.WriteU16(characterID);
	}
	if (flags1 & PlaceFlagHasMatrix) {
		cstBitstream.WriteMatrix(matrix);
	}
	if (flags1 & PlaceFlagHasColorTransform) {
		cstBitstream.WriteColorXFormRGBA(cxf);
	}
	if (flags1 & PlaceFlagHasRatio) {
		cstBitstream.WriteU16(ratio);
	}
	if (flags1 & PlaceFlagHasName) {
		cstBitstream.WriteString(name);
	}
	if (flags1 & PlaceFlagHasClipDepth) {
		cstBitstream.WriteU16(clipDepth);
	}
	if (flags2 & PlaceFlagHasBlendMode) {
		cstBitstream.WriteU8(blendMode);
	}
}
//#modified-fva; END

/*
========================
idSWFSpriteInstance::RemoveObject2
========================
*/
void idSWFSpriteInstance::RemoveObject2( idSWFBitStream & bitstream ) {
	RemoveDisplayEntry( bitstream.ReadU16() );
}

//#modified-fva; BEGIN
void idSWFSpriteInstance::CstWriteJson_RemoveObject2(JsonPrettyWriter &writer, idSWFBitStream &bitstream) {
	// see idSWFSpriteInstance::RemoveObject2
	writer.StartObject();
	{
		writer.Key("tag");
		writer.String("RemoveObject2");

		unsigned depth = bitstream.ReadU16();
		writer.Key("depth");
		writer.Uint(depth);
	}
	writer.EndObject();
}

// ===============
void idSWFSpriteInstance::CstLoadJson_RemoveObject2(JsonValue &objRemove, CstSWFBitstream &cstBitstream, idList<idStr> &errorMsg) {
	// may throw CstJson::Except
	assert(objRemove.IsObject());

	uint16 depth = CstJson::Object_GetUint16(objRemove, "depth", errorMsg);
	cstBitstream.WriteU16(depth);
}
//#modified-fva; END
