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

/*
========================
idSWFSprite::idSWFSprite
========================
*/
idSWFSprite::idSWFSprite( idSWF * _swf ) :
frameCount( 0 ),
swf( _swf ),
commandBuffer( NULL )
{
}

/*
========================
idSWFSprite::~idSWFSprite
========================
*/
idSWFSprite::~idSWFSprite() {
	Mem_Free( commandBuffer );
}

/*
========================
idSWF::DefineSprite
========================
*/
void idSWF::DefineSprite( idSWFBitStream & bitstream ) {
	uint16 characterID = bitstream.ReadU16();
	idSWFDictionaryEntry * entry = AddDictionaryEntry( characterID, SWF_DICT_SPRITE );
	if ( entry == NULL ) {
		return;
	}
	entry->sprite->Load( bitstream, false );
}

/*
========================
idSWFSprite::Load
========================
*/
void idSWFSprite::Load( idSWFBitStream & bitstream, bool parseDictionary ) {

	frameCount = bitstream.ReadU16();

	// run through the file once, building the dictionary and accumulating control tags
	frameOffsets.SetNum( frameCount + 1 );
	frameOffsets[0] = 0;

	unsigned int currentFrame = 1;

	while ( true ) {
		uint16 codeAndLength = bitstream.ReadU16();
		uint32 recordLength = ( codeAndLength & 0x3F );
		if ( recordLength == 0x3F ) {
			recordLength = bitstream.ReadU32();
		}

		idSWFBitStream tagStream( bitstream.ReadData( recordLength ), recordLength, false );

		swfTag_t tag = (swfTag_t)( codeAndLength >> 6 );

		// ----------------------
		// Definition tags
		// definition tags are only allowed in the main sprite
		// ----------------------
		if ( parseDictionary ) {
			bool handled = true;
			switch ( tag ) {
#define HANDLE_SWF_TAG( x ) case Tag_##x: swf->x( tagStream ); break;
			HANDLE_SWF_TAG( JPEGTables );
			HANDLE_SWF_TAG( DefineBits );
			HANDLE_SWF_TAG( DefineBitsJPEG2 );
			HANDLE_SWF_TAG( DefineBitsJPEG3 );
			HANDLE_SWF_TAG( DefineBitsLossless );
			HANDLE_SWF_TAG( DefineBitsLossless2 );
			HANDLE_SWF_TAG( DefineShape );
			HANDLE_SWF_TAG( DefineShape2 );
			HANDLE_SWF_TAG( DefineShape3 );
			HANDLE_SWF_TAG( DefineShape4 );
			HANDLE_SWF_TAG( DefineSprite );
			HANDLE_SWF_TAG( DefineSound );
			//HANDLE_SWF_TAG( DefineMorphShape ); // these don't work right
			HANDLE_SWF_TAG( DefineFont2 );
			HANDLE_SWF_TAG( DefineFont3 );
			HANDLE_SWF_TAG( DefineText );
			HANDLE_SWF_TAG( DefineText2 );
			HANDLE_SWF_TAG( DefineEditText );
#undef HANDLE_SWF_TAG
			default: handled = false;
			}
			if ( handled ) {
				continue;
			}
		}
		// ----------------------
		// Control tags
		// control tags are stored off in the commands list and processed at run time
		// except for a couple really special control tags like "End" and "FrameLabel"
		// ----------------------
		switch ( tag ) {
		case Tag_End:
			return;

		case Tag_ShowFrame:
			frameOffsets[ currentFrame ] = commands.Num();
			currentFrame++;
			break;

		case Tag_FrameLabel: {
				swfFrameLabel_t & label = frameLabels.Alloc();
				label.frameNum = currentFrame;
				label.frameLabel = tagStream.ReadString();
			}
			break;

		case Tag_DoInitAction: {
			tagStream.ReadU16();

			idSWFBitStream &initaction = doInitActions.Alloc();
			initaction.Load( tagStream.ReadData( recordLength - 2 ), recordLength - 2, true );
 		    }
			break;

		case Tag_DoAction:
		case Tag_PlaceObject2:
		case Tag_PlaceObject3:
		case Tag_RemoveObject2: {
				swfSpriteCommand_t & command = commands.Alloc();
				command.tag = tag;
				command.stream.Load( tagStream.ReadData( recordLength ), recordLength, true );
			}
			break;

		default:
			// We don't care, about sprite tags we don't support ... RobA
			//idLib::Printf( "Load Sprite: Unhandled tag %s\n", idSWF::GetTagName( tag ) );
			break;
		}
	}
}

/*
========================
idSWFSprite::Read
========================
*/
void idSWFSprite::Read( idFile * f ) {
	int num = 0;
	f->ReadBig( frameCount );
	f->ReadBig( num ); frameOffsets.SetNum( num );
	f->ReadBigArray( frameOffsets.Ptr(), frameOffsets.Num() );
	f->ReadBig( num ); frameLabels.SetNum( num );
	for ( int i = 0; i < frameLabels.Num(); i++ ) {
		f->ReadBig( frameLabels[i].frameNum );
		f->ReadString( frameLabels[i].frameLabel );
	}

	uint32 bufferSize;
	f->ReadBig( bufferSize );

	commandBuffer = (byte *)Mem_Alloc( bufferSize, TAG_SWF );
	f->Read( commandBuffer, bufferSize );

	byte * currentBuffer = commandBuffer;

	f->ReadBig( num ); commands.SetNum( num );
	for ( int i = 0; i < commands.Num(); i++ ) {
		uint32 streamLength = 0;

		f->ReadBig( commands[i].tag );
		f->ReadBig( streamLength );

		commands[i].stream.Load( currentBuffer, streamLength, false );
		currentBuffer += streamLength;
	}

	uint32 doInitActionLength = 0;
	f->ReadBig( num );
	doInitActions.SetNum( num );
	for ( int i = 0; i < num; i++ ) {
		f->ReadBig( doInitActionLength );
		idSWFBitStream &initaction = doInitActions[i];
		initaction.Load( currentBuffer, doInitActionLength, true );
		currentBuffer += doInitActionLength;
	}
}

/*
========================
idSWFSprite::Write
========================
*/
void idSWFSprite::Write( idFile * f ) {
	f->WriteBig( frameCount );
	f->WriteBig( frameOffsets.Num() );
	f->WriteBigArray( frameOffsets.Ptr(), frameOffsets.Num() );
	f->WriteBig( frameLabels.Num() );
	for ( int i = 0; i < frameLabels.Num(); i++ ) {
		f->WriteBig( frameLabels[i].frameNum );
		f->WriteString( frameLabels[i].frameLabel );
	}
	uint32 totalLength = 0;
	for ( int i = 0; i < commands.Num(); i++ ) {
		totalLength += commands[i].stream.Length();
	}
	for (int i = 0; i < doInitActions.Num(); i++ ) {
		totalLength += doInitActions[i].Length();
	}
	f->WriteBig( totalLength );
	for ( int i = 0; i < commands.Num(); i++ ) {
		f->Write( commands[i].stream.Ptr(), commands[i].stream.Length() );
	}
	for ( int i = 0; i < doInitActions.Num(); i++ ){
		f->Write( doInitActions[i].Ptr(), doInitActions[i].Length() );
	}

	f->WriteBig( commands.Num() );
	for ( int i = 0; i < commands.Num(); i++ ) {
		f->WriteBig( commands[i].tag );
		f->WriteBig( commands[i].stream.Length() );
	}

	f->WriteBig( doInitActions.Num() );
	for ( int i = 0; i < doInitActions.Num(); i++ ){
		f->WriteBig( doInitActions[i].Length() );
	}
}

//#modified-fva; BEGIN
bool idSWFSprite::CstWriteJson(JsonPrettyWriter &writer, bool isMain, unsigned characterID, idList<idStr> &errorMsg) {
	struct local {
		static void UpdateErrorMsg(idList<idStr> &_errorMsg, bool _isMain, unsigned _characterID) {
			idStr msg;
			if (_isMain) {
				msg = "Sprite (main)";
			} else {
				msg.Format("Sprite (id=%d)", _characterID);
			}
			_errorMsg.Append(msg);
		}
	};

	writer.StartObject();
	{
		// main sprite doesn't need type and characterID
		if (!isMain) {
			writer.Key("type");
			writer.String("SPRITE");
			writer.Key("characterID");
			writer.String(idStr(characterID).c_str());
		}
		writer.Key("frameCount");
		writer.Uint(frameCount);

		writer.Key("frameLabels");
		writer.StartArray();
		for (int i = 0; i < frameLabels.Num(); ++i) {
			writer.StartArray();
			writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
			{
				writer.Uint(frameLabels[i].frameNum);
				writer.String(frameLabels[i].frameLabel.c_str());
			}
			writer.EndArray();
			writer.SetFormatOptions(rapidjson::kFormatDefault);
		}
		writer.EndArray();

		writer.Key("commandSets");
		writer.StartArray();
		for (int i = 0; i < frameOffsets.Num() - 1; ++i) {
			int firstCmd = frameOffsets[i];
			int lastCmd = frameOffsets[i + 1]; // one past the last, actually
			if (firstCmd >= lastCmd) {
				continue;
			}
			writer.StartObject();
			{
				writer.Key("onFrame");
				writer.Uint((unsigned)(i + 1)); // frames are 1 based
				writer.Key("commands");
				writer.StartArray();
				for (int j = firstCmd; j < lastCmd; ++j) {
					swfSpriteCommand_t &command = commands[j];
					idSWFBitStream cmdStream(command.stream.Ptr(), command.stream.Length(), false);
					switch (command.tag) {
					case (Tag_DoAction): {
						if (!idSWFScriptFunction_Script::CstWriteJson_DoAction(writer, cmdStream, errorMsg)) {
							idStr msg;
							msg.Format("(%d) DoAction", j);
							errorMsg.Append(msg);
							local::UpdateErrorMsg(errorMsg, isMain, characterID);
							return false;
						}
						break;
					}
					case (Tag_PlaceObject2): {
						idSWFSpriteInstance::CstWriteJson_PlaceObject2(writer, cmdStream);
						break;
					}
					case (Tag_PlaceObject3): {
						idSWFSpriteInstance::CstWriteJson_PlaceObject3(writer, cmdStream);
						break;
					}
					case (Tag_RemoveObject2): {
						idSWFSpriteInstance::CstWriteJson_RemoveObject2(writer, cmdStream);
						break;
					}
					}
				}
				writer.EndArray();
			}
			writer.EndObject();
		}
		writer.EndArray();

		writer.Key("doInitActions");
		writer.StartArray();
		for (int i = 0; i < doInitActions.Num(); ++i) {
			idSWFBitStream initStream(doInitActions[i].Ptr(), doInitActions[i].Length(), false);
			if (!idSWFScriptFunction_Script::CstWriteJson_DoInitAction(writer, initStream, errorMsg)) {
				idStr msg;
				msg.Format("doInitActions (%d)", i);
				errorMsg.Append(msg);
				local::UpdateErrorMsg(errorMsg, isMain, characterID);
				return false;
			}
		}
		writer.EndArray();
	}
	writer.EndObject();
	return true;
}

// ===============
void idSWFSprite::CstLoadJson(JsonValue &objSprite, const idList<idStr> &characterIDs, idList<idStr> &errorMsg) {
	// may throw CstJson::Except
	assert(objSprite.IsObject());

	struct local {
		static void Clear(idSWFSprite &sprite) {
			sprite.frameCount = 0;
			sprite.frameOffsets.Clear();
			sprite.frameLabels.Clear();
			sprite.commands.Clear();
			sprite.doInitActions.Clear();
			if (sprite.commandBuffer) {
				Mem_Free(sprite.commandBuffer);
				sprite.commandBuffer = NULL;
			}
		}
	};
	local::Clear(*this);

	try {
		const char keyFrameCount[] = "frameCount";
		frameCount = CstJson::Object_GetUint16(objSprite, keyFrameCount, errorMsg);

		const char keyFrameLabels[] = "frameLabels";
		JsonValue &arrayLabels = CstJson::Object_GetArray(objSprite, keyFrameLabels, errorMsg);
		for (JsonSizeType i = 0; i < arrayLabels.Size(); ++i) {
			try {
				JsonValue &arrayLabelPair = CstJson::Array_GetArray(arrayLabels, i, errorMsg, 2);
				uint16 frameNum;
				const char *frameLabel;
				unsigned index;
				try {
					frameNum = CstJson::Array_GetUint16(arrayLabelPair, index = 0, errorMsg);
					if (frameNum == 0) {
						errorMsg.Append("Frame number can't be 0.");
						throw CstJson::Except();
					}
					if (frameNum > frameCount) {
						idStr msg; msg.Format("Frame number can't be greater than '%s'.", keyFrameCount); errorMsg.Append(msg);
						throw CstJson::Except();
					}
					frameLabel = CstJson::Array_GetString(arrayLabelPair, index = 1, errorMsg);
				}
				catch (CstJson::Except) {
					idStr msg; msg.Format("(%u)", index); errorMsg.Append(msg);
					throw;
				}
				for (int j = 0; j < frameLabels.Num(); ++j) {
					if (frameLabels[j].frameNum == frameNum || idStr::Cmp(frameLabels[j].frameLabel.c_str(), frameLabel) == 0) {
						errorMsg.Append("Not unique.");
						throw CstJson::Except();
					}
				}
				frameLabels.Append({ frameLabel, frameNum });
			}
			catch (CstJson::Except) {
				idStr msg; msg.Format("%s (%u)", keyFrameLabels, i); errorMsg.Append(msg);
				throw;
			}
		}

		struct infoFrame_t {
			uint16 onFrame;
			JsonSizeType numCommands;
		};
		idList<infoFrame_t> infoFrames; // used later to populate frameOffsets

		const char keyCommandSets[] = "commandSets";
		JsonValue &arrayCommandSets = CstJson::Object_GetArray(objSprite, keyCommandSets, errorMsg);
		for (JsonSizeType i = 0; i < arrayCommandSets.Size(); ++i) {
			JsonValue *pObjCommandSet = NULL;
			const char keyOnFrame[] = "onFrame";
			uint16 onFrame;
			try {
				pObjCommandSet = &CstJson::Array_GetObject(arrayCommandSets, i, errorMsg);
				onFrame = CstJson::Object_GetUint16(*pObjCommandSet, keyOnFrame, errorMsg);
				if (onFrame == 0) {
					errorMsg.Append("Value can't be 0.");
					errorMsg.Append(keyOnFrame);
					throw CstJson::Except();
				}
				if (onFrame > frameCount) {
					idStr msg; msg.Format("Value can't be greater than '%s'.", keyFrameCount); errorMsg.Append(msg);
					errorMsg.Append(keyOnFrame);
					throw CstJson::Except();
				}
				for (int j = 0; j < infoFrames.Num(); ++j) {
					if (infoFrames[j].onFrame == onFrame) {
						errorMsg.Append("Value must be unique.");
						errorMsg.Append(keyOnFrame);
						throw CstJson::Except();
					}
				}
			}
			catch (CstJson::Except) {
				idStr msg; msg.Format("%s (%u)", keyCommandSets, i); errorMsg.Append(msg);
				throw;
			}

			try {
				const char keyCommands[] = "commands";
				JsonValue &arrayCommands = CstJson::Object_GetArray(*pObjCommandSet, keyCommands, errorMsg);
				infoFrames.Append({ onFrame, arrayCommands.Size() });
				for (JsonSizeType j = 0; j < arrayCommands.Size(); ++j) {
					JsonValue *pObjCommand = NULL;
					const char keyTag[] = "tag";
					const char *tag;
					try {
						pObjCommand = &CstJson::Array_GetObject(arrayCommands, j, errorMsg);
						tag = CstJson::Object_GetString(*pObjCommand, keyTag, errorMsg);
					}
					catch (CstJson::Except) {
						idStr msg; msg.Format("%s (%u)", keyCommands, j); errorMsg.Append(msg);
						throw;
					}

					bool unknownTag = false;
					try {
						CstSWFBitstream cstBitstream;
						swfTag_t swfTag;

						if (idStr::Cmp(tag, "DoAction") == 0) {
							idSWFScriptFunction_Script::CstLoadJson_DoAction(*pObjCommand, cstBitstream, errorMsg);
							swfTag = Tag_DoAction;
						}
						else if (idStr::Cmp(tag, "PlaceObject2") == 0) {
							idSWFSpriteInstance::CstLoadJson_PlaceObject2(*pObjCommand, cstBitstream, characterIDs, errorMsg);
							swfTag = Tag_PlaceObject2;
						}
						else if (idStr::Cmp(tag, "PlaceObject3") == 0) {
							idSWFSpriteInstance::CstLoadJson_PlaceObject3(*pObjCommand, cstBitstream, characterIDs, errorMsg);
							swfTag = Tag_PlaceObject3;
						}
						else if (idStr::Cmp(tag, "RemoveObject2") == 0) {
							idSWFSpriteInstance::CstLoadJson_RemoveObject2(*pObjCommand, cstBitstream, errorMsg);
							swfTag = Tag_RemoveObject2;
						}
						else {
							unknownTag = true;
							throw CstJson::Except();
						}

						if (cstBitstream.Length() == 0) {
							errorMsg.Append("Bitstream is empty.");
							throw CstJson::Except();
						}
						swfSpriteCommand_t &command = commands.Alloc();
						command.tag = swfTag;
						command.stream.Load(cstBitstream.Ptr(), (uint32)cstBitstream.Length(), true);
					}
					catch (CstJson::Except) {
						idStr msg;
						if (unknownTag) {
							msg.Format("Unknown tag '%s'.", tag); errorMsg.Append(msg);
							msg.Format("(%u)", j); errorMsg.Append(msg);
						} else {
							msg.Format("(%u) %s", j, tag); errorMsg.Append(msg);
						}
						throw;
					}
				}
			}
			catch (CstJson::Except) {
				idStr msg; msg.Format("%s (%s=%u)", keyCommandSets, keyOnFrame, onFrame); errorMsg.Append(msg);
				throw;
			}
		}

		// populate frameOffsets
		uint32 commandCount = 0;
		frameOffsets.Append(0);
		for (uint16 i = 1; i <= frameCount; ++i) {
			for (int j = 0; j < infoFrames.Num(); ++j) {
				const infoFrame_t &info = infoFrames[j];
				if (info.onFrame == i) {
					commandCount += info.numCommands;
					break; // inner for
				}
			}
			frameOffsets.Append(commandCount);
		}

		const char keyDoInitActions[] = "doInitActions";
		JsonValue &arrayDoInitSet = CstJson::Object_GetArray(objSprite, keyDoInitActions, errorMsg);
		for (JsonSizeType i = 0; i < arrayDoInitSet.Size(); ++i) {
			try {
				CstSWFBitstream cstBitstream;

				JsonValue &arrayDoInitAction = CstJson::Array_GetArray(arrayDoInitSet, i, errorMsg);
				idSWFScriptFunction_Script::CstLoadJson_DoInitAction(arrayDoInitAction, cstBitstream, errorMsg);

				if (cstBitstream.Length() == 0) {
					errorMsg.Append("Bitstream is empty.");
					throw CstJson::Except();
				}
				idSWFBitStream &initAction = doInitActions.Alloc();
				initAction.Load(cstBitstream.Ptr(), cstBitstream.Length(), true);
			}
			catch (CstJson::Except) {
				idStr msg; msg.Format("%s (%u)", keyDoInitActions, i); errorMsg.Append(msg);
				throw;
			}
		}
	}
	catch (CstJson::Except) {
		local::Clear(*this);
		throw;
	}
}
//#modified-fva; END
