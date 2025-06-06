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

#include "Common_local.h"
#include "../renderer/Image.h"
#include "../renderer/ImageOpts.h"
#include "../../doomclassic/doom/doomlib.h"
#include "../../doomclassic/doom/globaldata.h"

/*

New for tech4x:

Unlike previous SMP work, the actual GPU command drawing is done in the main thread, which avoids the
OpenGL problems with needing windows to be created by the same thread that creates the context, as well
as the issues with passing context ownership back and forth on the 360.

The game tic and the generation of the draw command list is now run in a separate thread, and overlapped
with the interpretation of the previous draw command list.

While the game tic should be nicely contained, the draw command generation winds through the user interface
code, and is potentially hazardous.  For now, the overlap will be restricted to the renderer back end,
which should also be nicely contained.

*/
#define DEFAULT_FIXED_TIC "0"
#define DEFAULT_NO_SLEEP "0"

idCVar com_deltaTimeClamp( "com_deltaTimeClamp", "50", CVAR_INTEGER, "don't process more than this time in a single frame" );

idCVar com_fixedTic( "com_fixedTic", DEFAULT_FIXED_TIC, CVAR_BOOL, "run a single game frame per render frame" );
//#modified-fva; BEGIN
//idCVar com_noSleep( "com_noSleep", DEFAULT_NO_SLEEP, CVAR_BOOL, "don't sleep if the game is running too fast" );
idCVar com_noSleep("com_noSleep", DEFAULT_NO_SLEEP, CVAR_BOOL | CVAR_NOCHEAT, "don't sleep if the game is running too fast");
//#modified-fva; END
idCVar com_smp( "com_smp", "1", CVAR_BOOL|CVAR_SYSTEM|CVAR_NOCHEAT, "run the game and draw code in a separate thread" );
idCVar com_aviDemoSamples( "com_aviDemoSamples", "16", CVAR_SYSTEM, "" );
idCVar com_aviDemoWidth( "com_aviDemoWidth", "256", CVAR_SYSTEM, "" );
idCVar com_aviDemoHeight( "com_aviDemoHeight", "256", CVAR_SYSTEM, "" );
idCVar com_skipGameDraw( "com_skipGameDraw", "0", CVAR_SYSTEM | CVAR_BOOL, "" );

idCVar com_sleepGame( "com_sleepGame", "0", CVAR_SYSTEM | CVAR_INTEGER, "intentionally add a sleep in the game time" );
idCVar com_sleepDraw( "com_sleepDraw", "0", CVAR_SYSTEM | CVAR_INTEGER, "intentionally add a sleep in the draw time" );
idCVar com_sleepRender( "com_sleepRender", "0", CVAR_SYSTEM | CVAR_INTEGER, "intentionally add a sleep in the render time" );

idCVar net_drawDebugHud( "net_drawDebugHud", "0", CVAR_SYSTEM | CVAR_INTEGER, "0 = None, 1 = Hud 1, 2 = Hud 2, 3 = Snapshots" );

idCVar timescale( "timescale", "1", CVAR_SYSTEM | CVAR_FLOAT, "Number of game frames to run per render frame", 0.001f, 100.0f );
//#modified-fva; BEGIN
idCVar cst_dcBrightness("cst_dcBrightness", "1.0", CVAR_FLOAT | CVAR_ARCHIVE, "brightness control for doom 1 and doom 2", 0.0f, 1.0f);
idCVar cst_dcHudRun("cst_dcHudRun", "0", CVAR_BOOL | CVAR_ARCHIVE, "enable the HUD's run indicator for doom 1 and doom 2");

idCVar cst_brtAutoSetEnv("cst_brtAutoSetEnv", "1", CVAR_BOOL | CVAR_NOCHEAT, "auto disable multithreading and gpu skinning while the backend render tools are being used: 0 = No, 1 = Yes");
bool gCstLatched__com_smp = false;
bool gCstLatched__r_useParallelAddModels = false;
bool gCstLatched__r_useParallelAddShadows = false;
bool gCstLatched__r_useParallelAddLights = false;
bool gCstLatched__r_useGPUSkinning = false;
//#modified-fva; END

extern idCVar in_useJoystick;
extern idCVar in_joystickRumble;

/*
===============
idGameThread::Run

Run in a background thread for performance, but can also
be called directly in the foreground thread for comparison.
===============
*/
int idGameThread::Run() {
	commonLocal.frameTiming.startGameTime = Sys_Microseconds();

	// debugging tool to test frame dropping behavior
	if ( com_sleepGame.GetInteger() ) {
		Sys_Sleep( com_sleepGame.GetInteger() );
	}

	if ( numGameFrames == 0 ) {
		// Ensure there's no stale gameReturn data from a paused game
		ret = gameReturn_t();
	}

	if ( isClient ) {
		// run the game logic
		for ( int i = 0; i < numGameFrames; i++ ) {
			SCOPED_PROFILE_EVENT( "Client Prediction" );
			if ( userCmdMgr ) {
				game->ClientRunFrame( *userCmdMgr, ( i == numGameFrames - 1 ), ret );
			}
			if ( ret.syncNextGameFrame || ret.sessionCommand[0] != 0 ) {
				break;
			}
		}
	} else {
		// run the game logic
		for ( int i = 0; i < numGameFrames; i++ ) {
			SCOPED_PROFILE_EVENT( "GameTic" );
			if ( userCmdMgr ) {
				game->RunFrame( *userCmdMgr, ret );
			}
			if ( ret.syncNextGameFrame || ret.sessionCommand[0] != 0 ) {
				break;
			}
		}
	}

	// we should have consumed all of our usercmds
	if ( userCmdMgr ) {
		//#modified-fva; BEGIN
		//if ( userCmdMgr->HasUserCmdForPlayer( game->GetLocalClientNum() ) && common->GetCurrentGame() == DOOM3_BFG ) {
		int cstLocalClientNum = game->GetLocalClientNum();
		if (cstLocalClientNum >= 0 && userCmdMgr->HasUserCmdForPlayer(cstLocalClientNum) && common->GetCurrentGame() == DOOM3_BFG) {
		//#modified-fva; END
			idLib::Printf( "idGameThread::Run: didn't consume all usercmds\n" );
		}
	}

	commonLocal.frameTiming.finishGameTime = Sys_Microseconds();

	SetThreadGameTime( ( commonLocal.frameTiming.finishGameTime - commonLocal.frameTiming.startGameTime ) / 1000 );

	// build render commands and geometry
	{
		SCOPED_PROFILE_EVENT( "Draw" );
		commonLocal.Draw();
	}

	commonLocal.frameTiming.finishDrawTime = Sys_Microseconds();

	SetThreadRenderTime( ( commonLocal.frameTiming.finishDrawTime - commonLocal.frameTiming.finishGameTime ) / 1000 );

	SetThreadTotalTime( ( commonLocal.frameTiming.finishDrawTime - commonLocal.frameTiming.startGameTime ) / 1000 );

	return 0;
}

/*
===============
idGameThread::RunGameAndDraw

===============
*/
gameReturn_t idGameThread::RunGameAndDraw( int numGameFrames_, idUserCmdMgr & userCmdMgr_, bool isClient_, int startGameFrame ) {
	// this should always immediately return
	this->WaitForThread();

	// save the usercmds for the background thread to pick up
	userCmdMgr = &userCmdMgr_;

	isClient = isClient_;

	// grab the return value created by the last thread execution
	gameReturn_t latchedRet = ret;

	numGameFrames = numGameFrames_;

	// start the thread going
	//#modified-fva; BEGIN
	//if ( com_smp.GetBool() == false ) {
	if (gCstLatched__com_smp == false) {
	//#modified-fva; END
		// run it in the main thread so PIX profiling catches everything
		Run();
	} else {
		this->SignalWork();
	}

	// return the latched result while the thread runs in the background
	return latchedRet;
}


/*
===============
idCommonLocal::DrawWipeModel

Draw the fade material over everything that has been drawn
===============
*/
void idCommonLocal::DrawWipeModel() {

	if ( wipeStartTime >= wipeStopTime ) {
		return;
	}

	int currentTime = Sys_Milliseconds();

	if ( !wipeHold && currentTime > wipeStopTime ) {
		return;
	}

	float fade = ( float )( currentTime - wipeStartTime ) / ( wipeStopTime - wipeStartTime );
	renderSystem->SetColor4( 1, 1, 1, fade );
	renderSystem->DrawStretchPic( 0, 0, 640, 480, 0, 0, 1, 1, wipeMaterial );
}

/*
===============
idCommonLocal::Draw
===============
*/
void idCommonLocal::Draw() {
	// debugging tool to test frame dropping behavior
	if ( com_sleepDraw.GetInteger() ) {
		Sys_Sleep( com_sleepDraw.GetInteger() );
	}

	if ( loadGUI != NULL ) {
		loadGUI->Render( renderSystem, Sys_Milliseconds() );
	} else if ( currentGame == DOOM_CLASSIC || currentGame == DOOM2_CLASSIC ) {
		const float sysWidth = renderSystem->GetWidth() * renderSystem->GetPixelAspect();
		const float sysHeight = renderSystem->GetHeight();
		const float sysAspect = sysWidth / sysHeight;
		const float doomAspect = 4.0f / 3.0f;
		const float adjustment = sysAspect / doomAspect;
		//#modified-fva; BEGIN
		//const float barHeight = ( adjustment >= 1.0f ) ? 0.0f : ( 1.0f - adjustment ) * (float)SCREEN_HEIGHT * 0.25f;
		//const float barWidth = ( adjustment <= 1.0f ) ? 0.0f : ( adjustment - 1.0f ) * (float)SCREEN_WIDTH * 0.25f;
		const float cstInvAdjustment = 1.0f / adjustment;
		const float barHeight = (adjustment >= 1.0f) ? 0.0f : (1.0f - adjustment) * (float)SCREEN_HEIGHT * 0.5f;
		const float barWidth = (adjustment <= 1.0f) ? 0.0f : (1.0f - cstInvAdjustment) * (float)SCREEN_WIDTH * 0.5f;
		//#modified-fva; END
		if ( barHeight > 0.0f ) {
			renderSystem->SetColor( colorBlack );
			renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, barHeight, 0, 0, 1, 1, whiteMaterial );
			renderSystem->DrawStretchPic( 0, SCREEN_HEIGHT - barHeight, SCREEN_WIDTH, barHeight, 0, 0, 1, 1, whiteMaterial );
		}
		if ( barWidth > 0.0f ) {
			renderSystem->SetColor( colorBlack );
			renderSystem->DrawStretchPic( 0, 0, barWidth, SCREEN_HEIGHT, 0, 0, 1, 1, whiteMaterial );
			renderSystem->DrawStretchPic( SCREEN_WIDTH - barWidth, 0, barWidth, SCREEN_HEIGHT, 0, 0, 1, 1, whiteMaterial );
		}
		//#modified-fva; BEGIN
		//renderSystem->SetColor4( 1, 1, 1, 1 );
		const float cstBrightness = cst_dcBrightness.GetFloat();
		renderSystem->SetColor4(cstBrightness, cstBrightness, cstBrightness, 1.0f);
		//#modified-fva; END
		renderSystem->DrawStretchPic( barWidth, barHeight, SCREEN_WIDTH - barWidth * 2.0f, SCREEN_HEIGHT - barHeight * 2.0f, 0, 0, 1, 1, doomClassicMaterial );
		//#modified-fva; BEGIN
		Globals *data = (Globals*)DoomLib::GetGlobalData(0);
		if (data) {
			const float statusBarHeight = (float)(ST_HEIGHT) / (float)(ORIGINAL_HEIGHT) * (SCREEN_HEIGHT - barHeight * 2.0f);
			const float statusBarWidth = SCREEN_WIDTH - barWidth * 2.0f;
			const float statusBarX = barWidth;
			const float statusBarY = SCREEN_HEIGHT - barHeight - statusBarHeight;
			// status bar color overlay
			const idVec4 &color = data->cstScreenOverlayColor;
			if (color.w > 0.0f) {
				idVec4 scaledColor = color * cstBrightness;
				scaledColor.w = color.w;
				renderSystem->DrawFilled(scaledColor, statusBarX, statusBarY, statusBarWidth, statusBarHeight);
			}
			// run indicator
			if (cst_dcHudRun.GetBool()
				&& !data->menuactive && !data->demoplayback && (data->viewactive || data->automapactive) && !data->paused
				&& (usercmdGen->GetCurrentUsercmd().buttons & BUTTON_RUN)) {
				const float heightScale = (adjustment >= 1.0f) ? 1.0f : adjustment;
				const float widthScale = (adjustment <= 1.0f) ? 1.0f : cstInvAdjustment;
				const float hudRunHeight = 20.0f * heightScale;
				const float hudRunWidth = 8.0f * widthScale;
				const float hudRunX = statusBarX;
				const float hudRunY = statusBarY - hudRunHeight - 23.0f * heightScale; // must be above the text when the automap is active
				const float hudRunArrowX = hudRunX - 1.25f * widthScale;
				const float hudRunArrowY = hudRunY + 1.5f * heightScale;
				renderSystem->DrawFilled(idVec4(cstBrightness, 0.0f, 0.0f, 0.5f), hudRunX, hudRunY, hudRunWidth, hudRunHeight);
				renderSystem->SetColor4(0.0f, 0.0f, 0.0f, 1.0f);
				renderSystem->CstDrawSmallChar(hudRunArrowX + 1.0f * widthScale, hudRunArrowY + 1.0f * heightScale, widthScale, heightScale, '>'); // drop shadow
				renderSystem->SetColor4(cstBrightness, cstBrightness, cstBrightness, 1.0f);
				renderSystem->CstDrawSmallChar(hudRunArrowX, hudRunArrowY, widthScale, heightScale, '>');
			}
		}
		//#modified-fva; END
	} else if ( game && game->Shell_IsActive() ) {
		//#modified-fva; BEGIN
		game->CstGenUpdateCinematicCmds();
		//#modified-fva; END
		bool gameDraw = game->Draw( game->GetLocalClientNum() );
		if ( !gameDraw ) {
			renderSystem->SetColor( colorBlack );
			renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 1, 1, whiteMaterial );
		}
		game->Shell_Render();
	} else if ( readDemo ) {
		renderWorld->RenderScene( &currentDemoRenderView );
		renderSystem->DrawDemoPics();
	} else if ( mapSpawned ) {
		//#modified-fva; BEGIN
		game->CstGenUpdateCinematicCmds();
		//#modified-fva; END
		bool gameDraw = false;
		// normal drawing for both single and multi player
		if ( !com_skipGameDraw.GetBool() && Game()->GetLocalClientNum() >= 0 ) {
			// draw the game view
			int	start = Sys_Milliseconds();
			if ( game ) {
				gameDraw = game->Draw( Game()->GetLocalClientNum() );
			}
			int end = Sys_Milliseconds();
			time_gameDraw += ( end - start );	// note time used for com_speeds
		}
		if ( !gameDraw ) {
			renderSystem->SetColor( colorBlack );
			renderSystem->DrawStretchPic( 0, 0, 640, 480, 0, 0, 1, 1, whiteMaterial );
		}

		// save off the 2D drawing from the game
		if ( writeDemo ) {
			renderSystem->WriteDemoPics();
		}
	} else {
		renderSystem->SetColor4( 0, 0, 0, 1 );
		renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 1, 1, whiteMaterial );
	}

	{
		SCOPED_PROFILE_EVENT( "Post-Draw" );

		// draw the wipe material on top of this if it hasn't completed yet
		DrawWipeModel();

		Dialog().Render( loadGUI != NULL );

		// draw the half console / notify console on top of everything
		console->Draw( false );
		//#modified-fva; BEGIN
		CstEchoDraw();
		//#modified-fva; END
	}
}

/*
===============
idCommonLocal::UpdateScreen

This is an out-of-sequence screen update, not the normal game rendering
===============
*/
void idCommonLocal::UpdateScreen( bool captureToImage ) {
	if ( insideUpdateScreen ) {
		return;
	}
	insideUpdateScreen = true;

	// make sure the game / draw thread has completed
	gameThread.WaitForThread();

	// release the mouse capture back to the desktop
	Sys_GrabMouseCursor( false );

	// build all the draw commands without running a new game tic
	Draw();

	if ( captureToImage ) {
		renderSystem->CaptureRenderToImage( "_currentRender", false );
	}

	// this should exit right after vsync, with the GPU idle and ready to draw
	const emptyCommand_t * cmd = renderSystem->SwapCommandBuffers( &time_frontend, &time_backend, &time_shadows, &time_gpu );

	// get the GPU busy with new commands
	renderSystem->RenderCommandBuffers( cmd );

	insideUpdateScreen = false;
}
/*
================
idCommonLocal::ProcessGameReturn
================
*/
void idCommonLocal::ProcessGameReturn( const gameReturn_t & ret ) {
	// set joystick rumble
	if ( in_useJoystick.GetBool() && in_joystickRumble.GetBool() && !game->Shell_IsActive() && session->GetSignInManager().GetMasterInputDevice() >= 0 ) {
		Sys_SetRumble( session->GetSignInManager().GetMasterInputDevice(), ret.vibrationLow, ret.vibrationHigh );		// Only set the rumble on the active controller
	} else {
		for ( int i = 0; i < MAX_INPUT_DEVICES; i++ ) {
			Sys_SetRumble( i, 0, 0 );
		}
	}

	//#modified-fva; BEGIN
	// see cstCinematicSkipped
	//syncNextGameFrame = ret.syncNextGameFrame;
	//#modified-fva; END

	if ( ret.sessionCommand[0] ) {
		idCmdArgs args;

		args.TokenizeString( ret.sessionCommand, false );

		if ( !idStr::Icmp( args.Argv(0), "map" ) ) {
			MoveToNewMap( args.Argv( 1 ), false );
		} else if ( !idStr::Icmp( args.Argv(0), "devmap" ) ) {
			MoveToNewMap( args.Argv( 1 ), true );
		} else if ( !idStr::Icmp( args.Argv(0), "died" ) ) {
			if ( !IsMultiplayer() ) {
				game->Shell_Show( true );
			}
		} else if ( !idStr::Icmp( args.Argv(0), "disconnect" ) ) {
			cmdSystem->BufferCommandText( CMD_EXEC_INSERT, "stoprecording ; disconnect" );
		} else if ( !idStr::Icmp( args.Argv(0), "endOfDemo" ) ) {
			cmdSystem->BufferCommandText( CMD_EXEC_NOW, "endOfDemo" );
		}
	}
}

extern idCVar com_forceGenericSIMD;

/*
=================
idCommonLocal::Frame
=================
*/
void idCommonLocal::Frame() {
	try {
		SCOPED_PROFILE_EVENT( "Common::Frame" );

		// This is the only place this is incremented
		idLib::frameNumber++;

		// allow changing SIMD usage on the fly
		if ( com_forceGenericSIMD.IsModified() ) {
			idSIMD::InitProcessor( "doom", com_forceGenericSIMD.GetBool() );
			com_forceGenericSIMD.ClearModified();
		}

		// Do the actual switch between Doom 3 and the classics here so
		// that things don't get confused in the middle of the frame.
		PerformGameSwitch();

		//#modified-fva; BEGIN
		if (IsPlayingDoomClassic()) {
			int oldPlayer = DoomLib::GetPlayer();
			DoomLib::SetPlayer(0);
			if (g && (bool)g->automapactive != cstWasAutomapActive) {
				idKeyInput::ClearStates();
				if (g->automapactive) {
					idKeyInput::CstSetCurrentDomain(2);
				} else {
					idKeyInput::CstSetCurrentDomain(1);
				}
				cstWasAutomapActive = g->automapactive;
			}
			DoomLib::SetPlayer(oldPlayer);
		}
		//#modified-fva; END

		// pump all the events
		Sys_GenerateEvents();

		// write config file if anything changed
		WriteConfiguration();

		//#modified-fva; BEGIN
		cstSkipCinematicRequested = false;
		//#modified-fva; END
		eventLoop->RunEventLoop();

		//#modified-fva; BEGIN
		CstSetEnvForBackendRendertools();
		//#modified-fva; END

		//#modified-fva; BEGIN
		// moved to after the sound system is rendered
		/*
		// Activate the shell if it's been requested
		if ( showShellRequested && game ) {
			game->Shell_Show( true );
			showShellRequested = false;
		}
		*/
		//#modified-fva; END

		// if the console or another gui is down, we don't need to hold the mouse cursor
		bool chatting = false;
		if ( console->Active() || Dialog().IsDialogActive() || session->IsSystemUIShowing() || ( game && game->InhibitControls() && !IsPlayingDoomClassic() ) ) {
			Sys_GrabMouseCursor( false );
			usercmdGen->InhibitUsercmd( INHIBIT_SESSION, true );
			chatting = true;
		} else {
			Sys_GrabMouseCursor( true );
			usercmdGen->InhibitUsercmd( INHIBIT_SESSION, false );
		}

		const bool pauseGame = ( !mapSpawned || ( !IsMultiplayer() && ( Dialog().IsDialogPausing() || session->IsSystemUIShowing() || ( game && game->Shell_IsActive() ) ) ) ) && !IsPlayingDoomClassic();

		// save the screenshot and audio from the last draw if needed
		if ( aviCaptureMode ) {
			idStr name = va("demos/%s/%s_%05i.tga", aviDemoShortName.c_str(), aviDemoShortName.c_str(), aviDemoFrameCount++ );
			renderSystem->TakeScreenshot( com_aviDemoWidth.GetInteger(), com_aviDemoHeight.GetInteger(), name, com_aviDemoSamples.GetInteger(), NULL );

			// remove any printed lines at the top before taking the screenshot
			console->ClearNotifyLines();

			//#modified-fva; BEGIN
			CstClearMsgs(true);
			//#modified-fva; END

			// this will call Draw, possibly multiple times if com_aviDemoSamples is > 1
			renderSystem->TakeScreenshot( com_aviDemoWidth.GetInteger(), com_aviDemoHeight.GetInteger(), name, com_aviDemoSamples.GetInteger(), NULL );
		}

		//--------------------------------------------
		// wait for the GPU to finish drawing
		//
		// It is imporant to minimize the time spent between this
		// section and the call to renderSystem->RenderCommandBuffers(),
		// because the GPU is completely idle.
		//--------------------------------------------
		// this should exit right after vsync, with the GPU idle and ready to draw
		// This may block if the GPU isn't finished renderng the previous frame.
		frameTiming.startSyncTime = Sys_Microseconds();
		const emptyCommand_t * renderCommands = NULL;
		//#modified-fva; BEGIN
		//if ( com_smp.GetBool() ) {
		if (gCstLatched__com_smp) {
		//#modified-fva; END
			renderCommands = renderSystem->SwapCommandBuffers( &time_frontend, &time_backend, &time_shadows, &time_gpu );
		} else {
			// the GPU will stay idle through command generation for minimal
			// input latency
			renderSystem->SwapCommandBuffers_FinishRendering( &time_frontend, &time_backend, &time_shadows, &time_gpu );
		}
		frameTiming.finishSyncTime = Sys_Microseconds();

		//--------------------------------------------
		// Determine how many game tics we are going to run,
		// now that the previous frame is completely finished.
		//
		// It is important that any waiting on the GPU be done
		// before this, or there will be a bad stuttering when
		// dropping frames for performance management.
		//--------------------------------------------

		// input:
		// thisFrameTime
		// com_noSleep
		// com_engineHz
		// com_fixedTic
		// com_deltaTimeClamp
		// IsMultiplayer
		//
		// in/out state:
		// gameFrame
		// gameTimeResidual
		// lastFrameTime
		// syncNextFrame
		//
		// Output:
		// numGameFrames

		// How many game frames to run
		int numGameFrames = 0;
		//#modified-fva; BEGIN
		const double cstFramePeriodMsec_hr = 1000.0 / (double)com_engineHz_latched;
		const int cstHiResClockOption = cst_hiResClock.GetInteger();
		Sys_CstEnableThreadAffinity(cstHiResClockOption == 2);
		//#modified-fva; END

		for(;;) {
			const int thisFrameTime = Sys_Milliseconds();
			static int lastFrameTime = thisFrameTime;	// initialized only the first time
			const int deltaMilliseconds = thisFrameTime - lastFrameTime;
			lastFrameTime = thisFrameTime;
			//#modified-fva; BEGIN
			const int64 cstThisFrameTime_hr = Sys_CstHiResClockCount();
			static int64 cstLastFrameTime_hr = cstThisFrameTime_hr; // initialized only the first time
			double cstDeltaMilliseconds_hr = Sys_CstHiResClockCountToMilliseconds(cstThisFrameTime_hr - cstLastFrameTime_hr);
			if (cstDeltaMilliseconds_hr < 0.0) {
				// should never happen
				cstDeltaMilliseconds_hr = 0.0;
			}
			cstLastFrameTime_hr = cstThisFrameTime_hr;
			//#modified-fva; END

			// if there was a large gap in time since the last frame, or the frame
			// rate is very very low, limit the number of frames we will run
			//#modified-fva; BEGIN
			/*
			const int clampedDeltaMilliseconds = Min( deltaMilliseconds, com_deltaTimeClamp.GetInteger() );

			gameTimeResidual += clampedDeltaMilliseconds * timescale.GetFloat();
			*/
			if (cstHiResClockOption > 0) {
				const double cstClampedDeltaMilliseconds_hr = Min(cstDeltaMilliseconds_hr, (double)com_deltaTimeClamp.GetInteger());
				gameTimeResidual += cstClampedDeltaMilliseconds_hr * timescale.GetFloat();
			} else {
				const int clampedDeltaMilliseconds = Min(deltaMilliseconds, com_deltaTimeClamp.GetInteger());
				gameTimeResidual += clampedDeltaMilliseconds * timescale.GetFloat();
			}
			//#modified-fva; END

			// don't run any frames when paused
			if ( pauseGame ) {
				gameFrame++;
				gameTimeResidual = 0;
				break;
			}

			// debug cvar to force multiple game tics
			if ( com_fixedTic.GetInteger() > 0 ) {
				numGameFrames = com_fixedTic.GetInteger();
				gameFrame += numGameFrames;
				gameTimeResidual = 0;
				break;
			}

			if ( syncNextGameFrame ) {
				// don't sleep at all
				syncNextGameFrame = false;
				gameFrame++;
				numGameFrames++;
				gameTimeResidual = 0;
				break;
			}

			//#modified-fva; BEGIN
			/*
			for ( ;; ) {
				// How much time to wait before running the next frame,
				// based on com_engineHz
				const int frameDelay = FRAME_TO_MSEC( gameFrame + 1 ) - FRAME_TO_MSEC( gameFrame );
				if ( gameTimeResidual < frameDelay ) {
					break;
				}
				gameTimeResidual -= frameDelay;
				gameFrame++;
				numGameFrames++;
				// if there is enough residual left, we may run additional frames
			}
			*/
			if (cstHiResClockOption > 0) {
				for (;;) {
					if (gameTimeResidual < cstFramePeriodMsec_hr) {
						break;
					}
					gameTimeResidual -= cstFramePeriodMsec_hr;
					gameFrame++;
					numGameFrames++;
				}
			} else {
				for (;;) {
					const int frameDelay = FRAME_TO_MSEC(gameFrame + 1) - FRAME_TO_MSEC(gameFrame);
					if (gameTimeResidual < frameDelay) {
						break;
					}
					gameTimeResidual -= frameDelay;
					gameFrame++;
					numGameFrames++;
				}
			}
			//#modified-fva; END

			if ( numGameFrames > 0 ) {
				// ready to actually run them
				break;
			}

			// if we are vsyncing, we always want to run at least one game
			// frame and never sleep, which might happen due to scheduling issues
			// if we were just looking at real time.
			if ( com_noSleep.GetBool() ) {
				numGameFrames = 1;
				gameFrame += numGameFrames;
				gameTimeResidual = 0;
				break;
			}

			// not enough time has passed to run a frame, as might happen if
			// we don't have vsync on, or the monitor is running at 120hz while
			// com_engineHz is 60, so sleep a bit and check again
			//#modified-fva; BEGIN
			//Sys_Sleep( 0 );
			if (cstHiResClockOption > 0) {
				double waitMsec = cstFramePeriodMsec_hr - gameTimeResidual;
				Sys_CstSleepHiRes(waitMsec);
			} else {
				Sys_Sleep(0);
			}
			//#modified-fva; END
		}

		//--------------------------------------------
		// It would be better to push as much of this as possible
		// either before or after the renderSystem->SwapCommandBuffers(),
		// because the GPU is completely idle.
		//--------------------------------------------

		// Update session and syncronize to the new session state after sleeping
		session->UpdateSignInManager();
		session->Pump();
		session->ProcessSnapAckQueue();

		if ( session->GetState() == idSession::LOADING ) {
			// If the session reports we should be loading a map, load it!
			ExecuteMapChange();
			mapSpawnData.savegameFile = NULL;
			mapSpawnData.persistentPlayerInfo.Clear();
			return;
		} else if ( session->GetState() != idSession::INGAME && mapSpawned ) {
			// If the game is running, but the session reports we are not in a game, disconnect
			// This happens when a server disconnects us or we sign out
			LeaveGame();
			return;
		}

		if ( mapSpawned && !pauseGame ) {
			if ( IsClient() ) {
				RunNetworkSnapshotFrame();
			}
		}

		ExecuteReliableMessages();

		// send frame and mouse events to active guis
		GuiFrameEvents();

		//--------------------------------------------
		// Prepare usercmds and kick off the game processing
		// in a background thread
		//--------------------------------------------

		// get the previous usercmd for bypassed head tracking transform
		const usercmd_t	previousCmd = usercmdGen->GetCurrentUsercmd();

		// build a new usercmd
		int deviceNum = session->GetSignInManager().GetMasterInputDevice();
		usercmdGen->BuildCurrentUsercmd( deviceNum );
		if ( deviceNum == -1 ) {
			for ( int i = 0; i < MAX_INPUT_DEVICES; i++ ) {
				Sys_PollJoystickInputEvents( i );
				Sys_EndJoystickInputEvents();
			}
		}
		if ( pauseGame ) {
			usercmdGen->Clear();
		}

		usercmd_t newCmd = usercmdGen->GetCurrentUsercmd();

		// Store server game time - don't let time go past last SS time in case we are extrapolating
		if ( IsClient() ) {
			newCmd.serverGameMilliseconds = std::min( Game()->GetServerGameTimeMs(), Game()->GetSSEndTime() );
		} else {
			newCmd.serverGameMilliseconds = Game()->GetServerGameTimeMs();
		}

		//#modified-fva; BEGIN
		int cstLocalClientNum = Game()->GetLocalClientNum();
		//#modified-fva; END

		//#modified-fva; BEGIN
		//userCmdMgr.MakeReadPtrCurrentForPlayer( Game()->GetLocalClientNum() );
		if (cstLocalClientNum >= 0) {
			userCmdMgr.MakeReadPtrCurrentForPlayer(cstLocalClientNum);
		}
		//#modified-fva; END

		// Stuff a copy of this userCmd for each game frame we are going to run.
		// Ideally, the usercmds would be built in another thread so you could
		// still get 60hz control accuracy when the game is running slower.
		//#modified-fva; BEGIN
		/*
		for ( int i = 0 ; i < numGameFrames ; i++ ) {
			newCmd.clientGameMilliseconds = FRAME_TO_MSEC( gameFrame-numGameFrames+i+1 );
			userCmdMgr.PutUserCmdForPlayer( game->GetLocalClientNum(), newCmd );
		}
		*/
		if (cstLocalClientNum >= 0) {
			for (int i = 0; i < numGameFrames; i++) {
				newCmd.clientGameMilliseconds = FRAME_TO_MSEC(gameFrame - numGameFrames + i + 1);
				userCmdMgr.PutUserCmdForPlayer(cstLocalClientNum, newCmd);
			}
		}
		//#modified-fva; END

		// If we're in Doom or Doom 2, run tics and upload the new texture.
		if ( ( GetCurrentGame() == DOOM_CLASSIC || GetCurrentGame() == DOOM2_CLASSIC ) && !( Dialog().IsDialogPausing() || session->IsSystemUIShowing() ) ) {
			RunDoomClassicFrame();
		}

		//#modified-fva; BEGIN
		if (!IsClient() && mapSpawned && game) {
			// moved from idGameLocal::RunFrame
			game->CstSyncPlayersWithLobbyUsers();
		}
		//#modified-fva; END

		// start the game / draw command generation thread going in the background
		gameReturn_t ret = gameThread.RunGameAndDraw( numGameFrames, userCmdMgr, IsClient(), gameFrame - numGameFrames );

		//#modified-fva; BEGIN
		//if ( !com_smp.GetBool() ) {
		if (!gCstLatched__com_smp) {
		//#modified-fva; END
			// in non-smp mode, run the commands we just generated, instead of
			// frame-delayed ones from a background thread
			renderCommands = renderSystem->SwapCommandBuffers_FinishCommandBuffers();
		}

		//----------------------------------------
		// Run the render back end, getting the GPU busy with new commands
		// ASAP to minimize the pipeline bubble.
		//----------------------------------------
		frameTiming.startRenderTime = Sys_Microseconds();
		renderSystem->RenderCommandBuffers( renderCommands );
		if ( com_sleepRender.GetInteger() > 0 ) {
			// debug tool to test frame adaption
			Sys_Sleep( com_sleepRender.GetInteger() );
		}
		frameTiming.finishRenderTime = Sys_Microseconds();

		// make sure the game / draw thread has completed
		// This may block if the game is taking longer than the render back end
		gameThread.WaitForThread();

		// Send local usermds to the server.
		// This happens after the game frame has run so that prediction data is up to date.
		//#modified-fva; BEGIN
		//SendUsercmds( Game()->GetLocalClientNum() );
		if (cstLocalClientNum >= 0) {
			SendUsercmds(cstLocalClientNum);
		}
		//#modified-fva; END

		// Now that we have an updated game frame, we can send out new snapshots to our clients
		session->Pump(); // Pump to get updated usercmds to relay
		SendSnapshots();

		// Render the sound system using the latest commands from the game thread
		if ( pauseGame ) {
			soundWorld->Pause();
			soundSystem->SetPlayingSoundWorld( menuSoundWorld );
		} else {
			soundWorld->UnPause();
			soundSystem->SetPlayingSoundWorld( soundWorld );
		}
		//#modified-fva; BEGIN
		//soundSystem->Render();

		if (cstSkipCinematicRequested) {
			cstSkipCinematicRequested = false;
			if (!pauseGame && game && game->IsInGame() && game->CstSkipCinematic()) {
				soundWorld->CstSetMute(true);
			}
		}

		if (cstCinematicSkipped) {
			cstCinematicSkipped = false;
			if (gCstLatched__com_smp) {
				renderSystem->CstUpdateCinematicsNowOutsideRender(); // allow videos to catch up now
			}
			soundWorld->CstSetMute(false);

			// start a volume fade-in from silence
			soundWorld->CstSetPauseFade();

			// update the sound system
			const int oldSoundTime = soundWorld->GetSoundTime();
			soundSystem->Render();
			const int currentSoundTime = soundWorld->GetSoundTime();

			// adjust the soundWorld time to compensate for the time spent skipping the cinematic
			const int timeDiff = currentSoundTime - oldSoundTime;
			const int deltaFrameTime = FRAME_TO_MSEC(gameFrame + 1) - FRAME_TO_MSEC(gameFrame);
			const int timeAdjustment = timeDiff - deltaFrameTime;
			soundWorld->Skip(-timeAdjustment);

			// set syncNextGameFrame here instead of in ProcessGameReturn
			syncNextGameFrame = true;
		} else {
			soundSystem->Render();
		}

		// moved from the beginning of this function
		if (showShellRequested && game) {
			game->Shell_Show(true);
			showShellRequested = false;
		}
		//#modified-fva; END

		// process the game return for map changes, etc
		ProcessGameReturn( ret );

		idLobbyBase & lobby = session->GetActivePlatformLobbyBase();
		if ( lobby.HasActivePeers() ) {
			if ( net_drawDebugHud.GetInteger() == 1 ) {
				lobby.DrawDebugNetworkHUD();
			}
			if ( net_drawDebugHud.GetInteger() == 2 ) {
				lobby.DrawDebugNetworkHUD2();
			}
			lobby.DrawDebugNetworkHUD_ServerSnapshotMetrics( net_drawDebugHud.GetInteger() == 3 );
		}

		// report timing information
		if ( com_speeds.GetBool() ) {
			static int lastTime = Sys_Milliseconds();
			int	nowTime = Sys_Milliseconds();
			int	com_frameMsec = nowTime - lastTime;
			lastTime = nowTime;
			Printf( "frame:%d all:%3d gfr:%3d rf:%3lld bk:%3lld\n", idLib::frameNumber, com_frameMsec, time_gameFrame, time_frontend / 1000, time_backend / 1000 );
			time_gameFrame = 0;
			time_gameDraw = 0;
		}

		mainFrameTiming = frameTiming;

		session->GetSaveGameManager().Pump();
	} catch( idException & ) {
		return;			// an ERP_DROP was thrown
	}
}

/*
=================
idCommonLocal::RunDoomClassicFrame
=================
*/
void idCommonLocal::RunDoomClassicFrame() {
	static int doomTics = 0;

	if( DoomLib::expansionDirty ) {

		// re-Initialize the Doom Engine.
		DoomLib::Interface.Shutdown();
		DoomLib::Interface.Startup( 1, false );
		DoomLib::expansionDirty = false;
	}


	if ( DoomLib::Interface.Frame( doomTics, &userCmdMgr ) ) {
		Globals *data = (Globals*)DoomLib::GetGlobalData( 0 );

		idArray< unsigned int, 256 > palette;
		std::copy( data->XColorMap, data->XColorMap + palette.Num(), palette.Ptr() );

		// Do the palette lookup.
		for ( int row = 0; row < DOOMCLASSIC_RENDERHEIGHT; ++row ) {
			for ( int column = 0; column < DOOMCLASSIC_RENDERWIDTH; ++column ) {
				const int doomScreenPixelIndex = row * DOOMCLASSIC_RENDERWIDTH + column;
				const byte paletteIndex = data->screens[0][doomScreenPixelIndex];
				const unsigned int paletteColor = palette[paletteIndex];
				const byte red = (paletteColor & 0xFF000000) >> 24;
				const byte green = (paletteColor & 0x00FF0000) >> 16;
				const byte blue = (paletteColor & 0x0000FF00) >> 8;

				const int imageDataPixelIndex = row * DOOMCLASSIC_RENDERWIDTH * DOOMCLASSIC_BYTES_PER_PIXEL + column * DOOMCLASSIC_BYTES_PER_PIXEL;
				doomClassicImageData[imageDataPixelIndex]		= red;
				doomClassicImageData[imageDataPixelIndex + 1]	= green;
				doomClassicImageData[imageDataPixelIndex + 2]	= blue;
				doomClassicImageData[imageDataPixelIndex + 3]	= 255;
			}
		}
	}

	renderSystem->UploadImage( "_doomClassic", doomClassicImageData.Ptr(), DOOMCLASSIC_RENDERWIDTH, DOOMCLASSIC_RENDERHEIGHT );
	doomTics++;
}

//#modified-fva; BEGIN
void idCommonLocal::CstSetEnvForBackendRendertools() {
	extern idCVar r_useParallelAddModels;
	extern idCVar r_useParallelAddShadows;
	extern idCVar r_useParallelAddLights;
	extern idCVar r_useGPUSkinning;
	extern idCVar r_showSkel;

	if (cst_brtAutoSetEnv.GetBool()) {
		cstBrtEnabled = cstBrtEnableRequested.load(std::memory_order_relaxed) || r_showSkel.GetBool();
	} else {
		cstBrtEnabled = true;
	}
	cstBrtEnableRequested.store(false, std::memory_order_relaxed);

	if (cstBrtEnabled && cst_brtAutoSetEnv.GetBool()) {
		// if AutoSetEnv is enabled and any of the backend render tools will run this frame, disable multithreading and gpu skinning internally
		gCstLatched__com_smp = false;
		gCstLatched__r_useParallelAddModels = false;
		gCstLatched__r_useParallelAddShadows = false;
		gCstLatched__r_useParallelAddLights = false;
		gCstLatched__r_useGPUSkinning = false;
	} else {
		// otherwise, use the values of the cvars
		gCstLatched__com_smp = com_smp.GetBool();
		gCstLatched__r_useParallelAddModels = r_useParallelAddModels.GetBool();
		gCstLatched__r_useParallelAddShadows = r_useParallelAddShadows.GetBool();
		gCstLatched__r_useParallelAddLights = r_useParallelAddLights.GetBool();
		gCstLatched__r_useGPUSkinning = r_useGPUSkinning.GetBool();
	}
}
//#modified-fva; END
