//#modified-fva; BEGIN
#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

static idMenuScreen_Shell_CstBindingsBase::bindInfo_t keyboardBindsDoomClassicNormal[] = {
	{ "#str_02090",						"",							NULL				},	// HEADING: CONTROLS - MOVEMENT
	{ "#str_02100",						"_forward",					NULL				},	// Forward
	{ "#str_02101",						"_back",					NULL				},	// Backpedal
	{ "#str_02102",						"_moveLeft",				NULL				},	// Move Left
	{ "#str_02103",						"_moveRight",				NULL				},	// Move Right
	{ "#str_02104",						"_moveUp",					CST_STR_00300086	},	// Jump
	{ "#str_02106",						"_left",					NULL				},	// Turn Left
	{ "#str_02107",						"_right",					NULL				},	// Turn Right
	{ CST_STR_00300006,					"_speed",					NULL				},	// Run
	{ CST_STR_00300007,					"_cstLockForward",			CST_STR_00300041	},	// Lock Forward

	{ "#str_02095",						"",							NULL				},	// HEADING: CONTROLS - ATTACK/LOOK
	{ "#str_02112",						"_attack",					NULL				},	// Attack
	{ "#str_02114",						"_impulse14",				NULL				},	// Prev Weapon
	{ "#str_02113",						"_impulse15",				NULL				},	// Next Weapon
	{ "#str_swf_action_use",			"_use",						NULL				},	// Use
	{ CST_STR_00300070,					"_cstToggleAutomap",		NULL				},	// Toggle Automap

	{ "#str_02093",						"",							NULL				},	// HEADING: CONTROLS - WEAPONS
	{ CST_STR_00300071,					"_impulse1",				NULL				},	// Fists
	{ "#str_00100178",					"_impulse2",				NULL				},	// Pistol
	{ CST_STR_00300015,					"_impulse3",				CST_STR_00300046	},	// Shotgun/DBS
	{ CST_STR_00300016,					"_impulse4",				CST_STR_00300047	},	// DBS/Shotgun
	{ "#str_00100181",					"_impulse6",				NULL				},	// Chaingun
	{ "#str_00100184",					"_impulse9",				NULL				},	// Rockets
	{ "#str_00100183",					"_impulse8",				NULL				},	// Plasmagun
	{ "#str_00100185",					"_impulse10",				NULL				},	// BFG
	{ CST_STR_00300017,					"_impulse11",				NULL				},	// Chainsaw

	{ CST_STR_00300019,					"",							NULL				},	// HEADING: CONTROLS - HOLD LAYER
	{ CST_STR_00300020,					"_cstHoldLayer1",			CST_STR_00300049	},	// Hold Layer 1
	{ CST_STR_00300021,					"_cstHoldLayer2",			CST_STR_00300050	},	// Hold Layer 2
	{ CST_STR_00300022,					"_cstHoldLayer3",			CST_STR_00300051	},	// Hold Layer 3
	{ CST_STR_00300023,					"_cstHoldLayer4",			CST_STR_00300052	},	// Hold Layer 4
	{ CST_STR_00300024,					"_cstHoldLayer5",			CST_STR_00300053	},	// Hold Layer 5
	{ CST_STR_00300025,					"_cstHoldLayer6",			CST_STR_00300054	},	// Hold Layer 6
	{ CST_STR_00300026,					"_cstHoldLayer7",			CST_STR_00300055	},	// Hold Layer 7
	{ CST_STR_00300027,					"_cstHoldLayer8",			CST_STR_00300056	},	// Hold Layer 8
	{ CST_STR_00300028,					"_cstHoldLayer9",			CST_STR_00300057	},	// Hold Layer 9

	{ CST_STR_00300029,					"",							NULL				},	// HEADING: CONTROLS - SWITCH LAYER
	{ CST_STR_00300030,					"_cstSwitchLayer1",			CST_STR_00300058	},	// Switch To Layer 1
	{ CST_STR_00300031,					"_cstSwitchLayer2",			CST_STR_00300059	},	// Switch To Layer 2
	{ CST_STR_00300032,					"_cstSwitchLayer3",			CST_STR_00300060	},	// Switch To Layer 3
	{ CST_STR_00300033,					"_cstSwitchLayer4",			CST_STR_00300061	},	// Switch To Layer 4
	{ CST_STR_00300034,					"_cstSwitchLayer5",			CST_STR_00300062	},	// Switch To Layer 5
	{ CST_STR_00300035,					"_cstSwitchLayer6",			CST_STR_00300063	},	// Switch To Layer 6
	{ CST_STR_00300036,					"_cstSwitchLayer7",			CST_STR_00300064	},	// Switch To Layer 7
	{ CST_STR_00300037,					"_cstSwitchLayer8",			CST_STR_00300065	},	// Switch To Layer 8
	{ CST_STR_00300038,					"_cstSwitchLayer9",			CST_STR_00300066	},	// Switch To Layer 9

	{ CST_STR_00300072,					"",							NULL				},	// HEADING: CONTROLS - SAVE GAME
	{ CST_STR_00300073,					"cstDcSaveGame 0",			NULL				},	// Save To Slot 0
	{ CST_STR_00300074,					"cstDcSaveGame 1",			NULL				},	// Save To Slot 1
	{ CST_STR_00300075,					"cstDcSaveGame 2",			NULL				},	// Save To Slot 2
	{ CST_STR_00300076,					"cstDcSaveGame 3",			NULL				},	// Save To Slot 3
	{ CST_STR_00300077,					"cstDcSaveGame 4",			NULL				},	// Save To Slot 4
	{ CST_STR_00300078,					"cstDcSaveGame 5",			NULL				},	// Save To Slot 5

	{ CST_STR_00300079,					"",							NULL				},	// HEADING: CONTROLS - LOAD GAME
	{ CST_STR_00300080,					"cstDcLoadGame 0",			NULL				},	// Load From Slot 0
	{ CST_STR_00300081,					"cstDcLoadGame 1",			NULL				},	// Load From Slot 1
	{ CST_STR_00300082,					"cstDcLoadGame 2",			NULL				},	// Load From Slot 2
	{ CST_STR_00300083,					"cstDcLoadGame 3",			NULL				},	// Load From Slot 3
	{ CST_STR_00300084,					"cstDcLoadGame 4",			NULL				},	// Load From Slot 4
	{ CST_STR_00300085,					"cstDcLoadGame 5",			NULL				},	// Load From Slot 5

	{ "#str_04065",						"",							NULL				},	// HEADING: CONTROLS - OTHER
	{ "#str_04069",						"screenshot",				NULL				},	// Screenshot
	{ CST_STR_00300040,					"_cstCancelLayer",			CST_STR_00300068	}	// Cancel Layer
};

/*
========================
idMenuScreen_Shell_CstBindingsDoomClassicNormal::idMenuScreen_Shell_CstBindingsDoomClassicNormal
========================
*/
idMenuScreen_Shell_CstBindingsDoomClassicNormal::idMenuScreen_Shell_CstBindingsDoomClassicNormal() {
	keyboardBinds = keyboardBindsDoomClassicNormal;
	numBinds = sizeof(keyboardBindsDoomClassicNormal) / sizeof(keyboardBindsDoomClassicNormal[0]);
	bindingsDomain = 1; // 0 = Doom 3; 1 = Doom 1 & 2; 2 = Doom 1 & 2 Automap
	menuSpriteName = "cstMenuBindingsDoomClassicNormal";
	shellArea = CST_SHELL_AREA_BINDINGS_DOOM_CLASSIC_NORMAL;
	headingLabel = CST_STR_00300069;
	goBackShellArea = SHELL_AREA_CONTROLS;
	goBackLabel = "#str_04158";
}
//#modified-fva; END
