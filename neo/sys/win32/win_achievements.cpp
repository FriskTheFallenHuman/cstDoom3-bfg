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

#include "win_achievements.h"
#include "../sys_session_local.h"
//#modified-fva; BEGIN
#include "../../d3xp/_CstStrings.h"
//#modified-fva; END

extern idCVar achievements_Verbose;

#define STEAM_ACHIEVEMENT_PREFIX		"ach_"

//#modified-fva; BEGIN
// see:
//  achievement_t in Achievements.h
//  achievementInfo_t in Achievements.cpp
struct cstAchievementInfo_t {
	const char *title_ascii;
	const char *description_ascii;
	const char *title_localizationKey;
	const char *description_localizationKey;
};
cstAchievementInfo_t cstAchievementInfo[] = {
	{ "", "", "", ""},
	{ CST_STR_ACHIEVEMENT_01_TITLE__ASCII, CST_STR_ACHIEVEMENT_01_DESCR__ASCII, CST_STR_ACHIEVEMENT_01_TITLE, CST_STR_ACHIEVEMENT_01_DESCR },
	{ CST_STR_ACHIEVEMENT_02_TITLE__ASCII, CST_STR_ACHIEVEMENT_02_DESCR__ASCII, CST_STR_ACHIEVEMENT_02_TITLE, CST_STR_ACHIEVEMENT_02_DESCR },
	{ CST_STR_ACHIEVEMENT_03_TITLE__ASCII, CST_STR_ACHIEVEMENT_03_DESCR__ASCII, CST_STR_ACHIEVEMENT_03_TITLE, CST_STR_ACHIEVEMENT_03_DESCR },
	{ CST_STR_ACHIEVEMENT_04_TITLE__ASCII, CST_STR_ACHIEVEMENT_04_DESCR__ASCII, CST_STR_ACHIEVEMENT_04_TITLE, CST_STR_ACHIEVEMENT_04_DESCR },
	{ CST_STR_ACHIEVEMENT_05_TITLE__ASCII, CST_STR_ACHIEVEMENT_05_DESCR__ASCII, CST_STR_ACHIEVEMENT_05_TITLE, CST_STR_ACHIEVEMENT_05_DESCR },
	{ CST_STR_ACHIEVEMENT_06_TITLE__ASCII, CST_STR_ACHIEVEMENT_06_DESCR__ASCII, CST_STR_ACHIEVEMENT_06_TITLE, CST_STR_ACHIEVEMENT_06_DESCR },
	{ CST_STR_ACHIEVEMENT_07_TITLE__ASCII, CST_STR_ACHIEVEMENT_07_DESCR__ASCII, CST_STR_ACHIEVEMENT_07_TITLE, CST_STR_ACHIEVEMENT_07_DESCR },
	{ CST_STR_ACHIEVEMENT_08_TITLE__ASCII, CST_STR_ACHIEVEMENT_08_DESCR__ASCII, CST_STR_ACHIEVEMENT_08_TITLE, CST_STR_ACHIEVEMENT_08_DESCR },
	{ CST_STR_ACHIEVEMENT_09_TITLE__ASCII, CST_STR_ACHIEVEMENT_09_DESCR__ASCII, CST_STR_ACHIEVEMENT_09_TITLE, CST_STR_ACHIEVEMENT_09_DESCR },
	{ CST_STR_ACHIEVEMENT_10_TITLE__ASCII, CST_STR_ACHIEVEMENT_10_DESCR__ASCII, CST_STR_ACHIEVEMENT_10_TITLE, CST_STR_ACHIEVEMENT_10_DESCR },
	{ CST_STR_ACHIEVEMENT_11_TITLE__ASCII, CST_STR_ACHIEVEMENT_11_DESCR__ASCII, CST_STR_ACHIEVEMENT_11_TITLE, CST_STR_ACHIEVEMENT_11_DESCR },
	{ CST_STR_ACHIEVEMENT_12_TITLE__ASCII, CST_STR_ACHIEVEMENT_12_DESCR__ASCII, CST_STR_ACHIEVEMENT_12_TITLE, CST_STR_ACHIEVEMENT_12_DESCR },
	{ CST_STR_ACHIEVEMENT_13_TITLE__ASCII, CST_STR_ACHIEVEMENT_13_DESCR__ASCII, CST_STR_ACHIEVEMENT_13_TITLE, CST_STR_ACHIEVEMENT_13_DESCR },
	{ CST_STR_ACHIEVEMENT_14_TITLE__ASCII, CST_STR_ACHIEVEMENT_14_DESCR__ASCII, CST_STR_ACHIEVEMENT_14_TITLE, CST_STR_ACHIEVEMENT_14_DESCR },
	{ CST_STR_ACHIEVEMENT_15_TITLE__ASCII, CST_STR_ACHIEVEMENT_15_DESCR__ASCII, CST_STR_ACHIEVEMENT_15_TITLE, CST_STR_ACHIEVEMENT_15_DESCR },
	{ CST_STR_ACHIEVEMENT_16_TITLE__ASCII, CST_STR_ACHIEVEMENT_16_DESCR__ASCII, CST_STR_ACHIEVEMENT_16_TITLE, CST_STR_ACHIEVEMENT_16_DESCR },
	{ CST_STR_ACHIEVEMENT_17_TITLE__ASCII, CST_STR_ACHIEVEMENT_17_DESCR__ASCII, CST_STR_ACHIEVEMENT_17_TITLE, CST_STR_ACHIEVEMENT_17_DESCR },
	{ CST_STR_ACHIEVEMENT_18_TITLE__ASCII, CST_STR_ACHIEVEMENT_18_DESCR__ASCII, CST_STR_ACHIEVEMENT_18_TITLE, CST_STR_ACHIEVEMENT_18_DESCR },
	{ CST_STR_ACHIEVEMENT_19_TITLE__ASCII, CST_STR_ACHIEVEMENT_19_DESCR__ASCII, CST_STR_ACHIEVEMENT_19_TITLE, CST_STR_ACHIEVEMENT_19_DESCR },
	{ CST_STR_ACHIEVEMENT_20_TITLE__ASCII, CST_STR_ACHIEVEMENT_20_DESCR__ASCII, CST_STR_ACHIEVEMENT_20_TITLE, CST_STR_ACHIEVEMENT_20_DESCR },
	{ CST_STR_ACHIEVEMENT_21_TITLE__ASCII, CST_STR_ACHIEVEMENT_21_DESCR__ASCII, CST_STR_ACHIEVEMENT_21_TITLE, CST_STR_ACHIEVEMENT_21_DESCR },
	{ CST_STR_ACHIEVEMENT_22_TITLE__ASCII, CST_STR_ACHIEVEMENT_22_DESCR__ASCII, CST_STR_ACHIEVEMENT_22_TITLE, CST_STR_ACHIEVEMENT_22_DESCR },
	{ CST_STR_ACHIEVEMENT_23_TITLE__ASCII, CST_STR_ACHIEVEMENT_23_DESCR__ASCII, CST_STR_ACHIEVEMENT_23_TITLE, CST_STR_ACHIEVEMENT_23_DESCR },
	{ CST_STR_ACHIEVEMENT_24_TITLE__ASCII, CST_STR_ACHIEVEMENT_24_DESCR__ASCII, CST_STR_ACHIEVEMENT_24_TITLE, CST_STR_ACHIEVEMENT_24_DESCR },
	{ CST_STR_ACHIEVEMENT_25_TITLE__ASCII, CST_STR_ACHIEVEMENT_25_DESCR__ASCII, CST_STR_ACHIEVEMENT_25_TITLE, CST_STR_ACHIEVEMENT_25_DESCR },
	{ CST_STR_ACHIEVEMENT_26_TITLE__ASCII, CST_STR_ACHIEVEMENT_26_DESCR__ASCII, CST_STR_ACHIEVEMENT_26_TITLE, CST_STR_ACHIEVEMENT_26_DESCR },
	{ CST_STR_ACHIEVEMENT_27_TITLE__ASCII, CST_STR_ACHIEVEMENT_27_DESCR__ASCII, CST_STR_ACHIEVEMENT_27_TITLE, CST_STR_ACHIEVEMENT_27_DESCR },
	{ CST_STR_ACHIEVEMENT_28_TITLE__ASCII, CST_STR_ACHIEVEMENT_28_DESCR__ASCII, CST_STR_ACHIEVEMENT_28_TITLE, CST_STR_ACHIEVEMENT_28_DESCR },
	{ CST_STR_ACHIEVEMENT_29_TITLE__ASCII, CST_STR_ACHIEVEMENT_29_DESCR__ASCII, CST_STR_ACHIEVEMENT_29_TITLE, CST_STR_ACHIEVEMENT_29_DESCR },
	{ CST_STR_ACHIEVEMENT_30_TITLE__ASCII, CST_STR_ACHIEVEMENT_30_DESCR__ASCII, CST_STR_ACHIEVEMENT_30_TITLE, CST_STR_ACHIEVEMENT_30_DESCR },
	{ CST_STR_ACHIEVEMENT_31_TITLE__ASCII, CST_STR_ACHIEVEMENT_31_DESCR__ASCII, CST_STR_ACHIEVEMENT_31_TITLE, CST_STR_ACHIEVEMENT_31_DESCR },
	{ CST_STR_ACHIEVEMENT_32_TITLE__ASCII, CST_STR_ACHIEVEMENT_32_DESCR__ASCII, CST_STR_ACHIEVEMENT_32_TITLE, CST_STR_ACHIEVEMENT_32_DESCR },
	{ CST_STR_ACHIEVEMENT_33_TITLE__ASCII, CST_STR_ACHIEVEMENT_33_DESCR__ASCII, CST_STR_ACHIEVEMENT_33_TITLE, CST_STR_ACHIEVEMENT_33_DESCR },
	{ CST_STR_ACHIEVEMENT_34_TITLE__ASCII, CST_STR_ACHIEVEMENT_34_DESCR__ASCII, CST_STR_ACHIEVEMENT_34_TITLE, CST_STR_ACHIEVEMENT_34_DESCR },
	{ CST_STR_ACHIEVEMENT_35_TITLE__ASCII, CST_STR_ACHIEVEMENT_35_DESCR__ASCII, CST_STR_ACHIEVEMENT_35_TITLE, CST_STR_ACHIEVEMENT_35_DESCR },
	{ CST_STR_ACHIEVEMENT_36_TITLE__ASCII, CST_STR_ACHIEVEMENT_36_DESCR__ASCII, CST_STR_ACHIEVEMENT_36_TITLE, CST_STR_ACHIEVEMENT_36_DESCR },
	{ CST_STR_ACHIEVEMENT_37_TITLE__ASCII, CST_STR_ACHIEVEMENT_37_DESCR__ASCII, CST_STR_ACHIEVEMENT_37_TITLE, CST_STR_ACHIEVEMENT_37_DESCR },
	{ CST_STR_ACHIEVEMENT_38_TITLE__ASCII, CST_STR_ACHIEVEMENT_38_DESCR__ASCII, CST_STR_ACHIEVEMENT_38_TITLE, CST_STR_ACHIEVEMENT_38_DESCR },
	{ CST_STR_ACHIEVEMENT_39_TITLE__ASCII, CST_STR_ACHIEVEMENT_39_DESCR__ASCII, CST_STR_ACHIEVEMENT_39_TITLE, CST_STR_ACHIEVEMENT_39_DESCR },
	{ CST_STR_ACHIEVEMENT_40_TITLE__ASCII, CST_STR_ACHIEVEMENT_40_DESCR__ASCII, CST_STR_ACHIEVEMENT_40_TITLE, CST_STR_ACHIEVEMENT_40_DESCR },
	{ CST_STR_ACHIEVEMENT_41_TITLE__ASCII, CST_STR_ACHIEVEMENT_41_DESCR__ASCII, CST_STR_ACHIEVEMENT_41_TITLE, CST_STR_ACHIEVEMENT_41_DESCR },
	{ CST_STR_ACHIEVEMENT_42_TITLE__ASCII, CST_STR_ACHIEVEMENT_42_DESCR__ASCII, CST_STR_ACHIEVEMENT_42_TITLE, CST_STR_ACHIEVEMENT_42_DESCR },
	{ CST_STR_ACHIEVEMENT_43_TITLE__ASCII, CST_STR_ACHIEVEMENT_43_DESCR__ASCII, CST_STR_ACHIEVEMENT_43_TITLE, CST_STR_ACHIEVEMENT_43_DESCR },
	{ CST_STR_ACHIEVEMENT_44_TITLE__ASCII, CST_STR_ACHIEVEMENT_44_DESCR__ASCII, CST_STR_ACHIEVEMENT_44_TITLE, CST_STR_ACHIEVEMENT_44_DESCR },
	{ CST_STR_ACHIEVEMENT_45_TITLE__ASCII, CST_STR_ACHIEVEMENT_45_DESCR__ASCII, CST_STR_ACHIEVEMENT_45_TITLE, CST_STR_ACHIEVEMENT_45_DESCR },
	{ CST_STR_ACHIEVEMENT_46_TITLE__ASCII, CST_STR_ACHIEVEMENT_46_DESCR__ASCII, CST_STR_ACHIEVEMENT_46_TITLE, CST_STR_ACHIEVEMENT_46_DESCR },
	{ CST_STR_ACHIEVEMENT_47_TITLE__ASCII, CST_STR_ACHIEVEMENT_47_DESCR__ASCII, CST_STR_ACHIEVEMENT_47_TITLE, CST_STR_ACHIEVEMENT_47_DESCR },
	{ CST_STR_ACHIEVEMENT_48_TITLE__ASCII, CST_STR_ACHIEVEMENT_48_DESCR__ASCII, CST_STR_ACHIEVEMENT_48_TITLE, CST_STR_ACHIEVEMENT_48_DESCR },
	{ CST_STR_ACHIEVEMENT_49_TITLE__ASCII, CST_STR_ACHIEVEMENT_49_DESCR__ASCII, CST_STR_ACHIEVEMENT_49_TITLE, CST_STR_ACHIEVEMENT_49_DESCR },
	{ CST_STR_ACHIEVEMENT_50_TITLE__ASCII, CST_STR_ACHIEVEMENT_50_DESCR__ASCII, CST_STR_ACHIEVEMENT_50_TITLE, CST_STR_ACHIEVEMENT_50_DESCR },
	{ CST_STR_ACHIEVEMENT_51_TITLE__ASCII, CST_STR_ACHIEVEMENT_51_DESCR__ASCII, CST_STR_ACHIEVEMENT_51_TITLE, CST_STR_ACHIEVEMENT_51_DESCR },
	{ CST_STR_ACHIEVEMENT_52_TITLE__ASCII, CST_STR_ACHIEVEMENT_52_DESCR__ASCII, CST_STR_ACHIEVEMENT_52_TITLE, CST_STR_ACHIEVEMENT_52_DESCR },
	{ CST_STR_ACHIEVEMENT_53_TITLE__ASCII, CST_STR_ACHIEVEMENT_53_DESCR__ASCII, CST_STR_ACHIEVEMENT_53_TITLE, CST_STR_ACHIEVEMENT_53_DESCR },
	{ CST_STR_ACHIEVEMENT_54_TITLE__ASCII, CST_STR_ACHIEVEMENT_54_DESCR__ASCII, CST_STR_ACHIEVEMENT_54_TITLE, CST_STR_ACHIEVEMENT_54_DESCR },
	{ CST_STR_ACHIEVEMENT_55_TITLE__ASCII, CST_STR_ACHIEVEMENT_55_DESCR__ASCII, CST_STR_ACHIEVEMENT_55_TITLE, CST_STR_ACHIEVEMENT_55_DESCR },
	{ CST_STR_ACHIEVEMENT_56_TITLE__ASCII, CST_STR_ACHIEVEMENT_56_DESCR__ASCII, CST_STR_ACHIEVEMENT_56_TITLE, CST_STR_ACHIEVEMENT_56_DESCR },
	{ CST_STR_ACHIEVEMENT_57_TITLE__ASCII, CST_STR_ACHIEVEMENT_57_DESCR__ASCII, CST_STR_ACHIEVEMENT_57_TITLE, CST_STR_ACHIEVEMENT_57_DESCR },
	{ CST_STR_ACHIEVEMENT_58_TITLE__ASCII, CST_STR_ACHIEVEMENT_58_DESCR__ASCII, CST_STR_ACHIEVEMENT_58_TITLE, CST_STR_ACHIEVEMENT_58_DESCR },
	{ CST_STR_ACHIEVEMENT_59_TITLE__ASCII, CST_STR_ACHIEVEMENT_59_DESCR__ASCII, CST_STR_ACHIEVEMENT_59_TITLE, CST_STR_ACHIEVEMENT_59_DESCR },
	{ CST_STR_ACHIEVEMENT_60_TITLE__ASCII, CST_STR_ACHIEVEMENT_60_DESCR__ASCII, CST_STR_ACHIEVEMENT_60_TITLE, CST_STR_ACHIEVEMENT_60_DESCR },
	{ CST_STR_ACHIEVEMENT_61_TITLE__ASCII, CST_STR_ACHIEVEMENT_61_DESCR__ASCII, CST_STR_ACHIEVEMENT_61_TITLE, CST_STR_ACHIEVEMENT_61_DESCR },
	{ CST_STR_ACHIEVEMENT_62_TITLE__ASCII, CST_STR_ACHIEVEMENT_62_DESCR__ASCII, CST_STR_ACHIEVEMENT_62_TITLE, CST_STR_ACHIEVEMENT_62_DESCR },
	{ CST_STR_ACHIEVEMENT_63_TITLE__ASCII, CST_STR_ACHIEVEMENT_63_DESCR__ASCII, CST_STR_ACHIEVEMENT_63_TITLE, CST_STR_ACHIEVEMENT_63_DESCR },
	{ CST_STR_ACHIEVEMENT_64_TITLE__ASCII, CST_STR_ACHIEVEMENT_64_DESCR__ASCII, CST_STR_ACHIEVEMENT_64_TITLE, CST_STR_ACHIEVEMENT_64_DESCR },
	{ CST_STR_ACHIEVEMENT_65_TITLE__ASCII, CST_STR_ACHIEVEMENT_65_DESCR__ASCII, CST_STR_ACHIEVEMENT_65_TITLE, CST_STR_ACHIEVEMENT_65_DESCR }
};
//#modified-fva; END

/*
========================
idAchievementSystemWin::idAchievementSystemWin
========================
*/
idAchievementSystemWin::idAchievementSystemWin() {
}

/*
========================
idAchievementSystemWin::IsInitialized
========================
*/
bool idAchievementSystemWin::IsInitialized() {
	//#modified-fva; BEGIN
	//return false;
	return true;
	//#modified-fva; END
}

/*
================================
idAchievementSystemWin::AchievementUnlock
================================
*/
//#modified-fva; BEGIN
/*
void idAchievementSystemWin::AchievementUnlock( idLocalUser * user, int achievementID ) {
}
*/
void idAchievementSystemWin::AchievementUnlock(idLocalUser * user, int achievementID, bool cstNotify) {
	if (!user || achievementID < 0 || achievementID >= idAchievementSystem::MAX_ACHIEVEMENTS) {
		return;
	}
	idPlayerProfile *profile = user->GetProfile();
	if (!profile) {
		return;
	}
	if (profile->GetAchievement(achievementID)) {
		return; // already unlocked
	}
	profile->SetAchievement(achievementID);
	profile->SaveSettings(true);

	if (cstNotify && achievementID > 0 && achievementID < sizeof(cstAchievementInfo) / sizeof(cstAchievementInfo[0])) {
		idStr cmd = "cstPrintAchievement ";
		cmd += achievementID;
		cmd += '\n';
		cmdSystem->AppendCommandText(cmd.c_str());
	}
}
//#modified-fva; END

/*
========================
idAchievementSystemWin::AchievementLock
========================
*/
void idAchievementSystemWin::AchievementLock( idLocalUser * user, const int achievementID ) {
	//#modified-fva; BEGIN
	if (!user || achievementID < 0 || achievementID >= idAchievementSystem::MAX_ACHIEVEMENTS) {
		return;
	}
	idPlayerProfile *profile = user->GetProfile();
	if (!profile) {
		return;
	}
	profile->ClearAchievement(achievementID);
	profile->SaveSettings(true);
	//#modified-fva; END
}

/*
========================
idAchievementSystemWin::AchievementLockAll
========================
*/
void idAchievementSystemWin::AchievementLockAll( idLocalUser * user, const int maxId ) {
	//#modified-fva; BEGIN
	if (!user) {
		return;
	}
	idPlayerProfile *profile = user->GetProfile();
	if (!profile) {
		return;
	}
	int actualMax = (maxId < 0 || maxId >= idAchievementSystem::MAX_ACHIEVEMENTS) ? (idAchievementSystem::MAX_ACHIEVEMENTS - 1) : maxId;
	for (int i = 0; i <= actualMax; ++i) {
		profile->ClearAchievement(i);
	}
	profile->SaveSettings(true);
	//#modified-fva; END
}

/*
========================
idAchievementSystemWin::GetAchievementDescription
========================
*/
//#modified-fva; BEGIN
/*
bool idAchievementSystemWin::GetAchievementDescription( idLocalUser * user, const int achievementID, achievementDescription_t & data ) const {
	return false;
}
*/
bool idAchievementSystemWin::GetAchievementDescription(const int achievementID, achievementDescription_t & data, bool getLocalizationKeys) const {
	data.Clear();
	if (achievementID < 1 || achievementID > 65) {
		return false;
	}

	const char *title = NULL;
	if (getLocalizationKeys) {
		title = cstAchievementInfo[achievementID].title_localizationKey;
	} else {
		title = cstAchievementInfo[achievementID].title_ascii;
	}
	int titleLength = strlen(title);
	if (titleLength <= sizeof(data.name) - 1) {
		idStr::Copynz(data.name, title, titleLength + 1);
	} else {
		idStr::Copynz(data.name, title, sizeof(data.name));
	}

	const char *description = NULL;
	if (getLocalizationKeys) {
		description = cstAchievementInfo[achievementID].description_localizationKey;
	} else {
		description = cstAchievementInfo[achievementID].description_ascii;
	}
	int descriptionLength = strlen(description);
	if (descriptionLength <= sizeof(data.description) - 1) {
		idStr::Copynz(data.description, description, descriptionLength + 1);
	} else {
		idStr::Copynz(data.description, description, sizeof(data.description));
	}

	return true;
}
//#modified-fva; END

/*
========================
idAchievementSystemWin::GetAchievementState
========================
*/
bool idAchievementSystemWin::GetAchievementState( idLocalUser * user, idArray< bool, idAchievementSystem::MAX_ACHIEVEMENTS > & achievements ) const {
	//#modified-fva; BEGIN
	//return false;

	if (!user) {
		return false;
	}
	idPlayerProfile *profile = user->GetProfile();
	if (!profile) {
		return false;
	}
	for (int i = 0; i < idAchievementSystem::MAX_ACHIEVEMENTS; ++i) {
		achievements[i] = profile->GetAchievement(i);
	}
	return true;
	//#modified-fva; END
}

/*
================================
idAchievementSystemWin::Pump
================================
*/
void idAchievementSystemWin::Pump() {
}
