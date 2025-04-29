//#modified-fva; BEGIN
#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

static idMenuScreen_Shell_CstBindingsBase::bindInfo_t keyboardBindsDoom3[] = {
	{ "#str_02090",						"",							NULL				},	// HEADING: CONTROLS - MOVEMENT
	{ "#str_02100",						"_forward",					NULL				},	// Forward
	{ "#str_02101",						"_back",					NULL				},	// Backpedal
	{ "#str_02102",						"_moveLeft",				NULL				},	// Move Left
	{ "#str_02103",						"_moveRight",				NULL				},	// Move Right
	{ "#str_02104",						"_moveUp",					NULL				},	// Jump
	{ "#str_02105",						"_moveDown",				NULL				},	// Crouch
	{ "#str_02106",						"_left",					NULL				},	// Turn Left
	{ "#str_02107",						"_right",					NULL				},	// Turn Right
	{ CST_STR_00300006,					"_speed",					NULL				},	// Run
	{ CST_STR_00300007,					"_cstLockForward",			CST_STR_00300041	},	// Lock Forward

	{ "#str_02095",						"",							NULL				},	// HEADING: CONTROLS - ATTACK/LOOK
	{ "#str_02112",						"_attack",					NULL				},	// Attack
	{ "#str_02114",						"_impulse14",				NULL				},	// Prev Weapon
	{ "#str_02113",						"_impulse15",				NULL				},	// Next Weapon
	{ "#str_02115",						"_impulse13",				NULL				},	// Reload
	{ "#str_swf_action_use",			"_use",						NULL				},	// Use
	{ "#str_02116",						"_lookUp",					NULL				},	// Look Up
	{ "#str_02117",						"_lookDown",				NULL				},	// Look Down
	{ CST_STR_00300008,					"_zoom",					NULL				},	// Zoom
	{ CST_STR_00300009,					"_impulse19",				NULL				},	// PDA/Score - Open
	{ CST_STR_00300010,					"cst_keyPdaScoreClose",		CST_STR_00300042	},	// PDA/Score - Close
	{ CST_STR_00300011,					"_impulse16",				NULL				},	// Armor Flashlight
	{ CST_STR_00300012,					"_impulse26",				CST_STR_00300043	},	// Headlamp

	{ "#str_02093",						"",							NULL				},	// HEADING: CONTROLS - WEAPONS
	{ CST_STR_00300013,					"_impulse0",				CST_STR_00300044	},	// Fists/Grabber
	{ CST_STR_00300014,					"_impulse1",				CST_STR_00300045	},	// Grabber/Fists
	{ "#str_00100178",					"_impulse2",				NULL				},	// Pistol
	{ CST_STR_00300015,					"_impulse3",				CST_STR_00300046	},	// Shotgun/DBS
	{ CST_STR_00300016,					"_impulse4",				CST_STR_00300047	},	// DBS/Shotgun
	{ "#str_00100180",					"_impulse5",				NULL				},	// Machinegun
	{ "#str_00100181",					"_impulse6",				NULL				},	// Chaingun
	{ "#str_00100182",					"_impulse7",				NULL				},	// Grenades
	{ "#str_00100183",					"_impulse8",				NULL				},	// Plasmagun
	{ "#str_00100184",					"_impulse9",				NULL				},	// Rockets
	{ "#str_00100185",					"_impulse10",				NULL				},	// BFG
	{ CST_STR_00300017,					"_impulse11",				NULL				},	// Chainsaw
	{ "#str_swf_soulcube_artifact",		"_impulse12",				NULL				},	// Soulcube/Artifact
	{ CST_STR_00300018,					"_impulse32",				CST_STR_00300048	},	// Handheld Flashlight

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

	{ "#str_04065",						"",							NULL				},	// HEADING: CONTROLS - OTHER
	{ "#str_04067",						"savegame quick",			NULL				},	// Quick Save
	{ "#str_04068",						"loadgame quick",			NULL				},	// Quick Load
	{ "#str_04069",						"screenshot",				NULL				},	// Screenshot
	{ "#str_02068",						"clientMessageMode",		NULL				},	// Say Global
	{ "#str_02122",						"clientMessageMode 1",		NULL				},	// Say Team
	{ CST_STR_00300039,					"clientDropWeapon",			CST_STR_00300067	},	// Drop Weapon
	{ CST_STR_00300040,					"_cstCancelLayer",			CST_STR_00300068	}	// Cancel Layer
};

/*
========================
idMenuScreen_Shell_CstBindingsDoom3::idMenuScreen_Shell_CstBindingsDoom3
========================
*/
idMenuScreen_Shell_CstBindingsDoom3::idMenuScreen_Shell_CstBindingsDoom3() {
	keyboardBinds = keyboardBindsDoom3;
	numBinds = sizeof(keyboardBindsDoom3) / sizeof(keyboardBindsDoom3[0]);
	bindingsDomain = 0; // 0 = Doom 3; 1 = Doom 1 & 2; 2 = Doom 1 & 2 Automap
	menuSpriteName = "cstMenuBindingsDoom3";
	shellArea = CST_SHELL_AREA_BINDINGS_DOOM3;
	headingLabel = CST_STR_00300003;
	goBackShellArea = SHELL_AREA_CONTROLS;
	goBackLabel = "#str_04158";
}
//#modified-fva; END
