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
#include "../renderer/Font.h"

#pragma warning(disable: 4355) // 'this' : used in base member initializer list

#define BSWF_VERSION 16		// bumped to 16 for storing atlas image dimensions for unbuffered loads
#define BSWF_MAGIC ( ( 'B' << 24 ) | ( 'S' << 16 ) | ( 'W' << 8 ) | BSWF_VERSION )
//#modified-fva; BEGIN
static const char CST_BSWF_JSON_VERSION[] = "16-0";
//#modified-fva; END

/*
===================
idSWF::LoadSWF
===================
*/
bool idSWF::LoadSWF( const char * fullpath ) {

	idFile * rawfile = fileSystem->OpenFileRead( fullpath );
	if ( rawfile == NULL ) {
		idLib::Printf( "SWF File not found %s\n", fullpath );
		return false;
	}

	swfHeader_t header;
	rawfile->Read( &header, sizeof( header ) );

	if ( header.W != 'W' || header.S != 'S' ) {
		idLib::Warning( "Wrong signature bytes" );
		delete rawfile;
		return false;
	}

	if ( header.version > 9 ) {
		idLib::Warning( "Unsupported version %d", header.version );
		delete rawfile;
		return false;
	}

	bool compressed;
	if ( header.compression == 'F' ) {
		compressed = false;
	} else if ( header.compression == 'C' ) {
		compressed = true;
	} else {
		idLib::Warning( "Unsupported compression type %c", header.compression );
		delete rawfile;
		return false;
	}
	idSwap::Little( header.fileLength );

	// header.fileLength somewhat annoyingly includes the size of the header
	uint32 fileLength2 = header.fileLength - (uint32)sizeof( swfHeader_t );

	// slurp the raw file into a giant array, which is somewhat atrocious when loading from the preload since it's already an idFile_Memory
	byte * fileData = (byte *)Mem_Alloc( fileLength2, TAG_SWF );
	size_t fileSize = rawfile->Read( fileData, fileLength2 );
	delete rawfile;

	if ( compressed ) {
		byte * uncompressed = (byte *)Mem_Alloc( fileLength2, TAG_SWF );
		if ( !Inflate( fileData, (int)fileSize, uncompressed, fileLength2 ) ) {
			idLib::Warning( "Inflate error" );
			Mem_Free( uncompressed );
			return false;
		}
		Mem_Free( fileData );
		fileData = uncompressed;
	}
	idSWFBitStream bitstream( fileData, fileLength2, false );

	swfRect_t frameSize;
	bitstream.ReadRect( frameSize );

	if ( !frameSize.tl.Compare( vec2_zero ) ) {
		idLib::Warning( "Invalid frameSize top left" );
		Mem_Free( fileData );
		return false;
	}

	frameWidth = frameSize.br.x;
	frameHeight = frameSize.br.y;
	frameRate = bitstream.ReadU16();

	// parse everything
	mainsprite->Load( bitstream, true );

	// now that all images have been loaded, write out the combined image
	idStr atlasFileName = "generated/";
	atlasFileName += fullpath;
	atlasFileName.SetFileExtension( ".tga" );

	WriteSwfImageAtlas( atlasFileName );

	Mem_Free( fileData );

	return true;
}

/*
===================
idSWF::LoadBinary
===================
*/
bool idSWF::LoadBinary( const char * bfilename, ID_TIME_T sourceTime ) {
	idFile * f = fileSystem->OpenFileReadMemory( bfilename );
	if ( f == NULL || f->Length() <= 0 ) {
		return false;
	}

	uint32 magic = 0;
	ID_TIME_T btimestamp = 0;
	f->ReadBig( magic );
	f->ReadBig( btimestamp );

	if (  magic != BSWF_MAGIC || ( !fileSystem->InProductionMode() && sourceTime != btimestamp ) )  {
		delete f;
		return false;
	}

	f->ReadBig( frameWidth );
	f->ReadBig( frameHeight );
	f->ReadBig( frameRate );

	if ( mouseX == -1 ) {
		mouseX = ( frameWidth / 2 );
	}

	if ( mouseY == -1 ) {
		mouseY = ( frameHeight / 2 );
	}

	mainsprite->Read( f );

	int num = 0;
	f->ReadBig( num );
	dictionary.SetNum( num );
	for ( int i = 0; i < dictionary.Num(); i++ ) {
		f->ReadBig( dictionary[i].type );
		switch ( dictionary[i].type ) {
			case SWF_DICT_IMAGE: {
				idStr imageName;
				f->ReadString( imageName );
				if ( imageName[0] == '.' ) {
					// internal image in the atlas
					dictionary[i].material = NULL;
				} else {
					dictionary[i].material = declManager->FindMaterial( imageName );
				}
				for ( int j = 0 ; j < 2 ; j++ ) {
					f->ReadBig( dictionary[i].imageSize[j] );
					f->ReadBig( dictionary[i].imageAtlasOffset[j] );
				}
				for ( int j = 0 ; j < 4 ; j++ ) {
					f->ReadBig( dictionary[i].channelScale[j] );
				}
				break;
			}
			case SWF_DICT_MORPH:
			case SWF_DICT_SHAPE: {
				dictionary[i].shape = new (TAG_SWF) idSWFShape;
				idSWFShape * shape = dictionary[i].shape;
				f->ReadBig( shape->startBounds.tl );
				f->ReadBig( shape->startBounds.br );
				f->ReadBig( shape->endBounds.tl );
				f->ReadBig( shape->endBounds.br );
				f->ReadBig( num ); shape->fillDraws.SetNum( num );
				for ( int d = 0; d < shape->fillDraws.Num(); d++ ) {
					idSWFShapeDrawFill & fillDraw = shape->fillDraws[d];
					f->ReadBig( fillDraw.style.type );
					f->ReadBig( fillDraw.style.subType );
					f->Read( &fillDraw.style.startColor, 4 );
					f->Read( &fillDraw.style.endColor, 4 );
					f->ReadBigArray( (float *)&fillDraw.style.startMatrix, 6 );
					f->ReadBigArray( (float *)&fillDraw.style.endMatrix, 6 );
					f->ReadBig( fillDraw.style.gradient.numGradients );
					for ( int g = 0; g < fillDraw.style.gradient.numGradients; g++ ) {
						f->ReadBig( fillDraw.style.gradient.gradientRecords[g].startRatio );
						f->ReadBig( fillDraw.style.gradient.gradientRecords[g].endRatio );
						f->Read( &fillDraw.style.gradient.gradientRecords[g].startColor, 4 );
						f->Read( &fillDraw.style.gradient.gradientRecords[g].endColor, 4 );
					}
					f->ReadBig( fillDraw.style.focalPoint );
					f->ReadBig( fillDraw.style.bitmapID );
					f->ReadBig( num ); fillDraw.startVerts.SetNum( num );
					f->ReadBigArray( fillDraw.startVerts.Ptr(), fillDraw.startVerts.Num() );
					f->ReadBig( num ); fillDraw.endVerts.SetNum( num );
					f->ReadBigArray( fillDraw.endVerts.Ptr(), fillDraw.endVerts.Num() );
					f->ReadBig( num ); fillDraw.indices.SetNum( num );
					f->ReadBigArray( fillDraw.indices.Ptr(), fillDraw.indices.Num() );
				}
				f->ReadBig( num ); shape->lineDraws.SetNum( num );
				for ( int d = 0; d < shape->lineDraws.Num(); d++ ) {
					idSWFShapeDrawLine & lineDraw = shape->lineDraws[d];
					f->ReadBig( lineDraw.style.startWidth );
					f->ReadBig( lineDraw.style.endWidth );
					f->Read( &lineDraw.style.startColor, 4 );
					f->Read( &lineDraw.style.endColor, 4 );
					f->ReadBig( num ); lineDraw.startVerts.SetNum( num );
					f->ReadBigArray( lineDraw.startVerts.Ptr(), lineDraw.startVerts.Num() );
					f->ReadBig( num ); lineDraw.endVerts.SetNum( num );
					f->ReadBigArray( lineDraw.endVerts.Ptr(), lineDraw.endVerts.Num() );
					f->ReadBig( num ); lineDraw.indices.SetNum( num );
					f->ReadBigArray( lineDraw.indices.Ptr(), lineDraw.indices.Num() );
				}
				break;
			}
			case SWF_DICT_SPRITE: {
				dictionary[i].sprite = new (TAG_SWF) idSWFSprite( this );
				dictionary[i].sprite->Read( f );
				break;
			}
			case SWF_DICT_FONT: {
				dictionary[i].font = new (TAG_SWF) idSWFFont;
				idSWFFont * font = dictionary[i].font;
				idStr fontName;
				f->ReadString( fontName );
				font->fontID = renderSystem->RegisterFont( fontName );
				f->ReadBig( font->ascent );
				f->ReadBig( font->descent );
				f->ReadBig( font->leading );
				f->ReadBig( num ); font->glyphs.SetNum( num );
				for ( int g = 0; g < font->glyphs.Num(); g++ ) {
					f->ReadBig( font->glyphs[g].code );
					f->ReadBig( font->glyphs[g].advance );
					f->ReadBig( num ); font->glyphs[g].verts.SetNum( num );
					f->ReadBigArray( font->glyphs[g].verts.Ptr(), font->glyphs[g].verts.Num() );
					f->ReadBig( num ); font->glyphs[g].indices.SetNum( num );
					f->ReadBigArray( font->glyphs[g].indices.Ptr(), font->glyphs[g].indices.Num() );
				}
				break;
			}
			case SWF_DICT_TEXT: {
				dictionary[i].text = new (TAG_SWF) idSWFText;
				idSWFText * text = dictionary[i].text;
				f->ReadBig( text->bounds.tl );
				f->ReadBig( text->bounds.br );
				f->ReadBigArray( (float *)&text->matrix, 6 );
				f->ReadBig( num ); text->textRecords.SetNum( num );
				for ( int t = 0; t < text->textRecords.Num(); t++ ) {
					idSWFTextRecord & textRecord = text->textRecords[t];
					f->ReadBig( textRecord.fontID );
					f->Read( &textRecord.color, 4 );
					f->ReadBig( textRecord.xOffset );
					f->ReadBig( textRecord.yOffset );
					f->ReadBig( textRecord.textHeight );
					f->ReadBig( textRecord.firstGlyph );
					f->ReadBig( textRecord.numGlyphs );
				}
				f->ReadBig( num ); text->glyphs.SetNum( num );
				for ( int g = 0; g < text->glyphs.Num(); g++ ) {
					f->ReadBig( text->glyphs[g].index );
					f->ReadBig( text->glyphs[g].advance );
				}
				break;
			}
			case SWF_DICT_EDITTEXT: {
				dictionary[i].edittext = new (TAG_SWF) idSWFEditText;
				idSWFEditText * edittext = dictionary[i].edittext;
				f->ReadBig( edittext->bounds.tl );
				f->ReadBig( edittext->bounds.br );
				f->ReadBig( edittext->flags );
				f->ReadBig( edittext->fontID );
				f->ReadBig( edittext->fontHeight );
				f->Read( &edittext->color, 4 );
				f->ReadBig( edittext->maxLength );
				f->ReadBig( edittext->align );
				f->ReadBig( edittext->leftMargin );
				f->ReadBig( edittext->rightMargin );
				f->ReadBig( edittext->indent );
				f->ReadBig( edittext->leading );
				f->ReadString( edittext->variable );
				f->ReadString( edittext->initialText );
				break;
			}
		}
	}
	delete f;

	return true;
}

/*
===================
idSWF::WriteBinary
===================
*/
void idSWF::WriteBinary( const char * bfilename ) {
	idFileLocal file( fileSystem->OpenFileWrite( bfilename, "fs_basepath" ) );
	if ( file == NULL ) {
		return;
	}
	file->WriteBig( BSWF_MAGIC );
	file->WriteBig( timestamp );

	file->WriteBig( frameWidth );
	file->WriteBig( frameHeight );
	file->WriteBig( frameRate );

	mainsprite->Write( file );

	file->WriteBig( dictionary.Num() );
	for ( int i = 0; i < dictionary.Num(); i++ ) {
		file->WriteBig( dictionary[i].type );
		switch ( dictionary[i].type ) {
			case SWF_DICT_IMAGE: {
				if ( dictionary[i].material ) {
					file->WriteString( dictionary[i].material->GetName() );
				} else {
					file->WriteString( "." );
				}
				for ( int j = 0 ; j < 2 ; j++ ) {
					file->WriteBig( dictionary[i].imageSize[j] );
					file->WriteBig( dictionary[i].imageAtlasOffset[j] );
				}
				for ( int j = 0 ; j < 4 ; j++ ) {
					file->WriteBig( dictionary[i].channelScale[j] );
				}
				break;
			}
			case SWF_DICT_MORPH:
			case SWF_DICT_SHAPE: {
				idSWFShape * shape = dictionary[i].shape;
				file->WriteBig( shape->startBounds.tl );
				file->WriteBig( shape->startBounds.br );
				file->WriteBig( shape->endBounds.tl );
				file->WriteBig( shape->endBounds.br );
				file->WriteBig( shape->fillDraws.Num() );
				for ( int d = 0; d < shape->fillDraws.Num(); d++ ) {
					idSWFShapeDrawFill & fillDraw = shape->fillDraws[d];
					file->WriteBig( fillDraw.style.type );
					file->WriteBig( fillDraw.style.subType );
					file->Write( &fillDraw.style.startColor, 4 );
					file->Write( &fillDraw.style.endColor, 4 );
					file->WriteBigArray( (float *)&fillDraw.style.startMatrix, 6 );
					file->WriteBigArray( (float *)&fillDraw.style.endMatrix, 6 );
					file->WriteBig( fillDraw.style.gradient.numGradients );
					for ( int g = 0; g < fillDraw.style.gradient.numGradients; g++ ) {
						file->WriteBig( fillDraw.style.gradient.gradientRecords[g].startRatio );
						file->WriteBig( fillDraw.style.gradient.gradientRecords[g].endRatio );
						file->Write( &fillDraw.style.gradient.gradientRecords[g].startColor, 4 );
						file->Write( &fillDraw.style.gradient.gradientRecords[g].endColor, 4 );
					}
					file->WriteBig( fillDraw.style.focalPoint );
					file->WriteBig( fillDraw.style.bitmapID );
					file->WriteBig( fillDraw.startVerts.Num() );
					file->WriteBigArray( fillDraw.startVerts.Ptr(), fillDraw.startVerts.Num() );
					file->WriteBig( fillDraw.endVerts.Num() );
					file->WriteBigArray( fillDraw.endVerts.Ptr(), fillDraw.endVerts.Num() );
					file->WriteBig( fillDraw.indices.Num() );
					file->WriteBigArray( fillDraw.indices.Ptr(), fillDraw.indices.Num() );
				}
				file->WriteBig( shape->lineDraws.Num() );
				for ( int d = 0; d < shape->lineDraws.Num(); d++ ) {
					idSWFShapeDrawLine & lineDraw = shape->lineDraws[d];
					file->WriteBig( lineDraw.style.startWidth );
					file->WriteBig( lineDraw.style.endWidth );
					file->Write( &lineDraw.style.startColor, 4 );
					file->Write( &lineDraw.style.endColor, 4 );
					file->WriteBig( lineDraw.startVerts.Num() );
					file->WriteBigArray( lineDraw.startVerts.Ptr(), lineDraw.startVerts.Num() );
					file->WriteBig( lineDraw.endVerts.Num() );
					file->WriteBigArray( lineDraw.endVerts.Ptr(), lineDraw.endVerts.Num() );
					file->WriteBig( lineDraw.indices.Num() );
					file->WriteBigArray( lineDraw.indices.Ptr(), lineDraw.indices.Num() );
				}
				break;
			}
			case SWF_DICT_SPRITE: {
				dictionary[i].sprite->Write( file );
				break;
			}
			case SWF_DICT_FONT: {
				idSWFFont * font = dictionary[i].font;
				file->WriteString( font->fontID->GetName() );
				file->WriteBig( font->ascent );
				file->WriteBig( font->descent );
				file->WriteBig( font->leading );
				file->WriteBig( font->glyphs.Num() );
				for ( int g = 0; g < font->glyphs.Num(); g++ ) {
					file->WriteBig( font->glyphs[g].code );
					file->WriteBig( font->glyphs[g].advance );
					file->WriteBig( font->glyphs[g].verts.Num() );
					file->WriteBigArray( font->glyphs[g].verts.Ptr(), font->glyphs[g].verts.Num() );
					file->WriteBig( font->glyphs[g].indices.Num() );
					file->WriteBigArray( font->glyphs[g].indices.Ptr(), font->glyphs[g].indices.Num() );
				}
				break;
			}
			case SWF_DICT_TEXT: {
				idSWFText * text = dictionary[i].text;
				file->WriteBig( text->bounds.tl );
				file->WriteBig( text->bounds.br );
				file->WriteBigArray( (float *)&text->matrix, 6 );
				file->WriteBig( text->textRecords.Num() );
				for ( int t = 0; t < text->textRecords.Num(); t++ ) {
					idSWFTextRecord & textRecord = text->textRecords[t];
					file->WriteBig( textRecord.fontID );
					file->Write( &textRecord.color, 4 );
					file->WriteBig( textRecord.xOffset );
					file->WriteBig( textRecord.yOffset );
					file->WriteBig( textRecord.textHeight );
					file->WriteBig( textRecord.firstGlyph );
					file->WriteBig( textRecord.numGlyphs );
				}
				file->WriteBig( text->glyphs.Num() );
				for ( int g = 0; g < text->glyphs.Num(); g++ ) {
					file->WriteBig( text->glyphs[g].index );
					file->WriteBig( text->glyphs[g].advance );
				}
				break;
			}
			case SWF_DICT_EDITTEXT: {
				idSWFEditText * edittext = dictionary[i].edittext;
				file->WriteBig( edittext->bounds.tl );
				file->WriteBig( edittext->bounds.br );
				file->WriteBig( edittext->flags );
				file->WriteBig( edittext->fontID );
				file->WriteBig( edittext->fontHeight );
				file->Write( &edittext->color, 4 );
				file->WriteBig( edittext->maxLength );
				file->WriteBig( edittext->align );
				file->WriteBig( edittext->leftMargin );
				file->WriteBig( edittext->rightMargin );
				file->WriteBig( edittext->indent );
				file->WriteBig( edittext->leading );
				file->WriteString( edittext->variable );
				file->WriteString( edittext->initialText );
				break;
			}
		}
	}
}

//#modified-fva; BEGIN
void idSWF::CstWriteJson(const idStr &fileName) {
	idList<idStr> errorMsg;

	rapidjson::StringBuffer strBuffer;
	JsonPrettyWriter writer(strBuffer);
	writer.SetIndent('\t', 1u);

	idStr jsonFileName = cst_exportPath.GetString();
	if (!jsonFileName.IsEmpty()) {
		jsonFileName += '/';
	}
	jsonFileName += fileName;
	jsonFileName.SetFileExtension(".json");

	struct local {
		static bool Write(idSWF &swf, JsonPrettyWriter &writer, idList<idStr> &errorMsg) {
			writer.StartObject();
			{
				writer.Key("bswfJsonVersion");
				writer.String(CST_BSWF_JSON_VERSION);
				writer.Key("frameWidth");
				writer.Double(swf.frameWidth);
				writer.Key("frameHeight");
				writer.Double(swf.frameHeight);
				writer.Key("frameRate_x256");
				writer.Uint(swf.frameRate);

				writer.Key("mainSprite");
				if (!swf.mainsprite->CstWriteJson(writer, true, 0u, errorMsg)) {
					return false;
				}

				writer.Key("dictionary");
				writer.StartArray();
				for (int i = 0; i < swf.dictionary.Num(); ++i) {
					const idSWFDictionaryEntry &dictEntry = swf.dictionary[i];
					switch (dictEntry.type) {
					case SWF_DICT_NULL: {
						writer.StartObject();
						{
							writer.Key("type");
							writer.String("NULL");
							writer.Key("characterID");
							writer.String(idStr(i).c_str());
						}
						writer.EndObject();
						break;
					}
					case SWF_DICT_IMAGE: {
						swf.CstWriteJson_Image(dictEntry, writer, (unsigned)i);
						break;
					}
					case SWF_DICT_MORPH:
					case SWF_DICT_SHAPE: {
						if (!swf.CstWriteJson_Shape(*dictEntry.shape, dictEntry.type == SWF_DICT_MORPH, writer, (unsigned)i, errorMsg)) {
							return false;
						}
						break;
					}
					case SWF_DICT_SPRITE: {
						if (!dictEntry.sprite->CstWriteJson(writer, false, (unsigned)i, errorMsg)) {
							return false;
						}
						break;
					}
					case SWF_DICT_FONT: {
						swf.CstWriteJson_Font(*dictEntry.font, writer, (unsigned)i);
						break;
					}
					case SWF_DICT_TEXT: {
						swf.CstWriteJson_Text(*dictEntry.text, writer, (unsigned)i);
						break;
					}
					case SWF_DICT_EDITTEXT: {
						if (!swf.CstWriteJson_EditText(*dictEntry.edittext, writer, (unsigned)i, errorMsg)) {
							return false;
						}
						break;
					}
					}
				}
				writer.EndArray();
			}
			writer.EndObject();
			return true;
		}
	};

	if (!local::Write(*this, writer, errorMsg)) {
		idStr msg;
		msg.Format("Couldn't create '%s'", jsonFileName.c_str());
		if (errorMsg.Num() > 0) {
			for (int i = errorMsg.Num() - 1; i >= 0; --i) {
				msg += ": ";
				msg += errorMsg[i];
			}
		} else {
			msg += '.';
		}
		idLib::Warning(msg.c_str());
		return;
	}

	idFile *file = fileSystem->OpenFileWrite(jsonFileName.c_str(), "fs_basepath");
	if (!file) {
		idStr msg;
		msg.Format("Couldn't open '%s' for writing.", jsonFileName.c_str());
		idLib::Warning(msg.c_str());
		return;
	}
	file->Write(strBuffer.GetString(), strBuffer.GetSize());
	delete file;
}

// ===============
void idSWF::CstWriteJson_Image(const idSWFDictionaryEntry &dictEntry, JsonPrettyWriter &writer, unsigned characterID) {
	writer.StartObject();
	{
		writer.Key("type");
		writer.String("IMAGE");
		writer.Key("characterID");
		writer.String(idStr(characterID).c_str());
		if (dictEntry.material) {
			writer.Key("imageName");
			writer.String(dictEntry.material->GetName());
		} else {
			writer.Key("size");
			writer.StartArray();
			writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
			{
				writer.Int(dictEntry.imageSize.x);
				writer.Int(dictEntry.imageSize.y);
			}
			writer.EndArray();
			writer.SetFormatOptions(rapidjson::kFormatDefault);
			writer.Key("atlasOffset");
			writer.StartArray();
			writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
			{
				writer.Int(dictEntry.imageAtlasOffset.x);
				writer.Int(dictEntry.imageAtlasOffset.y);
			}
			writer.EndArray();
			writer.SetFormatOptions(rapidjson::kFormatDefault);
			writer.Key("channelScale");
			writer.StartArray();
			writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
			{
				writer.Double(dictEntry.channelScale.x);
				writer.Double(dictEntry.channelScale.y);
				writer.Double(dictEntry.channelScale.z);
				writer.Double(dictEntry.channelScale.w);
			}
			writer.EndArray();
			writer.SetFormatOptions(rapidjson::kFormatDefault);
		}
	}
	writer.EndObject();
}

// ===============
void idSWF::CstLoadJson_Image(idSWFDictionaryEntry &dictEntry, JsonValue &objImage, idList<idStr> &errorMsg) {
	// may throw CstJson::Except
	assert(objImage.IsObject());

	const char keyImageName[] = "imageName";
	if (objImage.HasMember(keyImageName)) {
		const char* imageName = CstJson::Object_GetString(objImage, keyImageName, errorMsg);
		dictEntry.material = declManager->FindMaterial(imageName);
	} else {
		dictEntry.material = NULL; // use the atlas

		const char keyImageSize[] = "size";
		JsonValue &arrayImageSize = CstJson::Object_GetArray(objImage, keyImageSize, errorMsg, 2);
		unsigned index;
		try {
			dictEntry.imageSize.x = CstJson::Array_GetInt32(arrayImageSize, index = 0, errorMsg);
			dictEntry.imageSize.y = CstJson::Array_GetInt32(arrayImageSize, index = 1, errorMsg);
		}
		catch (CstJson::Except) {
			idStr msg; msg.Format("%s (%u)", keyImageSize, index); errorMsg.Append(msg);
			throw;
		}

		const char keyAtlasOffset[] = "atlasOffset";
		JsonValue &arrayAtlasOffset = CstJson::Object_GetArray(objImage, keyAtlasOffset, errorMsg, 2);
		try {
			dictEntry.imageAtlasOffset.x = CstJson::Array_GetInt32(arrayAtlasOffset, index = 0, errorMsg);
			dictEntry.imageAtlasOffset.y = CstJson::Array_GetInt32(arrayAtlasOffset, index = 1, errorMsg);
		}
		catch (CstJson::Except) {
			idStr msg; msg.Format("%s (%u)", keyAtlasOffset, index); errorMsg.Append(msg);
			throw;
		}

		const char keyChannelScale[] = "channelScale";
		JsonValue &arrayChannelScale = CstJson::Object_GetArray(objImage, keyChannelScale, errorMsg, 4);
		try {
			dictEntry.channelScale.x = CstJson::Array_GetFloat(arrayChannelScale, index = 0, errorMsg);
			dictEntry.channelScale.y = CstJson::Array_GetFloat(arrayChannelScale, index = 1, errorMsg);
			dictEntry.channelScale.z = CstJson::Array_GetFloat(arrayChannelScale, index = 2, errorMsg);
			dictEntry.channelScale.w = CstJson::Array_GetFloat(arrayChannelScale, index = 3, errorMsg);
		}
		catch (CstJson::Except) {
			idStr msg; msg.Format("%s (%u)", keyChannelScale, index); errorMsg.Append(msg);
			throw;
		}
	}
}

// ===============
bool idSWF::CstWriteJson_Shape(const idSWFShape &shape, bool morph, JsonPrettyWriter &writer, unsigned characterID, idList<idStr> &errorMsg) {
	// see:
	//  idSWF::DefineShape/2/3/4
	//  idSWF::DefineMorphShape
	//  idSWFShapeParser::Parse
	//  idSWFShapeParser::ParseMorph
	//	struct swfFillStyle_t
	//  idSWFShapeParser::ReadFillStyle

	struct local {
		static void UpdateErrorMsg(idList<idStr> &_errorMsg, bool _morph, unsigned _characterID) {
			idStr msg;
			if (_morph) {
				msg.Format("Morph (id=%d)", _characterID);
			} else {
				msg.Format("Shape (id=%d)", _characterID);
			}
			_errorMsg.Append(msg);
		}
	};

	writer.StartObject();
	{
		writer.Key("type");
		if (morph) {
			writer.String("MORPH");
		} else {
			writer.String("SHAPE");
		}
		writer.Key("characterID");
		writer.String(idStr(characterID).c_str());

		if (morph) {
			writer.Key("startBounds");
		} else {
			writer.Key("bounds");
		}
		writer.StartArray();
		writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
		{
			writer.Double(shape.startBounds.tl.x); // top left
			writer.Double(shape.startBounds.tl.y);
			writer.Double(shape.startBounds.br.x); // bottom right
			writer.Double(shape.startBounds.br.y);
		}
		writer.EndArray();
		writer.SetFormatOptions(rapidjson::kFormatDefault);
		if (morph) {
			writer.Key("endBounds");
			writer.StartArray();
			writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
			{
				writer.Double(shape.endBounds.tl.x); // top left
				writer.Double(shape.endBounds.tl.y);
				writer.Double(shape.endBounds.br.x); // bottom right
				writer.Double(shape.endBounds.br.y);
			}
			writer.EndArray();
			writer.SetFormatOptions(rapidjson::kFormatDefault);
		}

		writer.Key("fillDraws");
		writer.StartArray();
		for (int d = 0; d < shape.fillDraws.Num(); ++d) {
			writer.StartObject();
			{
				const idSWFShapeDrawFill &fillDraw = shape.fillDraws[d];
				writer.Key("style");
				writer.StartObject();
				{
					const swfFillStyle_t &style = fillDraw.style;
					switch (style.type) {
					case 0: { // solid
						writer.Key("type");
						writer.String("solid");
						break;
					}
					case 1: { // gradient
						writer.Key("type");
						writer.String("gradient");
						writer.Key("subType");
						switch (style.subType) {
						case 0: writer.String("linear"); break;
						case 2: writer.String("radial"); break;
						case 3: writer.String("focal"); break;
						default: {
							idStr msg;
							msg.Format("fillDraws (%d): Unknown subType '%d' for style type 'gradient'.", d, style.subType);
							errorMsg.Append(msg);
							local::UpdateErrorMsg(errorMsg, morph, characterID);
							return false;
						}
						}
						break;
					}
					case 4: { // bitmap
						writer.Key("type");
						writer.String("bitmap");
						writer.Key("subType");
						switch (style.subType) {
						case 0: writer.String("repeat"); break;
						case 1: writer.String("clamp"); break;
						case 2: writer.String("nearRepeat"); break;
						case 3: writer.String("nearClamp"); break;
						default: {
							idStr msg;
							msg.Format("fillDraws (%d): Unknown subType '%d' for style type 'bitmap'.", d, style.subType);
							errorMsg.Append(msg);
							local::UpdateErrorMsg(errorMsg, morph, characterID);
							return false;
						}
						}
						break;
					}
					default: {
						idStr msg;
						msg.Format("fillDraws (%d): Unknown style type '%d'.", d, style.type);
						errorMsg.Append(msg);
						local::UpdateErrorMsg(errorMsg, morph, characterID);
						return false;
					}
					}

					if (style.type == 4) { // bitmap
						writer.Key("bitmapID");
						writer.String(idStr(style.bitmapID).c_str());
					}

					if (style.type == 0) { // solid
						if (morph) {
							writer.Key("startColor");
						} else {
							writer.Key("color");
						}
						writer.StartArray();
						writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
						{
							writer.Uint(style.startColor.r);
							writer.Uint(style.startColor.g);
							writer.Uint(style.startColor.b);
							writer.Uint(style.startColor.a);
						}
						writer.EndArray();
						writer.SetFormatOptions(rapidjson::kFormatDefault);
						if (morph) {
							writer.Key("endColor");
							writer.StartArray();
							writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
							{
								writer.Uint(style.endColor.r);
								writer.Uint(style.endColor.g);
								writer.Uint(style.endColor.b);
								writer.Uint(style.endColor.a);
							}
							writer.EndArray();
							writer.SetFormatOptions(rapidjson::kFormatDefault);
						}
					}

					if (style.type == 1 || style.type == 4) { // gradient or bitmap
						if (morph) {
							writer.Key("startMatrix");
						} else {
							writer.Key("matrix");
						}
						writer.StartArray();
						writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
						{
							writer.Double(style.startMatrix.xx);
							writer.Double(style.startMatrix.yy);
							writer.Double(style.startMatrix.xy);
							writer.Double(style.startMatrix.yx);
							writer.Double(style.startMatrix.tx);
							writer.Double(style.startMatrix.ty);
						}
						writer.EndArray();
						writer.SetFormatOptions(rapidjson::kFormatDefault);
						if (morph) {
							writer.Key("endMatrix");
							writer.StartArray();
							writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
							{
								writer.Double(style.endMatrix.xx);
								writer.Double(style.endMatrix.yy);
								writer.Double(style.endMatrix.xy);
								writer.Double(style.endMatrix.yx);
								writer.Double(style.endMatrix.tx);
								writer.Double(style.endMatrix.ty);
							}
							writer.EndArray();
							writer.SetFormatOptions(rapidjson::kFormatDefault);
						}
					}

					if (style.type == 1) { // gradient
						writer.Key("gradients");
						writer.StartArray();
						for (int g = 0; g < style.gradient.numGradients; ++g) {
							writer.StartObject();
							{
								if (morph) {
									writer.Key("startRatio");
								} else {
									writer.Key("ratio");
								}
								writer.Uint(style.gradient.gradientRecords[g].startRatio);
								if (morph) {
									writer.Key("endRatio");
									writer.Uint(style.gradient.gradientRecords[g].endRatio);
								}

								if (morph) {
									writer.Key("startColor");
								} else {
									writer.Key("color");
								}
								writer.StartArray();
								writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
								{
									writer.Uint(style.gradient.gradientRecords[g].startColor.r);
									writer.Uint(style.gradient.gradientRecords[g].startColor.g);
									writer.Uint(style.gradient.gradientRecords[g].startColor.b);
									writer.Uint(style.gradient.gradientRecords[g].startColor.a);
								}
								writer.EndArray();
								writer.SetFormatOptions(rapidjson::kFormatDefault);
								if (morph) {
									writer.Key("endColor");
									writer.StartArray();
									writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
									{
										writer.Uint(style.gradient.gradientRecords[g].endColor.r);
										writer.Uint(style.gradient.gradientRecords[g].endColor.g);
										writer.Uint(style.gradient.gradientRecords[g].endColor.b);
										writer.Uint(style.gradient.gradientRecords[g].endColor.a);
									}
									writer.EndArray();
									writer.SetFormatOptions(rapidjson::kFormatDefault);
								}
							}
							writer.EndObject();
						}
						writer.EndArray();
						if (style.subType == 3) { // focal gradient
							if (morph) {
								idStr msg;
								msg.Format("fillDraws (%d): Focal gradients aren't allowed in morph shapes.", d);
								errorMsg.Append(msg);
								local::UpdateErrorMsg(errorMsg, morph, characterID);
								return false;
							}
							writer.Key("focalPoint");
							writer.Double(style.focalPoint);
						}
					}
				}
				writer.EndObject();

				if (morph) {
					writer.Key("startVertices");
				} else {
					writer.Key("vertices");
				}
				writer.StartArray();
				writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
				for (int v = 0; v < fillDraw.startVerts.Num(); ++v) {
					writer.StartArray();
					{
						writer.Double(fillDraw.startVerts[v].x);
						writer.Double(fillDraw.startVerts[v].y);
					}
					writer.EndArray();
				}
				writer.EndArray();
				writer.SetFormatOptions(rapidjson::kFormatDefault);
				if (morph) {
					writer.Key("endVertices");
					writer.StartArray();
					writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
					for (int v = 0; v < fillDraw.endVerts.Num(); ++v) {
						writer.StartArray();
						{
							writer.Double(fillDraw.endVerts[v].x);
							writer.Double(fillDraw.endVerts[v].y);
						}
						writer.EndArray();
					}
					writer.EndArray();
					writer.SetFormatOptions(rapidjson::kFormatDefault);
				}

				writer.Key("indices");
				writer.StartArray();
				writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
				for (int i = 0; i < fillDraw.indices.Num(); ++i) {
					writer.Uint(fillDraw.indices[i]);
				}
				writer.EndArray();
				writer.SetFormatOptions(rapidjson::kFormatDefault);
			}
			writer.EndObject();
		}
		writer.EndArray();

		writer.Key("lineDraws");
		writer.StartArray();
		for (int d = 0; d < shape.lineDraws.Num(); ++d) {
			const idSWFShapeDrawLine &lineDraw = shape.lineDraws[d];
			writer.StartObject();
			{
				writer.Key("style");
				writer.StartObject();
				{
					const swfLineStyle_t &style = lineDraw.style;
					if (morph) {
						writer.Key("startWidth");
					} else {
						writer.Key("width");
					}
					writer.Uint(style.startWidth);
					if (morph) {
						writer.Key("endWidth");
						writer.Uint(style.endWidth);
					}

					if (morph) {
						writer.Key("startColor");
					} else {
						writer.Key("color");
					}
					writer.StartArray();
					writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
					{
						writer.Uint(style.startColor.r);
						writer.Uint(style.startColor.g);
						writer.Uint(style.startColor.b);
						writer.Uint(style.startColor.a);
					}
					writer.EndArray();
					writer.SetFormatOptions(rapidjson::kFormatDefault);
					if (morph) {
						writer.Key("endColor");
						writer.StartArray();
						writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
						{
							writer.Uint(style.endColor.r);
							writer.Uint(style.endColor.g);
							writer.Uint(style.endColor.b);
							writer.Uint(style.endColor.a);
						}
						writer.EndArray();
						writer.SetFormatOptions(rapidjson::kFormatDefault);
					}
				}
				writer.EndObject();

				if (morph) {
					writer.Key("startVertices");
				} else {
					writer.Key("vertices");
				}
				writer.StartArray();
				writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
				for (int v = 0; v < lineDraw.startVerts.Num(); ++v) {
					writer.StartArray();
					{
						writer.Double(lineDraw.startVerts[v].x);
						writer.Double(lineDraw.startVerts[v].y);
					}
					writer.EndArray();
				}
				writer.EndArray();
				writer.SetFormatOptions(rapidjson::kFormatDefault);
				if (morph) {
					writer.Key("endVertices");
					writer.StartArray();
					writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
					for (int v = 0; v < lineDraw.endVerts.Num(); ++v) {
						writer.StartArray();
						{
							writer.Double(lineDraw.endVerts[v].x);
							writer.Double(lineDraw.endVerts[v].y);
						}
						writer.EndArray();
					}
					writer.EndArray();
					writer.SetFormatOptions(rapidjson::kFormatDefault);
				}

				writer.Key("indices");
				writer.StartArray();
				writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
				for (int i = 0; i < lineDraw.indices.Num(); ++i) {
					writer.Uint(lineDraw.indices[i]);
				}
				writer.EndArray();
				writer.SetFormatOptions(rapidjson::kFormatDefault);
			}
			writer.EndObject();
		}
		writer.EndArray();
	}
	writer.EndObject();
	return true;
}

// ===============
void idSWF::CstLoadJson_Shape(idSWFShape &shape, bool morph, JsonValue &objShape, const idList<idStr> &characterIDs, idList<idStr> &errorMsg) {
	// may throw CstJson::Except
	assert(objShape.IsObject());

	const char keyStartBounds[] = "startBounds";
	const char keyBounds[] = "bounds";
	JsonValue *pArrayStartBounds = NULL;
	if (morph) {
		pArrayStartBounds = &CstJson::Object_GetArray(objShape, keyStartBounds, errorMsg, 4);
	} else {
		pArrayStartBounds = &CstJson::Object_GetArray(objShape, keyBounds, errorMsg, 4);
	}
	unsigned index;
	try {
		shape.startBounds.tl.x = CstJson::Array_GetFloat(*pArrayStartBounds, index = 0, errorMsg);
		shape.startBounds.tl.y = CstJson::Array_GetFloat(*pArrayStartBounds, index = 1, errorMsg);
		shape.startBounds.br.x = CstJson::Array_GetFloat(*pArrayStartBounds, index = 2, errorMsg);
		shape.startBounds.br.y = CstJson::Array_GetFloat(*pArrayStartBounds, index = 3, errorMsg);
	}
	catch (CstJson::Except) {
		if (morph) {
			idStr msg; msg.Format("%s (%u)", keyStartBounds, index); errorMsg.Append(msg);
		} else {
			idStr msg; msg.Format("%s (%u)", keyBounds, index); errorMsg.Append(msg);
		}
		throw;
	}
	if (morph) {
		const char keyEndBounds[] = "endBounds";
		JsonValue &arrayEndBounds = CstJson::Object_GetArray(objShape, keyEndBounds, errorMsg, 4);
		try {
			shape.endBounds.tl.x = CstJson::Array_GetFloat(arrayEndBounds, index = 0, errorMsg);
			shape.endBounds.tl.y = CstJson::Array_GetFloat(arrayEndBounds, index = 1, errorMsg);
			shape.endBounds.br.x = CstJson::Array_GetFloat(arrayEndBounds, index = 2, errorMsg);
			shape.endBounds.br.y = CstJson::Array_GetFloat(arrayEndBounds, index = 3, errorMsg);
		}
		catch (CstJson::Except) {
			idStr msg; msg.Format("%s (%u)", keyEndBounds, index); errorMsg.Append(msg);
			throw;
		}
	} else {
		shape.endBounds = shape.startBounds;
	}

	const char keyFillDraws[] = "fillDraws";
	JsonValue &arrayFillDraws = CstJson::Object_GetArray(objShape, keyFillDraws, errorMsg);
	shape.fillDraws.SetNum(arrayFillDraws.Size());
	for (JsonSizeType d = 0; d < arrayFillDraws.Size(); ++d) {
		try {
			idSWFShapeDrawFill &fillDraw = shape.fillDraws[d];
			JsonValue &objFillDraw = CstJson::Array_GetObject(arrayFillDraws, d, errorMsg);

			const char keyStyle[] = "style";
			JsonValue &objStyle = CstJson::Object_GetObject(objFillDraw, keyStyle, errorMsg);
			try {
				swfFillStyle_t &style = fillDraw.style;
				struct local {
					static bool GetTypeCode(const char *type, uint8 &typeCode) {
						if (idStr::Cmp("solid", type) == 0) { typeCode = 0; return true; }
						if (idStr::Cmp("gradient", type) == 0) { typeCode = 1; return true; }
						if (idStr::Cmp("bitmap", type) == 0) { typeCode = 4; return true; }
						return false;
					}
					static bool GetSubTypeCode(uint8 typeCode, const char *subType, uint8 &subTypeCode) {
						if (typeCode == 1) { // gradient
							if (idStr::Cmp("linear", subType) == 0) { subTypeCode = 0; return true; }
							if (idStr::Cmp("radial", subType) == 0) { subTypeCode = 2; return true; }
							if (idStr::Cmp("focal", subType) == 0) { subTypeCode = 3; return true; }
							return false;
						}
						if (typeCode == 4) { // bitmap
							if (idStr::Cmp("repeat", subType) == 0) { subTypeCode = 0; return true; }
							if (idStr::Cmp("clamp", subType) == 0) { subTypeCode = 1; return true; }
							if (idStr::Cmp("nearRepeat", subType) == 0) { subTypeCode = 2; return true; }
							if (idStr::Cmp("nearClamp", subType) == 0) { subTypeCode = 3; return true; }
							return false;
						}
						return false;
					}
				};

				const char keyType[] = "type";
				const char *type = CstJson::Object_GetString(objStyle, keyType, errorMsg);
				if (!local::GetTypeCode(type, style.type)) {
					idStr msg; msg.Format("Unknown type '%s'.", type); errorMsg.Append(msg);
					throw CstJson::Except();
				}

				if (style.type == 2 || style.type == 4) { // gradient or bitmap
					const char keySubType[] = "subType";
					const char *subType = CstJson::Object_GetString(objStyle, keySubType, errorMsg);
					if (!local::GetSubTypeCode(style.type, subType, style.subType)) {
						idStr msg; msg.Format("Unknown subType '%s' for type '%s'.", subType, type); errorMsg.Append(msg);
						throw CstJson::Except();
					}
				}

				if (style.type == 4) { // bitmap
					const char keyBitmapID[] = "bitmapID"; // bitmapID points to a charaterID
					idStr oldBitmapID = CstJson::Object_GetString(objStyle, keyBitmapID, errorMsg);
					int newBitmapID = 65535; // 65535 is a special case
					if (oldBitmapID != "65535") {
						newBitmapID = characterIDs.FindIndex(oldBitmapID);
						if (newBitmapID == -1) {
							errorMsg.Append("No dictionary entry matches the specified value.");
							errorMsg.Append(keyBitmapID);
							throw CstJson::Except();
						}
						// newBitmapID is the adjusted bitmapID (points to an adjusted characterID)
					}
					style.bitmapID = newBitmapID;
				}

				if (style.type == 0) { // solid
					const char keyStartColor[] = "startColor";
					const char keyColor[] = "color";
					JsonValue *pArrayStartColor = NULL;
					if (morph) {
						pArrayStartColor = &CstJson::Object_GetArray(objStyle, keyStartColor, errorMsg, 4);
					} else {
						pArrayStartColor = &CstJson::Object_GetArray(objStyle, keyColor, errorMsg, 4);
					}
					try {
						style.startColor.r = CstJson::Array_GetUint8(*pArrayStartColor, index = 0, errorMsg);
						style.startColor.g = CstJson::Array_GetUint8(*pArrayStartColor, index = 1, errorMsg);
						style.startColor.b = CstJson::Array_GetUint8(*pArrayStartColor, index = 2, errorMsg);
						style.startColor.a = CstJson::Array_GetUint8(*pArrayStartColor, index = 3, errorMsg);
					}
					catch (CstJson::Except) {
						if (morph) {
							idStr msg; msg.Format("%s (%u)", keyStartColor, index); errorMsg.Append(msg);
						} else {
							idStr msg; msg.Format("%s (%u)", keyColor, index); errorMsg.Append(msg);
						}
						throw;
					}
					if (morph) {
						const char keyEndColor[] = "endColor";
						JsonValue &arrayEndColor = CstJson::Object_GetArray(objStyle, keyEndColor, errorMsg, 4);
						try {
							style.endColor.r = CstJson::Array_GetUint8(arrayEndColor, index = 0, errorMsg);
							style.endColor.g = CstJson::Array_GetUint8(arrayEndColor, index = 1, errorMsg);
							style.endColor.b = CstJson::Array_GetUint8(arrayEndColor, index = 2, errorMsg);
							style.endColor.a = CstJson::Array_GetUint8(arrayEndColor, index = 3, errorMsg);
						}
						catch (CstJson::Except) {
							idStr msg; msg.Format("%s (%u)", keyEndColor, index); errorMsg.Append(msg);
							throw;
						}
					} else {
						style.endColor = style.startColor;
					}
				}

				if (style.type == 1 || style.type == 4) { // gradient or bitmap
					const char keyStartMatrix[] = "startMatrix";
					const char keyMatrix[] = "matrix";
					JsonValue *pArrayStartMatrix = NULL;
					if (morph) {
						pArrayStartMatrix = &CstJson::Object_GetArray(objStyle, keyStartMatrix, errorMsg, 6);
					} else {
						pArrayStartMatrix = &CstJson::Object_GetArray(objStyle, keyMatrix, errorMsg, 6);
					}
					try {
						style.startMatrix.xx = CstJson::Array_GetFloat(*pArrayStartMatrix, index = 0, errorMsg);
						style.startMatrix.yy = CstJson::Array_GetFloat(*pArrayStartMatrix, index = 1, errorMsg);
						style.startMatrix.xy = CstJson::Array_GetFloat(*pArrayStartMatrix, index = 2, errorMsg);
						style.startMatrix.yx = CstJson::Array_GetFloat(*pArrayStartMatrix, index = 3, errorMsg);
						style.startMatrix.tx = CstJson::Array_GetFloat(*pArrayStartMatrix, index = 4, errorMsg);
						style.startMatrix.ty = CstJson::Array_GetFloat(*pArrayStartMatrix, index = 5, errorMsg);
					}
					catch (CstJson::Except) {
						if (morph) {
							idStr msg; msg.Format("%s (%u)", keyStartMatrix, index); errorMsg.Append(msg);
						} else {
							idStr msg; msg.Format("%s (%u)", keyMatrix, index); errorMsg.Append(msg);
						}
						throw;
					}
					if (morph) {
						const char keyEndMatrix[] = "endMatrix";
						JsonValue &arrayEndMatrix = CstJson::Object_GetArray(objStyle, keyEndMatrix, errorMsg, 6);
						try {
							style.endMatrix.xx = CstJson::Array_GetFloat(arrayEndMatrix, index = 0, errorMsg);
							style.endMatrix.yy = CstJson::Array_GetFloat(arrayEndMatrix, index = 1, errorMsg);
							style.endMatrix.xy = CstJson::Array_GetFloat(arrayEndMatrix, index = 2, errorMsg);
							style.endMatrix.yx = CstJson::Array_GetFloat(arrayEndMatrix, index = 3, errorMsg);
							style.endMatrix.tx = CstJson::Array_GetFloat(arrayEndMatrix, index = 4, errorMsg);
							style.endMatrix.ty = CstJson::Array_GetFloat(arrayEndMatrix, index = 5, errorMsg);
						}
						catch (CstJson::Except) {
							idStr msg; msg.Format("%s (%u)", keyEndMatrix, index); errorMsg.Append(msg);
							throw;
						}
					} else {
						style.endMatrix = style.startMatrix;
					}

					if (style.type == 1) { // gradient
						const char keyGradients[] = "gradients";
						JsonValue &arrayGradients = CstJson::Object_GetArray(objStyle, keyGradients, errorMsg);
						if (arrayGradients.Size() > 15) {
							errorMsg.Append("The maximum number of gradients is 15.");
							throw CstJson::Except();
						}
						style.gradient.numGradients = arrayGradients.Size();
						for (uint8 g = 0; g < style.gradient.numGradients; ++g) {
							try {
								JsonValue &objGradient = CstJson::Array_GetObject(arrayGradients, g, errorMsg);
								if (morph) {
									style.gradient.gradientRecords[g].startRatio = CstJson::Object_GetUint8(objGradient, "startRatio", errorMsg);
									style.gradient.gradientRecords[g].endRatio = CstJson::Object_GetUint8(objGradient, "endRatio", errorMsg);
								} else {
									style.gradient.gradientRecords[g].startRatio = CstJson::Object_GetUint8(objGradient, "ratio", errorMsg);
									style.gradient.gradientRecords[g].endRatio = style.gradient.gradientRecords[g].startRatio;
								}

								const char keyStartColor[] = "startColor";
								const char keyColor[] = "color";
								JsonValue *pArrayStartColor = NULL;
								if (morph) {
									pArrayStartColor = &CstJson::Object_GetArray(objGradient, keyStartColor, errorMsg, 4);
								} else {
									pArrayStartColor = &CstJson::Object_GetArray(objGradient, keyColor, errorMsg, 4);
								}
								try {
									style.gradient.gradientRecords[g].startColor.r = CstJson::Array_GetUint8(*pArrayStartColor, index = 0, errorMsg);
									style.gradient.gradientRecords[g].startColor.g = CstJson::Array_GetUint8(*pArrayStartColor, index = 1, errorMsg);
									style.gradient.gradientRecords[g].startColor.b = CstJson::Array_GetUint8(*pArrayStartColor, index = 2, errorMsg);
									style.gradient.gradientRecords[g].startColor.a = CstJson::Array_GetUint8(*pArrayStartColor, index = 3, errorMsg);
								}
								catch (CstJson::Except) {
									if (morph) {
										idStr msg; msg.Format("%s (%u)", keyStartColor, index); errorMsg.Append(msg);
									} else {
										idStr msg; msg.Format("%s (%u)", keyColor, index); errorMsg.Append(msg);
									}
									throw;
								}
								if (morph) {
									const char keyEndColor[] = "endColor";
									JsonValue &arrayEndColor = CstJson::Object_GetArray(objGradient, keyEndColor, errorMsg, 4);
									try {
										style.gradient.gradientRecords[g].endColor.r = CstJson::Array_GetUint8(arrayEndColor, index = 0, errorMsg);
										style.gradient.gradientRecords[g].endColor.g = CstJson::Array_GetUint8(arrayEndColor, index = 1, errorMsg);
										style.gradient.gradientRecords[g].endColor.b = CstJson::Array_GetUint8(arrayEndColor, index = 2, errorMsg);
										style.gradient.gradientRecords[g].endColor.a = CstJson::Array_GetUint8(arrayEndColor, index = 3, errorMsg);
									}
									catch (CstJson::Except) {
										idStr msg; msg.Format("%s (%u)", keyEndColor, index); errorMsg.Append(msg);
										throw;
									}
								} else {
									style.gradient.gradientRecords[g].endColor = style.gradient.gradientRecords[g].startColor;
								}
							}
							catch (CstJson::Except) {
								idStr msg; msg.Format("%s (%u)", keyGradients, g); errorMsg.Append(msg);
								throw;
							}
						}
						if (style.subType == 3) { // focal gradient
							if (morph) {
								errorMsg.Append("Focal gradients aren't allowed in morph shapes.");
								throw CstJson::Except();
							}
							style.focalPoint = CstJson::Object_GetFloat(objStyle, "focalPoint", errorMsg);
						}
					}
				}
			}
			catch (CstJson::Except) {
				errorMsg.Append(keyStyle);
				throw;
			}

			const char keyStartVertices[] = "startVertices";
			const char keyVertices[] = "vertices";
			JsonValue *pArrayStartVertices = NULL;
			if (morph) {
				pArrayStartVertices = &CstJson::Object_GetArray(objFillDraw, keyStartVertices, errorMsg);
			} else {
				pArrayStartVertices = &CstJson::Object_GetArray(objFillDraw, keyVertices, errorMsg);
			}
			fillDraw.startVerts.SetNum(pArrayStartVertices->Size());
			for (JsonSizeType v = 0; v < pArrayStartVertices->Size(); ++v) {
				try {
					JsonValue &arrayVertex = CstJson::Array_GetArray(*pArrayStartVertices, v, errorMsg, 2);
					try {
						fillDraw.startVerts[v].x = CstJson::Array_GetFloat(arrayVertex, index = 0, errorMsg);
						fillDraw.startVerts[v].y = CstJson::Array_GetFloat(arrayVertex, index = 1, errorMsg);
					}
					catch (CstJson::Except) {
						idStr msg; msg.Format("(%u)", index); errorMsg.Append(msg);
						throw;
					}
				}
				catch (CstJson::Except) {
					if (morph) {
						idStr msg; msg.Format("%s (%u)", keyStartVertices, v); errorMsg.Append(msg);
					} else {
						idStr msg; msg.Format("%s (%u)", keyVertices, v); errorMsg.Append(msg);
					}
					throw;
				}
			}
			if (morph) {
				const char keyEndVertices[] = "endVertices";
				JsonValue &arrayEndVertices = CstJson::Object_GetArray(objFillDraw, keyEndVertices, errorMsg);
				if (arrayEndVertices.Size() != pArrayStartVertices->Size()) {
					idStr msg; msg.Format("'%s' and '%s' must have the same number of vertices.", keyStartVertices, keyEndVertices); errorMsg.Append(msg);
					throw CstJson::Except();
				}
				fillDraw.endVerts.SetNum(arrayEndVertices.Size());
				for (JsonSizeType v = 0; v < arrayEndVertices.Size(); ++v) {
					try {
						JsonValue &arrayVertex = CstJson::Array_GetArray(arrayEndVertices, v, errorMsg, 2);
						try {
							fillDraw.endVerts[v].x = CstJson::Array_GetFloat(arrayVertex, index = 0, errorMsg);
							fillDraw.endVerts[v].y = CstJson::Array_GetFloat(arrayVertex, index = 1, errorMsg);
						}
						catch (CstJson::Except) {
							idStr msg; msg.Format("(%u)", index); errorMsg.Append(msg);
							throw;
						}
					}
					catch (CstJson::Except) {
						idStr msg; msg.Format("%s (%u)", keyEndVertices, v); errorMsg.Append(msg);
						throw;
					}
				}
			}
			// endVerts aren't really used by non-morph shapes, so don't copy startVerts to endVerts in this case to save some space

			const char keyIndices[] = "indices";
			JsonValue &arrayIndices = CstJson::Object_GetArray(objFillDraw, keyIndices, errorMsg);
			fillDraw.indices.SetNum(arrayIndices.Size());
			for (JsonSizeType i = 0; i < arrayIndices.Size(); ++i) {
				try {
					fillDraw.indices[i] = CstJson::Array_GetUint16(arrayIndices, i, errorMsg);
				}
				catch (CstJson::Except) {
					idStr msg; msg.Format("%s (%u)", keyIndices, i); errorMsg.Append(msg);
					throw;
				}
			}
		}
		catch (CstJson::Except) {
			idStr msg; msg.Format("%s (%u)", keyFillDraws, d); errorMsg.Append(msg);
			throw;
		}
	}

	const char keyLineDraws[] = "lineDraws";
	JsonValue &arrayLineDraws = CstJson::Object_GetArray(objShape, keyLineDraws, errorMsg);
	shape.lineDraws.SetNum(arrayLineDraws.Size());
	for (JsonSizeType d = 0; d < arrayLineDraws.Size(); ++d) {
		try {
			idSWFShapeDrawLine &lineDraw = shape.lineDraws[d];
			JsonValue &objLineDraw = CstJson::Array_GetObject(arrayLineDraws, d, errorMsg);

			const char keyStyle[] = "style";
			JsonValue &objStyle = CstJson::Object_GetObject(objLineDraw, keyStyle, errorMsg);
			try {
				swfLineStyle_t &style = lineDraw.style;

				if (morph) {
					style.startWidth = CstJson::Object_GetUint16(objStyle, "startWidth", errorMsg);
					style.endWidth = CstJson::Object_GetUint16(objStyle, "endWidth", errorMsg);
				} else {
					style.startWidth = CstJson::Object_GetUint16(objStyle, "width", errorMsg);
					style.endWidth = style.startWidth;
				}

				const char keyStartColor[] = "startColor";
				const char keyColor[] = "color";
				JsonValue *pArrayStartColor = NULL;
				if (morph) {
					pArrayStartColor = &CstJson::Object_GetArray(objStyle, keyStartColor, errorMsg, 4);
				} else {
					pArrayStartColor = &CstJson::Object_GetArray(objStyle, keyColor, errorMsg, 4);
				}
				try {
					style.startColor.r = CstJson::Array_GetUint8(*pArrayStartColor, index = 0, errorMsg);
					style.startColor.g = CstJson::Array_GetUint8(*pArrayStartColor, index = 1, errorMsg);
					style.startColor.b = CstJson::Array_GetUint8(*pArrayStartColor, index = 2, errorMsg);
					style.startColor.a = CstJson::Array_GetUint8(*pArrayStartColor, index = 3, errorMsg);
				}
				catch (CstJson::Except) {
					if (morph) {
						idStr msg; msg.Format("%s (%u)", keyStartColor, index); errorMsg.Append(msg);
					} else {
						idStr msg; msg.Format("%s (%u)", keyColor, index); errorMsg.Append(msg);
					}
					throw;
				}
				if (morph) {
					const char keyEndColor[] = "endColor";
					JsonValue &arrayEndColor = CstJson::Object_GetArray(objStyle, keyEndColor, errorMsg, 4);
					try {
						style.endColor.r = CstJson::Array_GetUint8(arrayEndColor, index = 0, errorMsg);
						style.endColor.g = CstJson::Array_GetUint8(arrayEndColor, index = 1, errorMsg);
						style.endColor.b = CstJson::Array_GetUint8(arrayEndColor, index = 2, errorMsg);
						style.endColor.a = CstJson::Array_GetUint8(arrayEndColor, index = 3, errorMsg);
					}
					catch (CstJson::Except) {
						idStr msg; msg.Format("%s (%u)", keyEndColor, index); errorMsg.Append(msg);
						throw;
					}
				} else {
					style.endColor = style.startColor;
				}
			}
			catch (CstJson::Except) {
				errorMsg.Append(keyStyle);
				throw;
			}

			const char keyStartVertices[] = "startVertices";
			const char keyVertices[] = "vertices";
			JsonValue *pArrayStartVertices = NULL;
			if (morph) {
				pArrayStartVertices = &CstJson::Object_GetArray(objLineDraw, keyStartVertices, errorMsg);
			} else {
				pArrayStartVertices = &CstJson::Object_GetArray(objLineDraw, keyVertices, errorMsg);
			}
			lineDraw.startVerts.SetNum(pArrayStartVertices->Size());
			for (JsonSizeType v = 0; v < pArrayStartVertices->Size(); ++v) {
				try {
					JsonValue &arrayVertex = CstJson::Array_GetArray(*pArrayStartVertices, v, errorMsg, 2);
					try {
						lineDraw.startVerts[v].x = CstJson::Array_GetFloat(arrayVertex, index = 0, errorMsg);
						lineDraw.startVerts[v].y = CstJson::Array_GetFloat(arrayVertex, index = 1, errorMsg);
					}
					catch (CstJson::Except) {
						idStr msg; msg.Format("(%u)", index); errorMsg.Append(msg);
						throw;
					}
				}
				catch (CstJson::Except) {
					if (morph) {
						idStr msg; msg.Format("%s (%u)", keyStartVertices, index); errorMsg.Append(msg);
					} else {
						idStr msg; msg.Format("%s (%u)", keyVertices, index); errorMsg.Append(msg);
					}
					throw;
				}
			}
			if (morph) {
				const char keyEndVertices[] = "endVertices";
				JsonValue &arrayEndVertices = CstJson::Object_GetArray(objLineDraw, keyEndVertices, errorMsg);
				if (arrayEndVertices.Size() != pArrayStartVertices->Size()) {
					idStr msg; msg.Format("'%s' and '%s' must have the same number of vertices.", keyStartVertices, keyEndVertices); errorMsg.Append(msg);
					throw CstJson::Except();
				}
				lineDraw.endVerts.SetNum(arrayEndVertices.Size());
				for (JsonSizeType v = 0; v < arrayEndVertices.Size(); ++v) {
					try {
						JsonValue &arrayVertex = CstJson::Array_GetArray(arrayEndVertices, v, errorMsg, 2);
						try {
							lineDraw.endVerts[v].x = CstJson::Array_GetFloat(arrayVertex, index = 0, errorMsg);
							lineDraw.endVerts[v].y = CstJson::Array_GetFloat(arrayVertex, index = 1, errorMsg);
						}
						catch (CstJson::Except) {
							idStr msg; msg.Format("(%u)", index); errorMsg.Append(msg);
							throw;
						}
					}
					catch (CstJson::Except) {
						idStr msg; msg.Format("%s (%u)", keyEndVertices, v); errorMsg.Append(msg);
						throw;
					}
				}
			}
			// endVerts aren't really used by non-morph shapes, so don't copy startVerts to endVerts in this case to save some space

			const char keyIndices[] = "indices";
			JsonValue &arrayIndices = CstJson::Object_GetArray(objLineDraw, keyIndices, errorMsg);
			lineDraw.indices.SetNum(arrayIndices.Size());
			for (JsonSizeType i = 0; i < arrayIndices.Size(); ++i) {
				try {
					lineDraw.indices[i] = CstJson::Array_GetUint16(arrayIndices, i, errorMsg);
				}
				catch (CstJson::Except) {
					idStr msg; msg.Format("%s (%u)", keyIndices, i); errorMsg.Append(msg);
					throw;
				}
			}
		}
		catch (CstJson::Except) {
			idStr msg; msg.Format("%s (%u)", keyLineDraws, d); errorMsg.Append(msg);
			throw;
		}
	}
}

// ===============
void idSWF::CstWriteJson_Font(const idSWFFont &font, JsonPrettyWriter &writer, unsigned characterID) {
	writer.StartObject();
	{
		writer.Key("type");
		writer.String("FONT");
		writer.Key("characterID");
		writer.String(idStr(characterID).c_str());
		writer.Key("fontName");
		writer.String(font.fontID->GetName());
		writer.Key("ascent");
		writer.Int(font.ascent);
		writer.Key("descent");
		writer.Int(font.descent);
		writer.Key("leading");
		writer.Int(font.leading);

		writer.Key("glyphs");
		writer.StartArray();
		for (int g = 0; g < font.glyphs.Num(); ++g) {
			const idSWFFontGlyph &glyph = font.glyphs[g];
			writer.StartObject();
			{
				writer.Key("code");
				writer.Uint(glyph.code);
				writer.Key("advance");
				writer.Int(glyph.advance);
				writer.Key("vertices");
				writer.StartArray();
				writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
				for (int v = 0; v < glyph.verts.Num(); ++v) {
					writer.StartArray();
					{
						writer.Double(glyph.verts[v].x);
						writer.Double(glyph.verts[v].y);
					}
					writer.EndArray();
				}
				writer.EndArray();
				writer.SetFormatOptions(rapidjson::kFormatDefault);
				writer.Key("indices");
				writer.StartArray();
				writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
				for (int i = 0; i < glyph.indices.Num(); ++i) {
					writer.Uint(glyph.indices[i]);
				}
				writer.EndArray();
				writer.SetFormatOptions(rapidjson::kFormatDefault);
			}
			writer.EndObject();
		}
		writer.EndArray();
	}
	writer.EndObject();
}

// ===============
void idSWF::CstLoadJson_Font(idSWFFont &font, JsonValue &objFont, idList<idStr> &errorMsg) {
	// may throw CstJson::Except
	assert(objFont.IsObject());

	const char *fontName = CstJson::Object_GetString(objFont, "fontName", errorMsg);
	font.fontID = renderSystem->RegisterFont(fontName);

	font.ascent = CstJson::Object_GetInt16(objFont, "ascent", errorMsg);
	font.descent = CstJson::Object_GetInt16(objFont, "descent", errorMsg);
	font.leading = CstJson::Object_GetInt16(objFont, "leading", errorMsg);

	const char keyGlyphs[] = "glyphs";
	JsonValue &arrayGlyphs = CstJson::Object_GetArray(objFont, keyGlyphs, errorMsg);
	font.glyphs.SetNum(arrayGlyphs.Size());
	for (JsonSizeType g = 0; g < arrayGlyphs.Size(); ++g) {
		try {
			idSWFFontGlyph &glyph = font.glyphs[g];
			JsonValue &objGlyph = CstJson::Array_GetObject(arrayGlyphs, g, errorMsg);

			glyph.code = CstJson::Object_GetUint16(objGlyph, "code", errorMsg);
			glyph.advance = CstJson::Object_GetInt16(objGlyph, "advance", errorMsg);

			const char keyVertices[] = "vertices";
			JsonValue &arrayVertices = CstJson::Object_GetArray(objGlyph, keyVertices, errorMsg);
			glyph.verts.SetNum(arrayVertices.Size());
			for (JsonSizeType v = 0; v < arrayVertices.Size(); ++v) {
				try {
					JsonValue &arrayVertex = CstJson::Array_GetArray(arrayVertices, v, errorMsg, 2);
					unsigned index;
					try {
						glyph.verts[v].x = CstJson::Array_GetFloat(arrayVertex, index = 0, errorMsg);
						glyph.verts[v].y = CstJson::Array_GetFloat(arrayVertex, index = 1, errorMsg);
					}
					catch (CstJson::Except) {
						idStr msg; msg.Format("(%u)", index); errorMsg.Append(msg);
						throw;
					}
				}
				catch (CstJson::Except) {
					idStr msg; msg.Format("%s (%u)", keyVertices, v); errorMsg.Append(msg);
					throw;
				}
			}

			const char keyIndices[] = "indices";
			JsonValue &arrayIndices = CstJson::Object_GetArray(objGlyph, keyIndices, errorMsg);
			glyph.indices.SetNum(arrayIndices.Size());
			for (JsonSizeType i = 0; i < arrayIndices.Size(); ++i) {
				try {
					glyph.indices[i] = CstJson::Array_GetUint16(arrayIndices, i, errorMsg);
				}
				catch (CstJson::Except) {
					idStr msg; msg.Format("%s (%u)", keyIndices, i); errorMsg.Append(msg);
					throw;
				}
			}
		}
		catch (CstJson::Except) {
			idStr msg; msg.Format("%s (%u)", keyGlyphs, g); errorMsg.Append(msg);
			throw;
		}
	}
}

// ===============
void idSWF::CstWriteJson_Text(const idSWFText &text, JsonPrettyWriter &writer, unsigned characterID) {
	writer.StartObject();
	{
		writer.Key("type");
		writer.String("TEXT");
		writer.Key("characterID");
		writer.String(idStr(characterID).c_str());
		writer.Key("bounds");
		writer.StartArray();
		writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
		{
			writer.Double(text.bounds.tl.x); // top left
			writer.Double(text.bounds.tl.y);
			writer.Double(text.bounds.br.x); // bottom right
			writer.Double(text.bounds.br.y);
		}
		writer.EndArray();
		writer.SetFormatOptions(rapidjson::kFormatDefault);
		writer.Key("matrix");
		writer.StartArray();
		writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
		{
			writer.Double(text.matrix.xx);
			writer.Double(text.matrix.yy);
			writer.Double(text.matrix.xy);
			writer.Double(text.matrix.yx);
			writer.Double(text.matrix.tx);
			writer.Double(text.matrix.ty);
		}
		writer.EndArray();
		writer.SetFormatOptions(rapidjson::kFormatDefault);

		writer.Key("textRecords");
		writer.StartArray();
		for (int t = 0; t < text.textRecords.Num(); ++t) {
			const idSWFTextRecord &textRecord = text.textRecords[t];
			writer.StartObject();
			{
				writer.Key("fontID");
				writer.String(idStr(textRecord.fontID).c_str());
				writer.Key("color");
				writer.StartArray();
				writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
				{
					writer.Uint(textRecord.color.r);
					writer.Uint(textRecord.color.g);
					writer.Uint(textRecord.color.b);
					writer.Uint(textRecord.color.a);
				}
				writer.EndArray();
				writer.SetFormatOptions(rapidjson::kFormatDefault);
				writer.Key("offset");
				writer.StartArray();
				writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
				{
					writer.Int(textRecord.xOffset);
					writer.Int(textRecord.yOffset);
				}
				writer.EndArray();
				writer.SetFormatOptions(rapidjson::kFormatDefault);
				writer.Key("textHeight");
				writer.Uint(textRecord.textHeight);
				writer.Key("firstGlyph");
				writer.Uint(textRecord.firstGlyph);
				writer.Key("numGlyphs");
				writer.Uint(textRecord.numGlyphs);
			}
			writer.EndObject();
		}
		writer.EndArray();

		writer.Key("glyphs");
		writer.StartArray();
		for (int g = 0; g < text.glyphs.Num(); ++g) {
			const swfGlyphEntry_t &glyph = text.glyphs[g];
			writer.StartObject();
			{
				writer.Key("index");
				writer.Uint(glyph.index);
				writer.Key("advance");
				writer.Int(glyph.advance);
			}
			writer.EndObject();
		}
		writer.EndArray();
	}
	writer.EndObject();
}

// ===============
void idSWF::CstLoadJson_Text(idSWFText &text, JsonValue &objText, const idList<idStr> &characterIDs, idList<idStr> &errorMsg) {
	// may throw CstJson::Except
	assert(objText.IsObject());

	const char keyBounds[] = "bounds";
	JsonValue &arrayBounds = CstJson::Object_GetArray(objText, keyBounds, errorMsg, 4);
	unsigned index;
	try {
		text.bounds.tl.x = CstJson::Array_GetFloat(arrayBounds, index = 0, errorMsg);
		text.bounds.tl.y = CstJson::Array_GetFloat(arrayBounds, index = 1, errorMsg);
		text.bounds.br.x = CstJson::Array_GetFloat(arrayBounds, index = 2, errorMsg);
		text.bounds.br.y = CstJson::Array_GetFloat(arrayBounds, index = 3, errorMsg);
	}
	catch (CstJson::Except) {
		idStr msg; msg.Format("%s (%u)", keyBounds, index); errorMsg.Append(msg);
		throw;
	}

	const char keyMatrix[] = "matrix";
	JsonValue &arrayMatrix = CstJson::Object_GetArray(objText, keyMatrix, errorMsg, 6);
	try {
		text.matrix.xx = CstJson::Array_GetFloat(arrayMatrix, index = 0, errorMsg);
		text.matrix.yy = CstJson::Array_GetFloat(arrayMatrix, index = 1, errorMsg);
		text.matrix.xy = CstJson::Array_GetFloat(arrayMatrix, index = 2, errorMsg);
		text.matrix.yx = CstJson::Array_GetFloat(arrayMatrix, index = 3, errorMsg);
		text.matrix.tx = CstJson::Array_GetFloat(arrayMatrix, index = 4, errorMsg);
		text.matrix.ty = CstJson::Array_GetFloat(arrayMatrix, index = 5, errorMsg);
	}
	catch (CstJson::Except) {
		idStr msg; msg.Format("%s (%u)", keyMatrix, index); errorMsg.Append(msg);
		throw;
	}

	const char keyTextRecords[] = "textRecords";
	JsonValue &arrayTextRecords = CstJson::Object_GetArray(objText, keyTextRecords, errorMsg);
	text.textRecords.SetNum(arrayTextRecords.Size());
	for (JsonSizeType t = 0; t < arrayTextRecords.Size(); ++t) {
		try {
			idSWFTextRecord &textRecord = text.textRecords[t];
			JsonValue &objTextRecord = CstJson::Array_GetObject(arrayTextRecords, t, errorMsg);

			const char keyFontID[] = "fontID"; // fontID points to a charaterID
			idStr oldFontID = CstJson::Object_GetString(objTextRecord, keyFontID, errorMsg);
			int newFontID = characterIDs.FindIndex(oldFontID);
			if (newFontID == -1) {
				errorMsg.Append("No dictionary entry matches the specified value.");
				errorMsg.Append(keyFontID);
				throw CstJson::Except();
			}
			textRecord.fontID = newFontID; // adjusted fontID (points to an adjusted characterID)

			const char keyColor[] = "color";
			JsonValue &arrayColor = CstJson::Object_GetArray(objTextRecord, keyColor, errorMsg, 4);
			try {
				textRecord.color.r = CstJson::Array_GetUint8(arrayColor, index = 0, errorMsg);
				textRecord.color.g = CstJson::Array_GetUint8(arrayColor, index = 1, errorMsg);
				textRecord.color.b = CstJson::Array_GetUint8(arrayColor, index = 2, errorMsg);
				textRecord.color.a = CstJson::Array_GetUint8(arrayColor, index = 3, errorMsg);
			}
			catch (CstJson::Except) {
				idStr msg; msg.Format("%s (%u)", keyColor, index); errorMsg.Append(msg);
				throw;
			}

			const char keyOffset[] = "offset";
			JsonValue &arrayOffset = CstJson::Object_GetArray(objTextRecord, keyOffset, errorMsg, 2);
			try {
				textRecord.xOffset = CstJson::Array_GetInt16(arrayOffset, index = 0, errorMsg);
				textRecord.yOffset = CstJson::Array_GetInt16(arrayOffset, index = 1, errorMsg);
			}
			catch (CstJson::Except) {
				idStr msg; msg.Format("%s (%u)", keyOffset, index); errorMsg.Append(msg);
				throw;
			}

			textRecord.textHeight = CstJson::Object_GetUint16(objTextRecord, "textHeight", errorMsg);
			textRecord.firstGlyph = CstJson::Object_GetUint16(objTextRecord, "firstGlyph", errorMsg);
			textRecord.numGlyphs = CstJson::Object_GetUint8(objTextRecord, "numGlyphs", errorMsg);
		}
		catch (CstJson::Except) {
			idStr msg; msg.Format("%s (%u)", keyTextRecords, t); errorMsg.Append(msg);
			throw;
		}
	}

	const char keyGlyphs[] = "glyphs";
	JsonValue &arrayGlyphs = CstJson::Object_GetArray(objText, keyGlyphs, errorMsg);
	text.glyphs.SetNum(arrayGlyphs.Size());
	for (JsonSizeType g = 0; g < arrayGlyphs.Size(); ++g) {
		try {
			swfGlyphEntry_t &glyph = text.glyphs[g];
			JsonValue &objGlyph = CstJson::Array_GetObject(arrayGlyphs, g, errorMsg);

			glyph.index = CstJson::Object_GetUint32(objGlyph, "index", errorMsg);
			glyph.advance = CstJson::Object_GetInt32(objGlyph, "advance", errorMsg);
		}
		catch (CstJson::Except) {
			idStr msg; msg.Format("%s (%u)", keyGlyphs, g); errorMsg.Append(msg);
			throw;
		}
	}
}

// ===============
bool idSWF::CstWriteJson_EditText(const idSWFEditText &editText, JsonPrettyWriter &writer, unsigned characterID, idList<idStr> &errorMsg) {
	writer.StartObject();
	{
		writer.Key("type");
		writer.String("EDITTEXT");
		writer.Key("characterID");
		writer.String(idStr(characterID).c_str());
		writer.Key("bounds");
		writer.StartArray();
		writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
		{
			writer.Double(editText.bounds.tl.x); // top left
			writer.Double(editText.bounds.tl.y);
			writer.Double(editText.bounds.br.x); // bottom right
			writer.Double(editText.bounds.br.y);
		}
		writer.EndArray();
		writer.SetFormatOptions(rapidjson::kFormatDefault);
		writer.Key("flags");
		writer.StartArray();
		writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
		{
			if (editText.flags & SWF_ET_WORDWRAP) {
				writer.String("WordWrap");
			}
			if (editText.flags & SWF_ET_MULTILINE) {
				writer.String("Multiline");
			}
			if (editText.flags & SWF_ET_PASSWORD) {
				writer.String("Password");
			}
			if (editText.flags & SWF_ET_READONLY) {
				writer.String("ReadOnly");
			}
			if (editText.flags & SWF_ET_AUTOSIZE) {
				writer.String("AutoSize");
			}
			if (editText.flags & SWF_ET_BORDER) {
				writer.String("Border");
			}
		}
		writer.EndArray();
		writer.SetFormatOptions(rapidjson::kFormatDefault);
		writer.Key("fontID");
		writer.String(idStr(editText.fontID).c_str());
		writer.Key("fontHeight");
		writer.Uint(editText.fontHeight);
		writer.Key("color");
		writer.StartArray();
		writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
		{
			writer.Uint(editText.color.r);
			writer.Uint(editText.color.g);
			writer.Uint(editText.color.b);
			writer.Uint(editText.color.a);
		}
		writer.EndArray();
		writer.SetFormatOptions(rapidjson::kFormatDefault);
		writer.Key("maxLength");
		writer.Uint(editText.maxLength);
		writer.Key("align");
		switch (editText.align) {
		case SWF_ET_ALIGN_LEFT: writer.String("left"); break;
		case SWF_ET_ALIGN_RIGHT: writer.String("right"); break;
		case SWF_ET_ALIGN_CENTER: writer.String("center"); break;
		case SWF_ET_ALIGN_JUSTIFY: writer.String("justify"); break;
		default: {
			idStr msg;
			msg.Format("EditText (id=%d): Unknown align type '%d'.", characterID, (unsigned)editText.align);
			errorMsg.Append(msg);
			return false;
		}
		}
		writer.Key("leftMargin");
		writer.Uint(editText.leftMargin);
		writer.Key("rightMargin");
		writer.Uint(editText.rightMargin);
		writer.Key("indent");
		writer.Uint(editText.indent);
		writer.Key("leading");
		writer.Int(editText.leading);
		writer.Key("variable");
		writer.String(editText.variable.c_str());
		writer.Key("initialText");
		writer.String(editText.initialText.c_str());
	}
	writer.EndObject();
	return true;
}

// ===============
void idSWF::CstLoadJson_EditText(idSWFEditText &editText, JsonValue &objEditText, const idList<idStr> &characterIDs, idList<idStr> &errorMsg) {
	// may throw CstJson::Except
	assert(objEditText.IsObject());

	const char keyBounds[] = "bounds";
	JsonValue &arrayBounds = CstJson::Object_GetArray(objEditText, keyBounds, errorMsg, 4);
	unsigned index;
	try {
		editText.bounds.tl.x = CstJson::Array_GetFloat(arrayBounds, index = 0, errorMsg);
		editText.bounds.tl.y = CstJson::Array_GetFloat(arrayBounds, index = 1, errorMsg);
		editText.bounds.br.x = CstJson::Array_GetFloat(arrayBounds, index = 2, errorMsg);
		editText.bounds.br.y = CstJson::Array_GetFloat(arrayBounds, index = 3, errorMsg);
	}
	catch (CstJson::Except) {
		idStr msg; msg.Format("%s (%u)", keyBounds, index); errorMsg.Append(msg);
		throw;
	}

	const char keyFlags[] = "flags";
	JsonValue &arrayFlags = CstJson::Object_GetArray(objEditText, keyFlags, errorMsg);
	uint32 flags = 0;
	for (JsonSizeType i = 0; i < arrayFlags.Size(); ++i) {
		try {
			const char *flagText = CstJson::Array_GetString(arrayFlags, i, errorMsg);
			if (idStr::Cmp("WordWrap", flagText) == 0) { flags |= SWF_ET_WORDWRAP; }
			else if (idStr::Cmp("Multiline", flagText) == 0) { flags |= SWF_ET_MULTILINE; }
			else if (idStr::Cmp("Password", flagText) == 0) { flags |= SWF_ET_PASSWORD; }
			else if (idStr::Cmp("ReadOnly", flagText) == 0) { flags |= SWF_ET_READONLY; }
			else if (idStr::Cmp("AutoSize", flagText) == 0) { flags |= SWF_ET_AUTOSIZE; }
			else if (idStr::Cmp("Border", flagText) == 0) { flags |= SWF_ET_BORDER; }
			else {
				idStr msg; msg.Format("Unknown flag '%s'.", flagText); errorMsg.Append(msg);
				throw CstJson::Except();
			}
		}
		catch (CstJson::Except) {
			idStr msg; msg.Format("%s (%u)", keyFlags, i); errorMsg.Append(msg);
			throw;
		}
	}
	editText.flags = flags;

	const char keyFontID[] = "fontID"; // fontID points to a charaterID
	idStr oldFontID = CstJson::Object_GetString(objEditText, keyFontID, errorMsg);
	int newFontID = characterIDs.FindIndex(oldFontID);
	if (newFontID == -1) {
		errorMsg.Append("No dictionary entry matches the specified value.");
		errorMsg.Append(keyFontID);
		throw CstJson::Except();
	}
	editText.fontID = newFontID; // adjusted fontID (points to an adjusted characterID)

	editText.fontHeight = CstJson::Object_GetUint16(objEditText, "fontHeight", errorMsg);

	const char keyColor[] = "color";
	JsonValue &arrayColor = CstJson::Object_GetArray(objEditText, keyColor, errorMsg, 4);
	try {
		editText.color.r = CstJson::Array_GetUint8(arrayColor, index = 0, errorMsg);
		editText.color.g = CstJson::Array_GetUint8(arrayColor, index = 1, errorMsg);
		editText.color.b = CstJson::Array_GetUint8(arrayColor, index = 2, errorMsg);
		editText.color.a = CstJson::Array_GetUint8(arrayColor, index = 3, errorMsg);
	}
	catch (CstJson::Except) {
		idStr msg; msg.Format("%s (%u)", keyColor, index); errorMsg.Append(msg);
		throw;
	}

	editText.maxLength = CstJson::Object_GetUint16(objEditText, "maxLength", errorMsg);

	const char keyAlign[] = "align";
	const char *alignText = CstJson::Object_GetString(objEditText, keyAlign, errorMsg);
	swfEditTextAlign_t align;
	if (idStr::Cmp("left", alignText) == 0) { align = SWF_ET_ALIGN_LEFT; }
	else if (idStr::Cmp("right", alignText) == 0) { align = SWF_ET_ALIGN_RIGHT; }
	else if (idStr::Cmp("center", alignText) == 0) { align = SWF_ET_ALIGN_CENTER; }
	else if (idStr::Cmp("justify", alignText) == 0) { align = SWF_ET_ALIGN_JUSTIFY; }
	else {
		idStr msg; msg.Format("Unknown align type '%s'.", alignText); errorMsg.Append(msg);
		errorMsg.Append(keyAlign);
		throw CstJson::Except();
	}
	editText.align = align;

	editText.leftMargin = CstJson::Object_GetUint16(objEditText, "leftMargin", errorMsg);
	editText.rightMargin = CstJson::Object_GetUint16(objEditText, "rightMargin", errorMsg);
	editText.indent = CstJson::Object_GetUint16(objEditText, "indent", errorMsg);
	editText.leading = CstJson::Object_GetInt16(objEditText, "leading", errorMsg);
	editText.variable = CstJson::Object_GetString(objEditText, "variable", errorMsg);
	editText.initialText = CstJson::Object_GetString(objEditText, "initialText", errorMsg);
}

// ===============
#undef GetObject // this macro from windows GDI conflicts with rapidjson GetObject and confuses IntelliSense

// ===============
bool idSWF::CstLoadJson(const idStr &fileName) {
	struct local {
		static bool Load(idSWF &swf, rapidjson::Document &doc, idList<idStr> &errorMsg) {
			float oldFrameWidth = swf.frameWidth;
			float oldFrameHeight = swf.frameHeight;
			uint16 oldFrameRate = swf.frameRate;
			int oldMouseX = swf.mouseX;
			int oldMouseY = swf.mouseY;
			try {
				if (!doc.IsObject()) {
					errorMsg.Append("The root value must be an object.");
					throw CstJson::Except();
				}
				JsonValue &objDoc = doc;

				const char *version = CstJson::Object_GetString(objDoc, "bswfJsonVersion", errorMsg);
				if (idStr::Cmp(version, CST_BSWF_JSON_VERSION) != 0) {
					idStr msg; msg.Format("Version mismatch: Expected '%s'. Got '%s'.", CST_BSWF_JSON_VERSION, version); errorMsg.Append(msg);
					throw CstJson::Except();
				}

				swf.frameWidth = CstJson::Object_GetFloat(objDoc, "frameWidth", errorMsg);
				swf.frameHeight = CstJson::Object_GetFloat(objDoc, "frameHeight", errorMsg);
				swf.frameRate = CstJson::Object_GetUint16(objDoc, "frameRate_x256", errorMsg);

				if (swf.mouseX == -1) {
					swf.mouseX = (swf.frameWidth / 2);
				}
				if (swf.mouseY == -1) {
					swf.mouseY = (swf.frameHeight / 2);
				}

				// Dictionary entries are added in order of appearance. The characterID of each
				// entry and any references to it from other entries are adjusted as needed.
				idList<idStr> characterIDs;
				const char keyDictionary[] = "dictionary";
				JsonValue &arrayDict = CstJson::Object_GetArray(objDoc, keyDictionary, errorMsg);
				if (arrayDict.Size() > (1uLL << 16)) {
					errorMsg.Append("Too many dictionary entries."); // characterID values must fit in an uint16
					throw CstJson::Except();
				}

				// First pass through the dictionary: build the table of characterIDs.
				const char keyCharacterID[] = "characterID";
				for (JsonSizeType i = 0; i < arrayDict.Size(); ++i) {
					try {
						JsonValue &objDictEntry = CstJson::Array_GetObject(arrayDict, i, errorMsg);
						idStr characterID = CstJson::Object_GetString(objDictEntry, keyCharacterID, errorMsg);
						if (characterIDs.FindIndex(characterID) != -1) {
							idStr msg; msg.Format("The value '%s' isn't unique.", characterID.c_str()); errorMsg.Append(msg);
							errorMsg.Append(keyCharacterID);
							throw CstJson::Except();
						}
						characterIDs.Append(characterID);
					}
					catch (CstJson::Except) {
						idStr msg; msg.Format("%s (%u)", keyDictionary, i); errorMsg.Append(msg);
						throw;
					}
				}

				// Second pass through the dictionary: load the entries.
				swf.dictionary.SetNum(arrayDict.Size());
				for (JsonSizeType i = 0; i < arrayDict.Size(); ++i) {
					JsonValue *pObjDictEntry = NULL;
					const char *type;
					idStr oldCharacterID; // the new characterID is the position in the dictionary ('i' in this loop)
					try {
						pObjDictEntry = &CstJson::Array_GetObject(arrayDict, i, errorMsg);
						type = CstJson::Object_GetString(*pObjDictEntry, "type", errorMsg);
						oldCharacterID = CstJson::Object_GetString(*pObjDictEntry, keyCharacterID, errorMsg);
					}
					catch (CstJson::Except) {
						idStr msg; msg.Format("%s (%u)", keyDictionary, i); errorMsg.Append(msg);
						throw;
					}
					struct local {
						static bool GetSwfDictType(const char *type, swfDictType_t &swfDictType) {
							if (idStr::Cmp("NULL", type) == 0) { swfDictType = SWF_DICT_NULL; return true; }
							if (idStr::Cmp("IMAGE", type) == 0) { swfDictType = SWF_DICT_IMAGE; return true; }
							if (idStr::Cmp("SHAPE", type) == 0) { swfDictType = SWF_DICT_SHAPE; return true; }
							if (idStr::Cmp("MORPH", type) == 0) { swfDictType = SWF_DICT_MORPH; return true; }
							if (idStr::Cmp("SPRITE", type) == 0) { swfDictType = SWF_DICT_SPRITE; return true; }
							if (idStr::Cmp("FONT", type) == 0) { swfDictType = SWF_DICT_FONT; return true; }
							if (idStr::Cmp("TEXT", type) == 0) { swfDictType = SWF_DICT_TEXT; return true; }
							if (idStr::Cmp("EDITTEXT", type) == 0) { swfDictType = SWF_DICT_EDITTEXT; return true; }
							return false;
						}
					};
					swfDictType_t swfDictType;
					if (!local::GetSwfDictType(type, swfDictType)) {
						idStr msg;
						msg.Format("Unknown type '%s'.", type); errorMsg.Append(msg);
						msg.Format("%s (%u)", keyDictionary, i); errorMsg.Append(msg);
						throw CstJson::Except();
					}
					try {
						idSWFDictionaryEntry &dictEntry = swf.dictionary[i];
						dictEntry.type = swfDictType;
						switch (swfDictType) {
						case SWF_DICT_NULL: {
							// nothing to do
							break;
						}
						case SWF_DICT_IMAGE: {
							swf.CstLoadJson_Image(dictEntry, *pObjDictEntry, errorMsg);
							break;
						}
						case SWF_DICT_MORPH:
						case SWF_DICT_SHAPE: {
							dictEntry.shape = new (TAG_SWF) idSWFShape;
							swf.CstLoadJson_Shape(*dictEntry.shape, swfDictType == SWF_DICT_MORPH, *pObjDictEntry, characterIDs, errorMsg);
							break;
						}
						case SWF_DICT_SPRITE: {
							dictEntry.sprite = new (TAG_SWF) idSWFSprite(&swf);
							dictEntry.sprite->CstLoadJson(*pObjDictEntry, characterIDs, errorMsg);
							break;
						}
						case SWF_DICT_FONT: {
							dictEntry.font = new (TAG_SWF) idSWFFont;
							swf.CstLoadJson_Font(*dictEntry.font, *pObjDictEntry, errorMsg);
							break;
						}
						case SWF_DICT_TEXT: {
							dictEntry.text = new (TAG_SWF) idSWFText;
							swf.CstLoadJson_Text(*dictEntry.text, *pObjDictEntry, characterIDs, errorMsg);
							break;
						}
						case SWF_DICT_EDITTEXT: {
							dictEntry.edittext = new (TAG_SWF) idSWFEditText;
							swf.CstLoadJson_EditText(*dictEntry.edittext, *pObjDictEntry, characterIDs, errorMsg);
							break;
						}
						}
					}
					catch (CstJson::Except) {
						idStr msg; msg.Format("%s (id='%s')", type, oldCharacterID.c_str()); errorMsg.Append(msg);
						throw;
					}
				}

				// Load the main sprite.
				const char keyMainSprite[] = "mainSprite";
				try {
					JsonValue &objMainSprite = CstJson::Object_GetObject(objDoc, keyMainSprite, errorMsg);
					swf.mainsprite->CstLoadJson(objMainSprite, characterIDs, errorMsg);
				}
				catch (CstJson::Except) {
					errorMsg.Append(keyMainSprite);
					throw;
				}
			}
			catch (CstJson::Except) {
				swf.frameWidth = oldFrameWidth;
				swf.frameHeight = oldFrameHeight;
				swf.frameRate = oldFrameRate;
				swf.mouseX = oldMouseX;
				swf.mouseY = oldMouseY;
				swf.dictionary.Clear();
				return false;
			}
			return true;
		}
	};

	idStr jsonFileName = fileName;
	jsonFileName.SetFileExtension(".json");
	idFile_Memory *file = (idFile_Memory *)fileSystem->CstOpenFileRead_FileMemory(jsonFileName.c_str());
	if (!file) {
		idStr msg;
		msg.Format("Couldn't open '%s' for reading.", jsonFileName.c_str());
		idLib::Warning(msg.c_str());
		return false;
	}

	rapidjson::Document doc;
	const unsigned parseFlags = rapidjson::kParseCommentsFlag | rapidjson::kParseTrailingCommasFlag;
	doc.Parse<parseFlags>(file->GetDataPtr(), file->Length());
	delete file;
	if (doc.HasParseError()) {
		idStr msg;
		msg.Format("Failed parsing '%s' (offset %u): %s", jsonFileName.c_str(), doc.GetErrorOffset(), rapidjson::GetParseError_En(doc.GetParseError()));
		idLib::Warning(msg.c_str());
		return false;
	}

	idList<idStr> errorMsg;
	if (!local::Load(*this, doc, errorMsg)) {
		idStr msg;
		msg.Format("Couldn't load '%s'", jsonFileName.c_str());
		if (errorMsg.Num() > 0) {
			for (int i = errorMsg.Num() - 1; i >= 0; --i) {
				msg += ": ";
				msg += errorMsg[i];
			}
		} else {
			msg += '.';
		}
		idLib::Warning(msg.c_str());
		return false;
	}
	return true;
}

// ===============
JsonValue & CstJson::Object_GetObject(JsonValue &obj, const char *key, idList<idStr> &errorMsg) {
	assert(obj.IsObject());
	rapidjson::Value::MemberIterator iter = obj.FindMember(key);
	if (iter == obj.MemberEnd()) {
		idStr msg; msg.Format("Key '%s' not found.", key); errorMsg.Append(msg);
		throw Except();
	}
	if (!iter->value.IsObject()) {
		errorMsg.Append("Value must be an object.");
		errorMsg.Append(key);
		throw Except();
	}
	return iter->value;
}

// ===============
JsonValue & CstJson::Object_GetArray(JsonValue &obj, const char *key, idList<idStr> &errorMsg, int numElems) {
	assert(obj.IsObject());
	rapidjson::Value::MemberIterator iter = obj.FindMember(key);
	if (iter == obj.MemberEnd()) {
		idStr msg; msg.Format("Key '%s' not found.", key); errorMsg.Append(msg);
		throw Except();
	}
	if (!iter->value.IsArray()) {
		errorMsg.Append("Value must be an array.");
		errorMsg.Append(key);
		throw Except();
	}
	JsonValue &_array = iter->value;
	if (numElems >= 0 && _array.Size() != (unsigned)numElems) {
		idStr msg; msg.Format("Value must be an array[%d].", numElems); errorMsg.Append(msg);
		errorMsg.Append(key);
		throw Except();
	}
	return _array;
}

// ===============
uint32 CstJson::Object_GetUint(JsonValue &obj, const char *key, idList<idStr> &errorMsg, uint32 min, uint32 max) {
	assert(obj.IsObject());
	rapidjson::Value::ConstMemberIterator iter = obj.FindMember(key);
	if (iter == obj.MemberEnd()) {
		idStr msg; msg.Format("Key '%s' not found.", key); errorMsg.Append(msg);
		throw Except();
	}
	if (!iter->value.IsUint()) {
		errorMsg.Append("Value must be an uint.");
		errorMsg.Append(key);
		throw Except();
	}
	uint32 value = iter->value.GetUint();
	if (value < min) {
		idStr msg; msg.Format("Minimum is %u.", min); errorMsg.Append(msg);
		errorMsg.Append(key);
		throw Except();
	}
	if (value > max) {
		idStr msg; msg.Format("Maximum is %u.", max); errorMsg.Append(msg);
		errorMsg.Append(key);
		throw Except();
	}
	return value;
}

// ===============
uint8 CstJson::Object_GetUint8(JsonValue &obj, const char *key, idList<idStr> &errorMsg) {
	// may throw CstJson::Except
	return Object_GetUint(obj, key, errorMsg, 0, (1uLL << 8) - 1);
}

// ===============
uint16 CstJson::Object_GetUint16(JsonValue &obj, const char *key, idList<idStr> &errorMsg) {
	// may throw CstJson::Except
	return Object_GetUint(obj, key, errorMsg, 0, (1uLL << 16) - 1);
}

// ===============
uint32 CstJson::Object_GetUint32(JsonValue &obj, const char *key, idList<idStr> &errorMsg) {
	// may throw CstJson::Except
	return Object_GetUint(obj, key, errorMsg, 0, (1uLL << 32) - 1);
}

// ===============
int32 CstJson::Object_GetInt(JsonValue &obj, const char *key, idList<idStr> &errorMsg, int32 min, int32 max) {
	assert(obj.IsObject());
	rapidjson::Value::ConstMemberIterator iter = obj.FindMember(key);
	if (iter == obj.MemberEnd()) {
		idStr msg; msg.Format("Key '%s' not found.", key); errorMsg.Append(msg);
		throw Except();
	}
	if (!iter->value.IsInt()) {
		errorMsg.Append("Value must be an int.");
		errorMsg.Append(key);
		throw Except();
	}
	int32 value = iter->value.GetInt();
	if (value < min) {
		idStr msg; msg.Format("Minimum is %d.", min); errorMsg.Append(msg);
		errorMsg.Append(key);
		throw Except();
	}
	if (value > max) {
		idStr msg; msg.Format("Maximum is %d.", max); errorMsg.Append(msg);
		errorMsg.Append(key);
		throw Except();
	}
	return value;
}

// ===============
int8 CstJson::Object_GetInt8(JsonValue &obj, const char *key, idList<idStr> &errorMsg) {
	// may throw CstJson::Except
	return Object_GetInt(obj, key, errorMsg, -(1LL << 7), (1LL << 7) - 1);
}

// ===============
int16 CstJson::Object_GetInt16(JsonValue &obj, const char *key, idList<idStr> &errorMsg) {
	// may throw CstJson::Except
	return Object_GetInt(obj, key, errorMsg, -(1LL << 15), (1LL << 15) - 1);
}

// ===============
int32 CstJson::Object_GetInt32(JsonValue &obj, const char *key, idList<idStr> &errorMsg) {
	// may throw CstJson::Except
	return Object_GetInt(obj, key, errorMsg, -(1LL << 31), (1LL << 31) - 1);
}

// ===============
bool CstJson::Object_GetBool(JsonValue &obj, const char *key, idList<idStr> &errorMsg) {
	assert(obj.IsObject());
	rapidjson::Value::ConstMemberIterator iter = obj.FindMember(key);
	if (iter == obj.MemberEnd()) {
		idStr msg; msg.Format("Key '%s' not found.", key); errorMsg.Append(msg);
		throw Except();
	}
	if (!iter->value.IsBool()) {
		errorMsg.Append("Value must be a bool.");
		errorMsg.Append(key);
		throw Except();
	}
	return iter->value.GetBool();
}

// ===============
float CstJson::Object_GetFloat(JsonValue &obj, const char *key, idList<idStr> &errorMsg) {
	assert(obj.IsObject());
	rapidjson::Value::ConstMemberIterator iter = obj.FindMember(key);
	if (iter == obj.MemberEnd()) {
		idStr msg; msg.Format("Key '%s' not found.", key); errorMsg.Append(msg);
		throw Except();
	}
	if (!iter->value.IsFloat()) {
		errorMsg.Append("Value must be a float.");
		errorMsg.Append(key);
		throw Except();
	}
	return iter->value.GetFloat();
}

// ===============
double CstJson::Object_GetDouble(JsonValue &obj, const char *key, idList<idStr> &errorMsg) {
	assert(obj.IsObject());
	rapidjson::Value::ConstMemberIterator iter = obj.FindMember(key);
	if (iter == obj.MemberEnd()) {
		idStr msg; msg.Format("Key '%s' not found.", key); errorMsg.Append(msg);
		throw Except();
	}
	if (!iter->value.IsDouble()) {
		errorMsg.Append("Value must be a double.");
		errorMsg.Append(key);
		throw Except();
	}
	return iter->value.GetDouble();
}

// ===============
const char * CstJson::Object_GetString(JsonValue &obj, const char *key, idList<idStr> &errorMsg) {
	assert(obj.IsObject());
	rapidjson::Value::ConstMemberIterator iter = obj.FindMember(key);
	if (iter == obj.MemberEnd()) {
		idStr msg; msg.Format("Key '%s' not found.", key); errorMsg.Append(msg);
		throw Except();
	}
	if (!iter->value.IsString()) {
		errorMsg.Append("Value must be a string.");
		errorMsg.Append(key);
		throw Except();
	}
	return iter->value.GetString();
}

// ===============
JsonValue & CstJson::Array_GetObject(JsonValue &_array, JsonSizeType index, idList<idStr> &errorMsg) {
	assert(_array.IsArray());
	if (!_array[index].IsObject()) {
		errorMsg.Append("Expected an object.");
		throw Except();
	}
	return _array[index];
}

// ===============
JsonValue & CstJson::Array_GetArray(JsonValue &_array, JsonSizeType index, idList<idStr> &errorMsg, int numElems) {
	assert(_array.IsArray());
	if (!_array[index].IsArray()) {
		errorMsg.Append("Expected an array.");
		throw Except();
	}
	JsonValue &outArray = _array[index];
	if (numElems >= 0 && outArray.Size() != (unsigned)numElems) {
		idStr msg; msg.Format("Expected %d elements.", numElems); errorMsg.Append(msg);
		throw Except();
	}
	return outArray;
}

// ===============
uint32 CstJson::Array_GetUint(JsonValue &_array, JsonSizeType index, idList<idStr> &errorMsg, uint32 min, uint32 max) {
	assert(_array.IsArray());
	if (!_array[index].IsUint()) {
		errorMsg.Append("Expected an uint.");
		throw Except();
	}
	uint32 value = _array[index].GetUint();
	if (value < min) {
		idStr msg; msg.Format("Minimum is %u.", min); errorMsg.Append(msg);
		throw Except();
	}
	if (value > max) {
		idStr msg; msg.Format("Maximum is %u.", max); errorMsg.Append(msg);
		throw Except();
	}
	return value;
}

// ===============
uint8 CstJson::Array_GetUint8(JsonValue &_array, JsonSizeType index, idList<idStr> &errorMsg) {
	// may throw CstJson::Except
	return Array_GetUint(_array, index, errorMsg, 0, (1uLL << 8) - 1);
}

// ===============
uint16 CstJson::Array_GetUint16(JsonValue &_array, JsonSizeType index, idList<idStr> &errorMsg) {
	// may throw CstJson::Except
	return Array_GetUint(_array, index, errorMsg, 0, (1uLL << 16) - 1);
}

// ===============
uint32 CstJson::Array_GetUint32(JsonValue &_array, JsonSizeType index, idList<idStr> &errorMsg) {
	// may throw CstJson::Except
	return Array_GetUint(_array, index, errorMsg, 0, (1uLL << 32) - 1);
}

// ===============
int32 CstJson::Array_GetInt(JsonValue &_array, JsonSizeType index, idList<idStr> &errorMsg, int32 min, int32 max) {
	assert(_array.IsArray());
	if (!_array[index].IsInt()) {
		errorMsg.Append("Expected an int.");
		throw Except();
	}
	int32 value = _array[index].GetInt();
	if (value < min) {
		idStr msg; msg.Format("Minimum is %d.", min); errorMsg.Append(msg);
		throw Except();
	}
	if (value > max) {
		idStr msg; msg.Format("Maximum is %d.", max); errorMsg.Append(msg);
		throw Except();
	}
	return value;
}

// ===============
int8 CstJson::Array_GetInt8(JsonValue &_array, JsonSizeType index, idList<idStr> &errorMsg) {
	// may throw CstJson::Except
	return Array_GetInt(_array, index, errorMsg, -(1LL << 7), (1LL << 7) - 1);
}

// ===============
int16 CstJson::Array_GetInt16(JsonValue &_array, JsonSizeType index, idList<idStr> &errorMsg) {
	// may throw CstJson::Except
	return Array_GetInt(_array, index, errorMsg, -(1LL << 15), (1LL << 15) - 1);
}

// ===============
int32 CstJson::Array_GetInt32(JsonValue &_array, JsonSizeType index, idList<idStr> &errorMsg) {
	// may throw CstJson::Except
	return Array_GetInt(_array, index, errorMsg, -(1LL << 31), (1LL << 31) - 1);
}

// ===============
bool CstJson::Array_GetBool(JsonValue &_array, JsonSizeType index, idList<idStr> &errorMsg) {
	assert(_array.IsArray());
	if (!_array[index].IsBool()) {
		errorMsg.Append("Expected a bool.");
		throw Except();
	}
	return _array[index].GetBool();
}

// ===============
float CstJson::Array_GetFloat(JsonValue &_array, JsonSizeType index, idList<idStr> &errorMsg) {
	assert(_array.IsArray());
	if (!_array[index].IsFloat()) {
		errorMsg.Append("Expected a float.");
		throw Except();
	}
	return _array[index].GetFloat();
}

// ===============
double CstJson::Array_GetDouble(JsonValue &_array, JsonSizeType index, idList<idStr> &errorMsg) {
	assert(_array.IsArray());
	if (!_array[index].IsDouble()) {
		errorMsg.Append("Expected a double.");
		throw Except();
	}
	return _array[index].GetDouble();
}

// ===============
const char * CstJson::Array_GetString(JsonValue &_array, JsonSizeType index, idList<idStr> &errorMsg) {
	assert(_array.IsArray());
	if (!_array[index].IsString()) {
		errorMsg.Append("Expected a string.");
		throw Except();
	}
	return _array[index].GetString();
}
//#modified-fva; END
