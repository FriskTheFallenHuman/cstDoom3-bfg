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

#include "Game_local.h"
#include "..\..\doomclassic\doom\doomdef.h"
//#modified-fva; BEGIN
#include "_CstStrings.h"
//#modified-fva; END

idCVar achievements_Verbose( "achievements_Verbose", "1", CVAR_BOOL, "debug spam" );
idCVar g_demoMode( "g_demoMode", "0", CVAR_INTEGER, "this is a demo" );

bool idAchievementManager::cheatingDialogShown = false;

//#modified-fva; BEGIN
/*
const struct achievementInfo_t {
	int required;
	bool lifetime; // true means the current count is stored on the player profile.  Doesn't matter for single count achievements.
} achievementInfo [ACHIEVEMENTS_NUM] = {
	{ 50, true }, // ACHIEVEMENT_EARN_ALL_50_TROPHIES
	{ 1, true }, // ACHIEVEMENT_COMPLETED_DIFFICULTY_0
	{ 1, true }, // ACHIEVEMENT_COMPLETED_DIFFICULTY_1
	{ 1, true }, // ACHIEVEMENT_COMPLETED_DIFFICULTY_2
	{ 1, true }, // ACHIEVEMENT_COMPLETED_DIFFICULTY_3
	{ 64, false }, // ACHIEVEMENT_PDAS_BASE
	{ 14, false }, // ACHIEVEMENT_WATCH_ALL_VIDEOS
	{ 1, false }, // ACHIEVEMENT_KILL_MONSTER_WITH_1_HEALTH_LEFT
	{ 35, false }, // ACHIEVEMENT_OPEN_ALL_LOCKERS
	{ 20, true }, // ACHIEVEMENT_KILL_20_ENEMY_FISTS_HANDS
	{ 1, true }, // ACHIEVEMENT_KILL_SCI_NEXT_TO_RCR
	{ 1, true }, // ACHIEVEMENT_KILL_TWO_IMPS_ONE_SHOTGUN
	{ 1, true }, // ACHIEVEMENT_SCORE_25000_TURKEY_PUNCHER
	{ 50, true }, // ACHIEVEMENT_DESTROY_BARRELS
	{ 1, true }, // ACHIEVEMENT_GET_BFG_FROM_SECURITY_OFFICE
	{ 1, true }, // ACHIEVEMENT_COMPLETE_LEVEL_WITHOUT_TAKING_DMG
	{ 1, true }, // ACHIEVEMENT_FIND_RAGE_LOGO
	{ 1, true }, // ACHIEVEMENT_SPEED_RUN
	{ 1, true }, // ACHIEVEMENT_DEFEAT_VAGARY_BOSS
	{ 1, true }, // ACHIEVEMENT_DEFEAT_GUARDIAN_BOSS
	{ 1, true }, // ACHIEVEMENT_DEFEAT_SABAOTH_BOSS
	{ 1, true }, // ACHIEVEMENT_DEFEAT_CYBERDEMON_BOSS
	{ 1, true }, // ACHIEVEMENT_SENTRY_BOT_ALIVE_TO_DEST
	{ 20, true }, // ACHIEVEMENT_KILL_20_ENEMY_WITH_CHAINSAW
	{ 1, true }, // ACHIEVEMENT_ID_LOGO_SECRET_ROOM
	{ 1, true }, // ACHIEVEMENT_BLOODY_HANDWORK_OF_BETRUGER
	{ 1, true }, // ACHIEVEMENT_TWO_DEMONS_FIGHT_EACH_OTHER
	{ 20, true }, // ACHIEVEMENT_USE_SOUL_CUBE_TO_DEFEAT_20_ENEMY
	{ 1, true }, // ACHIEVEMENT_ROE_COMPLETED_DIFFICULTY_0
	{ 1, true }, // ACHIEVEMENT_ROE_COMPLETED_DIFFICULTY_1
	{ 1, true }, // ACHIEVEMENT_ROE_COMPLETED_DIFFICULTY_2
	{ 1, true }, // ACHIEVEMENT_ROE_COMPLETED_DIFFICULTY_3
	{ 22, false }, // ACHIEVEMENT_PDAS_ROE
	{ 1, true }, // ACHIEVEMENT_KILL_5_ENEMY_HELL_TIME
	{ 1, true }, // ACHIEVEMENT_DEFEAT_HELLTIME_HUNTER
	{ 1, true }, // ACHIEVEMENT_DEFEAT_BERSERK_HUNTER
	{ 1, true }, // ACHIEVEMENT_DEFEAT_INVULNERABILITY_HUNTER
	{ 1, true }, // ACHIEVEMENT_DEFEAT_MALEDICT_BOSS
	{ 20, true }, // ACHIEVEMENT_GRABBER_KILL_20_ENEMY
	{ 20, true }, // ACHIEVEMENT_ARTIFACT_WITH_BERSERK_PUNCH_20
	{ 1, true }, // ACHIEVEMENT_LE_COMPLETED_DIFFICULTY_0
	{ 1, true }, // ACHIEVEMENT_LE_COMPLETED_DIFFICULTY_1
	{ 1, true }, // ACHIEVEMENT_LE_COMPLETED_DIFFICULTY_2
	{ 1, true }, // ACHIEVEMENT_LE_COMPLETED_DIFFICULTY_3
	{ 10, false }, // ACHIEVEMENT_PDAS_LE
	{ 1, true }, // ACHIEVEMENT_MP_KILL_PLAYER_VIA_TELEPORT
	{ 1, true }, // ACHIEVEMENT_MP_CATCH_ENEMY_IN_ROFC
	{ 5, true }, // ACHIEVEMENT_MP_KILL_5_PLAYERS_USING_INVIS
	{ 1, true }, // ACHIEVEMENT_MP_COMPLETE_MATCH_WITHOUT_DYING
	{ 1, true }, // ACHIEVEMENT_MP_USE_BERSERK_TO_KILL_PLAYER
	{ 1, true }, // ACHIEVEMENT_MP_KILL_2_GUYS_IN_ROOM_WITH_BFG
};
*/
enum cstAchievementGameFlag_t {
	CST_AGF_DOOM1		= BIT(0),
	CST_AGF_DOOM2		= BIT(1),
	CST_AGF_DOOM3_BASE	= BIT(2),
	CST_AGF_DOOM3_ROE	= BIT(3),
	CST_AGF_DOOM3_LM	= BIT(4),
	CST_AGF_DOOM3_MP	= BIT(5),
};
const struct achievementInfo_t {
	int		required;
	bool	lifetime;	// true means the current count is stored on the player profile.  Doesn't matter for single count achievements.
	int		game;
} achievementInfo[ACHIEVEMENTS_NUM] = {
	{ 50, true, CST_AGF_DOOM3_BASE | CST_AGF_DOOM3_ROE | CST_AGF_DOOM3_LM | CST_AGF_DOOM3_MP }, // 00 ACHIEVEMENT_EARN_ALL_50_TROPHIES (???)
	{ 1, true, CST_AGF_DOOM3_BASE },											// 01 ACHIEVEMENT_COMPLETED_DIFFICULTY_0
	{ 1, true, CST_AGF_DOOM3_BASE },											// 02 ACHIEVEMENT_COMPLETED_DIFFICULTY_1
	{ 1, true, CST_AGF_DOOM3_BASE },											// 03 ACHIEVEMENT_COMPLETED_DIFFICULTY_2
	{ 1, true, CST_AGF_DOOM3_BASE },											// 04 ACHIEVEMENT_COMPLETED_DIFFICULTY_3
	{ 60, false, CST_AGF_DOOM3_BASE },											// required was 64; 05 ACHIEVEMENT_PDAS_BASE
	{ 15, false, CST_AGF_DOOM3_BASE },											// required was 14; 06 ACHIEVEMENT_WATCH_ALL_VIDEOS
	{ 1, true, CST_AGF_DOOM3_BASE | CST_AGF_DOOM3_ROE | CST_AGF_DOOM3_LM },		// lifetime was false; 07 ACHIEVEMENT_KILL_MONSTER_WITH_1_HEALTH_LEFT
	{ 40, false, CST_AGF_DOOM3_BASE },											// required was 35; 08 ACHIEVEMENT_OPEN_ALL_LOCKERS
	{ 20, true, CST_AGF_DOOM3_BASE },											// 09 ACHIEVEMENT_KILL_20_ENEMY_FISTS_HANDS
	{ 1, true, CST_AGF_DOOM3_BASE },											// 10 ACHIEVEMENT_KILL_SCI_NEXT_TO_RCR
	{ 1, true, CST_AGF_DOOM3_BASE | CST_AGF_DOOM3_ROE | CST_AGF_DOOM3_LM },		// 11 ACHIEVEMENT_KILL_TWO_IMPS_ONE_SHOTGUN
	{ 1, true, CST_AGF_DOOM3_BASE | CST_AGF_DOOM3_ROE },						// 12 ACHIEVEMENT_SCORE_25000_TURKEY_PUNCHER
	{ 50, true, CST_AGF_DOOM3_BASE | CST_AGF_DOOM3_ROE | CST_AGF_DOOM3_LM },	// 13 ACHIEVEMENT_DESTROY_BARRELS
	{ 1, true, CST_AGF_DOOM3_BASE },											// 14 ACHIEVEMENT_GET_BFG_FROM_SECURITY_OFFICE
	{ 1, true, CST_AGF_DOOM3_BASE | CST_AGF_DOOM3_ROE | CST_AGF_DOOM3_LM },		// 15 ACHIEVEMENT_COMPLETE_LEVEL_WITHOUT_TAKING_DMG
	{ 1, true, CST_AGF_DOOM3_LM },												// 16 ACHIEVEMENT_FIND_RAGE_LOGO
	{ 1, true, CST_AGF_DOOM3_BASE },											// 17 ACHIEVEMENT_SPEED_RUN
	{ 1, true, CST_AGF_DOOM3_BASE },											// 18 ACHIEVEMENT_DEFEAT_VAGARY_BOSS
	{ 1, true, CST_AGF_DOOM3_BASE },											// 19 ACHIEVEMENT_DEFEAT_GUARDIAN_BOSS
	{ 1, true, CST_AGF_DOOM3_BASE },											// 20 ACHIEVEMENT_DEFEAT_SABAOTH_BOSS
	{ 1, true, CST_AGF_DOOM3_BASE },											// 21 ACHIEVEMENT_DEFEAT_CYBERDEMON_BOSS
	{ 1, true, CST_AGF_DOOM3_BASE | CST_AGF_DOOM3_ROE | CST_AGF_DOOM3_LM },		// 22 ACHIEVEMENT_SENTRY_BOT_ALIVE_TO_DEST
	{ 20, true, CST_AGF_DOOM3_BASE | CST_AGF_DOOM3_ROE | CST_AGF_DOOM3_LM },	// 23 ACHIEVEMENT_KILL_20_ENEMY_WITH_CHAINSAW
	{ 1, true, CST_AGF_DOOM3_BASE },											// 24 ACHIEVEMENT_ID_LOGO_SECRET_ROOM
	{ 1, true, CST_AGF_DOOM3_BASE },											// 25 ACHIEVEMENT_BLOODY_HANDWORK_OF_BETRUGER
	{ 1, true, CST_AGF_DOOM3_BASE | CST_AGF_DOOM3_ROE | CST_AGF_DOOM3_LM },		// 26 ACHIEVEMENT_TWO_DEMONS_FIGHT_EACH_OTHER
	{ 20, true, CST_AGF_DOOM3_BASE | CST_AGF_DOOM3_LM },						// 27 ACHIEVEMENT_USE_SOUL_CUBE_TO_DEFEAT_20_ENEMY
	{ 1, true, CST_AGF_DOOM3_ROE },												// 28 ACHIEVEMENT_ROE_COMPLETED_DIFFICULTY_0
	{ 1, true, CST_AGF_DOOM3_ROE },												// 29 ACHIEVEMENT_ROE_COMPLETED_DIFFICULTY_1
	{ 1, true, CST_AGF_DOOM3_ROE },												// 30 ACHIEVEMENT_ROE_COMPLETED_DIFFICULTY_2
	{ 1, true, CST_AGF_DOOM3_ROE },												// 31 ACHIEVEMENT_ROE_COMPLETED_DIFFICULTY_3
	{ 22, false, CST_AGF_DOOM3_ROE },											// 32 ACHIEVEMENT_PDAS_ROE
	{ 1, true, CST_AGF_DOOM3_ROE },												// 33 ACHIEVEMENT_KILL_5_ENEMY_HELL_TIME
	{ 1, true, CST_AGF_DOOM3_ROE },												// 34 ACHIEVEMENT_DEFEAT_HELLTIME_HUNTER
	{ 1, true, CST_AGF_DOOM3_ROE },												// 35 ACHIEVEMENT_DEFEAT_BERSERK_HUNTER
	{ 1, true, CST_AGF_DOOM3_ROE },												// 36 ACHIEVEMENT_DEFEAT_INVULNERABILITY_HUNTER
	{ 1, true, CST_AGF_DOOM3_ROE },												// 37 ACHIEVEMENT_DEFEAT_MALEDICT_BOSS
	{ 20, true, CST_AGF_DOOM3_ROE },											// 38 ACHIEVEMENT_GRABBER_KILL_20_ENEMY
	{ 20, true, CST_AGF_DOOM3_ROE },											// 39 ACHIEVEMENT_ARTIFACT_WITH_BERSERK_PUNCH_20
	{ 1, true, CST_AGF_DOOM3_LM },												// 40 ACHIEVEMENT_LE_COMPLETED_DIFFICULTY_0
	{ 1, true, CST_AGF_DOOM3_LM },												// 41 ACHIEVEMENT_LE_COMPLETED_DIFFICULTY_1
	{ 1, true, CST_AGF_DOOM3_LM },												// 42 ACHIEVEMENT_LE_COMPLETED_DIFFICULTY_2
	{ 1, true, CST_AGF_DOOM3_LM },												// 43 ACHIEVEMENT_LE_COMPLETED_DIFFICULTY_3
	{ 10, false, CST_AGF_DOOM3_LM },											// 44 ACHIEVEMENT_PDAS_LE
	{ 1, true, CST_AGF_DOOM3_MP },												// 45 ACHIEVEMENT_MP_KILL_PLAYER_VIA_TELEPORT
	{ 1, true, CST_AGF_DOOM3_MP },												// 46 ACHIEVEMENT_MP_CATCH_ENEMY_IN_ROFC
	{ 5, true, CST_AGF_DOOM3_MP },												// 47 ACHIEVEMENT_MP_KILL_5_PLAYERS_USING_INVIS
	{ 1, true, CST_AGF_DOOM3_MP },												// 48 ACHIEVEMENT_MP_COMPLETE_MATCH_WITHOUT_DYING
	{ 1, true, CST_AGF_DOOM3_MP },												// 49 ACHIEVEMENT_MP_USE_BERSERK_TO_KILL_PLAYER
	{ 1, true, CST_AGF_DOOM3_MP },												// 50 ACHIEVEMENT_MP_KILL_2_GUYS_IN_ROOM_WITH_BFG
	{ 1, true, CST_AGF_DOOM1 },													// 51 ACHIEVEMENT_DOOM1_NEOPHYTE_COMPLETE_ANY_LEVEL
	{ 1, true, CST_AGF_DOOM1 },													// 52 ACHIEVEMENT_DOOM1_EPISODE1_COMPLETE_MEDIUM
	{ 1, true, CST_AGF_DOOM1 },													// 53 ACHIEVEMENT_DOOM1_EPISODE2_COMPLETE_MEDIUM
	{ 1, true, CST_AGF_DOOM1 },													// 54 ACHIEVEMENT_DOOM1_EPISODE3_COMPLETE_MEDIUM
	{ 1, true, CST_AGF_DOOM1 },													// 55 ACHIEVEMENT_DOOM1_EPISODE4_COMPLETE_MEDIUM
	{ 1, true, CST_AGF_DOOM1 },													// 56 ACHIEVEMENT_DOOM1_RAMPAGE_COMPLETE_ALL_HARD
	{ 1, true, CST_AGF_DOOM1 },													// 57 ACHIEVEMENT_DOOM1_NIGHTMARE_COMPLETE_ANY_LEVEL_NIGHTMARE
	{ 1, true, CST_AGF_DOOM1 },													// 58 ACHIEVEMENT_DOOM1_BURNING_OUT_OF_CONTROL_COMPLETE_KILLS_ITEMS_SECRETS
	{ 1, true, CST_AGF_DOOM2 },													// 59 ACHIEVEMENT_DOOM2_JUST_GETTING_STARTED_COMPLETE_ANY_LEVEL
	{ 1, true, CST_AGF_DOOM2 },													// 60 ACHIEVEMENT_DOOM2_FROM_EARTH_TO_HELL_COMPLETE_HELL_ON_EARTH
	{ 1, true, CST_AGF_DOOM2 },													// 61 ACHIEVEMENT_DOOM2_AND_BACK_AGAIN_COMPLETE_NO_REST
	{ 1, true, CST_AGF_DOOM2 },													// 62 ACHIEVEMENT_DOOM2_SUPERIOR_FIREPOWER_COMPLETE_ALL_HARD
	{ 1, true, CST_AGF_DOOM2 },													// 63 ACHIEVEMENT_DOOM2_REALLY_BIG_GUN_FIND_BFG_SINGLEPLAYER
	{ 1, true, CST_AGF_DOOM2 },													// 64 ACHIEVEMENT_DOOM2_BURNING_OUT_OF_CONTROL_COMPLETE_KILLS_ITEMS_SECRETS
	{ 1, true, CST_AGF_DOOM2 }													// 65 ACHIEVEMENT_DOOM2_IMPORTANT_LOOKING_DOOR_FIND_ANY_SECRET
};
//#modified-fva; END

/*
================================================================================================

	idAchievementManager

================================================================================================
*/

/*
========================
idAchievementManager::idAchievementManager
========================
*/
idAchievementManager::idAchievementManager() :
	lastImpKilledTime( 0 ),
	lastPlayerKilledTime( 0 ),
	playerTookDamage( false ) {
	counts.Zero();
	ResetHellTimeKills();
}

/*
========================
idAchievementManager::Init
========================
*/
void idAchievementManager::Init( idPlayer * player ) {
	owner = player;
	SyncAchievments();
}

/*
========================
idAchievementManager::SyncAchievments
========================
*/
void idAchievementManager::SyncAchievments() {
	idLocalUser * user = GetLocalUser();
	if ( user == NULL || user->GetProfile() == NULL ) {
		return;
	}

	// Set achievement counts
	for ( int i = 0; i < counts.Num(); i++ ) {
		//#modified-fva; BEGIN
		/*
		if ( user->GetProfile()->GetAchievement( i ) ) {
			counts[i] = achievementInfo[i].required;
		} else if ( achievementInfo[i].lifetime ) {
			counts[i] = user->GetStatInt( i );
		}
		*/
		if (user->GetProfile()->GetAchievement(i)) {
			if (achievementInfo[i].lifetime || counts[i] > achievementInfo[i].required) {
				counts[i] = achievementInfo[i].required;
			}
		} else {
			if (achievementInfo[i].lifetime) {
				counts[i] = user->GetStatInt(i);
			} else if (counts[i] >= achievementInfo[i].required) {
				counts[i] = achievementInfo[i].required - 1; // will be incremented when the achievement is unlocked
				EventCompletesAchievement((achievement_t)i);
			}
		}
		//#modified-fva; END
	}
}

/*
========================
idAchievementManager::GetLocalUser
========================
*/
idLocalUser * idAchievementManager::GetLocalUser() {
	if ( !verify( owner != NULL ) ) {
		return NULL;
	}
	return session->GetGameLobbyBase().GetLocalUserFromLobbyUser( gameLocal.lobbyUserIDs[ owner->GetEntityNumber() ] );
}

/*
========================
idAchievementManager::Save
========================
*/
void idAchievementManager::Save( idSaveGame * savefile ) const {
	owner.Save( savefile );

	for ( int i = 0; i < ACHIEVEMENTS_NUM; i++ ) {
		savefile->WriteInt( counts[i] );
	}

	savefile->WriteInt( lastImpKilledTime );
	savefile->WriteInt( lastPlayerKilledTime );
	savefile->WriteBool( playerTookDamage );
	savefile->WriteInt( currentHellTimeKills );
}

/*
========================
idAchievementManager::Restore
========================
*/
void idAchievementManager::Restore( idRestoreGame * savefile ) {
	owner.Restore( savefile );

	for ( int i = 0; i < ACHIEVEMENTS_NUM; i++ ) {
		savefile->ReadInt( counts[i] );
	}

	savefile->ReadInt( lastImpKilledTime );
	savefile->ReadInt( lastPlayerKilledTime );
	savefile->ReadBool( playerTookDamage );
	savefile->ReadInt( currentHellTimeKills );

	SyncAchievments();
}

/*
========================
idAchievementManager::EventCompletesAchievement
========================
*/
//#modified-fva; BEGIN
//void idAchievementManager::EventCompletesAchievement( const achievement_t eventId ) {
void idAchievementManager::EventCompletesAchievement(const achievement_t eventId, bool cstNotify) {
//#modified-fva; END
	if ( g_demoMode.GetBool() ) {
		return;
	}

	idLocalUser * localUser = GetLocalUser();
	if ( localUser == NULL || localUser->GetProfile() == NULL ) {

		// Send a Reliable Message to the User that needs to unlock this.
		if ( owner != NULL ) {
			int playerId = owner->entityNumber;
			const int bufferSize = sizeof( playerId ) + sizeof( eventId );
			byte buffer[ bufferSize ];
			idBitMsg msg;
			msg.InitWrite( buffer, bufferSize );

			msg.WriteByte( playerId );
			msg.WriteByte( eventId );

			msg.WriteByteAlign();
			idLib::Printf( "Host Sending Achievement\n");
			session->GetActingGameStateLobbyBase().SendReliableToLobbyUser( gameLocal.lobbyUserIDs[ owner->entityNumber ], GAME_RELIABLE_MESSAGE_ACHIEVEMENT_UNLOCK, msg );
		}

		return; // Remote user or build game
	}

	// Check to see if we've already given the achievement.
	// If so, don't do again because we don't want to autosave every time a trigger is hit
	if ( localUser->GetProfile()->GetAchievement( eventId ) ) {
		return;
	}

	//#modified-fva; BEGIN
	/*
#ifdef ID_RETAIL
	if ( common->GetConsoleUsed() ) {
		if ( !cheatingDialogShown ) {
			common->Dialog().AddDialog( GDM_ACHIEVEMENTS_DISABLED_DUE_TO_CHEATING, DIALOG_ACCEPT, NULL, NULL, true );
			cheatingDialogShown = true;
		}
		return;
	}
#endif
	*/
	//#modified-fva; END

	counts[eventId]++;

	if ( counts[eventId] >= achievementInfo[eventId].required ) {
		//#modified-fva; BEGIN
		//session->GetAchievementSystem().AchievementUnlock( localUser, eventId );
		session->GetAchievementSystem().AchievementUnlock(localUser, eventId, cstNotify);
		//#modified-fva; END
	} else {
		if ( achievementInfo[eventId].lifetime ) {
			localUser->SetStatInt( eventId, counts[eventId] );
		}
	}
}

/*
========================
idAchievementManager::IncrementHellTimeKills
========================
*/
void idAchievementManager::IncrementHellTimeKills() {
	currentHellTimeKills++;
	if ( currentHellTimeKills >= 5 ) {
		EventCompletesAchievement( ACHIEVEMENT_KILL_5_ENEMY_HELL_TIME );
	}
}

/*
========================
idAchievementManager::SavePersistentData
========================
*/
void idAchievementManager::SavePersistentData( idDict & playerInfo ) {
	for ( int i = 0; i < ACHIEVEMENTS_NUM; ++i ) {
		playerInfo.SetInt( va( "ach_%d", i ), counts[i] );
	}
}

/*
========================
idAchievementManager::RestorePersistentData
========================
*/
void idAchievementManager::RestorePersistentData( const idDict & spawnArgs ) {
	for( int i = 0; i < ACHIEVEMENTS_NUM; ++i ) {
		counts[i] = spawnArgs.GetInt( va( "ach_%d", i), "0" );
	}
}


/*
========================
idAchievementManager::LocalUser_CompleteAchievement
========================
*/
void idAchievementManager::LocalUser_CompleteAchievement( achievement_t id ) {
	idLocalUser * localUser = session->GetSignInManager().GetMasterLocalUser();

	// Check to see if we've already given the achievement.
	// If so, don't do again because we don't want to autosave every time a trigger is hit
	if( localUser == NULL || localUser->GetProfile()->GetAchievement( id ) ) {
		return;
	}

	//#modified-fva; BEGIN
	/*
#ifdef ID_RETAIL
	if ( common->GetConsoleUsed() ) {
		if ( !cheatingDialogShown ) {
			common->Dialog().AddDialog( GDM_ACHIEVEMENTS_DISABLED_DUE_TO_CHEATING, DIALOG_ACCEPT, NULL, NULL, true );
			cheatingDialogShown = true;
		}
		return;
	}
#endif
	*/
	//#modified-fva; END

	//#modified-fva; BEGIN
	//session->GetAchievementSystem().AchievementUnlock( localUser, id );
	session->GetAchievementSystem().AchievementUnlock(localUser, id, true);
	//#modified-fva; END
}

/*
========================
idAchievementManager::CheckDoomClassicsAchievements

Processed when the player finishes a level.
========================
*/
void idAchievementManager::CheckDoomClassicsAchievements( int killcount, int itemcount, int secretcount, int skill, int mission, int map, int episode, int totalkills, int totalitems, int totalsecret ) {

	const skill_t difficulty = (skill_t)skill;
	const currentGame_t currentGame = common->GetCurrentGame();
	const GameMission_t expansion = (GameMission_t)mission;


	idLocalUser * localUser = session->GetSignInManager().GetMasterLocalUser();
	if ( localUser != NULL && localUser->GetProfile() != NULL ) {

		// GENERAL ACHIEVEMENT UNLOCKING.
		if( currentGame == DOOM_CLASSIC ) {
			LocalUser_CompleteAchievement( ACHIEVEMENT_DOOM1_NEOPHYTE_COMPLETE_ANY_LEVEL );
		} else if( currentGame == DOOM2_CLASSIC ) {
			LocalUser_CompleteAchievement( ACHIEVEMENT_DOOM2_JUST_GETTING_STARTED_COMPLETE_ANY_LEVEL );
		}

		// Complete Any Level on Nightmare.
		//#modified-fva; BEGIN
		//if ( difficulty == sk_nightmare && currentGame == DOOM_CLASSIC ) {
		if (difficulty >= sk_nightmare && currentGame == DOOM_CLASSIC) {
		//#modified-fva; END
			LocalUser_CompleteAchievement( ACHIEVEMENT_DOOM1_NIGHTMARE_COMPLETE_ANY_LEVEL_NIGHTMARE );
		}

		const bool gotAllKills = killcount >= totalkills;
		const bool gotAllItems = itemcount >= totalitems;
		const bool gotAllSecrets = secretcount >= totalsecret;

		if ( gotAllItems && gotAllKills && gotAllSecrets ) {
			if( currentGame == DOOM_CLASSIC ) {
				LocalUser_CompleteAchievement( ACHIEVEMENT_DOOM1_BURNING_OUT_OF_CONTROL_COMPLETE_KILLS_ITEMS_SECRETS );
			} else if( currentGame == DOOM2_CLASSIC ) {
				LocalUser_CompleteAchievement( ACHIEVEMENT_DOOM2_BURNING_OUT_OF_CONTROL_COMPLETE_KILLS_ITEMS_SECRETS );
			}
		}

		// DOOM EXPANSION ACHIEVEMENTS
		if( expansion == doom ) {

			if(  map == 8 ) {

				// Medium or higher skill level.
				if( difficulty >= sk_medium ) {
					localUser->SetStatInt( STAT_DOOM_COMPLETED_EPISODE_1_MEDIUM + ( episode - 1 ), 1 );
				}

				// Hard or higher skill level.
				if( difficulty >= sk_hard ) {
					localUser->SetStatInt( STAT_DOOM_COMPLETED_EPISODE_1_HARD + ( episode - 1 ), 1 );
					localUser->SetStatInt( STAT_DOOM_COMPLETED_EPISODE_1_MEDIUM + ( episode - 1 ), 1 );
				}

				//#modified-fva; BEGIN
				//if ( difficulty == sk_nightmare ) {
				if (difficulty >= sk_nightmare) {
				//#modified-fva; END
					localUser->SetStatInt( STAT_DOOM_COMPLETED_EPISODE_1_HARD + ( episode - 1 ), 1 );
					localUser->SetStatInt( STAT_DOOM_COMPLETED_EPISODE_1_MEDIUM + ( episode - 1 ), 1 );
				}

				// Save the Settings.
				localUser->SaveProfileSettings();
			}

			// Check to see if we've completed all episodes.
			const int episode1completed = localUser->GetStatInt( STAT_DOOM_COMPLETED_EPISODE_1_MEDIUM );
			const int episode2completed = localUser->GetStatInt( STAT_DOOM_COMPLETED_EPISODE_2_MEDIUM );
			const int episode3completed = localUser->GetStatInt( STAT_DOOM_COMPLETED_EPISODE_3_MEDIUM );
			const int episode4completed = localUser->GetStatInt( STAT_DOOM_COMPLETED_EPISODE_4_MEDIUM );

			const int episode1completed_hard = localUser->GetStatInt( STAT_DOOM_COMPLETED_EPISODE_1_HARD );
			const int episode2completed_hard = localUser->GetStatInt( STAT_DOOM_COMPLETED_EPISODE_2_HARD );
			const int episode3completed_hard = localUser->GetStatInt( STAT_DOOM_COMPLETED_EPISODE_3_HARD );
			const int episode4completed_hard = localUser->GetStatInt( STAT_DOOM_COMPLETED_EPISODE_4_HARD );

			if ( currentGame == DOOM_CLASSIC ) {
				if ( episode1completed ) {
					LocalUser_CompleteAchievement( ACHIEVEMENT_DOOM1_EPISODE1_COMPLETE_MEDIUM );
				}

				if ( episode2completed ) {
					LocalUser_CompleteAchievement( ACHIEVEMENT_DOOM1_EPISODE2_COMPLETE_MEDIUM );
				}

				if ( episode3completed ) {
					LocalUser_CompleteAchievement( ACHIEVEMENT_DOOM1_EPISODE3_COMPLETE_MEDIUM );
				}

				if ( episode4completed ) {
					LocalUser_CompleteAchievement( ACHIEVEMENT_DOOM1_EPISODE4_COMPLETE_MEDIUM );
				}

				if ( episode1completed_hard && episode2completed_hard && episode3completed_hard && episode4completed_hard ) {
					LocalUser_CompleteAchievement( ACHIEVEMENT_DOOM1_RAMPAGE_COMPLETE_ALL_HARD );
				}
			}
		} else if( expansion == doom2 ) {

			if( map == 30 ) {

				if ( currentGame == DOOM2_CLASSIC ) {
					LocalUser_CompleteAchievement( ACHIEVEMENT_DOOM2_FROM_EARTH_TO_HELL_COMPLETE_HELL_ON_EARTH );

					if ( difficulty >= sk_hard ) {
						LocalUser_CompleteAchievement( ACHIEVEMENT_DOOM2_SUPERIOR_FIREPOWER_COMPLETE_ALL_HARD );
					}
				}
			}
		} else if( expansion ==  pack_nerve ) {
			if( map == 8 ) {

				if ( currentGame == DOOM2_CLASSIC ) {
					LocalUser_CompleteAchievement( ACHIEVEMENT_DOOM2_AND_BACK_AGAIN_COMPLETE_NO_REST );
				}
			}
		}

	}
}

//#modified-fva; BEGIN
void idAchievementManager::CstGetAchievementsDetails(idList<cstAchievementDetails_t> & detailsList, bool getLocalizationKeys) {
	detailsList.Clear();

	idPlayer * player = gameLocal.GetLocalPlayer();
	idLocalUser * user = (player == NULL) ? session->GetSignInManager().GetMasterLocalUser() : session->GetGameLobbyBase().GetLocalUserFromLobbyUser(gameLocal.lobbyUserIDs[player->GetEntityNumber()]);
	if (user == NULL) {
		return;
	}
	idPlayerProfile * profile = user->GetProfile();

	for (int i = 1; i < ACHIEVEMENTS_NUM; ++i) {
		const achievementInfo_t &localAchievementInfo = achievementInfo[i];

		int state;
		if (!profile) {
			state = -1; // unknown
		} else if (!profile->GetAchievement(i)) {
			state = 0; // locked
		} else {
			state = 1; // unlocked
		}

		int count = -1;
		if (state == 1) {
			// if unlocked, set count to required independently of the actual count
			count = localAchievementInfo.required;
		} else {
			if (localAchievementInfo.lifetime) {
				// stored in the profile
				count = user->GetStatInt(i);
			} else {
				// stored in savegame files
				if (player != NULL) {
					// doom 3; check only singleplayer as mp achievements are stored in the profile (already tested for lifetime)
					if (!common->IsMultiplayer()) {
						switch (player->GetExpansionType()) {
							case GAME_BASE: {
								if (localAchievementInfo.game & CST_AGF_DOOM3_BASE) {
									count = player->GetAchievementManager().GetCount((achievement_t)i);
								}
								break;
							}
							case GAME_D3XP: {
								if (localAchievementInfo.game & CST_AGF_DOOM3_ROE) {
									count = player->GetAchievementManager().GetCount((achievement_t)i);
								}
								break;
							}
							case GAME_D3LE: {
								if (localAchievementInfo.game & CST_AGF_DOOM3_LM) {
									count = player->GetAchievementManager().GetCount((achievement_t)i);
								}
								break;
							}
						}
					}
				}
				// no need to check doom 1 & 2 as their achievements are stored in the profile (already tested for lifetime)
			}
		}

		achievementDescription_t data;
		bool descriptionValid = session->GetAchievementSystem().GetAchievementDescription(i, data, getLocalizationKeys);

		cstAchievementDetails_t details;
		details.achievementID = i;
		details.name = (descriptionValid && !data.hidden) ? data.name : "";
		details.description = (descriptionValid && !data.hidden) ? data.description : "";
		details.gameDependent = !localAchievementInfo.lifetime;
		details.state = state;
		details.count = count;
		details.required = localAchievementInfo.required;
		detailsList.Append(details);
	}
}
//#modified-fva; END

/*
=================
AchievementsReset
=================
*/
CONSOLE_COMMAND( AchievementsReset, "Lock an achievement", NULL ) {
	idLocalUser * user = session->GetSignInManager().GetMasterLocalUser();
	if ( user == NULL ) {
		idLib::Printf( "Must be signed in\n" );
		return;
	}
	if ( args.Argc() == 1 ) {
		for ( int i = 0; i < ACHIEVEMENTS_NUM; i++ ) {
			user->SetStatInt( i, 0 );
			session->GetAchievementSystem().AchievementLock( user, i );
		}
	} else {
		int i = atoi( args.Argv( 1 ) );
		user->SetStatInt( i, 0 );
		session->GetAchievementSystem().AchievementLock( user, i );
	}
	user->SaveProfileSettings();
}

/*
=================
AchievementsUnlock
=================
*/
CONSOLE_COMMAND( AchievementsUnlock, "Unlock an achievement", NULL ) {
	idLocalUser * user = session->GetSignInManager().GetMasterLocalUser();
	if ( user == NULL ) {
		idLib::Printf( "Must be signed in\n" );
		return;
	}
	if ( args.Argc() == 1 ) {
		for ( int i = 0; i < ACHIEVEMENTS_NUM; i++ ) {
			user->SetStatInt( i, achievementInfo[i].required );
			//#modified-fva; BEGIN
			//session->GetAchievementSystem().AchievementUnlock( user, i );
			session->GetAchievementSystem().AchievementUnlock(user, i, true);
			//#modified-fva; END
		}
	} else {
		int i = atoi( args.Argv( 1 ) );
		user->SetStatInt( i, achievementInfo[i].required );
		//#modified-fva; BEGIN
		//session->GetAchievementSystem().AchievementUnlock( user, i );
		session->GetAchievementSystem().AchievementUnlock(user, i, true);
		//#modified-fva; END
	}
	user->SaveProfileSettings();
}

/*
=================
AchievementsList
=================
*/
CONSOLE_COMMAND( AchievementsList, "Lists achievements and status", NULL ) {
	idPlayer * player = gameLocal.GetLocalPlayer();
	idLocalUser * user = ( player == NULL ) ? session->GetSignInManager().GetMasterLocalUser() : session->GetGameLobbyBase().GetLocalUserFromLobbyUser( gameLocal.lobbyUserIDs[ player->GetEntityNumber() ] );
	if ( user == NULL ) {
		idLib::Printf( "Must be signed in\n" );
		return;
	}
	idPlayerProfile * profile = user->GetProfile();

	idArray<bool, 128> achievementState;
	bool achievementStateValid = session->GetAchievementSystem().GetAchievementState( user, achievementState );

	for ( int i = 0; i < ACHIEVEMENTS_NUM; i++ ) {
		const char * pInfo = "";
		if ( profile == NULL ) {
			pInfo = S_COLOR_RED  "unknown" S_COLOR_DEFAULT;
		} else if ( !profile->GetAchievement( i ) ) {
			pInfo = S_COLOR_YELLOW "locked" S_COLOR_DEFAULT;
		} else {
			pInfo = S_COLOR_GREEN "unlocked" S_COLOR_DEFAULT;
		}
		const char * sInfo = "";
		if ( !achievementStateValid ) {
			sInfo = S_COLOR_RED  "unknown" S_COLOR_DEFAULT;
		} else if ( !achievementState[i] ) {
			sInfo = S_COLOR_YELLOW "locked" S_COLOR_DEFAULT;
		} else {
			sInfo = S_COLOR_GREEN "unlocked" S_COLOR_DEFAULT;
		}
		int count = 0;
		if ( achievementInfo[i].lifetime ) {
			count = user->GetStatInt( i );
		} else if ( player != NULL ) {
			count = player->GetAchievementManager().GetCount( (achievement_t) i );
		} else {
			count = 0;
		}

		achievementDescription_t data;
		//#modified-fva; BEGIN
		//bool descriptionValid = session->GetAchievementSystem().GetAchievementDescription( user, i, data );
		bool descriptionValid = session->GetAchievementSystem().GetAchievementDescription(i, data, false);
		//#modified-fva; END

		idLib::Printf( "%02d: %2d/%2d | %12.12s | %12.12s | %s%s\n", i, count, achievementInfo[i].required, pInfo, sInfo, descriptionValid ? data.hidden ? "(hidden) " : "" : "(unknown) ", descriptionValid ? data.name : "" );
	}
}

//#modified-fva; BEGIN
CONSOLE_COMMAND_SHIP(cstAchievementsReset, "locks the specified set of achievements", NULL) {
	// similar to the AchievementsReset, but user oriented

	const char usage[] =
		"usage:\n"
		" cstAchievementsReset all_sp\n"
		" cstAchievementsReset all_mp\n"
		" cstAchievementsReset all\n";

	idPlayer * player = gameLocal.GetLocalPlayer();
	idLocalUser * user = (player == NULL) ? session->GetSignInManager().GetMasterLocalUser() : session->GetGameLobbyBase().GetLocalUserFromLobbyUser(gameLocal.lobbyUserIDs[player->GetEntityNumber()]);
	if (user == NULL) {
		idLib::Printf("must be signed in\n");
		return;
	}

	if (args.Argc() != 2) {
		idLib::Printf(usage);
		return;
	}
	bool lockSP = false;
	bool lockMP = false;
	if (idStr::Icmp(args.Argv(1), "all_sp") == 0) {
		lockSP = true;
	} else if (idStr::Icmp(args.Argv(1), "all_mp") == 0) {
		lockMP = true;
	} else if (idStr::Icmp(args.Argv(1), "all") == 0) {
		lockSP = true;
		lockMP = true;
	} else {
		idLib::Printf(usage);
		return;
	}

	for (int i = 1; i < ACHIEVEMENTS_NUM; ++i) {
		if ((lockSP && !(achievementInfo[i].game & CST_AGF_DOOM3_MP)) || (lockMP && (achievementInfo[i].game & CST_AGF_DOOM3_MP))) {
			if (achievementInfo[i].lifetime || player == NULL || player->GetAchievementManager().GetCount((achievement_t)i) < achievementInfo[i].required) {
				user->SetStatInt(i, 0);
				session->GetAchievementSystem().AchievementLock(user, i);
			}
		}
	}
	if (lockSP) {
		// these are hidden
		for (int i = STAT_DOOM_COMPLETED_EPISODE_1_MEDIUM; i <= STAT_DOOM_COMPLETED_EPISODE_4_HARD; ++i) {
			user->SetStatInt(i, 0);
			session->GetAchievementSystem().AchievementLock(user, i);
		}
	}
	// lock the unused achievements too
	user->SetStatInt(0, 0);
	session->GetAchievementSystem().AchievementLock(user, 0);
	for (int i = STAT_DOOM_COMPLETED_EPISODE_4_HARD + 1; i < idAchievementSystem::MAX_ACHIEVEMENTS; ++i) {
		user->SetStatInt(i, 0);
		session->GetAchievementSystem().AchievementLock(user, i);
	}

	user->SaveProfileSettings();
}

// ===============
CONSOLE_COMMAND_SHIP(cstAchievementsList, "lists achievements and status", NULL) {
	// similar to the AchievementsList command, but user oriented (shows an interpreted version of the data)

	idList<cstAchievementDetails_t> detailsList;
	idAchievementManager::CstGetAchievementsDetails(detailsList, false);

	if (detailsList.Num() == 0) {
		idLib::Printf("must be signed in\n");
		return;
	}

	for (int i = 0; i < detailsList.Num(); ++i) {
		const cstAchievementDetails_t & details = detailsList[i];

		idStr stateStr;
		if (details.state == 0) {
			stateStr = S_COLOR_YELLOW "locked" S_COLOR_DEFAULT;
		} else if (details.state == 1) {
			stateStr = S_COLOR_GREEN "unlocked" S_COLOR_DEFAULT;
		} else {
			stateStr = S_COLOR_RED "unknown" S_COLOR_DEFAULT;
		}
		idStr countStr = (details.count >= 0) ? idStr(details.count) : "?";
		idStr name = details.name.IsEmpty() ? "unknown" : details.name;

		idLib::Printf("%02d: %2.2s/%2d | %12.12s | %s\n", details.achievementID, countStr.c_str(), details.required, stateStr.c_str(), name.c_str());
	}
}

// ===============
CONSOLE_COMMAND_SHIP(cstPrintAchievement, "prints an achievement message on the console and onscreen", NULL) {
	// this is called when an achievement is unlocked

	if (args.Argc() != 2) {
		return;
	}

	int achievementID = atoi(args.Argv(1));
	if (achievementID < 1 || achievementID >= ACHIEVEMENTS_NUM) {
		return;
	}

	// print on the console
	achievementDescription_t info;
	if (session->GetAchievementSystem().GetAchievementDescription(achievementID, info, false)) {
		idStr msg = CST_STR_ACHIEVEMENT_UNLOCKED__ASCII;
		msg += info.name;
		msg += '\n';
		common->Printf(msg.c_str());
	}

	// print onscreen
	info.Clear();
	if (session->GetAchievementSystem().GetAchievementDescription(achievementID, info, true)) {
		idStr msg = idLocalization::GetString(CST_STR_ACHIEVEMENT_UNLOCKED);
		msg += idLocalization::GetString(info.name);
		common->CstPrintAchievementMsg(msg);
	}
}
//#modified-fva; END
