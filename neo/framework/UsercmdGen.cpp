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

idCVar joy_mergedThreshold( "joy_mergedThreshold", "1", CVAR_BOOL | CVAR_ARCHIVE, "If the thresholds aren't merged, you drift more off center" );
idCVar joy_newCode( "joy_newCode", "1", CVAR_BOOL | CVAR_ARCHIVE, "Use the new codepath" );
idCVar joy_triggerThreshold( "joy_triggerThreshold", "0.05", CVAR_FLOAT | CVAR_ARCHIVE, "how far the joystick triggers have to be pressed before they register as down" );
idCVar joy_deadZone( "joy_deadZone", "0.2", CVAR_FLOAT | CVAR_ARCHIVE, "specifies how large the dead-zone is on the joystick" );
idCVar joy_range( "joy_range", "1.0", CVAR_FLOAT | CVAR_ARCHIVE, "allow full range to be mapped to a smaller offset" );
idCVar joy_gammaLook( "joy_gammaLook", "1", CVAR_INTEGER | CVAR_ARCHIVE, "use a log curve instead of a power curve for movement" );
idCVar joy_powerScale( "joy_powerScale", "2", CVAR_FLOAT | CVAR_ARCHIVE, "Raise joystick values to this power" );
idCVar joy_pitchSpeed( "joy_pitchSpeed", "100",	CVAR_ARCHIVE | CVAR_FLOAT, "pitch speed when pressing up or down on the joystick", 60, 600 );
idCVar joy_yawSpeed( "joy_yawSpeed", "240",	CVAR_ARCHIVE | CVAR_FLOAT, "pitch speed when pressing left or right on the joystick", 60, 600 );

// these were a bad idea!
idCVar joy_dampenLook( "joy_dampenLook", "1", CVAR_BOOL | CVAR_ARCHIVE, "Do not allow full acceleration on look" );
idCVar joy_deltaPerMSLook( "joy_deltaPerMSLook", "0.003", CVAR_FLOAT | CVAR_ARCHIVE, "Max amount to be added on look per MS" );

idCVar in_mouseSpeed( "in_mouseSpeed", "1",	CVAR_ARCHIVE | CVAR_FLOAT, "speed at which the mouse moves", 0.25f, 4.0f );
//#modified-fva; BEGIN
//idCVar in_alwaysRun( "in_alwaysRun", "1", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_BOOL, "always run (reverse _speed button) - only in MP" );
idCVar in_alwaysRun("in_alwaysRun", "0", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_BOOL, "always run in Doom 3");
idCVar cst_dcAlwaysRun("cst_dcAlwaysRun", "0", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_BOOL, "always run in Doom 1 & 2");
//#modified-fva; END

idCVar in_useJoystick( "in_useJoystick", "0", CVAR_ARCHIVE | CVAR_BOOL, "enables/disables the gamepad for PC use" );
idCVar in_joystickRumble( "in_joystickRumble", "1", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_BOOL, "enable joystick rumble" );
idCVar in_invertLook( "in_invertLook", "0", CVAR_ARCHIVE | CVAR_BOOL, "inverts the look controls so the forward looks up (flight controls) - the proper way to play games!" );
idCVar in_mouseInvertLook( "in_mouseInvertLook", "0", CVAR_ARCHIVE | CVAR_BOOL, "inverts the look controls so the forward looks up (flight controls) - the proper way to play games!" );

//#modified-fva; BEGIN
idCVar cst_joyLeftyFlip("cst_joyLeftyFlip", "0", CVAR_BOOL | CVAR_ARCHIVE, "");
idCVar cst_amInheritBinds("cst_amInheritBinds", "1", CVAR_BOOL | CVAR_ARCHIVE, "if 1, the automap inherits key bindings from the normal Doom 1 & 2 key bindings; if 0, the automap key bindings are independent");
//#modified-fva; END

/*
================
usercmd_t::ByteSwap
================
*/
void usercmd_t::ByteSwap() {
	angles[0] = LittleShort( angles[0] );
	angles[1] = LittleShort( angles[1] );
	angles[2] = LittleShort( angles[2] );
}

/*
================
usercmd_t::Serialize
================
*/
void usercmd_t::Serialize( idSerializer & ser, const usercmd_t & base ) {
	ser.SerializeDelta( buttons, base.buttons );
	ser.SerializeDelta( forwardmove, base.forwardmove );
	ser.SerializeDelta( rightmove, base.rightmove );
	ser.SerializeDelta( angles[0], base.angles[0] );
	ser.SerializeDelta( angles[1], base.angles[1] );
	ser.SerializeDelta( angles[2], base.angles[2] );
	ser.SerializeDelta( pos.x, base.pos.x );
	ser.SerializeDelta( pos.y, base.pos.y );
	ser.SerializeDelta( pos.z, base.pos.z );
	ser.SerializeDelta( clientGameMilliseconds, base.clientGameMilliseconds );
	ser.SerializeDelta( serverGameMilliseconds, base.serverGameMilliseconds );
	ser.SerializeDelta( fireCount, base.fireCount );
	ser.SerializeDelta( speedSquared, base.speedSquared );
	ser.SerializeDelta( impulse, base.impulse );
	ser.SerializeDelta( impulseSequence, base.impulseSequence );
	//#modified-fva; BEGIN
	ser.SerializeDelta(cstLayer, base.cstLayer);
	//#modified-fva; END
}

/*
================
usercmd_t::operator==
================
*/
bool usercmd_t::operator==( const usercmd_t &rhs ) const {
	return ( buttons == rhs.buttons &&
			forwardmove == rhs.forwardmove &&
			rightmove == rhs.rightmove &&
			angles[0] == rhs.angles[0] &&
			angles[1] == rhs.angles[1] &&
			angles[2] == rhs.angles[2] &&
			impulse == rhs.impulse &&
			impulseSequence == rhs.impulseSequence &&
			//#modified-fva; BEGIN
			cstLayer == rhs.cstLayer &&
			//#modified-fva; END
			mx == rhs.mx &&
			my == rhs.my &&
			fireCount == rhs.fireCount &&
			speedSquared == speedSquared );
}


const int KEY_MOVESPEED	= 127;

userCmdString_t	userCmdStrings[] = {
	{ "_moveUp",		UB_MOVEUP },
	{ "_moveDown",		UB_MOVEDOWN },
	{ "_left",			UB_LOOKLEFT },
	{ "_right",			UB_LOOKRIGHT },
	{ "_forward",		UB_MOVEFORWARD },
	{ "_back",			UB_MOVEBACK },
	{ "_lookUp",		UB_LOOKUP },
	{ "_lookDown",		UB_LOOKDOWN },
	{ "_moveLeft",		UB_MOVELEFT },
	{ "_moveRight",		UB_MOVERIGHT },

	{ "_attack",		UB_ATTACK },
	{ "_speed",			UB_SPEED },
	{ "_zoom",			UB_ZOOM },
	{ "_showScores",	UB_SHOWSCORES },
	{ "_use",			UB_USE },

	{ "_impulse0",		UB_IMPULSE0 },
	{ "_impulse1",		UB_IMPULSE1 },
	{ "_impulse2",		UB_IMPULSE2 },
	{ "_impulse3",		UB_IMPULSE3 },
	{ "_impulse4",		UB_IMPULSE4 },
	{ "_impulse5",		UB_IMPULSE5 },
	{ "_impulse6",		UB_IMPULSE6 },
	{ "_impulse7",		UB_IMPULSE7 },
	{ "_impulse8",		UB_IMPULSE8 },
	{ "_impulse9",		UB_IMPULSE9 },
	{ "_impulse10",		UB_IMPULSE10 },
	{ "_impulse11",		UB_IMPULSE11 },
	{ "_impulse12",		UB_IMPULSE12 },
	{ "_impulse13",		UB_IMPULSE13 },
	{ "_impulse14",		UB_IMPULSE14 },
	{ "_impulse15",		UB_IMPULSE15 },
	{ "_impulse16",		UB_IMPULSE16 },
	{ "_impulse17",		UB_IMPULSE17 },
	{ "_impulse18",		UB_IMPULSE18 },
	{ "_impulse19",		UB_IMPULSE19 },
	{ "_impulse20",		UB_IMPULSE20 },
	{ "_impulse21",		UB_IMPULSE21 },
	{ "_impulse22",		UB_IMPULSE22 },
	{ "_impulse23",		UB_IMPULSE23 },
	{ "_impulse24",		UB_IMPULSE24 },
	{ "_impulse25",		UB_IMPULSE25 },
	{ "_impulse26",		UB_IMPULSE26 },
	{ "_impulse27",		UB_IMPULSE27 },
	{ "_impulse28",		UB_IMPULSE28 },
	{ "_impulse29",		UB_IMPULSE29 },
	{ "_impulse30",		UB_IMPULSE30 },
	{ "_impulse31",		UB_IMPULSE31 },
	//#modified-fva; BEGIN
	{ "_impulse32",		UB_IMPULSE32 },
	{ "_impulse33",		UB_IMPULSE33 },
	{ "_impulse34",		UB_IMPULSE34 },
	{ "_impulse35",		UB_IMPULSE35 },
	{ "_impulse36",		UB_IMPULSE36 },
	{ "_impulse37",		UB_IMPULSE37 },
	{ "_impulse38",		UB_IMPULSE38 },
	{ "_impulse39",		UB_IMPULSE39 },
	{ "_impulse40",		UB_IMPULSE40 },
	{ "_impulse41",		UB_IMPULSE41 },
	{ "_impulse42",		UB_IMPULSE42 },
	{ "_impulse43",		UB_IMPULSE43 },
	{ "_impulse44",		UB_IMPULSE44 },
	{ "_impulse45",		UB_IMPULSE45 },
	{ "_impulse46",		UB_IMPULSE46 },
	{ "_impulse47",		UB_IMPULSE47 },
	{ "_impulse48",		UB_IMPULSE48 },
	{ "_impulse49",		UB_IMPULSE49 },
	{ "_impulse50",		UB_IMPULSE50 },
	{ "_impulse51",		UB_IMPULSE51 },
	{ "_impulse52",		UB_IMPULSE52 },
	{ "_impulse53",		UB_IMPULSE53 },
	{ "_impulse54",		UB_IMPULSE54 },
	{ "_impulse55",		UB_IMPULSE55 },
	{ "_impulse56",		UB_IMPULSE56 },
	{ "_impulse57",		UB_IMPULSE57 },
	{ "_impulse58",		UB_IMPULSE58 },
	{ "_impulse59",		UB_IMPULSE59 },
	{ "_impulse60",		UB_IMPULSE60 },
	{ "_impulse61",		UB_IMPULSE61 },
	{ "_impulse62",		UB_IMPULSE62 },
	{ "_impulse63",		UB_IMPULSE63 },

	{ "_cstSwitchLayer1", UB_CST_SWITCH_LAYER_1 },
	{ "_cstSwitchLayer2", UB_CST_SWITCH_LAYER_2 },
	{ "_cstSwitchLayer3", UB_CST_SWITCH_LAYER_3 },
	{ "_cstSwitchLayer4", UB_CST_SWITCH_LAYER_4 },
	{ "_cstSwitchLayer5", UB_CST_SWITCH_LAYER_5 },
	{ "_cstSwitchLayer6", UB_CST_SWITCH_LAYER_6 },
	{ "_cstSwitchLayer7", UB_CST_SWITCH_LAYER_7 },
	{ "_cstSwitchLayer8", UB_CST_SWITCH_LAYER_8 },
	{ "_cstSwitchLayer9", UB_CST_SWITCH_LAYER_9 },

	{ "_cstHoldLayer1", UB_CST_HOLD_LAYER_1 },
	{ "_cstHoldLayer2", UB_CST_HOLD_LAYER_2 },
	{ "_cstHoldLayer3", UB_CST_HOLD_LAYER_3 },
	{ "_cstHoldLayer4", UB_CST_HOLD_LAYER_4 },
	{ "_cstHoldLayer5", UB_CST_HOLD_LAYER_5 },
	{ "_cstHoldLayer6", UB_CST_HOLD_LAYER_6 },
	{ "_cstHoldLayer7", UB_CST_HOLD_LAYER_7 },
	{ "_cstHoldLayer8", UB_CST_HOLD_LAYER_8 },
	{ "_cstHoldLayer9", UB_CST_HOLD_LAYER_9 },

	{ "_cstCancelLayer", UB_CST_CANCEL_LAYER },

	{ "_cstLockForward", UB_CST_LOCK_FORWARD },

	{ "_cstToggleAutomap", UB_CST_TOGGLE_AUTOMAP },
	{ "_cstAutomapPanRight", UB_CST_AUTOMAP_PAN_RIGHT },
	{ "_cstAutomapPanLeft", UB_CST_AUTOMAP_PAN_LEFT },
	{ "_cstAutomapPanUp", UB_CST_AUTOMAP_PAN_UP },
	{ "_cstAutomapPanDown", UB_CST_AUTOMAP_PAN_DOWN },
	{ "_cstAutomapZoomOut", UB_CST_AUTOMAP_ZOOM_OUT },
	{ "_cstAutomapZoomIn", UB_CST_AUTOMAP_ZOOM_IN },
	{ "_cstAutomapGoBig", UB_CST_AUTOMAP_GO_BIG },
	{ "_cstAutomapFollow", UB_CST_AUTOMAP_FOLLOW },
	{ "_cstAutomapGrid", UB_CST_AUTOMAP_GRID },
	{ "_cstAutomapMark", UB_CST_AUTOMAP_MARK },
	{ "_cstAutomapClearMark", UB_CST_AUTOMAP_CLEAR_MARK },
	//#modified-fva; END

	{ NULL,				UB_NONE },
};

 class buttonState_t {
 public:
	int		on;
	bool	held;

			buttonState_t() { Clear(); };
	void	Clear();
	void	SetKeyState( int keystate, bool toggle );
};

/*
================
buttonState_t::Clear
================
*/
void buttonState_t::Clear() {
	held = false;
	on = 0;
}

/*
================
buttonState_t::SetKeyState
================
*/
void buttonState_t::SetKeyState( int keystate, bool toggle ) {
	if ( !toggle ) {
		held = false;
		on = keystate;
	} else if ( !keystate ) {
		held = false;
	} else if ( !held ) {
		held = true;
		on ^= 1;
	}
}


const int NUM_USER_COMMANDS = sizeof(userCmdStrings) / sizeof(userCmdString_t);

const int MAX_CHAT_BUFFER = 127;

class idUsercmdGenLocal : public idUsercmdGen {
public:
					idUsercmdGenLocal();

	void			Init();

	void			InitForNewMap();

	void			Shutdown();

	void			Clear();

	void			ClearAngles();

	void			InhibitUsercmd( inhibit_t subsystem, bool inhibit );

	int				CommandStringUsercmdData( const char *cmdString );

	void			BuildCurrentUsercmd( int deviceNum );

	usercmd_t		GetCurrentUsercmd() { return cmd; };

	void			MouseState( int *x, int *y, int *button, bool *down );

	int				ButtonState( int key );
	int				KeyState( int key );

private:
	void			MakeCurrent();
	void			InitCurrent();

	bool			Inhibited();
	void			AdjustAngles();
	void			KeyMove();
	void			CircleToSquare( float & axis_x, float & axis_y ) const;
	void			HandleJoystickAxis( int keyNum, float unclampedValue, float threshold, bool positive );
	void			JoystickMove();
	void			JoystickMove2();
	void			MouseMove();
	void			CmdButtons();

	void			AimAssist();

	void			Mouse();
	void			Keyboard();
	void			Joystick( int deviceNum );

	void			Key( int keyNum, bool down );
	//#modified-fva; BEGIN
	bool			CstGetMoveWithAutomapPan();
	void			CstPushAutomapEvent(int action, bool keyDown, int domain);
	//#modified-fva; END

	idVec3			viewangles;
	int				impulseSequence;
	int				impulse;
	//#modified-fva; BEGIN
	int				cstLayer;
	//#modified-fva; END

	buttonState_t	toggled_crouch;
	buttonState_t	toggled_run;
	buttonState_t	toggled_zoom;

	int				buttonState[UB_MAX_BUTTONS];
	bool			keyState[K_LAST_KEY];

	int				inhibitCommands;	// true when in console or menu locally

	bool			initialized;

	usercmd_t		cmd;		// the current cmd being built

	int				continuousMouseX, continuousMouseY;	// for gui event generatioin, never zerod
	int				mouseButton;						// for gui event generatioin
	bool			mouseDown;

	int				mouseDx, mouseDy;	// added to by mouse events
	float			joystickAxis[MAX_JOYSTICK_AXIS];	// set by joystick events

	//#modified-fva; BEGIN
	struct CstCmdAux {
		CstCmdAux() { Clear(); }
		void	Clear() { activeLayer = 0; isHoldingLayer = false; isForwardLocked = false; }
		int		activeLayer;
		bool	isHoldingLayer;
		bool	isForwardLocked;
	};
	CstCmdAux	cstCmdAux;
	//#modified-fva; END

	int				pollTime;
	int				lastPollTime;
	float			lastLookValuePitch;
	float			lastLookValueYaw;

	static idCVar	in_yawSpeed;
	static idCVar	in_pitchSpeed;
	static idCVar	in_angleSpeedKey;
	static idCVar	in_toggleRun;
	//#modified-fva; BEGIN
	static idCVar	cst_dcToggleRun;
	//#modified-fva; END
	static idCVar	in_toggleCrouch;
	static idCVar	in_toggleZoom;
	static idCVar	sensitivity;
	static idCVar	m_pitch;
	static idCVar	m_yaw;
	static idCVar	m_smooth;
	static idCVar	m_showMouseRate;
};

idCVar idUsercmdGenLocal::in_yawSpeed( "in_yawspeed", "140", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_FLOAT, "yaw change speed when holding down _left or _right button" );
idCVar idUsercmdGenLocal::in_pitchSpeed( "in_pitchspeed", "140", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_FLOAT, "pitch change speed when holding down look _lookUp or _lookDown button" );
idCVar idUsercmdGenLocal::in_angleSpeedKey( "in_anglespeedkey", "1.5", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_FLOAT, "angle change scale when holding down _speed button" );
//#modified-fva; BEGIN
//idCVar idUsercmdGenLocal::in_toggleRun( "in_toggleRun", "0", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_BOOL, "pressing _speed button toggles run on/off - only in MP" );
idCVar idUsercmdGenLocal::in_toggleRun("in_toggleRun", "0", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_BOOL, "pressing _speed toggles run on/off in Doom 3");
idCVar idUsercmdGenLocal::cst_dcToggleRun("cst_dcToggleRun", "0", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_BOOL, "pressing _speed toggles run on/off in Doom 1 & 2");
//#modified-fva; END
idCVar idUsercmdGenLocal::in_toggleCrouch( "in_toggleCrouch", "0", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_BOOL, "pressing _movedown button toggles player crouching/standing" );
idCVar idUsercmdGenLocal::in_toggleZoom( "in_toggleZoom", "0", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_BOOL, "pressing _zoom button toggles zoom on/off" );
idCVar idUsercmdGenLocal::sensitivity( "sensitivity", "5", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_FLOAT, "mouse view sensitivity" );
idCVar idUsercmdGenLocal::m_pitch( "m_pitch", "0.022", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_FLOAT, "mouse pitch scale" );
idCVar idUsercmdGenLocal::m_yaw( "m_yaw", "0.022", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_FLOAT, "mouse yaw scale" );
idCVar idUsercmdGenLocal::m_smooth( "m_smooth", "1", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_INTEGER, "number of samples blended for mouse viewing", 1, 8, idCmdSystem::ArgCompletion_Integer<1,8> );
idCVar idUsercmdGenLocal::m_showMouseRate( "m_showMouseRate", "0", CVAR_SYSTEM | CVAR_BOOL, "shows mouse movement" );

static idUsercmdGenLocal localUsercmdGen;
idUsercmdGen	*usercmdGen = &localUsercmdGen;

/*
================
idUsercmdGenLocal::idUsercmdGenLocal
================
*/
idUsercmdGenLocal::idUsercmdGenLocal() {
	initialized = false;

	pollTime = 0;
	lastPollTime = 0;
	lastLookValuePitch = 0.0f;
	lastLookValueYaw = 0.0f;

	impulseSequence = 0;
	impulse = 0;
	//#modified-fva; BEGIN
	cstLayer = 0;
	//#modified-fva; END

	toggled_crouch.Clear();
	toggled_run.Clear();
	toggled_zoom.Clear();
	toggled_run.on = false;

	ClearAngles();
	Clear();
}

/*
================
idUsercmdGenLocal::InhibitUsercmd
================
*/
void idUsercmdGenLocal::InhibitUsercmd( inhibit_t subsystem, bool inhibit ) {
	if ( inhibit ) {
		inhibitCommands |= 1 << subsystem;
	} else {
		inhibitCommands &= ( 0xffffffff ^ ( 1 << subsystem ) );
	}
}

/*
===============
idUsercmdGenLocal::ButtonState

Returns (the fraction of the frame) that the key was down
===============
*/
int	idUsercmdGenLocal::ButtonState( int key ) {
	if ( key<0 || key>=UB_MAX_BUTTONS ) {
		return -1;
	}
	return ( buttonState[key] > 0 ) ? 1 : 0;
}

/*
===============
idUsercmdGenLocal::KeyState

Returns (the fraction of the frame) that the key was down
bk20060111
===============
*/
int	idUsercmdGenLocal::KeyState( int key ) {
	if ( key<0 || key>=K_LAST_KEY ) {
		return -1;
	}
	return ( keyState[key] ) ? 1 : 0;
}


//=====================================================================

/*
================
idUsercmdGenLocal::Inhibited

is user cmd generation inhibited
================
*/
bool idUsercmdGenLocal::Inhibited() {
	return ( inhibitCommands != 0);
}

/*
================
idUsercmdGenLocal::AdjustAngles

Moves the local angle positions
================
*/
void idUsercmdGenLocal::AdjustAngles() {
	float speed = MS2SEC( 16 );

	//#modified-fva; BEGIN
	//if ( toggled_run.on || ( in_alwaysRun.GetBool() && common->IsMultiplayer() ) ) {
	bool cstAlwaysRun = common->GetCurrentGame() == DOOM3_BFG ? in_alwaysRun.GetBool() : cst_dcAlwaysRun.GetBool();
	if (toggled_run.on || cstAlwaysRun) {
	//#modified-fva; END
		speed *= in_angleSpeedKey.GetFloat();
	}

	viewangles[YAW] -= speed * in_yawSpeed.GetFloat() * ButtonState( UB_LOOKRIGHT );
	viewangles[YAW] += speed * in_yawSpeed.GetFloat() * ButtonState( UB_LOOKLEFT );

	viewangles[PITCH] -= speed * in_pitchSpeed.GetFloat() * ButtonState( UB_LOOKUP );
	viewangles[PITCH] += speed * in_pitchSpeed.GetFloat() * ButtonState( UB_LOOKDOWN );
}

/*
================
idUsercmdGenLocal::KeyMove

Sets the usercmd_t based on key states
================
*/
void idUsercmdGenLocal::KeyMove() {
	int forward = 0;
	int side = 0;

	//#modified-fva; BEGIN
	/*
	side += KEY_MOVESPEED * ButtonState( UB_MOVERIGHT );
	side -= KEY_MOVESPEED * ButtonState( UB_MOVELEFT );

	forward += KEY_MOVESPEED * ButtonState( UB_MOVEFORWARD );
	forward -= KEY_MOVESPEED * ButtonState( UB_MOVEBACK );
	*/

	side += KEY_MOVESPEED * ButtonState(UB_MOVERIGHT);
	side -= KEY_MOVESPEED * ButtonState(UB_MOVELEFT);

	int cstForwardState = cstCmdAux.isForwardLocked ? 1 : ButtonState(UB_MOVEFORWARD);
	forward += KEY_MOVESPEED * cstForwardState;
	forward -= KEY_MOVESPEED * ButtonState(UB_MOVEBACK);

	if (CstGetMoveWithAutomapPan()) {
		side += KEY_MOVESPEED * ButtonState(UB_CST_AUTOMAP_PAN_RIGHT);
		side -= KEY_MOVESPEED * ButtonState(UB_CST_AUTOMAP_PAN_LEFT);
		forward += KEY_MOVESPEED * ButtonState(UB_CST_AUTOMAP_PAN_UP);
		forward -= KEY_MOVESPEED * ButtonState(UB_CST_AUTOMAP_PAN_DOWN);
	}
	//#modified-fva; END

	cmd.forwardmove += idMath::ClampChar( forward );
	cmd.rightmove += idMath::ClampChar( side );
}

/*
=================
idUsercmdGenLocal::MouseMove
=================
*/
void idUsercmdGenLocal::MouseMove() {
	float		mx, my;
	static int	history[8][2];
	static int	historyCounter;
	int			i;

	history[historyCounter&7][0] = mouseDx;
	history[historyCounter&7][1] = mouseDy;

	// allow mouse movement to be smoothed together
	int smooth = m_smooth.GetInteger();
	if ( smooth < 1 ) {
		smooth = 1;
	}
	if ( smooth > 8 ) {
		smooth = 8;
	}
	mx = 0;
	my = 0;
	for ( i = 0 ; i < smooth ; i++ ) {
		mx += history[ ( historyCounter - i + 8 ) & 7 ][0];
		my += history[ ( historyCounter - i + 8 ) & 7 ][1];
	}
	mx /= smooth;
	my /= smooth;

	historyCounter++;

	if ( idMath::Fabs( mx ) > 1000 || idMath::Fabs( my ) > 1000 ) {
		Sys_DebugPrintf( "idUsercmdGenLocal::MouseMove: Ignoring ridiculous mouse delta.\n" );
		mx = my = 0;
	}

	mx *= sensitivity.GetFloat();
	my *= sensitivity.GetFloat();

	if ( m_showMouseRate.GetBool() ) {
		Sys_DebugPrintf( "[%3i %3i  = %5.1f %5.1f] ", mouseDx, mouseDy, mx, my );
	}

	mouseDx = 0;
	mouseDy = 0;

	viewangles[YAW] -= m_yaw.GetFloat() * mx * in_mouseSpeed.GetFloat();
	viewangles[PITCH] += m_pitch.GetFloat() * in_mouseSpeed.GetFloat() * ( in_mouseInvertLook.GetBool() ? -my : my );
}

/*
========================
idUsercmdGenLocal::CircleToSquare
========================
*/
void idUsercmdGenLocal::CircleToSquare( float & axis_x, float & axis_y ) const {
	// bring everything in the first quadrant
	bool flip_x = false;
	if ( axis_x < 0.0f ) {
		flip_x = true;
		axis_x *= -1.0f;
	}
	bool flip_y = false;
	if ( axis_y < 0.0f ) {
		flip_y = true;
		axis_y *= -1.0f;
	}

	// swap the two axes so we project against the vertical line X = 1
	bool swap = false;
	if ( axis_y > axis_x ) {
		float tmp = axis_x;
		axis_x = axis_y;
		axis_y = tmp;
		swap = true;
	}

	if ( axis_x < 0.001f ) {
		// on one of the axes where no correction is needed
		return;
	}

	// length (max 1.0f at the unit circle)
	float len = idMath::Sqrt( axis_x * axis_x + axis_y * axis_y );
	if ( len > 1.0f ) {
		len = 1.0f;
	}
	// thales
	float axis_y_us = axis_y / axis_x;

	// use a power curve to shift the correction to happen closer to the unit circle
	float correctionRatio = Square( len );
	axis_x += correctionRatio * ( len - axis_x );
	axis_y += correctionRatio * ( axis_y_us - axis_y );

	// go back through the symmetries
	if ( swap ) {
		float tmp = axis_x;
		axis_x = axis_y;
		axis_y = tmp;
	}
	if ( flip_x ) {
		axis_x *= -1.0f;
	}
	if ( flip_y ) {
		axis_y *= -1.0f;
	}
}

/*
========================
idUsercmdGenLocal::HandleJoystickAxis
========================
*/
void idUsercmdGenLocal::HandleJoystickAxis( int keyNum, float unclampedValue, float threshold, bool positive ) {
	if ( ( unclampedValue > 0.0f ) && !positive ) {
		return;
	}
	if ( ( unclampedValue < 0.0f ) && positive ) {
		return;
	}
	float value = 0.0f;
	bool pressed = false;
	if ( unclampedValue > threshold ) {
		value = idMath::Fabs( ( unclampedValue - threshold ) / ( 1.0f - threshold ) );
		pressed = true;
	} else if ( unclampedValue < -threshold ) {
		value = idMath::Fabs( ( unclampedValue + threshold ) / ( 1.0f - threshold ) );
		pressed = true;
	}

	//#modified-fva; BEGIN
	//int action = idKeyInput::GetUsercmdAction( keyNum );
	int action = UB_NONE;
	if (keyNum == K_JOY_TRIGGER1 || keyNum == K_JOY_TRIGGER2) {
		int cstCurrentDomain = idKeyInput::CstGetCurrentDomain();
		if (keyState[keyNum] == false) { // if not previously pressed
			if (pressed) {
				idKeyInput::CstSetKeyLayer(keyNum, cstCmdAux.activeLayer);
			}
			action = idKeyInput::GetUsercmdAction(keyNum, cstCmdAux.activeLayer, cstCurrentDomain);
		} else {
			int cstKeyLayer = idKeyInput::CstGetKeyLayer(keyNum);
			action = idKeyInput::GetUsercmdAction(keyNum, cstKeyLayer, cstCurrentDomain);
		}
	} else {
		action = idKeyInput::GetUsercmdAction(keyNum, 0, 0);
	}
	//#modified-fva; END

	//#modified-fva; BEGIN
	//if ( action >= UB_ATTACK ) {
	if (action >= UB_ATTACK || action == UB_NONE) {
	//#modified-fva; END
		Key( keyNum, pressed );
		return;
	}
	//#modified-fva; BEGIN
	keyState[keyNum] = pressed;
	//#modified-fva; END
	if ( !pressed ) {
		return;
	}

	float lookValue = 0.0f;
	if ( joy_gammaLook.GetBool() ) {
		lookValue = idMath::Pow( 1.04712854805f, value * 100.0f ) * 0.01f;
	} else {
		lookValue = idMath::Pow( value, joy_powerScale.GetFloat() );
	}

	idGame * game = common->Game();
	if ( game != NULL ) {
		lookValue *= game->GetAimAssistSensitivity();
	}

	switch ( action ) {
		case UB_MOVEFORWARD: {
			//#modified-fva; BEGIN
			cstCmdAux.isForwardLocked = false;
			//#modified-fva; END
			float move = (float)cmd.forwardmove + ( KEY_MOVESPEED * value );
			cmd.forwardmove = idMath::ClampChar( idMath::Ftoi( move ) );
			break;
		}
		case UB_MOVEBACK: {
			//#modified-fva; BEGIN
			cstCmdAux.isForwardLocked = false;
			//#modified-fva; END
			float move = (float)cmd.forwardmove - ( KEY_MOVESPEED * value );
			cmd.forwardmove = idMath::ClampChar( idMath::Ftoi( move ) );
			break;
		}
		case UB_MOVELEFT: {
			float move = (float)cmd.rightmove - ( KEY_MOVESPEED * value );
			cmd.rightmove = idMath::ClampChar( idMath::Ftoi( move ) );
			break;
		}
		case UB_MOVERIGHT: {
			float move = (float)cmd.rightmove + ( KEY_MOVESPEED * value );
			cmd.rightmove = idMath::ClampChar( idMath::Ftoi( move ) );
			break;
		}
		case UB_LOOKUP: {
			if ( joy_dampenLook.GetBool() ) {
				lookValue = Min( lookValue, ( pollTime - lastPollTime ) * joy_deltaPerMSLook.GetFloat() + lastLookValuePitch );
				lastLookValuePitch = lookValue;
			}

			float invertPitch = in_invertLook.GetBool() ? -1.0f : 1.0f;
			viewangles[PITCH] -= MS2SEC( pollTime - lastPollTime ) * lookValue * joy_pitchSpeed.GetFloat() * invertPitch;
			break;
		}
		case UB_LOOKDOWN: {
			if ( joy_dampenLook.GetBool() ) {
				lookValue = Min( lookValue, ( pollTime - lastPollTime ) * joy_deltaPerMSLook.GetFloat() + lastLookValuePitch );
				lastLookValuePitch = lookValue;
			}

			float invertPitch = in_invertLook.GetBool() ? -1.0f : 1.0f;
			viewangles[PITCH] += MS2SEC( pollTime - lastPollTime ) * lookValue * joy_pitchSpeed.GetFloat() * invertPitch;
			break;
		}
		case UB_LOOKLEFT: {
			if ( joy_dampenLook.GetBool() ) {
				lookValue = Min( lookValue, ( pollTime - lastPollTime ) * joy_deltaPerMSLook.GetFloat() + lastLookValueYaw );
				lastLookValueYaw = lookValue;
			}
			viewangles[YAW] += MS2SEC( pollTime - lastPollTime ) * lookValue * joy_yawSpeed.GetFloat();
			break;
		}
		case UB_LOOKRIGHT: {
			if ( joy_dampenLook.GetBool() ) {
				lookValue = Min( lookValue, ( pollTime - lastPollTime ) * joy_deltaPerMSLook.GetFloat() + lastLookValueYaw );
				lastLookValueYaw = lookValue;
			}
			viewangles[YAW] -= MS2SEC( pollTime - lastPollTime ) * lookValue * joy_yawSpeed.GetFloat();
			break;
		}
	}
}

/*
=================
idUsercmdGenLocal::JoystickMove
=================
*/
void idUsercmdGenLocal::JoystickMove() {
	float threshold = joy_deadZone.GetFloat();
	float triggerThreshold = joy_triggerThreshold.GetFloat();

	float axis_y = joystickAxis[ AXIS_LEFT_Y ];
	float axis_x = joystickAxis[ AXIS_LEFT_X ];
	CircleToSquare( axis_x, axis_y );

	HandleJoystickAxis( K_JOY_STICK1_UP, axis_y, threshold, false );
	HandleJoystickAxis( K_JOY_STICK1_DOWN, axis_y, threshold, true );
	HandleJoystickAxis( K_JOY_STICK1_LEFT, axis_x, threshold, false );
	HandleJoystickAxis( K_JOY_STICK1_RIGHT, axis_x, threshold, true );

	axis_y = joystickAxis[ AXIS_RIGHT_Y ];
	axis_x = joystickAxis[ AXIS_RIGHT_X ];
	CircleToSquare( axis_x, axis_y );

	HandleJoystickAxis( K_JOY_STICK2_UP, axis_y, threshold, false );
	HandleJoystickAxis( K_JOY_STICK2_DOWN, axis_y, threshold, true );
	HandleJoystickAxis( K_JOY_STICK2_LEFT, axis_x, threshold, false );
	HandleJoystickAxis( K_JOY_STICK2_RIGHT, axis_x, threshold, true );

	HandleJoystickAxis( K_JOY_TRIGGER1, joystickAxis[ AXIS_LEFT_TRIG ], triggerThreshold, true );
	HandleJoystickAxis( K_JOY_TRIGGER2, joystickAxis[ AXIS_RIGHT_TRIG ], triggerThreshold, true );
}

enum transferFunction_t {
	FUNC_LINEAR,
	FUNC_LOGARITHMIC,
	FUNC_EXPONENTIAL
};

/*
=================
JoypadFunction
=================
*/
idVec2 JoypadFunction(
	const idVec2 raw,
	const float	aimAssistScale,
	const float threshold,
	const float range,
	const transferFunction_t shape,
	const bool	mergedThreshold ) {

	if ( range <= threshold ) {
		return idVec2( 0.0f, 0.0f );
	}

	idVec2	threshed;
	if ( !mergedThreshold ) {
		// if the thresholding is performed independently, you can more easily move
		// or look in a pure axial direction without drifting
		for ( int i = 0 ; i < 2 ; i++ ) {
			const float v = raw[i];
			float t;
			if ( v > 0.0f ) {
				t = Max( 0.0f, v - threshold );
			} else {
				t = Min( 0.0f, v + threshold );
			}
			threshed[i] = t;
		}
	} else {
		// thresholding together is the most predictable in free-form movement,
		// but you tend to slide off axis based on which side your thumb is
		// on the pad
		const float	rawLength = raw.Length();
		const float	afterThreshold = Max( 0.0f, rawLength - threshold );

		idVec2 rawDir = raw;
		rawDir.Normalize();

		threshed = rawDir * afterThreshold;
	}

	// threshold and range reduce the range of raw values, but we
	// scale them back up to the full 0.0 - 1.0 range
	const float rangeScale = 1.0f / ( range - threshold );
	idVec2 reScaled = threshed * rangeScale;

	const float rescaledLen = reScaled.Length();

	// if inside the deadband area, return a solid 0,0
	if ( rescaledLen <= 0.0f ) {
		return idVec2( 0.0f, 0.0f );
	}

	reScaled.Normalize();

	// apply the acceleration
	float accelerated;

	if ( shape == FUNC_EXPONENTIAL ) {
		accelerated = idMath::Pow( 1.04712854805f, rescaledLen * 100.0f ) * 0.01f;
	} else if ( shape == FUNC_LOGARITHMIC ) {
		const float power = 2.0f;
		accelerated = idMath::Pow( rescaledLen, power );
	} else {	// FUNC_LINEAR
		accelerated = rescaledLen;
	}

	// optionally slow down for aim-assist
	const float aimAssisted = accelerated * aimAssistScale;

	const float clamped = ( aimAssisted > 1.0f ) ? 1.0f : aimAssisted;

	return reScaled * clamped;
}

/*
=================
DrawJoypadTexture

Draws axis and threshold / range rings into an RGBA image
=================
*/
void	DrawJoypadTexture(
	const int	size,
	byte	image[],

	const idVec2 raw,

	const float threshold,
	const float range,
	const transferFunction_t shape,
	const bool	mergedThreshold ) {

//	assert( raw.x >= -1.0f && raw.x <= 1.0f && raw.y >= -1.0f && raw.y <= 1.0f );
	idVec2	clamped;
	for ( int i = 0 ; i < 2 ; i++ ) {
		clamped[i] = Max( -1.0f, Min( raw[i], 1.0f ) );
	}

	const int halfSize = size/2;

	// find the offsets that will give certain values for
	// the rings
	static const int NUM_RINGS = 5;
	float	ringSizes[NUM_RINGS] = {};
	float	ringValue[NUM_RINGS] = { 0.0f, 0.25f, 0.5f, 0.75f, 0.99f };
	int		ringNum = 0;
	for ( int i = 1 ; i < size ; i++ ) {
		const float	v = (float)i / (size-1);

		const idVec2 mapped = JoypadFunction(
			idVec2( v, 0.0f ), 1.0f, threshold, range, shape, mergedThreshold );
		if ( mapped.x > ringValue[ ringNum ] ) {
			ringSizes[ ringNum ] = v * halfSize;
			ringNum++;
			if ( ringNum == NUM_RINGS ) {
				break;
			}
		}
	}

	memset( image, 0, size * size * 4 );
#define PLOT(x,y) ((int *)image)[(int)(y)*size+(int)(x)]=0xffffffff
#define CPLOT(x,y) ((int *)image)[(int)(halfSize+y)*size+(int)(halfSize+x)]=0xffffffff

	int	clampedX = halfSize + Min( halfSize-1, (int)(halfSize * clamped.x) );
	int	clampedY = halfSize + Min( halfSize-1, (int)(halfSize * clamped.y) );

	// draw the box edge outline and center lines
	for ( int i = 0 ; i < size ; i++ ) {
		PLOT( i, 0 );
		PLOT( i, size-1 );
		PLOT( 0, i );
		PLOT( size-1, i );
		PLOT( i, clampedY );
		PLOT( clampedX, i );
	}
	const int iThresh = size * threshold * 0.5f;
	if ( !mergedThreshold ) {
		const int open = size * 0.5f - iThresh;
		for ( int i = 0 ; i < open ; i++ ) {
			PLOT( i, halfSize - iThresh );
			PLOT( i, halfSize + iThresh );
			PLOT( size-1-i, halfSize - iThresh );
			PLOT( size-1-i, halfSize + iThresh );

			PLOT( halfSize - iThresh, i );
			PLOT( halfSize + iThresh, i );
			PLOT( halfSize - iThresh, size-1-i );
			PLOT( halfSize + iThresh, size-1-i );
		}
	}

	// I'm not going to bother writing a proper circle drawing algorithm...
	const int octantPoints = size * 2;
	float rad = 0.0f;
	float radStep = idMath::PI / ( 4 * octantPoints );
	for ( int point = 0 ; point < octantPoints ; point++, rad += radStep ) {
		float	s, c;
		idMath::SinCos( rad, s, c );
		for ( int ringNum = 0 ; ringNum < NUM_RINGS ; ringNum++ ) {
			const float ringSize = ringSizes[ ringNum ];
			const int	ix = idMath::Floor( ringSize * c );
			const int	iy = idMath::Floor( ringSize * s );
#if 0
			if ( !mergedThreshold && ( ix < iThresh || iy < iThresh ) ) {
				continue;
			}
#endif
			CPLOT( ix, iy );
			CPLOT( iy, ix );
			CPLOT( -ix, iy );
			CPLOT( -iy, ix );
			CPLOT( ix, -iy );
			CPLOT( iy, -ix );
			CPLOT( -ix, -iy );
			CPLOT( -iy, -ix );
		}
	}

#undef PLOT
}

static idVec2	lastLookJoypad;

/*
=================
DrawJoypadTexture

Can be called to fill in a scratch texture for visualization
=================
*/
void DrawJoypadTexture( const int size, byte image[] ) {
	const float threshold =			joy_deadZone.GetFloat();
	const float range =				joy_range.GetFloat();
	const bool mergedThreshold =	joy_mergedThreshold.GetBool();
	const transferFunction_t shape =(transferFunction_t)joy_gammaLook.GetInteger();

	DrawJoypadTexture( size, image, lastLookJoypad, threshold, range, shape, mergedThreshold );
}


/*
=================
idUsercmdGenLocal::JoystickMove2
=================
*/
void idUsercmdGenLocal::JoystickMove2() {
	const bool invertLook =			in_invertLook.GetBool();
	const float threshold =			joy_deadZone.GetFloat();
	const float range =				joy_range.GetFloat();
	const transferFunction_t shape =(transferFunction_t)joy_gammaLook.GetInteger();
	const bool mergedThreshold =	joy_mergedThreshold.GetBool();
	const float pitchSpeed =		joy_pitchSpeed.GetFloat();
	const float yawSpeed =			joy_yawSpeed.GetFloat();

	idGame * game = common->Game();
	const float aimAssist = game != NULL ? game->GetAimAssistSensitivity() : 1.0f;

	idVec2 leftRaw( joystickAxis[ AXIS_LEFT_X ], joystickAxis[ AXIS_LEFT_Y ] );
	idVec2 rightRaw( joystickAxis[ AXIS_RIGHT_X ], joystickAxis[ AXIS_RIGHT_Y ] );

	// optional stick swap
	//#modified-fva; BEGIN
	//if ( idKeyInput::GetUsercmdAction( K_JOY_STICK1_LEFT ) == UB_LOOKLEFT ) {
	if (idKeyInput::GetUsercmdAction(K_JOY_STICK1_LEFT, 0, 0) == UB_LOOKLEFT) {
	//#modified-fva; END
		const idVec2	temp = leftRaw;
		leftRaw = rightRaw;
		rightRaw = temp;
	}

	// optional invert look by inverting the right Y axis
	if ( invertLook ) {
		rightRaw.y = -rightRaw.y;
	}

	// save for visualization
	lastLookJoypad = rightRaw;

	idVec2 leftMapped = JoypadFunction( leftRaw, 1.0f, threshold, range, shape, mergedThreshold );
	idVec2 rightMapped = JoypadFunction( rightRaw, aimAssist, threshold, range, shape, mergedThreshold );

	// because idPhysics_Player::CmdScale scales mvoement values down so that 1,1 = sqrt(2), sqrt(2),
	// we need to expand our circular values out to a square
	CircleToSquare( leftMapped.x, leftMapped.y );

	// add on top of mouse / keyboard move values
	//#modified-fva; BEGIN
	if (idMath::Fabs(leftMapped.y) > 0.1f) {
		cstCmdAux.isForwardLocked = false;
	}
	//#modified-fva; END
	cmd.forwardmove = idMath::ClampChar( cmd.forwardmove + KEY_MOVESPEED * -leftMapped.y );
	cmd.rightmove = idMath::ClampChar( cmd.rightmove + KEY_MOVESPEED * leftMapped.x );

	viewangles[PITCH] += MS2SEC( pollTime - lastPollTime ) * rightMapped.y * pitchSpeed;
	viewangles[YAW] += MS2SEC( pollTime - lastPollTime ) * -rightMapped.x * yawSpeed;

	const float triggerThreshold = joy_triggerThreshold.GetFloat();
	HandleJoystickAxis( K_JOY_TRIGGER1, joystickAxis[ AXIS_LEFT_TRIG ], triggerThreshold, true );
	HandleJoystickAxis( K_JOY_TRIGGER2, joystickAxis[ AXIS_RIGHT_TRIG ], triggerThreshold, true );
}

/*
==============
idUsercmdGenLocal::CmdButtons
==============
*/
void idUsercmdGenLocal::CmdButtons() {
	cmd.buttons = 0;

	// check the attack button
	if ( ButtonState( UB_ATTACK ) ) {
		cmd.buttons |= BUTTON_ATTACK;
	}

	// check the use button
	if ( ButtonState( UB_USE ) ) {
		cmd.buttons |= BUTTON_USE;
	}

	// check the run button
	//#modified-fva; BEGIN
	//if ( toggled_run.on || ( in_alwaysRun.GetBool() && common->IsMultiplayer() ) ) {
	bool cstAlwaysRun = common->GetCurrentGame() == DOOM3_BFG ? in_alwaysRun.GetBool() : cst_dcAlwaysRun.GetBool();
	if (toggled_run.on || cstAlwaysRun) {
	//#modified-fva; END
		cmd.buttons |= BUTTON_RUN;
	}

	// check the zoom button
	if ( toggled_zoom.on ) {
		cmd.buttons |= BUTTON_ZOOM;
	}

	if ( ButtonState( UB_MOVEUP ) ) {
		cmd.buttons |= BUTTON_JUMP;
	}
	if ( toggled_crouch.on ) {
		cmd.buttons |= BUTTON_CROUCH;
	}
}

/*
================
idUsercmdGenLocal::InitCurrent

inits the current command for this frame
================
*/
void idUsercmdGenLocal::InitCurrent() {
	memset( &cmd, 0, sizeof( cmd ) );
	cmd.impulseSequence = impulseSequence;
	cmd.impulse = impulse;
	//#modified-fva; BEGIN
	cmd.cstLayer = cstLayer;
	//#modified-fva; END
	//#modified-fva; BEGIN
	//cmd.buttons |= ( in_alwaysRun.GetBool() && common->IsMultiplayer() ) ? BUTTON_RUN : 0;
	bool cstAlwaysRun = common->GetCurrentGame() == DOOM3_BFG ? in_alwaysRun.GetBool() : cst_dcAlwaysRun.GetBool();
	cmd.buttons |= cstAlwaysRun ? BUTTON_RUN : 0;
	//#modified-fva; END
}

/*
================
idUsercmdGenLocal::MakeCurrent

creates the current command for this frame
================
*/
void idUsercmdGenLocal::MakeCurrent() {
	idVec3 oldAngles = viewangles;

	if ( !Inhibited() ) {
		// update toggled key states
		toggled_crouch.SetKeyState( ButtonState( UB_MOVEDOWN ), in_toggleCrouch.GetBool() );
		//#modified-fva; BEGIN
		//toggled_run.SetKeyState( ButtonState( UB_SPEED ), in_toggleRun.GetBool() && common->IsMultiplayer() );
		bool cstToggleRun = common->GetCurrentGame() == DOOM3_BFG ? in_toggleRun.GetBool() : cst_dcToggleRun.GetBool();
		toggled_run.SetKeyState(ButtonState(UB_SPEED), cstToggleRun);
		//#modified-fva; END
		toggled_zoom.SetKeyState( ButtonState( UB_ZOOM ), in_toggleZoom.GetBool() );

		// get basic movement from mouse
		MouseMove();

		// get basic movement from joystick and set key bits
		// must be done before CmdButtons!
		if ( joy_newCode.GetBool() ) {
			JoystickMove2();
		} else {
			JoystickMove();
		}

		// keyboard angle adjustment
		AdjustAngles();

		// set button bits
		CmdButtons();

		// get basic movement from keyboard
		KeyMove();

		// aim assist
		AimAssist();

		// check to make sure the angles haven't wrapped
		if ( viewangles[PITCH] - oldAngles[PITCH] > 90 ) {
			viewangles[PITCH] = oldAngles[PITCH] + 90;
		} else if ( oldAngles[PITCH] - viewangles[PITCH] > 90 ) {
			viewangles[PITCH] = oldAngles[PITCH] - 90;
		}
	} else {
		mouseDx = 0;
		mouseDy = 0;
		//#modified-fva; BEGIN
		eventLoop->CstClearPushedEvents();
		cstCmdAux.Clear();
		//#modified-fva; END
	}

	for ( int i = 0; i < 3; i++ ) {
		cmd.angles[i] = ANGLE2SHORT( viewangles[i] );
	}

	cmd.mx = continuousMouseX;
	cmd.my = continuousMouseY;

	impulseSequence = cmd.impulseSequence;
	impulse = cmd.impulse;
	//#modified-fva; BEGIN
	cstLayer = cmd.cstLayer;
	//#modified-fva; END

}

/*
================
idUsercmdGenLocal::AimAssist
================
*/
void idUsercmdGenLocal::AimAssist() {
	// callback to the game to update the aim assist for the current device
	idAngles aimAssistAngles( 0.0f, 0.0f, 0.0f );

	idGame * game = common->Game();
	if ( game != NULL ) {
		game->GetAimAssistAngles( aimAssistAngles );
	}

	viewangles[YAW] += aimAssistAngles.yaw;
	viewangles[PITCH] += aimAssistAngles.pitch;
	viewangles[ROLL] += aimAssistAngles.roll;
}

//=====================================================================


/*
================
idUsercmdGenLocal::CommandStringUsercmdData

Returns the button if the command string is used by the usercmd generator.
================
*/
int	idUsercmdGenLocal::CommandStringUsercmdData( const char *cmdString ) {
	for ( userCmdString_t *ucs = userCmdStrings ; ucs->string ; ucs++ ) {
		if ( idStr::Icmp( cmdString, ucs->string ) == 0 ) {
			return ucs->button;
		}
	}
	return UB_NONE;
}

/*
================
idUsercmdGenLocal::Init
================
*/
void idUsercmdGenLocal::Init() {
	initialized = true;
}

/*
================
idUsercmdGenLocal::InitForNewMap
================
*/
void idUsercmdGenLocal::InitForNewMap() {
	//#modified-fva; BEGIN
	memset(&cmd, 0, sizeof(cmd));
	//#modified-fva; END
	impulseSequence = 0;
	impulse = 0;
	//#modified-fva; BEGIN
	cstLayer = 0;
	//#modified-fva; END

	toggled_crouch.Clear();
	toggled_run.Clear();
	toggled_zoom.Clear();
	toggled_run.on = false;

	Clear();
	ClearAngles();
}

/*
================
idUsercmdGenLocal::Shutdown
================
*/
void idUsercmdGenLocal::Shutdown() {
	initialized = false;
}

/*
================
idUsercmdGenLocal::Clear
================
*/
void idUsercmdGenLocal::Clear() {
	// clears all key states
	memset( buttonState, 0, sizeof( buttonState ) );
	memset( keyState, false, sizeof( keyState ) );
	memset( joystickAxis, 0, sizeof( joystickAxis ) );

	inhibitCommands = false;

	mouseDx = mouseDy = 0;
	mouseButton = 0;
	mouseDown = false;

	//#modified-fva; BEGIN
	cstCmdAux.Clear();
	//#modified-fva; END
}

/*
================
idUsercmdGenLocal::ClearAngles
================
*/
void idUsercmdGenLocal::ClearAngles() {
	viewangles.Zero();
}

//======================================================================


/*
===================
idUsercmdGenLocal::Key

Handles mouse/keyboard button actions
===================
*/
void idUsercmdGenLocal::Key( int keyNum, bool down ) {

	// Sanity check, sometimes we get double message :(
	//#modified-fva; BEGIN
	//if ( keyState[ keyNum ] == down ) {
	if (keyNum < 0 || keyNum >= K_LAST_KEY || keyState[keyNum] == down) {
	//#modified-fva; END
		return;
	}
	keyState[ keyNum ] = down;

	//#modified-fva; BEGIN
	//int action = idKeyInput::GetUsercmdAction( keyNum );
	//#modified-fva; END

	if ( down ) {
		//#modified-fva; BEGIN
		idKeyInput::CstSetKeyLayer(keyNum, cstCmdAux.activeLayer);
		int cstCurrentDomain = idKeyInput::CstGetCurrentDomain();

		int action = idKeyInput::GetUsercmdAction(keyNum, cstCmdAux.activeLayer, cstCurrentDomain);
		if (cstCurrentDomain == 2 && cst_amInheritBinds.GetBool() && action == UB_NONE) {
			const char *binding = idKeyInput::GetBinding(keyNum, cstCmdAux.activeLayer, 2);
			if (!binding || !binding[0]) {
				cstCurrentDomain = 1;
				action = idKeyInput::GetUsercmdAction(keyNum, cstCmdAux.activeLayer, 1);
			}
		}
		idKeyInput::CstSetKeyDomain(keyNum, cstCurrentDomain);

		if (action >= UB_CST_SWITCH_LAYER_1 && action <= UB_CST_HOLD_LAYER_9) {
			int cstLayerChangeImpulse;
			if (action <= UB_CST_SWITCH_LAYER_9) {
				// switch layer command
				cstCmdAux.activeLayer = 1 + action - UB_CST_SWITCH_LAYER_1;
				cstCmdAux.isHoldingLayer = false;
				cstLayerChangeImpulse = IMPULSE_62;
			} else {
				// hold layer command
				cstCmdAux.activeLayer = 1 + action - UB_CST_HOLD_LAYER_1;
				cstCmdAux.isHoldingLayer = true;
				cstLayerChangeImpulse = IMPULSE_63;
			}
			cmd.cstLayer = cstCmdAux.activeLayer;
			cmd.impulse = cstLayerChangeImpulse;
			cmd.impulseSequence++;
		}
		else if (action == UB_CST_CANCEL_LAYER) {
			// cancel layer command
			cstCmdAux.activeLayer = 0;
			cstCmdAux.isHoldingLayer = false;
		}
		else {
			int cstLatchedActiveLayer = cstCmdAux.activeLayer;
			if (!cstCmdAux.isHoldingLayer) {
				// switch mode: go back to main layer
				cstCmdAux.activeLayer = 0;
			}
			if (action == UB_CST_LOCK_FORWARD) {
				cstCmdAux.isForwardLocked = true;
				return; // <== watch out
			}
			else if (action == UB_MOVEFORWARD || action == UB_MOVEBACK || action == UB_CST_AUTOMAP_PAN_UP || action == UB_CST_AUTOMAP_PAN_DOWN) {
				cstCmdAux.isForwardLocked = false;
			}
			else if (action == UB_NONE) {
				// not an action command: redirect to eventLoop
				if (!Inhibited()) {
					sysEvent_t ev;
					ev.evType = SE_CST_KEY;
					ev.evValue = keyNum + 1; // encoded key for a keydown; for a keyup, this would be -(keyNum + 1)
					ev.evValue2 = (cstCurrentDomain * CST_NUM_LAYERS) + cstLatchedActiveLayer; // encoded layer
					ev.evPtrLength = 0;
					ev.evPtr = NULL;
					eventLoop->CstPushEvent(ev);
				}
				return; // <== watch out
			}
			if (action >= UB_CST_TOGGLE_AUTOMAP && action <= UB_CST_AUTOMAP_CLEAR_MARK) {
				CstPushAutomapEvent(action, true, cstCurrentDomain);
			}
		}
		//#modified-fva; END
		buttonState[ action ]++;
		if ( !Inhibited()  ) {
			//#modified-fva; BEGIN
			//if ( action >= UB_IMPULSE0 && action <= UB_IMPULSE31 ) {
			if (action >= UB_IMPULSE0 && action <= UB_IMPULSE61) { // note: impulses 62 and 63 should not be issued by key bindings
			//#modified-fva; END
				cmd.impulse = action - UB_IMPULSE0;
				cmd.impulseSequence++;
			}
		}
	} else {
		//#modified-fva; BEGIN
		int cstKeyLayer = idKeyInput::CstGetKeyLayer(keyNum);
		int cstKeyDomain = idKeyInput::CstGetKeyDomain(keyNum);
		int action = idKeyInput::GetUsercmdAction(keyNum, cstKeyLayer, cstKeyDomain);
		if (action >= UB_CST_HOLD_LAYER_1 && action <= UB_CST_HOLD_LAYER_9) {
			// hold layer command: on key up, if the corresponding layer is still active, go back to the main layer
			if (cstCmdAux.isHoldingLayer && cstCmdAux.activeLayer == (1 + action - UB_CST_HOLD_LAYER_1)) {
				cstCmdAux.activeLayer = 0;
				cstCmdAux.isHoldingLayer = false;
			}
		}
		else if (action >= UB_CST_TOGGLE_AUTOMAP && action <= UB_CST_AUTOMAP_CLEAR_MARK) {
			CstPushAutomapEvent(action, false, cstKeyDomain);
		}
		idKeyInput::CstSetKeyLayer(keyNum, cstCmdAux.activeLayer);
		//#modified-fva; END
		buttonState[ action ]--;
		// we might have one held down across an app active transition
		if ( buttonState[ action ] < 0 ) {
			buttonState[ action ] = 0;
		}
	}
}

/*
===================
idUsercmdGenLocal::Mouse
===================
*/
void idUsercmdGenLocal::Mouse() {
	int	mouseEvents[MAX_MOUSE_EVENTS][2];

	int numEvents = Sys_PollMouseInputEvents( mouseEvents );

	// Study each of the buffer elements and process them.
	for ( int i = 0; i < numEvents; i++ ) {
		int action = mouseEvents[i][0];
		int value = mouseEvents[i][1];
		switch ( action ) {
		case M_ACTION1:
		case M_ACTION2:
		case M_ACTION3:
		case M_ACTION4:
		case M_ACTION5:
		case M_ACTION6:
		case M_ACTION7:
		case M_ACTION8:
			mouseButton = K_MOUSE1 + ( action - M_ACTION1 );
			mouseDown = ( value != 0 );
			Key( mouseButton, mouseDown );
			break;
		case M_DELTAX:
			mouseDx += value;
			continuousMouseX += value;
			break;
		case M_DELTAY:
			mouseDy += value;
			continuousMouseY += value;
			break;
		case M_DELTAZ:	// mouse wheel, may have multiple clicks
			{
				int key = value < 0 ? K_MWHEELDOWN : K_MWHEELUP;
				value = abs( value );
				while( value-- > 0 ) {
					Key( key, true );
					Key( key, false );
					mouseButton = key;
					mouseDown = true;
				}
			}
			break;
		default:	// some other undefined button
			break;
		}
	}
}

/*
===============
idUsercmdGenLocal::Keyboard
===============
*/
void idUsercmdGenLocal::Keyboard() {

	int numEvents = Sys_PollKeyboardInputEvents();

    // Study each of the buffer elements and process them.
	for ( int i = 0; i < numEvents; i++ ) {
		int key;
		bool state;
		if ( Sys_ReturnKeyboardInputEvent( i, key, state ) ) {
			Key( key, state );
		}
	}

	Sys_EndKeyboardInputEvents();
}

/*
===============
idUsercmdGenLocal::Joystick
===============
*/
void idUsercmdGenLocal::Joystick( int deviceNum ) {
	int numEvents = Sys_PollJoystickInputEvents( deviceNum );

	// Study each of the buffer elements and process them.
	for ( int i = 0; i < numEvents; i++ ) {
		int action;
		int value;
		if ( Sys_ReturnJoystickInputEvent( i, action, value ) ) {
			if ( action >= J_ACTION1 && action <= J_ACTION_MAX ) {
				int joyButton = K_JOY1 + ( action - J_ACTION1 );
				Key( joyButton, ( value != 0 ) );
			} else if ( ( action >= J_AXIS_MIN ) && ( action <= J_AXIS_MAX ) ) {
				joystickAxis[ action - J_AXIS_MIN ] = static_cast<float>( value ) / 32767.0f;
			} else if ( action >= J_DPAD_UP && action <= J_DPAD_RIGHT ) {
				int joyButton = K_JOY_DPAD_UP + ( action - J_DPAD_UP );
				Key( joyButton, ( value != 0 ) );
			} else {
				assert( !"Unknown joystick event" );
			}
		}
	}

	Sys_EndJoystickInputEvents();
}

/*
================
idUsercmdGenLocal::MouseState
================
*/
void idUsercmdGenLocal::MouseState( int *x, int *y, int *button, bool *down ) {
	*x = continuousMouseX;
	*y = continuousMouseY;
	*button = mouseButton;
	*down = mouseDown;
}

/*
================
idUsercmdGenLocal::BuildCurrentUsercmd
================
*/
void idUsercmdGenLocal::BuildCurrentUsercmd( int deviceNum ) {

	pollTime = Sys_Milliseconds();
	if ( pollTime - lastPollTime > 100 ) {
		lastPollTime = pollTime - 100;
	}

	// initialize current usercmd
	InitCurrent();

	// process the system mouse events
	Mouse();

	// process the system keyboard events
	Keyboard();

	// process the system joystick events
	if ( deviceNum >= 0 && in_useJoystick.GetBool() ) {
		Joystick( deviceNum );
	}

	// create the usercmd
	MakeCurrent();

	lastPollTime = pollTime;
}

//#modified-fva; BEGIN
#include "../../doomclassic/doom/doomlib.h"
#include "../../doomclassic/doom/globaldata.h"

// ===============
bool idUsercmdGenLocal::CstGetMoveWithAutomapPan() {
	// if the automap is on and follow mode is on, using the automap pan buttons also moves the player

	if (idKeyInput::CstGetCurrentDomain() != 2) {
		return false; // not on automap
	}
	int oldPlayer = DoomLib::GetPlayer();
	DoomLib::SetPlayer(0);
	bool followMode = false;
	if (g) {
		followMode = g->followplayer;
	}
	DoomLib::SetPlayer(oldPlayer);
	return followMode;
}

// ===============
void idUsercmdGenLocal::CstPushAutomapEvent(int action, bool keyDown, int domain) {
	if (domain == 0) {
		return; // no automap in Doom 3
	}
	int keyNum = -1;
	switch (action) {
	case UB_CST_TOGGLE_AUTOMAP: (domain == 2) ? keyNum = AM_ENDKEY : keyNum = AM_STARTKEY; break;
	case UB_CST_AUTOMAP_PAN_RIGHT: keyNum = AM_PANRIGHTKEY; break;
	case UB_CST_AUTOMAP_PAN_LEFT: keyNum = AM_PANLEFTKEY; break;
	case UB_CST_AUTOMAP_PAN_UP: keyNum = AM_PANUPKEY; break;
	case UB_CST_AUTOMAP_PAN_DOWN: keyNum = AM_PANDOWNKEY; break;
	case UB_CST_AUTOMAP_ZOOM_OUT: keyNum = AM_ZOOMOUTKEY; break;
	case UB_CST_AUTOMAP_ZOOM_IN: keyNum = AM_ZOOMINKEY; break;
	case UB_CST_AUTOMAP_GO_BIG: keyNum = AM_GOBIGKEY; break;
	case UB_CST_AUTOMAP_FOLLOW: keyNum = AM_FOLLOWKEY; break;
	case UB_CST_AUTOMAP_GRID: keyNum = AM_GRIDKEY; break;
	case UB_CST_AUTOMAP_MARK: keyNum = AM_MARKKEY; break;
	case UB_CST_AUTOMAP_CLEAR_MARK: keyNum = AM_CLEARMARKKEY; break;
	}
	if (keyNum > -1) {
		sysEvent_t ev;
		ev.evType = SE_CST_AUTOMAP_KEY;
		ev.evValue = keyNum;
		ev.evValue2 = keyDown ? 1 : 0;
		ev.evPtrLength = 0;
		ev.evPtr = NULL;
		eventLoop->CstPushEvent(ev);
	}
}
//#modified-fva; END
