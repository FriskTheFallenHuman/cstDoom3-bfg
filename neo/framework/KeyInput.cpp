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

typedef struct {
	keyNum_t		keynum;
	const char *	name;
	const char *	strId;	// localized string id
} keyname_t;

#define NAMEKEY( code, strId ) { K_##code, #code, strId }
#define NAMEKEY2( code ) { K_##code, #code, #code }

#define ALIASKEY( alias, code ) { K_##code, alias, "" }

// names not in this list can either be lowercase ascii, or '0xnn' hex sequences
keyname_t keynames[] =
{
	NAMEKEY( ESCAPE, "#str_07020" ),
	NAMEKEY2( 1 ),
	NAMEKEY2( 2 ),
	NAMEKEY2( 3 ),
	NAMEKEY2( 4 ),
	NAMEKEY2( 5 ),
	NAMEKEY2( 6 ),
	NAMEKEY2( 7 ),
	NAMEKEY2( 8 ),
	NAMEKEY2( 9 ),
	NAMEKEY2( 0 ),
	NAMEKEY( MINUS, "-" ),
	NAMEKEY( EQUALS, "=" ),
	NAMEKEY( BACKSPACE, "#str_07022" ),
	NAMEKEY( TAB, "#str_07018" ),
	NAMEKEY2( Q ),
	NAMEKEY2( W ),
	NAMEKEY2( E ),
	NAMEKEY2( R ),
	NAMEKEY2( T ),
	NAMEKEY2( Y ),
	NAMEKEY2( U ),
	NAMEKEY2( I ),
	NAMEKEY2( O ),
	NAMEKEY2( P ),
	NAMEKEY( LBRACKET, "[" ),
	NAMEKEY( RBRACKET, "]" ),
	NAMEKEY( ENTER, "#str_07019" ),
	NAMEKEY( LCTRL, "#str_07028" ),
	NAMEKEY2( A ),
	NAMEKEY2( S ),
	NAMEKEY2( D ),
	NAMEKEY2( F ),
	NAMEKEY2( G ),
	NAMEKEY2( H ),
	NAMEKEY2( J ),
	NAMEKEY2( K ),
	NAMEKEY2( L ),
	NAMEKEY( SEMICOLON, "#str_07129" ),
	NAMEKEY( APOSTROPHE, "#str_07130" ),
	NAMEKEY( GRAVE, "`" ),
	NAMEKEY( LSHIFT, "#str_07029" ),
	NAMEKEY( BACKSLASH, "\\" ),
	NAMEKEY2( Z ),
	NAMEKEY2( X ),
	NAMEKEY2( C ),
	NAMEKEY2( V ),
	NAMEKEY2( B ),
	NAMEKEY2( N ),
	NAMEKEY2( M ),
	NAMEKEY( COMMA, "," ),
	NAMEKEY( PERIOD, "." ),
	NAMEKEY( SLASH, "/" ),
	NAMEKEY( RSHIFT, "#str_bind_RSHIFT" ),
	NAMEKEY( KP_STAR, "#str_07126" ),
	NAMEKEY( LALT, "#str_07027" ),
	NAMEKEY( SPACE, "#str_07021" ),
	NAMEKEY( CAPSLOCK, "#str_07034" ),
	NAMEKEY( F1, "#str_07036" ),
	NAMEKEY( F2, "#str_07037" ),
	NAMEKEY( F3, "#str_07038" ),
	NAMEKEY( F4, "#str_07039" ),
	NAMEKEY( F5, "#str_07040" ),
	NAMEKEY( F6, "#str_07041" ),
	NAMEKEY( F7, "#str_07042" ),
	NAMEKEY( F8, "#str_07043" ),
	NAMEKEY( F9, "#str_07044" ),
	NAMEKEY( F10, "#str_07045" ),
	NAMEKEY( NUMLOCK, "#str_07125" ),
	NAMEKEY( SCROLL, "#str_07035" ),
	NAMEKEY( KP_7, "#str_07110" ),
	NAMEKEY( KP_8, "#str_07111" ),
	NAMEKEY( KP_9, "#str_07112" ),
	NAMEKEY( KP_MINUS, "#str_07123" ),
	NAMEKEY( KP_4, "#str_07113" ),
	NAMEKEY( KP_5, "#str_07114" ),
	NAMEKEY( KP_6, "#str_07115" ),
	NAMEKEY( KP_PLUS, "#str_07124" ),
	NAMEKEY( KP_1, "#str_07116" ),
	NAMEKEY( KP_2, "#str_07117" ),
	NAMEKEY( KP_3, "#str_07118" ),
	NAMEKEY( KP_0, "#str_07120" ),
	NAMEKEY( KP_DOT, "#str_07121" ),
	NAMEKEY( F11, "#str_07046" ),
	NAMEKEY( F12, "#str_07047" ),
	NAMEKEY2( F13 ),
	NAMEKEY2( F14 ),
	NAMEKEY2( F15 ),
	NAMEKEY2( KANA ),
	NAMEKEY2( CONVERT ),
	NAMEKEY2( NOCONVERT ),
	NAMEKEY2( YEN ),
	NAMEKEY( KP_EQUALS, "#str_07127" ),
	NAMEKEY2( CIRCUMFLEX ),
	NAMEKEY( AT, "@" ),
	NAMEKEY( COLON, ":" ),
	NAMEKEY( UNDERLINE, "_" ),
	NAMEKEY2( KANJI ),
	NAMEKEY2( STOP ),
	NAMEKEY2( AX ),
	NAMEKEY2( UNLABELED ),
	NAMEKEY( KP_ENTER, "#str_07119" ),
	NAMEKEY( RCTRL, "#str_bind_RCTRL" ),
	NAMEKEY( KP_COMMA, "," ),
	NAMEKEY( KP_SLASH, "#str_07122" ),
	NAMEKEY( PRINTSCREEN, "#str_07179" ),
	NAMEKEY( RALT, "#str_bind_RALT" ),
	NAMEKEY( PAUSE, "#str_07128" ),
	NAMEKEY( HOME, "#str_07052" ),
	NAMEKEY( UPARROW, "#str_07023" ),
	NAMEKEY( PGUP, "#str_07051" ),
	NAMEKEY( LEFTARROW, "#str_07025" ),
	NAMEKEY( RIGHTARROW, "#str_07026" ),
	NAMEKEY( END, "#str_07053" ),
	NAMEKEY( DOWNARROW, "#str_07024" ),
	NAMEKEY( PGDN, "#str_07050" ),
	NAMEKEY( INS, "#str_07048" ),
	NAMEKEY( DEL, "#str_07049" ),
	NAMEKEY( LWIN, "#str_07030" ),
	NAMEKEY( RWIN, "#str_07031" ),
	NAMEKEY( APPS, "#str_07032" ),
	NAMEKEY2( POWER ),
	NAMEKEY2( SLEEP ),

	// --

	NAMEKEY( MOUSE1, "#str_07054" ),
	NAMEKEY( MOUSE2, "#str_07055" ),
	NAMEKEY( MOUSE3, "#str_07056" ),
	NAMEKEY( MOUSE4, "#str_07057" ),
	NAMEKEY( MOUSE5, "#str_07058" ),
	NAMEKEY( MOUSE6, "#str_07059" ),
	NAMEKEY( MOUSE7, "#str_07060" ),
	NAMEKEY( MOUSE8, "#str_07061" ),

	NAMEKEY( MWHEELDOWN, "#str_07132" ),
	NAMEKEY( MWHEELUP, "#str_07131" ),

	NAMEKEY( JOY1, "#str_07062" ),
	NAMEKEY( JOY2, "#str_07063" ),
	NAMEKEY( JOY3, "#str_07064" ),
	NAMEKEY( JOY4, "#str_07065" ),
	NAMEKEY( JOY5, "#str_07066" ),
	NAMEKEY( JOY6, "#str_07067" ),
	NAMEKEY( JOY7, "#str_07068" ),
	NAMEKEY( JOY8, "#str_07069" ),
	NAMEKEY( JOY9, "#str_07070" ),
	NAMEKEY( JOY10, "#str_07071" ),
	NAMEKEY( JOY11, "#str_07072" ),
	NAMEKEY( JOY12, "#str_07073" ),
	NAMEKEY( JOY13, "#str_07074" ),
	NAMEKEY( JOY14, "#str_07075" ),
	NAMEKEY( JOY15, "#str_07076" ),
	NAMEKEY( JOY16, "#str_07077" ),

	NAMEKEY2( JOY_DPAD_UP ),
	NAMEKEY2( JOY_DPAD_DOWN ),
	NAMEKEY2( JOY_DPAD_LEFT ),
	NAMEKEY2( JOY_DPAD_RIGHT ),

	NAMEKEY2( JOY_STICK1_UP ),
	NAMEKEY2( JOY_STICK1_DOWN ),
	NAMEKEY2( JOY_STICK1_LEFT ),
	NAMEKEY2( JOY_STICK1_RIGHT ),

	NAMEKEY2( JOY_STICK2_UP ),
	NAMEKEY2( JOY_STICK2_DOWN ),
	NAMEKEY2( JOY_STICK2_LEFT ),
	NAMEKEY2( JOY_STICK2_RIGHT ),

	NAMEKEY2( JOY_TRIGGER1 ),
	NAMEKEY2( JOY_TRIGGER2 ),

	//------------------------
	// Aliases to make it easier to bind or to support old configs
	//------------------------
	ALIASKEY( "ALT", LALT ),
	ALIASKEY( "RIGHTALT", RALT ),
	ALIASKEY( "CTRL", LCTRL ),
	ALIASKEY( "SHIFT", LSHIFT ),
	ALIASKEY( "MENU", APPS ),
	ALIASKEY( "COMMAND", LALT ),

	ALIASKEY( "KP_HOME", KP_7 ),
	ALIASKEY( "KP_UPARROW", KP_8 ),
	ALIASKEY( "KP_PGUP", KP_9 ),
	ALIASKEY( "KP_LEFTARROW", KP_4 ),
	ALIASKEY( "KP_RIGHTARROW", KP_6 ),
	ALIASKEY( "KP_END", KP_1 ),
	ALIASKEY( "KP_DOWNARROW", KP_2 ),
	ALIASKEY( "KP_PGDN", KP_3 ),
	ALIASKEY( "KP_INS", KP_0 ),
	ALIASKEY( "KP_DEL", KP_DOT ),
	ALIASKEY( "KP_NUMLOCK", NUMLOCK ),

	ALIASKEY( "-", MINUS ),
	ALIASKEY( "=", EQUALS ),
	ALIASKEY( "[", LBRACKET ),
	ALIASKEY( "]", RBRACKET ),
	ALIASKEY( "\\", BACKSLASH ),
	ALIASKEY( "/", SLASH ),
	ALIASKEY( ",", COMMA ),
	ALIASKEY( ".", PERIOD ),

	{K_NONE, NULL, NULL}
};

//#modified-fva; BEGIN
/*
class idKey {
public:
					idKey() { down = false; repeats = 0; usercmdAction = 0; }
	bool			down;
	int				repeats;		// if > 1, it is autorepeating
	idStr			binding;
	int				usercmdAction;	// for testing by the asyncronous usercmd generation
};

bool		key_overstrikeMode = false;
idKey *		keys = NULL;
*/

struct CstDomain {
	CstDomain() { usercmdAction = 0; }
	idStr	binding;
	int		usercmdAction;
};

struct CstLayer {
	CstDomain	domain[CST_NUM_DOMAINS];
};

struct CstKey {
	CstKey() { down = false; currentLayer = 0; currentDomain = 0; }
	bool		down;
	int			currentLayer;
	int			currentDomain;
	CstLayer	layer[CST_NUM_LAYERS];
};

static bool key_overstrikeMode = false;
static int cstCurrentDomain = 0;
static CstKey *cstKeys;

// ===============
int idKeyInput::CstGetKeyLayer(int keyNum) {
	return cstKeys[keyNum].currentLayer;
}

// ===============
void idKeyInput::CstSetKeyLayer(int keyNum, int layer) {
	if (layer < 0 || layer >= CST_NUM_LAYERS) {
		return;
	}
	cstKeys[keyNum].currentLayer = layer;
}

// ===============
int idKeyInput::CstGetKeyDomain(int keyNum) {
	return cstKeys[keyNum].currentDomain;
}

// ===============
void idKeyInput::CstSetKeyDomain(int keyNum, int domain) {
	if (domain < 0 || domain >= CST_NUM_DOMAINS) {
		return;
	}
	cstKeys[keyNum].currentDomain = domain;
}

// ===============
int idKeyInput::CstGetCurrentDomain() {
	return cstCurrentDomain;
}

// ===============
void idKeyInput::CstSetCurrentDomain(int domain) {
	if (domain < 0 || domain >= CST_NUM_DOMAINS) {
		return;
	}
	cstCurrentDomain = domain;
}
//#modified-fva; END


/*
===================
idKeyInput::ArgCompletion_KeyName
===================
*/
void idKeyInput::ArgCompletion_KeyName( const idCmdArgs &args, void(*callback)( const char *s ) ) {
	for ( keyname_t * kn = keynames; kn->name; kn++ ) {
		callback( va( "%s %s", args.Argv( 0 ), kn->name ) );
	}
}

/*
===================
idKeyInput::GetOverstrikeMode
===================
*/
bool idKeyInput::GetOverstrikeMode() {
	return key_overstrikeMode;
}

/*
===================
idKeyInput::SetOverstrikeMode
===================
*/
void idKeyInput::SetOverstrikeMode( bool state ) {
	key_overstrikeMode = state;
}

/*
===================
idKeyInput::IsDown
===================
*/
bool idKeyInput::IsDown( int keynum ) {
	if ( keynum == -1 ) {
		return false;
	}

	//#modified-fva; BEGIN
	//return keys[keynum].down;
	return cstKeys[keynum].down;
	//#modified-fva; END
}

/*
========================
idKeyInput::StringToKeyNum
========================
*/
keyNum_t idKeyInput::StringToKeyNum( const char * str ) {

	if ( !str || !str[0] ) {
		return K_NONE;
	}

	// scan for a text match
	for ( keyname_t * kn = keynames; kn->name; kn++ ) {
		if ( !idStr::Icmp( str, kn->name ) ) {
			return kn->keynum;
		}
	}

	return K_NONE;
}

/*
========================
idKeyInput::KeyNumToString
========================
*/
const char * idKeyInput::KeyNumToString( keyNum_t keynum ) {
	// check for a key string
	for ( keyname_t * kn = keynames; kn->name; kn++ ) {
		if ( keynum == kn->keynum ) {
			return kn->name;
		}
	}
	return "?";
}


/*
========================
idKeyInput::LocalizedKeyName
========================
*/
const char * idKeyInput::LocalizedKeyName( keyNum_t keynum ) {
	if ( keynum < K_JOY1 ) {
		// On the PC, we want to turn the scan code in to a key label that matches the currently selected keyboard layout
		unsigned char keystate[256] = { 0 };
		WCHAR temp[5];

		int scancode = (int)keynum;
		int vkey = MapVirtualKey( keynum, MAPVK_VSC_TO_VK_EX );
		int result = -1;
		while ( result < 0 ) {
			result = ToUnicode( vkey, scancode, keystate, temp, sizeof( temp ) / sizeof( temp[0] ), 0 );
		}
		if ( result > 0 && temp[0] > ' ' && iswprint( temp[0] ) ) {
			static idStr bindStr;
			bindStr.Empty();
			bindStr.AppendUTF8Char( temp[0] );
			return bindStr;
		}
	}

	// check for a key string
	for ( keyname_t * kn = keynames; kn->name; kn++ ) {
		if ( keynum == kn->keynum ) {
			return idLocalization::GetString( kn->strId );
		}
	}
	return "????";
}

/*
===================
idKeyInput::SetBinding
===================
*/
//#modified-fva; BEGIN
/*
void idKeyInput::SetBinding( int keynum, const char *binding ) {
	if ( keynum == -1 ) {
		return;
	}

	// Clear out all button states so we aren't stuck forever thinking this key is held down
	usercmdGen->Clear();

	// allocate memory for new binding
	keys[keynum].binding = binding;

	// find the action for the async command generation
	keys[keynum].usercmdAction = usercmdGen->CommandStringUsercmdData( binding );

	// consider this like modifying an archived cvar, so the
	// file write will be triggered at the next oportunity
	cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
}
*/
void idKeyInput::SetBinding(int keynum, const char *binding, int cstLayer, int cstDomain) {
	if (keynum < 0 || keynum >= K_LAST_KEY || cstLayer < 0 || cstLayer >= CST_NUM_LAYERS || cstDomain < 0 || cstDomain >= CST_NUM_DOMAINS) {
		return;
	}

	// Clear out all button states so we aren't stuck forever thinking this key is held down
	usercmdGen->Clear();

	// allocate memory for new binding
	cstKeys[keynum].layer[cstLayer].domain[cstDomain].binding = binding;

	// find the action for the async command generation
	cstKeys[keynum].layer[cstLayer].domain[cstDomain].usercmdAction = usercmdGen->CommandStringUsercmdData(binding);

	// consider this like modifying an archived cvar, so the
	// file write will be triggered at the next oportunity
	cvarSystem->SetModifiedFlags(CVAR_ARCHIVE);
}
//#modified-fva; END


/*
===================
idKeyInput::GetBinding
===================
*/
//#modified-fva; BEGIN
/*
const char *idKeyInput::GetBinding( int keynum ) {
	if ( keynum == -1 ) {
		return "";
	}

	return keys[ keynum ].binding;
}
*/
const char *idKeyInput::GetBinding(int keynum, int cstLayer, int cstDomain) {
	if (keynum < 0 || keynum >= K_LAST_KEY || cstLayer < 0 || cstLayer >= CST_NUM_LAYERS || cstDomain < 0 || cstDomain >= CST_NUM_DOMAINS) {
		return "";
	}
	return cstKeys[keynum].layer[cstLayer].domain[cstDomain].binding.c_str();
}
//#modified-fva; END

/*
===================
idKeyInput::GetUsercmdAction
===================
*/
//#modified-fva; BEGIN
/*
int idKeyInput::GetUsercmdAction( int keynum ) {
	return keys[ keynum ].usercmdAction;
}
*/
int idKeyInput::GetUsercmdAction(int keynum, int cstLayer, int cstDomain) {
	return cstKeys[keynum].layer[cstLayer].domain[cstDomain].usercmdAction;
}
//#modified-fva; END

/*
===================
Key_Unbind_f
===================
*/
void Key_Unbind_f( const idCmdArgs &args ) {
	int		b;

	if ( args.Argc() != 2 ) {
		//#modified-fva; BEGIN
		//common->Printf( "unbind <key> : remove commands from a key\n" );
		common->Printf("unbind <key> : remove commands from a key - domain 0, layer 0\n");
		//#modified-fva; END
		return;
	}

	b = idKeyInput::StringToKeyNum( args.Argv(1) );
	if ( b == -1 ) {
		// If it wasn't a key, it could be a command
		//#modified-fva; BEGIN
		//if ( !idKeyInput::UnbindBinding( args.Argv(1) ) ) {
		if (!idKeyInput::UnbindBinding(args.Argv(1), 0, 0)) {
		//#modified-fva; END
			common->Printf( "\"%s\" isn't a valid key\n", args.Argv(1) );
		}
	} else {
		//#modified-fva; BEGIN
		//idKeyInput::SetBinding( b, "" );
		idKeyInput::SetBinding(b, "", 0, 0);
		//#modified-fva; END
	}
}

/*
===================
Key_Unbindall_f
===================
*/
//#modified-fva; BEGIN
/*
void Key_Unbindall_f( const idCmdArgs &args ) {
	for ( int i = 0; i < K_LAST_KEY; i++ ) {
		idKeyInput::SetBinding( i, "" );
	}
}
*/
void Key_Unbindall_f(const idCmdArgs &args) {
	for (int keyNum = 0; keyNum < K_LAST_KEY; keyNum++) {
		for (int layer = 0; layer < CST_NUM_LAYERS; layer++) {
			for (int domain = 0; domain < CST_NUM_DOMAINS; domain++) {
				idKeyInput::SetBinding(keyNum, "", layer, domain);
			}
		}
	}
	cvarSystem->SetCVarString("cst_keyPdaScoreClose", "");
}
//#modified-fva; END

/*
===================
Key_Bind_f
===================
*/
void Key_Bind_f( const idCmdArgs &args ) {
	int			i, c, b;
	char		cmd[MAX_STRING_CHARS];

	c = args.Argc();

	if ( c < 2 ) {
		//#modified-fva; BEGIN
		//common->Printf( "bind <key> [command] : attach a command to a key\n" );
		common->Printf("bind <key> [command] : attach a command to a key - domain 0, layer 0\n");
		//#modified-fva; END
		return;
	}
	b = idKeyInput::StringToKeyNum( args.Argv(1) );
	if ( b == -1 ) {
		common->Printf( "\"%s\" isn't a valid key\n", args.Argv(1) );
		return;
	}

	if ( c == 2 ) {
		//#modified-fva; BEGIN
		/*
		if ( keys[b].binding.Length() ) {
			common->Printf( "\"%s\" = \"%s\"\n", args.Argv(1), keys[b].binding.c_str() );
		}
		*/
		if (cstKeys[b].layer[0].domain[0].binding.Length()) {
			common->Printf("\"%s\" = \"%s\"\n", args.Argv(1), cstKeys[b].layer[0].domain[0].binding.c_str());
		}
		//#modified-fva; END
		else {
			common->Printf( "\"%s\" is not bound\n", args.Argv(1) );
		}
		return;
	}

	// copy the rest of the command line
	cmd[0] = 0;		// start out with a null string
	for ( i = 2; i < c; i++ ) {
		strcat( cmd, args.Argv( i ) );
		if ( i != (c-1) ) {
			strcat( cmd, " " );
		}
	}

	//#modified-fva; BEGIN
	//idKeyInput::SetBinding( b, cmd );
	idKeyInput::SetBinding(b, cmd, 0, 0);
	//#modified-fva; END
}

/*
============
Key_BindUnBindTwo_f

binds keynum to bindcommand and unbinds if there are already two binds on the key
============
*/
void Key_BindUnBindTwo_f( const idCmdArgs &args ) {
	int c = args.Argc();
	if ( c < 3 ) {
		common->Printf( "bindunbindtwo <keynum> [command]\n" );
		return;
	}
	int key = atoi( args.Argv( 1 ) );
	idStr bind = args.Argv( 2 );
	//#modified-fva; BEGIN
	//if ( idKeyInput::NumBinds( bind ) >= 2 && !idKeyInput::KeyIsBoundTo( key, bind ) ) {
	if (idKeyInput::NumBinds(bind, 0, 0) >= 2 && !idKeyInput::KeyIsBoundTo(key, bind, 0, 0)) {
	//#modified-fva; END
		//#modified-fva; BEGIN
		//idKeyInput::UnbindBinding( bind );
		idKeyInput::UnbindBinding(bind, 0, 0);
		//#modified-fva; END
	}
	//#modified-fva; BEGIN
	//idKeyInput::SetBinding( key, bind );
	idKeyInput::SetBinding(key, bind, 0, 0);
	//#modified-fva; END
}



/*
============
idKeyInput::WriteBindings

Writes lines containing "bind key value"
============
*/
//#modified-fva; BEGIN
/*
void idKeyInput::WriteBindings( idFile *f ) {
	f->Printf( "unbindall\n" );

	for ( int i = 0; i < K_LAST_KEY; i++ ) {
		if ( keys[i].binding.Length() ) {
			const char *name = KeyNumToString( (keyNum_t)i );
			f->Printf( "bind \"%s\" \"%s\"\n", name, keys[i].binding.c_str() );
		}
	}
}
*/
void idKeyInput::WriteBindings(idFile *f) {
	f->Printf("unbindall\n");

	for (int _domain = 0; _domain < CST_NUM_DOMAINS; _domain++) {
		for (int _layer = 0; _layer < CST_NUM_LAYERS; _layer++) {
			for (int _key = 0; _key < K_LAST_KEY; _key++) {
				if (cstKeys[_key].layer[_layer].domain[_domain].binding.Length()) {
					f->Printf("cstBind \"%d\" \"%d\" \"%s\" \"%s\"\n", _domain, _layer, KeyNumToString((keyNum_t)_key), cstKeys[_key].layer[_layer].domain[_domain].binding.c_str());
				}
			}
		}
	}
}
//#modified-fva; END

/*
============
Key_ListBinds_f
============
*/
void Key_ListBinds_f( const idCmdArgs &args ) {
	for ( int i = 0; i < K_LAST_KEY; i++ ) {
		//#modified-fva; BEGIN
		/*
		if ( keys[i].binding.Length() ) {
			common->Printf( "%s \"%s\"\n", idKeyInput::KeyNumToString( (keyNum_t)i ), keys[i].binding.c_str() );
		}
		*/
		if (cstKeys[i].layer[0].domain[0].binding.Length()) {
			common->Printf("%s \"%s\"\n", idKeyInput::KeyNumToString((keyNum_t)i), cstKeys[i].layer[0].domain[0].binding.c_str());
		}
		//#modified-fva; END
	}
}

/*
============
idKeyInput::KeysFromBinding
returns the localized name of the key for the binding
============
*/
//#modified-fva; BEGIN
//const char *idKeyInput::KeysFromBinding( const char *bind ) {
const char *idKeyInput::KeysFromBinding(const char *bind, int cstLayer, int cstDomain) {
//#modified-fva; END
	static char keyName[MAX_STRING_CHARS];
	keyName[0] = 0;

	//#modified-fva; BEGIN
	//if ( bind && *bind ) {
	if (bind && *bind && cstLayer >= 0 && cstLayer < CST_NUM_LAYERS && cstDomain >= 0 && cstDomain < CST_NUM_DOMAINS) {
	//#modified-fva; END
		for ( int i = 0; i < K_LAST_KEY; i++ ) {
			//#modified-fva; BEGIN
			//if ( keys[i].binding.Icmp( bind ) == 0 ) {
			if (cstKeys[i].layer[cstLayer].domain[cstDomain].binding.Icmp(bind) == 0) {
			//#modified-fva; END
				if ( keyName[0] != '\0' ) {
					idStr::Append( keyName, sizeof( keyName ), idLocalization::GetString( "#str_07183" ) );
				}
				idStr::Append( keyName, sizeof( keyName ), LocalizedKeyName( (keyNum_t)i ) );
			}
		}
	}
	if ( keyName[0] == '\0' ) {
		idStr::Copynz( keyName, idLocalization::GetString( "#str_07133" ), sizeof( keyName ) );
	}
	//#modified-fva; BEGIN
	//idStr::ToLower( keyName );
	idStr::CstToLowerUTF8_Partial(keyName);
	//#modified-fva; END
	return keyName;
}

/*
========================
idKeyInput::KeyBindingsFromBinding

return: bindings for keyboard mouse and gamepad
========================
*/
//#modified-fva; BEGIN
//keyBindings_t idKeyInput::KeyBindingsFromBinding( const char * bind, bool firstOnly, bool localized ) {
keyBindings_t idKeyInput::KeyBindingsFromBinding(const char *bind, int cstLayer, int cstDomain, bool firstOnly, bool localized) {
//#modified-fva; END
	idStr keyboard;
	idStr mouse;
	idStr gamepad;

	if ( bind && *bind ) {
		for ( int i = 0; i < K_LAST_KEY; i++ ) {
			//#modified-fva; BEGIN
			//if ( keys[i].binding.Icmp( bind ) == 0 ) {
			if (cstKeys[i].layer[cstLayer].domain[cstDomain].binding.Icmp(bind) == 0) {
			//#modified-fva; END
				if ( i >= K_JOY1 && i <= K_JOY_DPAD_RIGHT ) {
					const char * gamepadKey = "";
					if ( localized ) {
						gamepadKey = LocalizedKeyName( (keyNum_t)i );
					} else {
						gamepadKey = KeyNumToString( (keyNum_t)i );
					}
					if ( idStr::Icmp( gamepadKey, "" ) != 0 ) {
						if ( !gamepad.IsEmpty() ) {
							if ( firstOnly ) {
								continue;
							}
							gamepad.Append( ", " );
						}
						gamepad.Append( gamepadKey );
					}
				} else if ( i >= K_MOUSE1 && i <= K_MWHEELUP ) {
					const char * mouseKey = "";
					if ( localized ) {
						mouseKey = LocalizedKeyName( (keyNum_t)i );
					} else {
						mouseKey = KeyNumToString( (keyNum_t)i );
					}
					if ( idStr::Icmp( mouseKey, "" ) != 0 ) {
						if ( !mouse.IsEmpty() ) {
							if ( firstOnly ) {
								continue;
							}
							mouse.Append( ", " );
						}
						mouse.Append( mouseKey );
					}
				} else {
					const char * tmp = "";
					if ( localized ) {
						tmp = LocalizedKeyName( (keyNum_t)i );
					} else {
						tmp = KeyNumToString( (keyNum_t)i );
					}
					if ( idStr::Icmp( tmp, "" ) != 0 && idStr::Icmp( tmp, keyboard ) != 0 ) {
						if ( !keyboard.IsEmpty() ) {
							if ( firstOnly ) {
								continue;
							}
							keyboard.Append( ", " );
						}
						keyboard.Append( tmp );
					}
				}
			}
		}
	}

	keyBindings_t bindings;
	bindings.gamepad = gamepad;
	bindings.mouse = mouse;
	bindings.keyboard = keyboard;

	return bindings;
}

/*
============
idKeyInput::BindingFromKey
returns the binding for the localized name of the key
============
*/
//#modified-fva; BEGIN
/*
const char * idKeyInput::BindingFromKey( const char *key ) {
	const int keyNum = idKeyInput::StringToKeyNum( key );
	if ( keyNum < 0 || keyNum >= K_LAST_KEY ) {
		return NULL;
	}
	return keys[keyNum].binding.c_str();
}
*/
const char *idKeyInput::BindingFromKey(const char *key, int cstLayer, int cstDomain) {
	const int keyNum = idKeyInput::StringToKeyNum(key);
	if (keyNum < 0 || keyNum >= K_LAST_KEY || cstLayer < 0 || cstLayer >= CST_NUM_LAYERS || cstDomain < 0 || cstDomain >= CST_NUM_DOMAINS) {
		return NULL;
	}
	return cstKeys[keyNum].layer[cstLayer].domain[cstDomain].binding.c_str();
}
//#modified-fva; END

/*
============
idKeyInput::UnbindBinding
============
*/
//#modified-fva; BEGIN
/*
bool idKeyInput::UnbindBinding( const char *binding ) {
	bool unbound = false;
	if ( binding && *binding ) {
		for ( int i = 0; i < K_LAST_KEY; i++ ) {
			if ( keys[i].binding.Icmp( binding ) == 0 ) {
				SetBinding( i, "" );
				unbound = true;
			}
		}
	}
	return unbound;
}
*/
bool idKeyInput::UnbindBinding(const char *binding, int cstLayer, int cstDomain) {
	bool unbound = false;
	if (binding && *binding && cstLayer >= 0 && cstLayer < CST_NUM_LAYERS && cstDomain >= 0 && cstDomain < CST_NUM_DOMAINS) {
		for (int i = 0; i < K_LAST_KEY; i++) {
			if (cstKeys[i].layer[cstLayer].domain[cstDomain].binding.Icmp(binding) == 0) {
				SetBinding(i, "", cstLayer, cstDomain);
				unbound = true;
			}
		}
	}
	return unbound;
}
//#modified-fva; END

/*
============
idKeyInput::NumBinds
============
*/
//#modified-fva; BEGIN
/*
int idKeyInput::NumBinds( const char *binding ) {
	int count = 0;

	if ( binding && *binding ) {
		for ( int i = 0; i < K_LAST_KEY; i++ ) {
			if ( keys[i].binding.Icmp( binding ) == 0 ) {
				count++;
			}
		}
	}
	return count;
}
*/
int idKeyInput::NumBinds(const char *binding, int cstLayer, int cstDomain) {
	int count = 0;

	if (binding && *binding && cstLayer >= 0 && cstLayer < CST_NUM_LAYERS && cstDomain >= 0 && cstDomain < CST_NUM_DOMAINS) {
		for (int i = 0; i < K_LAST_KEY; i++) {
			if (cstKeys[i].layer[cstLayer].domain[cstDomain].binding.Icmp(binding) == 0) {
				count++;
			}
		}
	}
	return count;
}
//#modified-fva; END

/*
============
idKeyInput::KeyIsBountTo
============
*/
//#modified-fva; BEGIN
/*
bool idKeyInput::KeyIsBoundTo( int keynum, const char *binding ) {
	if ( keynum >= 0 && keynum < K_LAST_KEY ) {
		return ( keys[keynum].binding.Icmp( binding ) == 0 );
	}
	return false;
}
*/
bool idKeyInput::KeyIsBoundTo(int keynum, const char *binding, int cstLayer, int cstDomain) {
	if (keynum >= 0 && keynum < K_LAST_KEY && cstLayer >= 0 && cstLayer < CST_NUM_LAYERS && cstDomain >= 0 && cstDomain < CST_NUM_DOMAINS) {
		return (cstKeys[keynum].layer[cstLayer].domain[cstDomain].binding.Icmp(binding) == 0);
	}
	return false;
}
//#modified-fva; END

/*
===================
idKeyInput::PreliminaryKeyEvent

Tracks global key up/down state
Called by the system for both key up and key down events
===================
*/
void idKeyInput::PreliminaryKeyEvent( int keynum, bool down ) {
	//#modified-fva; BEGIN
	//keys[keynum].down = down;
	cstKeys[keynum].down = down;
	//#modified-fva; END
}

/*
=================
idKeyInput::ExecKeyBinding
=================
*/
bool idKeyInput::ExecKeyBinding( int keynum ) {
	// commands that are used by the async thread
	// don't add text
	//#modified-fva; BEGIN
	//if ( keys[keynum].usercmdAction ) {
	if (cstKeys[keynum].layer[0].domain[0].usercmdAction) {
	//#modified-fva; END
		return false;
	}

	// send the bound action
	//#modified-fva; BEGIN
	/*
	if ( keys[keynum].binding.Length() ) {
		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, keys[keynum].binding.c_str() );
		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "\n" );
	}
	*/
	if (cstKeys[keynum].layer[0].domain[0].binding.Length()) {
		cmdSystem->BufferCommandText(CMD_EXEC_APPEND, cstKeys[keynum].layer[0].domain[0].binding.c_str());
		cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "\n");
	}
	//#modified-fva; END
	return true;
}

//#modified-fva; BEGIN
void idKeyInput::CstExecKeyBinding(int keyNum, int _layer, int _domain) {
	// note: CstExecKeyBindings is called only on key downs
	if (cstKeys[keyNum].layer[_layer].domain[_domain].binding.Length()) {
		cmdSystem->BufferCommandText(CMD_EXEC_APPEND, cstKeys[keyNum].layer[_layer].domain[_domain].binding.c_str());
		cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "\n");
	}
}
//#modified-fva; END

/*
===================
idKeyInput::ClearStates
===================
*/
void idKeyInput::ClearStates() {
	for ( int i = 0; i < K_LAST_KEY; i++ ) {
		//#modified-fva; BEGIN
		/*
		if ( keys[i].down ) {
			PreliminaryKeyEvent( i, false );
		}
		keys[i].down = false;
		*/
		if (cstKeys[i].down) {
			PreliminaryKeyEvent(i, false);
		}
		cstKeys[i].down = false;
		cstKeys[i].currentLayer = 0;
		cstKeys[i].currentDomain = 0;
		//#modified-fva; END
	}

	//#modified-fva; BEGIN
	eventLoop->CstClearPushedEvents();
	//#modified-fva; END

	// clear the usercommand states
	usercmdGen->Clear();
}

//#modified-fva; BEGIN
static void Key_CstBind_f(const idCmdArgs &args) {
	// code below is adapted from Key_Bind_f

	int c = args.Argc();
	if (c < 4) {
		common->Printf("cstBind <domain> <layer> <key> [command] : binds a command to a domain of a layer of a key\n");
		return;
	}

	int cstDomain = atoi(args.Argv(1));
	if (cstDomain < 0 || cstDomain >= CST_NUM_DOMAINS || (cstDomain == 0 && strcmp(args.Argv(1), "0") != 0)) {
		common->Printf("\"%s\" isn't a valid domain\n", args.Argv(1));
		return;
	}

	int cstLayer = atoi(args.Argv(2));
	if (cstLayer < 0 || cstLayer >= CST_NUM_LAYERS || (cstLayer == 0 && strcmp(args.Argv(2), "0") != 0)) {
		common->Printf("\"%s\" isn't a valid layer\n", args.Argv(2));
		return;
	}

	int b = idKeyInput::StringToKeyNum(args.Argv(3));
	if (b == -1) {
		common->Printf("\"%s\" isn't a valid key\n", args.Argv(3));
		return;
	}

	if (c == 4) {
		if (cstKeys[b].layer[cstLayer].domain[cstDomain].binding.Length()) {
			common->Printf("\"[D%s][L%s]%s\" = \"%s\"\n", args.Argv(1), args.Argv(2), args.Argv(3), cstKeys[b].layer[cstLayer].domain[cstDomain].binding.c_str());
		} else {
			common->Printf("\"[D%s][L%s]%s\" is not bound\n", args.Argv(1), args.Argv(2), args.Argv(3));
		}
		return;
	}

	// copy the rest of the command line
	char cmd[MAX_STRING_CHARS];
	cmd[0] = 0; // start out with a null string
	for (int i = 4; i < c; i++) {
		strcat(cmd, args.Argv(i));
		if (i != (c - 1)) {
			strcat(cmd, " ");
		}
	}

	idKeyInput::SetBinding(b, cmd, cstLayer, cstDomain);
}

// =====
static void Key_CstBindUnbindTwo_f(const idCmdArgs &args) {
	// code below is adapted from Key_BindUnBindTwo_f

	int c = args.Argc();
	if (c < 5) {
		common->Printf("cstBindUnbindTwo <domain> <layer> <keynum> <command>\n");
		return;
	}

	int cstDomain = atoi(args.Argv(1));
	if (cstDomain < 0 || cstDomain >= CST_NUM_DOMAINS || (cstDomain == 0 && strcmp(args.Argv(1), "0") != 0)) {
		common->Printf("\"%s\" isn't a valid domain\n", args.Argv(1));
		return;
	}

	int cstLayer = atoi(args.Argv(2));
	if (cstLayer < 0 || cstLayer >= CST_NUM_LAYERS || (cstLayer == 0 && strcmp(args.Argv(2), "0") != 0)) {
		common->Printf("\"%s\" isn't a valid layer\n", args.Argv(2));
		return;
	}

	int key = atoi(args.Argv(3));
	idStr bind = args.Argv(4);
	if (idKeyInput::NumBinds(bind, cstLayer, cstDomain) >= 2 && !idKeyInput::KeyIsBoundTo(key, bind, cstLayer, cstDomain)) {
		idKeyInput::UnbindBinding(bind, cstLayer, cstDomain);
	}
	idKeyInput::SetBinding(key, bind, cstLayer, cstDomain);
}

// =====
static void Key_CstUnbind_f(const idCmdArgs &args) {
	// code below is adapted from Key_Unbind_f

	if (args.Argc() != 4) {
		common->Printf("cstUnbind <domain> <layer> <key> : unbinds any command from a domain of a layer of a key\n");
		return;
	}

	int cstDomain = atoi(args.Argv(1));
	if (cstDomain < 0 || cstDomain >= CST_NUM_DOMAINS || (cstDomain == 0 && strcmp(args.Argv(1), "0") != 0)) {
		common->Printf("\"%s\" isn't a valid domain\n", args.Argv(1));
		return;
	}

	int cstLayer = atoi(args.Argv(2));
	if (cstLayer < 0 || cstLayer >= CST_NUM_LAYERS || (cstLayer == 0 && strcmp(args.Argv(2), "0") != 0)) {
		common->Printf("\"%s\" isn't a valid layer\n", args.Argv(2));
		return;
	}

	int keyNum = idKeyInput::StringToKeyNum(args.Argv(3));
	if (keyNum == -1) {
		// If it wasn't a key, it could be a command
		if (!idKeyInput::UnbindBinding(args.Argv(3), cstLayer, cstDomain)) {
			common->Printf("\"%s\" isn't a valid key\n", args.Argv(3));
		}
	} else {
		idKeyInput::SetBinding(keyNum, "", cstLayer, cstDomain);
	}
}

// =====
static void Key_CstUnbindDomain_f(const idCmdArgs &args) {
	if (args.Argc() != 2) {
		common->Printf("cstUnbindDomain <domain> : unbinds any commands from a domain of all layers of all keys\n");
		return;
	}

	int cstDomain = atoi(args.Argv(1));
	if (cstDomain < 0 || cstDomain >= CST_NUM_DOMAINS || (cstDomain == 0 && strcmp(args.Argv(1), "0") != 0)) {
		common->Printf("\"%s\" isn't a valid domain\n", args.Argv(1));
		return;
	}

	for (int keyNum = 0; keyNum < K_LAST_KEY; keyNum++) {
		for (int cstLayer = 0; cstLayer < CST_NUM_LAYERS; cstLayer++) {
			idKeyInput::SetBinding(keyNum, "", cstLayer, cstDomain);
		}
	}

	if (cstDomain == 0) { // doom 3
		cvarSystem->SetCVarString("cst_keyPdaScoreClose", "");
	}
}

// =====
static void Key_CstListBinds_f(const idCmdArgs &args) {
	for (int domainNum = 0; domainNum < CST_NUM_DOMAINS; domainNum++) {
		for (int layerNum = 0; layerNum < CST_NUM_LAYERS; layerNum++) {
			for (int keyNum = 0; keyNum < K_LAST_KEY; keyNum++) {
				if (cstKeys[keyNum].layer[layerNum].domain[domainNum].binding.Length()) {
					const char *keyName = idKeyInput::KeyNumToString((keyNum_t)keyNum);
					common->Printf("%d %d %s \"%s\"\n", domainNum, layerNum, keyName, cstKeys[keyNum].layer[layerNum].domain[domainNum].binding.c_str());
				}
			}
		}
	}
}

// =====
static void Key_CstListBindsDomain_f(const idCmdArgs &args) {
	if (args.Argc() != 2) {
		common->Printf("cstListBindsDomain <domain> : lists commands bound to a domain of all layers of all keys\n");
		return;
	}

	int cstDomain = atoi(args.Argv(1));
	if (cstDomain < 0 || cstDomain >= CST_NUM_DOMAINS || (cstDomain == 0 && strcmp(args.Argv(1), "0") != 0)) {
		common->Printf("\"%s\" isn't a valid domain\n", args.Argv(1));
		return;
	}

	for (int layerNum = 0; layerNum < CST_NUM_LAYERS; layerNum++) {
		for (int keyNum = 0; keyNum < K_LAST_KEY; keyNum++) {
			if (cstKeys[keyNum].layer[layerNum].domain[cstDomain].binding.Length()) {
				const char *keyName = idKeyInput::KeyNumToString((keyNum_t)keyNum);
				common->Printf("%d %d %s \"%s\"\n", cstDomain, layerNum, keyName, cstKeys[keyNum].layer[layerNum].domain[cstDomain].binding.c_str());
			}
		}
	}
}
//#modified-fva; END

/*
===================
idKeyInput::Init
===================
*/
void idKeyInput::Init() {

	//#modified-fva; BEGIN
	//keys = new (TAG_SYSTEM) idKey[K_LAST_KEY];
	cstKeys = new (TAG_SYSTEM) CstKey[K_LAST_KEY];
	//#modified-fva; END

	// register our functions
	//#modified-fva; BEGIN
	/*
	cmdSystem->AddCommand( "bind", Key_Bind_f, CMD_FL_SYSTEM, "binds a command to a key", idKeyInput::ArgCompletion_KeyName );
	cmdSystem->AddCommand( "bindunbindtwo", Key_BindUnBindTwo_f, CMD_FL_SYSTEM, "binds a key but unbinds it first if there are more than two binds" );
	cmdSystem->AddCommand( "unbind", Key_Unbind_f, CMD_FL_SYSTEM, "unbinds any command from a key", idKeyInput::ArgCompletion_KeyName );
	cmdSystem->AddCommand( "unbindall", Key_Unbindall_f, CMD_FL_SYSTEM, "unbinds any commands from all keys" );
	cmdSystem->AddCommand( "listBinds", Key_ListBinds_f, CMD_FL_SYSTEM, "lists key bindings" );
	*/
	cmdSystem->AddCommand("bind", Key_Bind_f, CMD_FL_SYSTEM, "binds a command to a key - domain 0, layer 0", idKeyInput::ArgCompletion_KeyName);
	cmdSystem->AddCommand("bindunbindtwo", Key_BindUnBindTwo_f, CMD_FL_SYSTEM, "binds a key but unbinds it first if there are more than two binds - domain 0, layer 0");
	cmdSystem->AddCommand("unbind", Key_Unbind_f, CMD_FL_SYSTEM, "unbinds any command from a key - domain 0, layer 0", idKeyInput::ArgCompletion_KeyName);
	cmdSystem->AddCommand("unbindall", Key_Unbindall_f, CMD_FL_SYSTEM, "unbinds any commands from all domains of all layers of all keys");
	cmdSystem->AddCommand("listBinds", Key_ListBinds_f, CMD_FL_SYSTEM, "lists key bindings - domain 0, layer 0");
	//#modified-fva; END

	//#modified-fva; BEGIN
	cmdSystem->AddCommand("cstBind", Key_CstBind_f, CMD_FL_SYSTEM, "binds a command to a domain of a layer of a key");
	cmdSystem->AddCommand("cstBindUnbindTwo", Key_CstBindUnbindTwo_f, CMD_FL_SYSTEM, "similar to bindunbindtwo, but uses the specified domain/layer");
	cmdSystem->AddCommand("cstUnbind", Key_CstUnbind_f, CMD_FL_SYSTEM, "unbinds any command from a domain of a layer of a key");
	cmdSystem->AddCommand("cstUnbindDomain", Key_CstUnbindDomain_f, CMD_FL_SYSTEM, "unbinds any commands from a domain of all layers of all keys");
	cmdSystem->AddCommand("cstListBinds", Key_CstListBinds_f, CMD_FL_SYSTEM, "lists commands bound to all domains of all layers of all keys");
	cmdSystem->AddCommand("cstListBindsDomain", Key_CstListBindsDomain_f, CMD_FL_SYSTEM, "lists commands bound to a domain of all layers of all keys");
	//#modified-fva; END
}

/*
===================
idKeyInput::Shutdown
===================
*/
void idKeyInput::Shutdown() {
	//#modified-fva; BEGIN
	//delete [] keys;
	//keys = NULL;
	delete[] cstKeys;
	cstKeys = NULL;
	//#modified-fva; END
}


/*
========================
Key_CovertHIDCode
Converts from a USB HID code to a K_ code
========================
*/
int Key_CovertHIDCode( int hid ) {
	if ( hid >= 0 && hid <= 106 ) {
		int table[] = {
			K_NONE, K_NONE, K_NONE, K_NONE,
			K_A, K_B, K_C, K_D, K_E, K_F, K_G, K_H, K_I, K_J, K_K, K_L, K_M, K_N, K_O, K_P, K_Q, K_R, K_S, K_T, K_U, K_V, K_W, K_X, K_Y, K_Z,
			K_1, K_2, K_3, K_4, K_5, K_6, K_7, K_8, K_9, K_0,
			K_ENTER, K_ESCAPE, K_BACKSPACE, K_TAB, K_SPACE,
			K_MINUS, K_EQUALS, K_LBRACKET, K_RBRACKET, K_BACKSLASH, K_NONE, K_SEMICOLON, K_APOSTROPHE, K_GRAVE, K_COMMA, K_PERIOD, K_SLASH, K_CAPSLOCK,
			K_F1, K_F2, K_F3, K_F4, K_F5, K_F6, K_F7, K_F8, K_F9, K_F10, K_F11, K_F12,
			K_PRINTSCREEN, K_SCROLL, K_PAUSE, K_INS, K_HOME, K_PGUP, K_DEL, K_END, K_PGDN, K_RIGHTARROW, K_LEFTARROW, K_DOWNARROW, K_UPARROW,
			K_NUMLOCK, K_KP_SLASH, K_KP_STAR, K_KP_MINUS, K_KP_PLUS, K_KP_ENTER,
			K_KP_1, K_KP_2, K_KP_3, K_KP_4, K_KP_5, K_KP_6, K_KP_7, K_KP_8, K_KP_9, K_KP_0, K_KP_DOT,
			K_NONE, K_APPS, K_POWER, K_KP_EQUALS,
			K_F13, K_F14, K_F15
		};
		return table[hid];
	}
	if ( hid >= 224 && hid <= 231 ) {
		int table[] = {
			K_LCTRL, K_LSHIFT, K_LALT, K_LWIN,
			K_RCTRL, K_RSHIFT, K_RALT, K_RWIN
		};
		return table[hid - 224];
	}
	return K_NONE;
}
