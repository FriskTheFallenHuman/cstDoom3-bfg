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

/*
================================================================================================

Contains the windows implementation of the network session

================================================================================================
*/

#include "precompiled.h"
#pragma hdrstop
#include "../../framework/Common_local.h"
#include "../sys_session_local.h"
#include "../sys_stats.h"
#include "../sys_savegame.h"
#include "../sys_lobby_backend_direct.h"
#include "../sys_voicechat.h"
#include "win_achievements.h"
#include "win_local.h"

/*
========================
Global variables
========================
*/

extern idCVar net_port;


/*
========================
idSessionLocalWin::idSessionLocalWin
========================
*/
class idSessionLocalWin : public idSessionLocal {
friend class idLobbyToSessionCBLocal;

public:
	idSessionLocalWin();
	virtual ~idSessionLocalWin();

	// idSessionLocal interface
	virtual void		Initialize();
	virtual void		Shutdown();

	virtual void		InitializeSoundRelatedSystems();
	virtual void		ShutdownSoundRelatedSystems();

	virtual void		PlatformPump();

	virtual void		InviteFriends();
	virtual void		InviteParty();
	virtual void		ShowPartySessions();

	virtual void		ShowSystemMarketplaceUI() const;

	virtual void					ListServers( const idCallback & callback );
	virtual void					CancelListServers();
	virtual int						NumServers() const;
	virtual const serverInfo_t *	ServerInfo( int i ) const;
	virtual void					ConnectToServer( int i );
	virtual void					ShowServerGamerCardUI( int i );

	virtual void			ShowLobbyUserGamerCardUI( lobbyUserID_t lobbyUserID );

	virtual void			ShowOnlineSignin() {}
	virtual void			UpdateRichPresence() {}
	virtual void			CheckVoicePrivileges() {}

	virtual bool			ProcessInputEvent( const sysEvent_t * ev );

	// System UI
	virtual bool			IsSystemUIShowing() const;
	virtual void			SetSystemUIShowing( bool show );

	// Invites
	virtual void			HandleBootableInvite( int64 lobbyId = 0 );
	virtual void			ClearBootableInvite();
	virtual void			ClearPendingInvite();

	virtual bool			HasPendingBootableInvite();
	virtual void			SetDiscSwapMPInvite( void * parm );
	virtual void *			GetDiscSwapMPInviteParms();

	virtual void			EnumerateDownloadableContent();

	virtual void 			HandleServerQueryRequest( lobbyAddress_t & remoteAddr, idBitMsg & msg, int msgType );
	virtual void 			HandleServerQueryAck( lobbyAddress_t & remoteAddr, idBitMsg & msg );

	// Leaderboards
	virtual void			LeaderboardUpload( lobbyUserID_t lobbyUserID, const leaderboardDefinition_t * leaderboard, const column_t * stats, const idFile_Memory * attachment = NULL );
	virtual void			LeaderboardDownload( int sessionUserIndex, const leaderboardDefinition_t * leaderboard, int startingRank, int numRows, const idLeaderboardCallback & callback );
	virtual void			LeaderboardDownloadAttachment( int sessionUserIndex, const leaderboardDefinition_t * leaderboard, int64 attachmentID );

	// Scoring (currently just for TrueSkill)
	virtual void			SetLobbyUserRelativeScore( lobbyUserID_t lobbyUserID, int relativeScore, int team ) {}

	virtual void			LeaderboardFlush();

	virtual idNetSessionPort &	GetPort( bool dedicated = false );
	virtual idLobbyBackend *	CreateLobbyBackend( const idMatchParameters & p, float skillLevel, idLobbyBackend::lobbyBackendType_t lobbyType );
	virtual idLobbyBackend *	FindLobbyBackend( const idMatchParameters & p, int numPartyUsers, float skillLevel, idLobbyBackend::lobbyBackendType_t lobbyType );
	virtual idLobbyBackend *	JoinFromConnectInfo( const lobbyConnectInfo_t & connectInfo , idLobbyBackend::lobbyBackendType_t lobbyType );
	virtual void				DestroyLobbyBackend( idLobbyBackend * lobbyBackend );
	virtual void				PumpLobbies();
	virtual void				JoinAfterSwap( void * joinID );

	virtual bool				GetLobbyAddressFromNetAddress( const netadr_t & netAddr, lobbyAddress_t & outAddr ) const;
	virtual bool				GetNetAddressFromLobbyAddress( const lobbyAddress_t & lobbyAddress, netadr_t & outNetAddr ) const;

public:
	void	Connect_f( const idCmdArgs &args );

private:
	void					EnsurePort();

	idLobbyBackend *		CreateLobbyInternal( idLobbyBackend::lobbyBackendType_t lobbyType );

	idArray< idLobbyBackend *, 3 > lobbyBackends;

	idNetSessionPort		port;
	bool					canJoinLocalHost;

	idLobbyToSessionCBLocal	* lobbyToSessionCB;
};

idSessionLocalWin sessionLocalWin;
idSession * session = &sessionLocalWin;

/*
========================
idLobbyToSessionCBLocal
========================
*/
class idLobbyToSessionCBLocal : public idLobbyToSessionCB {
public:
	idLobbyToSessionCBLocal( idSessionLocalWin * sessionLocalWin_ ) : sessionLocalWin( sessionLocalWin_ ) { }

	virtual bool CanJoinLocalHost() const { sessionLocalWin->EnsurePort(); return sessionLocalWin->canJoinLocalHost; }
	virtual class idLobbyBackend * GetLobbyBackend( idLobbyBackend::lobbyBackendType_t type ) const { return sessionLocalWin->lobbyBackends[ type ]; }

private:
	idSessionLocalWin *			sessionLocalWin;
};

idLobbyToSessionCBLocal lobbyToSessionCBLocal( &sessionLocalWin );
idLobbyToSessionCB * lobbyToSessionCB = &lobbyToSessionCBLocal;

class idVoiceChatMgrWin : public idVoiceChatMgr {
public:
	//#modified-fva; BEGIN
	virtual ~idVoiceChatMgrWin() {}
	//#modified-fva; END

	virtual bool	GetLocalChatDataInternal( int talkerIndex, byte * data, int & dataSize ) { return false; }
	virtual void	SubmitIncomingChatDataInternal( int talkerIndex, const byte * data, int dataSize ) { }
	virtual bool	TalkerHasData( int talkerIndex ) { return false; }
	virtual bool	RegisterTalkerInternal( int index ) { return true; }
	virtual void	UnregisterTalkerInternal( int index ) { }
};

/*
========================
idSessionLocalWin::idSessionLocalWin
========================
*/
idSessionLocalWin::idSessionLocalWin() {
	signInManager		= new (TAG_SYSTEM) idSignInManagerWin;
	saveGameManager		= new (TAG_SAVEGAMES) idSaveGameManager();
	voiceChat			= new (TAG_SYSTEM) idVoiceChatMgrWin();
	lobbyToSessionCB	= new (TAG_SYSTEM) idLobbyToSessionCBLocal( this );

	canJoinLocalHost	= false;

	lobbyBackends.Zero();
}

/*
========================
idSessionLocalWin::idSessionLocalWin
========================
*/
idSessionLocalWin::~idSessionLocalWin() {
	delete voiceChat;
	delete lobbyToSessionCB;
}

/*
========================
idSessionLocalWin::Initialize
========================
*/
void idSessionLocalWin::Initialize() {
	idSessionLocal::Initialize();

	// The shipping path doesn't load title storage
	// Instead, we inject values through code which is protected through steam DRM
	titleStorageVars.Set( "MAX_PLAYERS_ALLOWED", "8" );
	titleStorageLoaded = true;

	// First-time check for downloadable content once game is launched
	EnumerateDownloadableContent();

	GetPartyLobby().Initialize( idLobby::TYPE_PARTY, sessionCallbacks );
	GetGameLobby().Initialize( idLobby::TYPE_GAME, sessionCallbacks );
	GetGameStateLobby().Initialize( idLobby::TYPE_GAME_STATE, sessionCallbacks );

	achievementSystem = new (TAG_SYSTEM) idAchievementSystemWin();
	achievementSystem->Init();
}

/*
========================
idSessionLocalWin::Shutdown
========================
*/
void idSessionLocalWin::Shutdown() {
	NET_VERBOSE_PRINT( "NET: Shutdown\n" );
	idSessionLocal::Shutdown();

	MoveToMainMenu();

	// Wait until we fully shutdown
	while ( localState != STATE_IDLE && localState != STATE_PRESS_START ) {
		Pump();
	}

	if ( achievementSystem != NULL ) {
		achievementSystem->Shutdown();
		delete achievementSystem;
		achievementSystem = NULL;
	}
}

/*
========================
idSessionLocalWin::InitializeSoundRelatedSystems
========================
*/
void idSessionLocalWin::InitializeSoundRelatedSystems() {
	if ( voiceChat != NULL ) {
		voiceChat->Init( NULL );
	}
}

/*
========================
idSessionLocalWin::ShutdownSoundRelatedSystems
========================
*/
void idSessionLocalWin::ShutdownSoundRelatedSystems() {
	if ( voiceChat != NULL ) {
		voiceChat->Shutdown();
	}
}

/*
========================
idSessionLocalWin::PlatformPump
========================
*/
void idSessionLocalWin::PlatformPump() {
}

/*
========================
idSessionLocalWin::InviteFriends
========================
*/
void idSessionLocalWin::InviteFriends() {
}

/*
========================
idSessionLocalWin::InviteParty
========================
*/
void idSessionLocalWin::InviteParty() {
}

/*
========================
idSessionLocalWin::ShowPartySessions
========================
*/
void idSessionLocalWin::ShowPartySessions() {
}

/*
========================
idSessionLocalWin::ShowSystemMarketplaceUI
========================
*/
void idSessionLocalWin::ShowSystemMarketplaceUI() const {
}

/*
========================
idSessionLocalWin::ListServers
========================
*/
void idSessionLocalWin::ListServers( const idCallback & callback ) {
	//#modified-fva; BEGIN
	//ListServersCommon();
	idCallback *cstCallback = callback.Clone();
	if (cstCallback) {
		cstCallback->Call();
		delete cstCallback;
		cstCallback = NULL;
	}
	//#modified-fva; END
}

/*
========================
idSessionLocalWin::CancelListServers
========================
*/
void idSessionLocalWin::CancelListServers() {
}

/*
========================
idSessionLocalWin::NumServers
========================
*/
int idSessionLocalWin::NumServers() const {
	return 0;
}

/*
========================
idSessionLocalWin::ServerInfo
========================
*/
const serverInfo_t * idSessionLocalWin::ServerInfo( int i ) const {
	return NULL;
}

/*
========================
idSessionLocalWin::ConnectToServer
========================
*/
void idSessionLocalWin::ConnectToServer( int i ) {
}

/*
========================
idSessionLocalWin::Connect_f
========================
*/
//#modified-fva; BEGIN
/*
void idSessionLocalWin::Connect_f( const idCmdArgs &args ) {
	if ( args.Argc() < 2 ) {
		idLib::Printf( "Usage: Connect to IP.  Use with net_port. \n");
		return;
	}

	Cancel();

	if ( signInManager->GetMasterLocalUser() == NULL ) {
		signInManager->RegisterLocalUser( 0 );
	}

	lobbyConnectInfo_t connectInfo;

	Sys_StringToNetAdr( args.Argv(1), &connectInfo.netAddr, true );
	connectInfo.netAddr.port = net_port.GetInteger();

	ConnectAndMoveToLobby( GetPartyLobby(), connectInfo, false );
}
*/
void idSessionLocalWin::Connect_f(const idCmdArgs &args) {
	if (args.Argc() < 2) {
		const char * usage =
			"Connect to the specified IP and port. Usage:\n"
			"\n"
			" connect <ip>\n"
			" connect <ip> <port>\n"
			"\n"
			"If <port> isn't specified, the default port (%s) is used.\n";
		idLib::Printf(usage, net_port.GetDefaultString());
		return;
	}

	lobbyConnectInfo_t connectInfo;
	if (!Sys_StringToNetAdr(args.Argv(1), &connectInfo.netAddr, true)) {
		idLib::Printf("Invalid IP.\n");
		return;
	}
	if (args.Argc() > 2) {
		int port = atoi(args.Argv(2));
		if (port < 0 || port > 65535) {
			idLib::Printf("Invalid port.\n");
			return;
		}
		connectInfo.netAddr.port = (unsigned short)port;
	} else {
		connectInfo.netAddr.port = (unsigned short)atoi(net_port.GetDefaultString());
	}
	idLib::Printf("Connecting to %s\n", Sys_NetAdrToString(connectInfo.netAddr));

	Cancel();
	common->CstSetServerDedicated(false);

	if (signInManager->GetMasterLocalUser() == NULL) {
		signInManager->RegisterLocalUser(0);
	}

	ConnectAndMoveToLobby(GetPartyLobby(), connectInfo, false);
}
//#modified-fva; END

/*
========================
void Connect_f
========================
*/
//#modified-fva; BEGIN
//CONSOLE_COMMAND( connect, "Connect to the specified IP", NULL ) {
CONSOLE_COMMAND_SHIP(connect, "Connect to the specified IP and port", NULL) {
//#modified-fva; END
	sessionLocalWin.Connect_f( args );
}

/*
========================
idSessionLocalWin::ShowServerGamerCardUI
========================
*/
void idSessionLocalWin::ShowServerGamerCardUI( int i ) {
}

/*
========================
idSessionLocalWin::ShowLobbyUserGamerCardUI(
========================
*/
void idSessionLocalWin::ShowLobbyUserGamerCardUI( lobbyUserID_t lobbyUserID ) {
}

/*
========================
idSessionLocalWin::ProcessInputEvent
========================
*/
bool idSessionLocalWin::ProcessInputEvent( const sysEvent_t * ev ) {
	if ( GetSignInManager().ProcessInputEvent( ev ) ) {
		return true;
	}
	return false;
}

/*
========================
idSessionLocalWin::IsSystemUIShowing
========================
*/
bool idSessionLocalWin::IsSystemUIShowing() const {
	return !win32.activeApp || isSysUIShowing; // If the user alt+tabs away, treat it the same as bringing up the steam overlay
}

/*
========================
idSessionLocalWin::SetSystemUIShowing
========================
*/
void idSessionLocalWin::SetSystemUIShowing( bool show ) {
	isSysUIShowing = show;
}

/*
========================
idSessionLocalWin::HandleServerQueryRequest
========================
*/
void idSessionLocalWin::HandleServerQueryRequest( lobbyAddress_t & remoteAddr, idBitMsg & msg, int msgType ) {
	NET_VERBOSE_PRINT( "HandleServerQueryRequest from %s\n", remoteAddr.ToString() );
}

/*
========================
idSessionLocalWin::HandleServerQueryAck
========================
*/
void idSessionLocalWin::HandleServerQueryAck( lobbyAddress_t & remoteAddr, idBitMsg & msg ) {
	NET_VERBOSE_PRINT( "HandleServerQueryAck from %s\n", remoteAddr.ToString() );

}

/*
========================
idSessionLocalWin::ClearBootableInvite
========================
*/
void idSessionLocalWin::ClearBootableInvite() {
}

/*
========================
idSessionLocalWin::ClearPendingInvite
========================
*/
void idSessionLocalWin::ClearPendingInvite() {
}

/*
========================
idSessionLocalWin::HandleBootableInvite
========================
*/
void idSessionLocalWin::HandleBootableInvite( int64 lobbyId ) {
}

/*
========================
idSessionLocalWin::HasPendingBootableInvite
========================
*/
bool idSessionLocalWin::HasPendingBootableInvite() {
	return false;
}

/*
========================
idSessionLocal::SetDiscSwapMPInvite
========================
*/
void idSessionLocalWin::SetDiscSwapMPInvite( void * parm ) {
}

/*
========================
idSessionLocal::GetDiscSwapMPInviteParms
========================
*/
void * idSessionLocalWin::GetDiscSwapMPInviteParms() {
	return NULL;
}

/*
========================
idSessionLocalWin::EnumerateDownloadableContent
========================
*/
void idSessionLocalWin::EnumerateDownloadableContent() {
}

/*
========================
idSessionLocalWin::LeaderboardUpload
========================
*/
void idSessionLocalWin::LeaderboardUpload( lobbyUserID_t lobbyUserID, const leaderboardDefinition_t * leaderboard, const column_t * stats, const idFile_Memory * attachment ) {
}

/*
========================
idSessionLocalWin::LeaderboardFlush
========================
*/
void idSessionLocalWin::LeaderboardFlush() {
}

/*
========================
idSessionLocalWin::LeaderboardDownload
========================
*/
void idSessionLocalWin::LeaderboardDownload( int sessionUserIndex, const leaderboardDefinition_t * leaderboard, int startingRank, int numRows, const idLeaderboardCallback & callback ) {
}

/*
========================
idSessionLocalWin::LeaderboardDownloadAttachment
========================
*/
void idSessionLocalWin::LeaderboardDownloadAttachment( int sessionUserIndex, const leaderboardDefinition_t * leaderboard, int64 attachmentID ) {
}

/*
========================
idSessionLocalWin::EnsurePort
========================
*/
void idSessionLocalWin::EnsurePort() {
	// Init the port using reqular windows sockets
	if ( port.IsOpen() ) {
		return;		// Already initialized
	}

	if ( port.InitPort( net_port.GetInteger(), false ) ) {
		canJoinLocalHost = false;
	} else {
		// Assume this is another instantiation on the same machine, and just init using any available port
		port.InitPort( PORT_ANY, false );
		canJoinLocalHost = true;
	}
}

/*
========================
idSessionLocalWin::GetPort
========================
*/
idNetSessionPort & idSessionLocalWin::GetPort( bool dedicated ) {
	EnsurePort();
	return port;
}

/*
========================
idSessionLocalWin::CreateLobbyBackend
========================
*/
idLobbyBackend * idSessionLocalWin::CreateLobbyBackend( const idMatchParameters & p, float skillLevel, idLobbyBackend::lobbyBackendType_t lobbyType ) {
	idLobbyBackend * lobbyBackend = CreateLobbyInternal( lobbyType );
	lobbyBackend->StartHosting( p, skillLevel, lobbyType );
	return lobbyBackend;
}

/*
========================
idSessionLocalWin::FindLobbyBackend
========================
*/
idLobbyBackend * idSessionLocalWin::FindLobbyBackend( const idMatchParameters & p, int numPartyUsers, float skillLevel, idLobbyBackend::lobbyBackendType_t lobbyType ) {
	idLobbyBackend * lobbyBackend = CreateLobbyInternal( lobbyType );
	lobbyBackend->StartFinding( p, numPartyUsers, skillLevel );
	return lobbyBackend;
}

/*
========================
idSessionLocalWin::JoinFromConnectInfo
========================
*/
idLobbyBackend * idSessionLocalWin::JoinFromConnectInfo( const lobbyConnectInfo_t & connectInfo, idLobbyBackend::lobbyBackendType_t lobbyType ) {
	idLobbyBackend * lobbyBackend = CreateLobbyInternal( lobbyType );
	lobbyBackend->JoinFromConnectInfo( connectInfo );
	return lobbyBackend;
}

/*
========================
idSessionLocalWin::DestroyLobbyBackend
========================
*/
void idSessionLocalWin::DestroyLobbyBackend( idLobbyBackend * lobbyBackend ) {
    assert( lobbyBackend != NULL );
    assert( lobbyBackends[lobbyBackend->GetLobbyType()] == lobbyBackend );

	lobbyBackends[lobbyBackend->GetLobbyType()] = NULL;

	lobbyBackend->Shutdown();
	delete lobbyBackend;
}

/*
========================
idSessionLocalWin::PumpLobbies
========================
*/
void idSessionLocalWin::PumpLobbies() {
	assert( lobbyBackends[idLobbyBackend::TYPE_PARTY] == NULL || lobbyBackends[idLobbyBackend::TYPE_PARTY]->GetLobbyType() == idLobbyBackend::TYPE_PARTY );
	assert( lobbyBackends[idLobbyBackend::TYPE_GAME] == NULL || lobbyBackends[idLobbyBackend::TYPE_GAME]->GetLobbyType() == idLobbyBackend::TYPE_GAME );
	assert( lobbyBackends[idLobbyBackend::TYPE_GAME_STATE] == NULL || lobbyBackends[idLobbyBackend::TYPE_GAME_STATE]->GetLobbyType() == idLobbyBackend::TYPE_GAME_STATE );

	// Pump lobbyBackends
	for ( int i = 0; i < lobbyBackends.Num(); i++ ) {
		if ( lobbyBackends[i] != NULL ) {
			lobbyBackends[i]->Pump();
		}
	}
}

/*
========================
idSessionLocalWin::CreateLobbyInternal
========================
*/
idLobbyBackend * idSessionLocalWin::CreateLobbyInternal( idLobbyBackend::lobbyBackendType_t lobbyType ) {
	EnsurePort();
	idLobbyBackend * lobbyBackend = new (TAG_NETWORKING) idLobbyBackendDirect();

	lobbyBackend->SetLobbyType( lobbyType );

	assert( lobbyBackends[lobbyType] == NULL );
	lobbyBackends[lobbyType] = lobbyBackend;

	return lobbyBackend;
}

/*
========================
idSessionLocalWin::JoinAfterSwap
========================
*/
void idSessionLocalWin::JoinAfterSwap( void * joinID ) {
}

/*
========================
idSessionLocalWin::GetLobbyAddressFromNetAddress
========================
*/
bool idSessionLocalWin::GetLobbyAddressFromNetAddress( const netadr_t & netAddr, lobbyAddress_t & outAddr ) const {
	return false;
}

/*
========================
idSessionLocalWin::GetNetAddressFromLobbyAddress
========================
*/
bool idSessionLocalWin::GetNetAddressFromLobbyAddress( const lobbyAddress_t & lobbyAddress, netadr_t & outNetAddr ) const {
	return false;
}
