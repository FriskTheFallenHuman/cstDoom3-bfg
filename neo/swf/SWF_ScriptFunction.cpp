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

idCVar swf_debug( "swf_debug", "0", CVAR_INTEGER|CVAR_ARCHIVE, "debug swf scripts.  1 shows traces/errors.  2 also shows warnings.  3 also shows disassembly.  4 shows parameters in the disassembly." );
idCVar swf_debugInvoke( "swf_debugInvoke", "0", CVAR_INTEGER, "debug swf functions being called from game." );

idSWFConstantPool::idSWFConstantPool() {
}


/*
========================
idSWFConstantPool::Clear
========================
*/
void idSWFConstantPool::Clear() {
	for ( int i = 0; i < pool.Num(); i++ ) {
		pool[i]->Release();
	}
	pool.Clear();
}

/*
========================
idSWFConstantPool::Copy
========================
*/
void idSWFConstantPool::Copy( const idSWFConstantPool & other ) {
	Clear();
	pool.SetNum( other.pool.Num() );
	for ( int i = 0; i < pool.Num(); i++ ) {
		pool[i] = other.pool[i];
		pool[i]->AddRef();
	}
}

/*
========================
idSWFScriptFunction_Script::~idSWFScriptFunction_Script
========================
*/
idSWFScriptFunction_Script::~idSWFScriptFunction_Script() {
	for ( int i = 0; i < scope.Num(); i++ ) {
		if ( verify( scope[i] ) ) {
			scope[i]->Release();
		}
	}
	if ( prototype ) {
		prototype->Release();
	}
}

/*
========================
idSWFScriptFunction_Script::Call
========================
*/
void idSWFScriptFunction_Script::SetScope( idList<idSWFScriptObject *> & newScope ) {
	assert( scope.Num() == 0 );
	for ( int i = 0; i < scope.Num(); i++ ) {
		if ( verify( scope[i] ) ) {
			scope[i]->Release();
		}
	}
	scope.Clear();
	scope.Append( newScope );
	for ( int i = 0; i < newScope.Num(); i++ ) {
		if ( verify( scope[i] ) ) {
			scope[i]->AddRef();
		}
	}
}

/*
========================
idSWFScriptFunction_Script::Call
========================
*/
idSWFScriptVar idSWFScriptFunction_Script::Call( idSWFScriptObject * thisObject, const idSWFParmList & parms ) {
	idSWFBitStream bitstream( data, length, false );

	// We assume scope[0] is the global scope
	assert( scope.Num() > 0 );

	if ( thisObject == NULL ) {
		thisObject = scope[0];
	}

	idSWFScriptObject * locals = idSWFScriptObject::Alloc();

	idSWFStack stack;
	stack.SetNum( parms.Num() + 1 );
	for ( int i = 0; i < parms.Num(); i++ ) {
		stack[ parms.Num() - i - 1 ] = parms[i];

		// Unfortunately at this point we don't have the function name anymore, so our warning messages aren't very detailed
		if ( i < parameters.Num() ) {
			if ( parameters[i].reg > 0 && parameters[i].reg < registers.Num() ) {
				registers[ parameters[i].reg ] = parms[i];
			}
			locals->Set( parameters[i].name, parms[i] );
		}
	}
	// Set any additional parameters to undefined
	for ( int i = parms.Num(); i < parameters.Num(); i++ ) {
		if ( parameters[i].reg > 0 && parameters[i].reg < registers.Num() ) {
			registers[ parameters[i].reg ].SetUndefined();
		}
		locals->Set( parameters[i].name, idSWFScriptVar() );
	}
	stack.A().SetInteger( parms.Num() );

	int preloadReg = 1;
	if ( flags & BIT( 0 ) ) {
		// load "this" into a register
		registers[ preloadReg ].SetObject( thisObject );
		preloadReg++;
	}
	if ( ( flags & BIT( 1 ) ) == 0 ) {
		// create "this"
		locals->Set( "this", idSWFScriptVar( thisObject ) );
	}
	if ( flags & BIT( 2 ) ) {
		idSWFScriptObject * arguments = idSWFScriptObject::Alloc();
		// load "arguments" into a register
		arguments->MakeArray();

		int numElements = parms.Num();

		for ( int i = 0; i < numElements; i++ ) {
			arguments->Set( i, parms[i] );
		}

		registers[ preloadReg ].SetObject( arguments );
		preloadReg++;

		arguments->Release();
	}
	if ( ( flags & BIT( 3 ) ) == 0 ) {
		idSWFScriptObject * arguments = idSWFScriptObject::Alloc();

		// create "arguments"
		arguments->MakeArray();

		int numElements = parms.Num();

		for ( int i = 0; i < numElements; i++ ) {
			arguments->Set( i, parms[i] );
		}

		locals->Set( "arguments", idSWFScriptVar( arguments ) );

		arguments->Release();
	}
	if ( flags & BIT( 4 ) ) {
		// load "super" into a register
		registers[ preloadReg ].SetObject( thisObject->GetPrototype() );
		preloadReg++;
	}
	if ( ( flags & BIT( 5 ) ) == 0 ) {
		// create "super"
		locals->Set( "super", idSWFScriptVar( thisObject->GetPrototype() ) );
	}
	if ( flags & BIT( 6 ) ) {
		// preload _root
		registers[ preloadReg ] = scope[0]->Get( "_root" );
		preloadReg++;
	}
	if ( flags & BIT( 7 ) ) {
		// preload _parent
		if ( thisObject->GetSprite() != NULL && thisObject->GetSprite()->parent != NULL ) {
			registers[ preloadReg ].SetObject( thisObject->GetSprite()->parent->scriptObject );
		} else {
			registers[ preloadReg ].SetNULL();
		}
		preloadReg++;
	}
	if ( flags & BIT( 8 ) ) {
		// load "_global" into a register
		registers[ preloadReg ].SetObject( scope[0] );
		preloadReg++;
	}

	int scopeSize = scope.Num();
	scope.Append( locals );
	locals->AddRef();

	idSWFScriptVar retVal = Run( thisObject, stack, bitstream );

	assert( scope.Num() == scopeSize + 1 );
	for ( int i = scopeSize; i < scope.Num(); i++ ) {
		if ( verify( scope[i] ) ) {
			scope[i]->Release();
		}
	}
	scope.SetNum( scopeSize );

	locals->Release();
	locals = NULL;

	return retVal;
}

/*
========================
<anonymous>::Split
========================
*/
namespace {
	const char * GetPropertyName( int index ) {
		switch ( index ) {
		case 0: return "_x";
		case 1: return "_y";
		case 2: return "_xscale";
		case 3: return "_yscale";
		case 4: return "_currentframe";
		case 5: return "_totalframes";
		case 6: return "_alpha";
		case 7: return "_visible";
		case 8: return "_width";
		case 9: return "_height";
		case 10: return "_rotation";
		case 11: return "_target";
		case 12: return "_framesloaded";
		case 13: return "_name";
		case 14: return "_droptarget";
		case 15: return "_url";
		case 16: return "_highquality";
		case 17: return "_focusrect";
		case 18: return "_soundbuftime";
		case 19: return "_quality";
		case 20: return "_mousex";
		case 21: return "_mousey";
		}
		return "";
	}

	const char *GetSwfActionName(swfAction_t code)
	{
		switch (code)
		{
		case Action_End: return "Action_End";

			// swf 3
		case Action_NextFrame: return "Action_NextFrame";
		case Action_PrevFrame: return "Action_PrevFrame";
		case Action_Play: return "Action_Play";
		case Action_Stop: return "Action_Stop";
		case Action_ToggleQuality: return "Action_ToggleQuality";
		case Action_StopSounds: return "Action_StopSounds";

		case Action_GotoFrame: return "Action_GotoFrame";
		case Action_GetURL: return "Action_GetURL";
		case Action_WaitForFrame: return "Action_WaitForFrame";
		case Action_SetTarget: return "Action_SetTarget";
		case Action_GoToLabel: return "Action_GoToLabel";

			// swf 4
		case Action_Add: return "Action_Add";
		case Action_Subtract: return "Action_Subtract";
		case Action_Multiply: return "Action_Multiply";
		case Action_Divide: return "Action_Divide";
		case Action_Equals: return "Action_Equals";
		case Action_Less: return "Action_Less";
		case Action_And: return "Action_And";
		case Action_Or: return "Action_Or";
		case Action_Not: return "Action_Not";
		case Action_StringEquals: return "Action_StringEquals";
		case Action_StringLength: return "Action_StringLength";
		case Action_StringExtract: return "Action_StringExtract";
		case Action_Pop: return "Action_Pop";
		case Action_ToInteger: return "Action_ToInteger";
		case Action_GetVariable: return "Action_GetVariable";
		case Action_SetVariable: return "Action_SetVariable";
		case Action_SetTarget2: return "Action_SetTarget2";
		case Action_StringAdd: return "Action_StringAdd";
		case Action_GetProperty: return "Action_GetProperty";
		case Action_SetProperty: return "Action_SetProperty";
		case Action_CloneSprite: return "Action_CloneSprite";
		case Action_RemoveSprite: return "Action_RemoveSprite";
		case Action_Trace: return "Action_Trace";
		case Action_StartDrag: return "Action_StartDrag";
		case Action_EndDrag: return "Action_EndDrag";
		case Action_StringLess: return "Action_StringLess";
		case Action_RandomNumber: return "Action_RandomNumber";
		case Action_MBStringLength: return "Action_MBStringLength";
		case Action_CharToAscii: return "Action_CharToAscii";
		case Action_AsciiToChar: return "Action_AsciiToChar";
		case Action_GetTime: return "Action_GetTime";
		case Action_MBStringExtract: return "Action_MBStringExtract";
		case Action_MBCharToAscii: return "Action_MBCharToAscii";
		case Action_MBAsciiToChar: return "Action_MBAsciiToChar";

		case Action_WaitForFrame2: return "Action_WaitForFrame2";
		case Action_Push: return "Action_Push";
		case Action_Jump: return "Action_Jump";
		case Action_GetURL2: return "Action_GetURL2";
		case Action_If: return "Action_If";
		case Action_Call: return "Action_Call";
		case Action_GotoFrame2: return "Action_GotoFrame2";

			// swf 5
		case Action_Delete: return "Action_Delete";
		case Action_Delete2: return "Action_Delete2";
		case Action_DefineLocal: return "Action_DefineLocal";
		case Action_CallFunction: return "Action_CallFunction";
		case Action_Return: return "Action_Return";
		case Action_Modulo: return "Action_Modulo";
		case Action_NewObject: return "Action_NewObject";
		case Action_DefineLocal2: return "Action_DefineLocal2";
		case Action_InitArray: return "Action_InitArray";
		case Action_InitObject: return "Action_InitObject";
		case Action_TypeOf: return "Action_TypeOf";
		case Action_TargetPath: return "Action_TargetPath";
		case Action_Enumerate: return "Action_Enumerate";
		case Action_Add2: return "Action_Add2";
		case Action_Less2: return "Action_Less2";
		case Action_Equals2: return "Action_Equals2";
		case Action_ToNumber: return "Action_ToNumber";
		case Action_ToString: return "Action_ToString";
		case Action_PushDuplicate: return "Action_PushDuplicate";
		case Action_StackSwap: return "Action_StackSwap";
		case Action_GetMember: return "Action_GetMember";
		case Action_SetMember: return "Action_SetMember";
		case Action_Increment: return "Action_Increment";
		case Action_Decrement: return "Action_Decrement";
		case Action_CallMethod: return "Action_CallMethod";
		case Action_NewMethod: return "Action_NewMethod";
		case Action_BitAnd: return "Action_BitAnd";
		case Action_BitOr: return "Action_BitOr";
		case Action_BitXor: return "Action_BitXor";
		case Action_BitLShift: return "Action_BitLShift";
		case Action_BitRShift: return "Action_BitRShift";
		case Action_BitURShift: return "Action_BitURShift";

		case Action_StoreRegister: return "Action_StoreRegister";
		case Action_ConstantPool: return "Action_ConstantPool";
		case Action_With: return "Action_With";
		case Action_DefineFunction: return "Action_DefineFunction";

			// swf 6
		case Action_InstanceOf: return "Action_InstanceOf";
		case Action_Enumerate2: return "Action_Enumerate2";
		case Action_StrictEquals: return "Action_StrictEquals";
		case Action_Greater: return "Action_Greater";
		case Action_StringGreater: return "Action_StringGreater";

			// swf 7
		case Action_Extends: return "Action_Extends";
		case Action_CastOp: return "Action_CastOp";
		case Action_ImplementsOp: return "Action_ImplementsOp";
		case Action_Throw: return "Action_Throw";
		case Action_Try: return "Action_Try";

		case Action_DefineFunction2: return "Action_DefineFunction2";
		default:
			return "UNKNOWN CODE";
		}
	}
}

/*
========================
idSWFScriptFunction_Script::Run
========================
*/
idSWFScriptVar idSWFScriptFunction_Script::Run( idSWFScriptObject * thisObject, idSWFStack & stack, idSWFBitStream & bitstream ) {
	static int callstackLevel = -1;
	idSWFSpriteInstance * thisSprite = thisObject->GetSprite();
	idSWFSpriteInstance * currentTarget = thisSprite;

	if ( currentTarget == NULL ) {
		thisSprite = currentTarget = defaultSprite;
	}

	callstackLevel++;

	while ( bitstream.Tell() < bitstream.Length() ) {
		swfAction_t code = (swfAction_t)bitstream.ReadU8();
		uint16 recordLength = 0;
		if ( code >= 0x80 ) {
			recordLength = bitstream.ReadU16();
		}

		if ( swf_debug.GetInteger() >= 3 ) {
			// stack[0] is always 0 so don't read it
			if ( swf_debug.GetInteger() >= 4 ) {
				for ( int i = stack.Num()-1; i >= 0 ; i-- ) {
					idLib::Printf("  %c: %s (%s)\n", (char)(64 + stack.Num() - i), stack[i].ToString().c_str(), stack[i].TypeOf());
				}

				for ( int i = 0; i < registers.Num(); i++ ) {
					if ( !registers[i].IsUndefined() ) {
						idLib::Printf(" R%d: %s (%s)\n", i, registers[i].ToString().c_str(), registers[i].TypeOf());
					}
				}
			}

			idLib::Printf( "SWF%d: code %s\n", callstackLevel, GetSwfActionName(code) );
		}

		switch ( code ) {
			case Action_Return:
				callstackLevel--;
				return stack.A();
			case Action_End:
				callstackLevel--;
				return idSWFScriptVar();
			case Action_NextFrame:
				if ( verify( currentTarget != NULL ) ) {
					currentTarget->NextFrame();
				} else if ( swf_debug.GetInteger() > 0 ) {
					idLib::Printf( "SWF: no target movie clip for nextFrame\n" );
				}
				break;
			case Action_PrevFrame:
				if ( verify( currentTarget != NULL ) ) {
					currentTarget->PrevFrame();
				} else if ( swf_debug.GetInteger() > 0 ) {
					idLib::Printf( "SWF: no target movie clip for prevFrame\n" );
				}
				break;
			case Action_Play:
				if ( verify( currentTarget != NULL ) ) {
					currentTarget->Play();
				} else if ( swf_debug.GetInteger() > 0 ) {
					idLib::Printf( "SWF: no target movie clip for play\n" );
				}
				break;
			case Action_Stop:
				if ( verify( currentTarget != NULL ) ) {
					currentTarget->Stop();
				} else if ( swf_debug.GetInteger() > 0 ) {
					idLib::Printf( "SWF: no target movie clip for stop\n" );
				}
				break;
			case Action_ToggleQuality: break;
			case Action_StopSounds: break;
			case Action_GotoFrame: {
				assert( recordLength == 2 );
				int frameNum = bitstream.ReadU16() + 1;
				if ( verify( currentTarget != NULL ) ) {
					currentTarget->RunTo( frameNum );
				} else if ( swf_debug.GetInteger() > 0 ) {
					idLib::Printf( "SWF: no target movie clip for runTo %d\n", frameNum );
				}
				break;
			}
			case Action_SetTarget: {
				const char * targetName = (const char *)bitstream.ReadData( recordLength );
				if ( verify( thisSprite != NULL ) ) {
					currentTarget = thisSprite->ResolveTarget( targetName );
				} else if ( swf_debug.GetInteger() > 0 ) {
					idLib::Printf( "SWF: no target movie clip for setTarget %s\n", targetName );
				}
				break;
			}
			case Action_GoToLabel: {
				const char * targetName = (const char *)bitstream.ReadData( recordLength );
				if ( verify( currentTarget != NULL ) ) {
					currentTarget->RunTo( currentTarget->FindFrame( targetName ) );
				} else if ( swf_debug.GetInteger() > 0 ) {
					idLib::Printf( "SWF: no target movie clip for runTo %s\n", targetName );
				}
				break;
			}
			case Action_Push: {
				idSWFBitStream pushstream( bitstream.ReadData( recordLength ), recordLength, false );
				while ( pushstream.Tell() < pushstream.Length() ) {
					uint8 type = pushstream.ReadU8();
					switch ( type ) {
					case 0: stack.Alloc().SetString( pushstream.ReadString() ); break;
					case 1: stack.Alloc().SetFloat( pushstream.ReadFloat() ); break;
					case 2: stack.Alloc().SetNULL(); break;
					case 3: stack.Alloc().SetUndefined(); break;
					case 4: stack.Alloc() = registers[ pushstream.ReadU8() ]; break;
					case 5: stack.Alloc().SetBool( pushstream.ReadU8() != 0 ); break;
					case 6: stack.Alloc().SetFloat( (float)pushstream.ReadDouble() ); break;
					case 7: stack.Alloc().SetInteger( pushstream.ReadS32() ); break;
					case 8: stack.Alloc().SetString( constants.Get( pushstream.ReadU8() ) ); break;
					case 9: stack.Alloc().SetString( constants.Get( pushstream.ReadU16() ) ); break;
					}
				}
				break;
			}
			case Action_Pop:
				stack.Pop( 1 );
				break;
			case Action_Add:
				stack.B().SetFloat( stack.B().ToFloat() + stack.A().ToFloat() );
				stack.Pop( 1 );
				break;
			case Action_Subtract:
				stack.B().SetFloat( stack.B().ToFloat() - stack.A().ToFloat() );
				stack.Pop( 1 );
				break;
			case Action_Multiply:
				stack.B().SetFloat( stack.B().ToFloat() * stack.A().ToFloat() );
				stack.Pop( 1 );
				break;
			case Action_Divide:
				stack.B().SetFloat( stack.B().ToFloat() / stack.A().ToFloat() );
				stack.Pop( 1 );
				break;
			case Action_Equals:
				stack.B().SetBool( stack.B().ToFloat() == stack.A().ToFloat() );
				stack.Pop( 1 );
				break;
			case Action_Less:
				stack.B().SetBool( stack.B().ToFloat() < stack.A().ToFloat() );
				stack.Pop( 1 );
				break;
			case Action_And:
				stack.B().SetBool( stack.B().ToBool() && stack.A().ToBool() );
				stack.Pop( 1 );
				break;
			case Action_Or:
				stack.B().SetBool( stack.B().ToBool() || stack.A().ToBool() );
				stack.Pop( 1 );
				break;
			case Action_Not:
				stack.A().SetBool( !stack.A().ToBool() );
				break;
			case Action_StringEquals:
				stack.B().SetBool( stack.B().ToString() == stack.A().ToString() );
				stack.Pop( 1 );
				break;
			case Action_StringLength:
				stack.A().SetInteger( stack.A().ToString().Length() );
				break;
			case Action_StringAdd:
				stack.B().SetString( stack.B().ToString() + stack.A().ToString() );
				stack.Pop( 1 );
				break;
			case Action_StringExtract:
				stack.C().SetString( stack.C().ToString().Mid( stack.B().ToInteger(), stack.A().ToInteger() ) );
				stack.Pop( 2 );
				break;
			case Action_StringLess:
				stack.B().SetBool( stack.B().ToString() < stack.A().ToString() );
				stack.Pop( 1 );
				break;
			case Action_StringGreater:
				stack.B().SetBool( stack.B().ToString() > stack.A().ToString() );
				stack.Pop( 1 );
				break;
			case Action_ToInteger:
				stack.A().SetInteger( stack.A().ToInteger() );
				break;
			case Action_CharToAscii:
				stack.A().SetInteger( stack.A().ToString()[0] );
				break;
			case Action_AsciiToChar:
				stack.A().SetString( va( "%c", stack.A().ToInteger() ) );
				break;
			case Action_Jump:
				bitstream.Seek( bitstream.ReadS16() );
				break;
			case Action_If: {
				int16 offset = bitstream.ReadS16();
				if ( stack.A().ToBool() ) {
					bitstream.Seek( offset );
				}
				stack.Pop( 1 );
				break;
			}
			case Action_GetVariable: {
				idStr variableName = stack.A().ToString();
				for ( int i = scope.Num() - 1; i >= 0; i-- ) {
					stack.A() = scope[i]->Get( variableName );
					if ( !stack.A().IsUndefined() ) {
						break;
					}
				}
				if ( stack.A().IsUndefined() && swf_debug.GetInteger() > 1 ) {
					idLib::Printf( "SWF: unknown variable %s\n", variableName.c_str() );
				}
				break;
			}
			case Action_SetVariable: {
				idStr variableName = stack.B().ToString();
				bool found = false;
				for ( int i = scope.Num() - 1; i >= 0; i-- ) {
					if ( scope[i]->HasProperty( variableName ) ) {
						scope[i]->Set( variableName, stack.A() );
						found = true;
						break;
					}
				}
				if ( !found ) {
					thisObject->Set( variableName, stack.A() );
				}
				stack.Pop( 2 );
				break;
			}
			case Action_GotoFrame2: {

				uint32 frameNum = 0;
				uint8 flags = bitstream.ReadU8();
				if ( flags & 2 ) {
					frameNum += bitstream.ReadU16();
				}

				if ( verify( thisSprite != NULL ) ) {
					if ( stack.A().IsString() ) {
						frameNum += thisSprite->FindFrame( stack.A().ToString() );
					} else {
						frameNum += (uint32)stack.A().ToInteger();
					}
					if ( ( flags & 1 ) != 0 ){
						thisSprite->Play();
					} else {
						thisSprite->Stop();
					}
					thisSprite->RunTo( frameNum );
				} else if ( swf_debug.GetInteger() > 0 ) {
					if ( ( flags & 1 ) != 0 ){
						idLib::Printf( "SWF: no target movie clip for gotoAndPlay\n" );
					} else {
						idLib::Printf( "SWF: no target movie clip for gotoAndStop\n" );
					}
				}
				stack.Pop( 1 );
				break;
			}
			case Action_GetProperty: {
				if ( verify( thisSprite != NULL ) ) {
					idSWFSpriteInstance * target = thisSprite->ResolveTarget( stack.B().ToString() );
					stack.B() = target->scriptObject->Get( GetPropertyName( stack.A().ToInteger() ) );
				} else if ( swf_debug.GetInteger() > 0 ) {
					idLib::Printf( "SWF: no target movie clip for getProperty\n" );
				}
				stack.Pop( 1 );
				break;
			}
			case Action_SetProperty: {
				if ( verify( thisSprite != NULL ) ) {
					idSWFSpriteInstance * target = thisSprite->ResolveTarget( stack.C().ToString() );
					target->scriptObject->Set( GetPropertyName( stack.B().ToInteger() ), stack.A() );
				} else if ( swf_debug.GetInteger() > 0 ) {
					idLib::Printf( "SWF: no target movie clip for setProperty\n" );
				}
				stack.Pop( 3 );
				break;
			}
			case Action_Trace:
				idLib::PrintfIf( swf_debug.GetInteger() > 0, "SWF Trace: %s\n", stack.A().ToString().c_str() );
				stack.Pop( 1 );
				break;
			case Action_GetTime:
				stack.Alloc().SetInteger( Sys_Milliseconds() );
				break;
			case Action_RandomNumber:
				assert( thisSprite && thisSprite->sprite && thisSprite->sprite->GetSWF() );
				stack.A().SetInteger( thisSprite->sprite->GetSWF()->GetRandom().RandomInt( stack.A().ToInteger() ) );
				break;
			case Action_CallFunction: {
				idStr functionName = stack.A().ToString();
				idSWFScriptVar function;
				idSWFScriptObject * object = NULL;
				for ( int i = scope.Num() - 1; i >= 0; i-- ) {
					function = scope[i]->Get( functionName );
					if ( !function.IsUndefined() ) {
						object = scope[i];
						break;
					}
				}
				stack.Pop( 1 );

				idSWFParmList parms;
				parms.SetNum( stack.A().ToInteger() );
				stack.Pop( 1 );
				for ( int i = 0; i < parms.Num(); i++ ) {
					parms[i] = stack.A();
					stack.Pop( 1 );
				}

				if ( function.IsFunction() && verify( object ) ) {
					stack.Alloc() = function.GetFunction()->Call( object, parms );
				} else {
					idLib::PrintfIf( swf_debug.GetInteger() > 0, "SWF: unknown function %s\n", functionName.c_str() );
					stack.Alloc().SetUndefined();
				}

				break;
			}
			case Action_CallMethod: {
				idStr functionName = stack.A().ToString();
				// If the top stack is undefined but there is an object, it's calling the constructor
				if ( functionName.IsEmpty() || stack.A().IsUndefined() || stack.A().IsNULL() ) {
					functionName = "__constructor__";
				}
				idSWFScriptObject * object = NULL;
				idSWFScriptVar function;
				if ( stack.B().IsObject() ) {
					object = stack.B().GetObject();
					function = object->Get( functionName );
					if ( !function.IsFunction() ) {
						idLib::PrintfIf( swf_debug.GetInteger() > 1, "SWF: unknown method %s on %s\n", functionName.c_str(), object->DefaultValue( true ).ToString().c_str() );
					}
				} else {
					idLib::PrintfIf( swf_debug.GetInteger() > 1, "SWF: NULL object for method %s\n", functionName.c_str() );
				}

				stack.Pop( 2 );

				idSWFParmList parms;
				parms.SetNum( stack.A().ToInteger() );
				stack.Pop( 1 );
				for ( int i = 0; i < parms.Num(); i++ ) {
					parms[i] = stack.A();
					stack.Pop( 1 );
				}

				if ( function.IsFunction() ) {
					stack.Alloc() = function.GetFunction()->Call( object, parms );
				} else {
					stack.Alloc().SetUndefined();
				}
				break;
			}
			case Action_ConstantPool: {
				constants.Clear();
				uint16 numConstants = bitstream.ReadU16();
				for ( int i = 0; i < numConstants; i++ ) {
					constants.Append( idSWFScriptString::Alloc( bitstream.ReadString() ) );
				}
				break;
			}
			case Action_DefineFunction: {
				idStr functionName = bitstream.ReadString();

				idSWFScriptFunction_Script * newFunction = idSWFScriptFunction_Script::Alloc();
				newFunction->SetScope( scope );
				newFunction->SetConstants( constants );
				newFunction->SetDefaultSprite( defaultSprite );

				uint16 numParms = bitstream.ReadU16();
				newFunction->AllocParameters( numParms );
				for ( int i = 0; i < numParms; i++ ) {
					newFunction->SetParameter( i, 0, bitstream.ReadString() );
				}
				uint16 codeSize = bitstream.ReadU16();
				newFunction->SetData( bitstream.ReadData( codeSize ), codeSize );

				if ( functionName.IsEmpty() ) {
					stack.Alloc().SetFunction( newFunction );
				} else {
					thisObject->Set( functionName, idSWFScriptVar( newFunction ) );
				}
				newFunction->Release();
				break;
			}
			case Action_DefineFunction2: {
				idStr functionName = bitstream.ReadString();

				idSWFScriptFunction_Script * newFunction = idSWFScriptFunction_Script::Alloc();
				newFunction->SetScope( scope );
				newFunction->SetConstants( constants );
				newFunction->SetDefaultSprite( defaultSprite );

				uint16 numParms = bitstream.ReadU16();

				// The number of registers is from 0 to 255, although valid values are 1 to 256.
				// There must always be at least one register for DefineFunction2, to hold "this" or "super" when required.
				uint8 numRegs = bitstream.ReadU8() + 1;

				// Note that SWF byte-ordering causes the flag bits to be reversed per-byte
				// from how the swf_file_format_spec_v10.pdf document describes the ordering in ActionDefineFunction2.
				// PreloadThisFlag is byte 0, not 7, PreloadGlobalFlag is 8, not 15.
				uint16 flags = bitstream.ReadU16();

				newFunction->AllocParameters( numParms );
				newFunction->AllocRegisters( numRegs );
				newFunction->SetFlags( flags );

				for ( int i = 0; i < numParms; i++ ) {
					uint8 reg = bitstream.ReadU8();
					const char * name = bitstream.ReadString();
					if ( reg >= numRegs ) {
						idLib::Warning( "SWF: Parameter %s in function %s bound to out of range register %d", name, functionName.c_str(), reg );
						reg = 0;
					}
					newFunction->SetParameter( i, reg, name );
				}

				uint16 codeSize = bitstream.ReadU16();
				newFunction->SetData( bitstream.ReadData( codeSize ), codeSize );

				if ( functionName.IsEmpty() ) {
					stack.Alloc().SetFunction( newFunction );
				} else {
					thisObject->Set( functionName, idSWFScriptVar( newFunction ) );
				}
				newFunction->Release();
				break;
			}
			case Action_Enumerate: {
				idStr variableName = stack.A().ToString();
				for ( int i = scope.Num() - 1; i >= 0; i-- ) {
					stack.A() = scope[i]->Get( variableName );
					if ( !stack.A().IsUndefined() ) {
						break;
					}
				}
				if ( !stack.A().IsObject() ) {
					stack.A().SetNULL();
				} else {
					idSWFScriptObject * object = stack.A().GetObject();
					object->AddRef();
					stack.A().SetNULL();
					for ( int i = 0; i < object->NumVariables(); i++ ) {
						stack.Alloc().SetString( object->EnumVariable( i ) );
					}
					object->Release();
				}
				break;
			}
			case Action_Enumerate2: {
				if ( !stack.A().IsObject() ) {
					stack.A().SetNULL();
				} else {
					idSWFScriptObject * object = stack.A().GetObject();
					object->AddRef();
					stack.A().SetNULL();
					for ( int i = 0; i < object->NumVariables(); i++ ) {
						stack.Alloc().SetString( object->EnumVariable( i ) );
					}
					object->Release();
				}
				break;
			}
			case Action_Equals2: {
				stack.B().SetBool( stack.A().AbstractEquals( stack.B() ) );
				stack.Pop( 1 );
				break;
			}
			case Action_StrictEquals: {
				stack.B().SetBool( stack.A().StrictEquals( stack.B() ) );
				stack.Pop( 1 );
				break;
			}
			case Action_GetMember: {
				if ( ( stack.B().IsUndefined() || stack.B().IsNULL() ) && swf_debug.GetInteger() > 1 ) {
					idLib::Printf( "SWF: tried to get member %s on an invalid object in sprite '%s'\n", stack.A().ToString().c_str(), thisSprite != NULL ? thisSprite->GetName() : "" );
				}
				if ( stack.B().IsObject() ) {
					idSWFScriptObject * object = stack.B().GetObject();
					if ( stack.A().IsNumeric() ) {
						stack.B() = object->Get( stack.A().ToInteger() );
					} else {
						stack.B() = object->Get( stack.A().ToString() );
					}
					if ( stack.B().IsUndefined() && swf_debug.GetInteger() > 1 ) {
						idLib::Printf( "SWF: unknown member %s\n", stack.A().ToString().c_str() );
					}
				} else if ( stack.B().IsString() ) {
					idStr propertyName = stack.A().ToString();
					if ( propertyName.Cmp( "length" ) == 0 ) {
						stack.B().SetInteger( stack.B().ToString().Length() );
					} else if ( propertyName.Cmp( "value" ) == 0 ) {
						// Do nothing
					} else {
						stack.B().SetUndefined();
					}
				} else if ( stack.B().IsFunction() ) {
					idStr propertyName = stack.A().ToString();
					if ( propertyName.Cmp( "prototype" ) == 0 ) {
						// if this is a function, it's a class definition function, and it just wants the prototype object
						// create it if it hasn't been already, and return it
						idSWFScriptFunction * sfs = stack.B().GetFunction();
						idSWFScriptObject * object = sfs->GetPrototype();

						if ( object == NULL ) {
							object = idSWFScriptObject::Alloc();
							// Set the __proto__ to the main Object prototype
							idSWFScriptVar baseObjConstructor = scope[0]->Get( "Object" );
							idSWFScriptFunction *baseObj = baseObjConstructor.GetFunction();
							object->Set( "__proto__", baseObj->GetPrototype() );
							sfs->SetPrototype( object );
						}

						stack.B() = idSWFScriptVar( object );
					} else {
						stack.B().SetUndefined();
					}
				} else {
					stack.B().SetUndefined();
				}
				stack.Pop( 1 );
				break;
			}
			case Action_SetMember: {
				if ( stack.C().IsObject() ) {
					idSWFScriptObject * object = stack.C().GetObject();
					if ( stack.B().IsNumeric() ) {
						object->Set( stack.B().ToInteger(), stack.A() );
					} else {
						object->Set( stack.B().ToString(), stack.A() );
					}
				}
				stack.Pop( 3 );
				break;
			}
			case Action_InitArray: {
				idSWFScriptObject * object = idSWFScriptObject::Alloc();
				object->MakeArray();

				int numElements = stack.A().ToInteger();
				stack.Pop( 1 );

				for ( int i = 0; i < numElements; i++ ) {
					object->Set( i, stack.A() );
					stack.Pop( 1 );
				}

				stack.Alloc().SetObject( object );

				object->Release();
				break;
			}
			case Action_InitObject: {
				idSWFScriptObject * object = idSWFScriptObject::Alloc();

				int numElements = stack.A().ToInteger();
				stack.Pop( 1 );

				for ( int i = 0; i < numElements; i++ ) {
					object->Set( stack.B().ToString(), stack.A() );
					stack.Pop( 2 );
				}

				stack.Alloc().SetObject( object );

				object->Release();
				break;
			}
			case Action_NewObject: {
				idSWFScriptObject * object = idSWFScriptObject::Alloc();

				idStr functionName = stack.A().ToString();
				stack.Pop( 1 );

				if ( functionName.Cmp( "Array" ) == 0 ) {
					object->MakeArray();

					int numElements = stack.A().ToInteger();
					stack.Pop( 1 );

					for ( int i = 0; i < numElements; i++ ) {
						object->Set( i, stack.A() );
						stack.Pop( 1 );
					}

					idSWFScriptVar baseObjConstructor = scope[0]->Get( "Object" );
					idSWFScriptFunction *baseObj = baseObjConstructor.GetFunction();
					object->Set( "__proto__", baseObj->GetPrototype() );
					// object prototype is not set here because it will be auto created from Object later
				} else {
					idSWFParmList parms;
					parms.SetNum( stack.A().ToInteger() );
					stack.Pop( 1 );
					for ( int i = 0; i < parms.Num(); i++ ) {
						parms[i] = stack.A();
						stack.Pop( 1 );
					}

					idSWFScriptVar objdef = scope[0]->Get( functionName );
					if ( objdef.IsFunction() ) {
						idSWFScriptFunction * constructorFunction = objdef.GetFunction();
						object->Set( "__proto__", constructorFunction->GetPrototype() );
						object->SetPrototype( constructorFunction->GetPrototype() );
						constructorFunction->Call( object, parms );
					} else {
						idLib::Warning( "SWF: Unknown class definition %s", functionName.c_str() );
					}
				}

				stack.Alloc().SetObject( object );

				object->Release();
				break;
			}
			case Action_Extends: {
				idSWFScriptFunction * superclassConstructorFunction = stack.A().GetFunction();
				idSWFScriptFunction *subclassConstructorFunction = stack.B().GetFunction();
				stack.Pop( 2 );

				idSWFScriptObject * scriptObject = idSWFScriptObject::Alloc();
				scriptObject->SetPrototype( superclassConstructorFunction->GetPrototype() );
				scriptObject->Set( "__proto__", idSWFScriptVar( superclassConstructorFunction->GetPrototype() ) );
				scriptObject->Set( "__constructor__", idSWFScriptVar( superclassConstructorFunction ) );

				subclassConstructorFunction->SetPrototype( scriptObject );

				scriptObject->Release();
				break;
			}
			case Action_TargetPath: {
				if ( !stack.A().IsObject() ) {
					stack.A().SetUndefined();
				} else {
					idSWFScriptObject * object = stack.A().GetObject();
					if ( object->GetSprite() == NULL ) {
						stack.A().SetUndefined();
					} else {
						idStr dotName = object->GetSprite()->name.c_str();
						for ( idSWFSpriteInstance * target = object->GetSprite()->parent; target != NULL; target = target->parent ) {
							dotName = target->name + "." + dotName;
						}
						stack.A().SetString( dotName );
					}
				}
				break;
			}
			case Action_With: {
				int withSize = bitstream.ReadU16();
				idSWFBitStream bitstream2( bitstream.ReadData( withSize ), withSize, false );
				if ( stack.A().IsObject() ) {
					idSWFScriptObject * withObject = stack.A().GetObject();
					withObject->AddRef();
					stack.Pop( 1 );
					scope.Append( withObject );
					Run( thisObject, stack, bitstream2 );
					scope.SetNum( scope.Num() - 1 );
					withObject->Release();
				} else {
					if ( swf_debug.GetInteger() > 0 ) {
						idLib::Printf( "SWF: with() invalid object specified\n" );
					}
					stack.Pop( 1 );
				}
				break;
			}
			case Action_ToNumber:
				stack.A().SetFloat( stack.A().ToFloat() );
				break;
			case Action_ToString:
				stack.A().SetString( stack.A().ToString() );
				break;
			case Action_TypeOf:
				stack.A().SetString( stack.A().TypeOf() );
				break;
			case Action_Add2: {
				if ( stack.A().IsString() || stack.B().IsString() ) {
					stack.B().SetString( stack.B().ToString() + stack.A().ToString() );
				} else {
					stack.B().SetFloat( stack.B().ToFloat() + stack.A().ToFloat() );
				}
				stack.Pop( 1 );
				break;
			}
			case Action_Less2: {
				if ( stack.A().IsString() && stack.B().IsString() ) {
					stack.B().SetBool( stack.B().ToString() < stack.A().ToString() );
				} else {
					stack.B().SetBool( stack.B().ToFloat() < stack.A().ToFloat() );
				}
				stack.Pop( 1 );
				break;
			}
			case Action_Greater: {
				if ( stack.A().IsString() && stack.B().IsString() ) {
					stack.B().SetBool( stack.B().ToString() > stack.A().ToString() );
				} else {
					stack.B().SetBool( stack.B().ToFloat() > stack.A().ToFloat() );
				}
				stack.Pop( 1 );
				break;
			}
			case Action_Modulo: {
				int32 a = stack.A().ToInteger();
				int32 b = stack.B().ToInteger();
				if ( a == 0 ) {
					stack.B().SetUndefined();
				} else {
					stack.B().SetInteger( b % a );
				}
				stack.Pop( 1 );
				break;
			}
			case Action_BitAnd:
				stack.B().SetInteger( stack.B().ToInteger() & stack.A().ToInteger() );
				stack.Pop( 1 );
				break;
			case Action_BitLShift:
				stack.B().SetInteger( stack.B().ToInteger() << stack.A().ToInteger() );
				stack.Pop( 1 );
				break;
			case Action_BitOr:
				stack.B().SetInteger( stack.B().ToInteger() | stack.A().ToInteger() );
				stack.Pop( 1 );
				break;
			case Action_BitRShift:
				stack.B().SetInteger( stack.B().ToInteger() >> stack.A().ToInteger() );
				stack.Pop( 1 );
				break;
			case Action_BitURShift:
				stack.B().SetInteger( (uint32)stack.B().ToInteger() >> stack.A().ToInteger() );
				stack.Pop( 1 );
				break;
			case Action_BitXor:
				stack.B().SetInteger( stack.B().ToInteger() ^ stack.A().ToInteger() );
				stack.Pop( 1 );
				break;
			case Action_Decrement:
				stack.A().SetFloat( stack.A().ToFloat() - 1.0f );
				break;
			case Action_Increment:
				stack.A().SetFloat( stack.A().ToFloat() + 1.0f );
				break;
			case Action_PushDuplicate: {
				idSWFScriptVar dup = stack.A();
				stack.Alloc() = dup;
				break;
			}
			case Action_StackSwap: {
				idSWFScriptVar temp = stack.A();
				stack.A() = stack.B();
				stack.A() = temp;
				break;
			}
			case Action_StoreRegister: {
				uint8 registerNumber = bitstream.ReadU8();
				registers[ registerNumber ] = stack.A();
				break;
			}
			case Action_DefineLocal: {
				scope[scope.Num()-1]->Set( stack.B().ToString(), stack.A() );
				stack.Pop( 2 );
				break;
			}
			case Action_DefineLocal2: {
				scope[scope.Num()-1]->Set( stack.A().ToString(), idSWFScriptVar() );
				stack.Pop( 1 );
				break;
			}
			case Action_Delete: {
				if ( swf_debug.GetInteger() > 0 ) {
					idLib::Printf( "SWF: Delete ignored\n" );
				}
				// We no longer support deleting variables because the performance cost of updating the hash tables is not worth it
				stack.Pop( 2 );
				break;
			}
			case Action_Delete2: {
				if ( swf_debug.GetInteger() > 0 ) {
					idLib::Printf( "SWF: Delete2 ignored\n" );
				}
				// We no longer support deleting variables because the performance cost of updating the hash tables is not worth it
				stack.Pop( 1 );
				break;
			}
			// These are functions we just don't support because we never really needed to
			case Action_CloneSprite:
			case Action_RemoveSprite:
			case Action_Call:
			case Action_SetTarget2:
			case Action_NewMethod:
			default:
				idLib::Warning( "SWF: Unhandled Action %s", idSWF::GetActionName( code ) );
				// We have to abort here because the rest of the script is basically meaningless now
				assert( false );
				callstackLevel--;
				return idSWFScriptVar();
		}
	}
	callstackLevel--;
	return idSWFScriptVar();
}

//#modified-fva; BEGIN
bool idSWFScriptFunction_Script::CstWriteJson_DoAction(JsonPrettyWriter &writer, idSWFBitStream &bitstream, idList<idStr> &errorMsg) {
	writer.StartObject();
	{
		writer.Key("tag");
		writer.String("DoAction");
		writer.Key("actions");
		if (!CstWriteJson_Actions(writer, bitstream, errorMsg)) {
			return false;
		}
	}
	writer.EndObject();
	return true;
}

// ===============
bool idSWFScriptFunction_Script::CstWriteJson_DoInitAction(JsonPrettyWriter &writer, idSWFBitStream &bitstream, idList<idStr> &errorMsg) {
	return CstWriteJson_Actions(writer, bitstream, errorMsg);
}

// ===============
bool idSWFScriptFunction_Script::CstWriteJson_Actions(JsonPrettyWriter &writer, idSWFBitStream &bitstream, idList<idStr> &errorMsg) {
	// See:
	//  idSWFSpriteInstance::RunTo,
	//  idSWFSpriteInstance::DoAction,
	//  idSWFSpriteInstance::RunActions,
	//  idSWFScriptFunction_Script::Call,
	//  idSWFScriptFunction_Script::Run

	struct local {
		static idStr GetActionName(swfAction_t code) {
			idStr name = "Action";
			name += idSWF::GetActionName(code);
			return name;
		}
		static idStr GetJumpLabelName(int jumpIndex) {
			idStr name = "jump_";
			name += idStr(jumpIndex);
			return name;
		}
	};

	writer.StartArray();
	{
		// Position of each action in the bitstream
		idList<uint32> actionPos;
		// Destination of jump actions (Action_If, Action_Jump) in the bitstream. No duplicate values.
		idList<uint32> jumpPos;

		int actionIndex = -1;
		const uint32 initialStreamPos = bitstream.Tell();

		// First pass: Build the position tables.
		idSWFBitStream auxStream(bitstream.Ptr(), bitstream.Length(), false);
		auxStream.Seek((int32)initialStreamPos);
		while (auxStream.Tell() < auxStream.Length()) {
			actionPos.Append(auxStream.Tell());
			++actionIndex;

			swfAction_t code = (swfAction_t)auxStream.ReadU8();
			uint16 recordLength = 0;
			if (code >= 0x80) {
				recordLength = auxStream.ReadU16();
			}

			idStr actionName = local::GetActionName(code);

			switch (code) {
			case Action_Return:
			case Action_End:
			case Action_NextFrame:
			case Action_PrevFrame:
			case Action_Play:
			case Action_Stop:
			case Action_ToggleQuality:
			case Action_StopSounds: {
				break;
			}
			case Action_GotoFrame: {
				auxStream.ReadU16();
				break;
			}
			case Action_SetTarget:
			case Action_GoToLabel:
			case Action_Push: {
				auxStream.ReadData(recordLength);
				break;
			}
			case Action_Pop:
			case Action_Add:
			case Action_Subtract:
			case Action_Multiply:
			case Action_Divide:
			case Action_Equals:
			case Action_Less:
			case Action_And:
			case Action_Or:
			case Action_Not:
			case Action_StringEquals:
			case Action_StringLength:
			case Action_StringAdd:
			case Action_StringExtract:
			case Action_StringLess:
			case Action_StringGreater:
			case Action_ToInteger:
			case Action_CharToAscii:
			case Action_AsciiToChar: {
				break;
			}
			case Action_Jump:
			case Action_If: {
				int16 offset = auxStream.ReadS16();
				// auxStream is pointing to the next action right now
				int64 destinationPos = (int64)auxStream.Tell() + offset;
				if (destinationPos < (int64)initialStreamPos || destinationPos > (int64)auxStream.Length()) {
					idStr msg;
					msg.Format("(%d) %s: Jump destination isn't reachable.", actionIndex, actionName.c_str());
					errorMsg.Append(msg);
					return false;
				}
				jumpPos.AddUnique((uint32)destinationPos);
				break;
			}
			case Action_GetVariable:
			case Action_SetVariable: {
				break;
			}
			case Action_GotoFrame2: {
				uint8 flags = auxStream.ReadU8();
				if (flags & 2) {
					auxStream.ReadU16();
				}
				break;
			}
			case Action_GetProperty:
			case Action_SetProperty:
			case Action_Trace:
			case Action_GetTime:
			case Action_RandomNumber:
			case Action_CallFunction:
			case Action_CallMethod: {
				break;
			}
			case Action_ConstantPool: {
				uint16 numConstants = auxStream.ReadU16();
				for (unsigned i = 0; i < numConstants; ++i) {
					auxStream.ReadString();
				}
				break;
			}
			case Action_DefineFunction: {
				auxStream.ReadString();
				uint16 numParms = auxStream.ReadU16();
				for (int i = 0; i < numParms; ++i) {
					auxStream.ReadString();
				}
				uint16 codeSize = auxStream.ReadU16();
				auxStream.ReadData(codeSize);
				break;
			}
			case Action_DefineFunction2: {
				auxStream.ReadString();
				uint16 numParms = auxStream.ReadU16();
				auxStream.ReadU8();
				auxStream.ReadU16();
				for (unsigned i = 0; i < numParms; ++i) {
					auxStream.ReadU8();
					auxStream.ReadString();
				}
				uint16 codeSize = auxStream.ReadU16();
				auxStream.ReadData(codeSize);
				break;
			}
			case Action_Enumerate:
			case Action_Enumerate2:
			case Action_Equals2:
			case Action_StrictEquals:
			case Action_GetMember:
			case Action_SetMember:
			case Action_InitArray:
			case Action_InitObject:
			case Action_NewObject:
			case Action_Extends:
			case Action_TargetPath: {
				break;
			}
			case Action_With: {
				int withSize = auxStream.ReadU16();
				auxStream.ReadData(withSize);
				break;
			}
			case Action_ToNumber:
			case Action_ToString:
			case Action_TypeOf:
			case Action_Add2:
			case Action_Less2:
			case Action_Greater:
			case Action_Modulo:
			case Action_BitAnd:
			case Action_BitLShift:
			case Action_BitOr:
			case Action_BitRShift:
			case Action_BitURShift:
			case Action_BitXor:
			case Action_Decrement:
			case Action_Increment:
			case Action_PushDuplicate:
			case Action_StackSwap: {
				break;
			}
			case Action_StoreRegister: {
				auxStream.ReadU8();
				break;
			}
			case Action_DefineLocal:
			case Action_DefineLocal2:
			case Action_Delete:
			case Action_Delete2: {
				break;
			}
			// not supported
			case Action_CloneSprite:
			case Action_RemoveSprite:
			case Action_Call:
			case Action_SetTarget2:
			case Action_NewMethod:
			default: {
				idStr msg;
				msg.Format("(%d) %s: This action isn't supported.", actionIndex, actionName.c_str());
				errorMsg.Append(msg);
				return false;
			}
			}
		}
		// the end of stream position is also valid for jumps (works like ActionEnd)
		actionPos.Append(bitstream.Length());

		// Sorting the jump table facilitates generating the labels
		class SortUint32 : public idSort_Quick<uint32, SortUint32> {
		public:
			int Compare(const uint32 &a, const uint32 &b) const {
				if (a < b) {
					return -1;
				}
				if (a > b) {
					return 1;
				}
				return 0;
			}
		};
		jumpPos.SortWithTemplate(SortUint32());

		actionIndex = -1;

		// Second pass: Write the action sequence.
		while (bitstream.Tell() < bitstream.Length()) {
			// if a jump to this action exists, insert a jump label before it
			{
				int jumpIndex = jumpPos.FindIndex(bitstream.Tell());
				if (jumpIndex >= 0) {
					writer.StartArray();
					writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
					{
						writer.String("CstJumpLabel");
						writer.String(local::GetJumpLabelName(jumpIndex).c_str());
					}
					writer.EndArray();
					writer.SetFormatOptions(rapidjson::kFormatDefault);
				}
			}
			++actionIndex;

			swfAction_t code = (swfAction_t)bitstream.ReadU8();
			uint16 recordLength = 0;
			if (code >= 0x80) {
				recordLength = bitstream.ReadU16();
			}

			idStr actionName = local::GetActionName(code);

			switch (code) {
			case Action_Return:
			case Action_End: // end of actions
			case Action_NextFrame:
			case Action_PrevFrame:
			case Action_Play:
			case Action_Stop:
			case Action_ToggleQuality: // ignored when executed
			case Action_StopSounds: { // ignored when executed
				writer.String(actionName.c_str());
				break;
			}
			case Action_GotoFrame: {
				unsigned frameNum = bitstream.ReadU16() + 1; // frames are 1 based
				writer.StartArray();
				writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
				{
					writer.String(actionName.c_str());
					writer.Uint(frameNum);
				}
				writer.EndArray();
				writer.SetFormatOptions(rapidjson::kFormatDefault);
				break;
			}
			case Action_SetTarget:
			case Action_GoToLabel: {
				const char *targetName = (const char *)bitstream.ReadData(recordLength);
				writer.StartArray();
				writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
				{
					writer.String(actionName.c_str());
					writer.String(targetName);
				}
				writer.EndArray();
				writer.SetFormatOptions(rapidjson::kFormatDefault);
				break;
			}
			case Action_Push: {
				idSWFBitStream pushstream(bitstream.ReadData(recordLength), recordLength, false);
				writer.StartArray();
				{
					writer.String(actionName.c_str());
					writer.StartArray();
					while (pushstream.Tell() < pushstream.Length()) {
						uint8 type = pushstream.ReadU8();
						bool useArray = !(type == 2 || type == 3);
						if (useArray) {
							writer.StartArray();
							writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
						}
						switch (type) {
						case 0: writer.String("string"); writer.String(pushstream.ReadString()); break;
						case 1: writer.String("float"); writer.Double(pushstream.ReadFloat()); break;
						case 2: writer.String("null"); break;
						case 3: writer.String("undefined"); break;
						case 4: writer.String("reg"); writer.Uint(pushstream.ReadU8()); break;
						case 5: writer.String("bool"); writer.Bool(pushstream.ReadU8() != 0); break;
						case 6: writer.String("double"); writer.Double(pushstream.ReadDouble()); break;
						case 7: writer.String("int"); writer.Int(pushstream.ReadS32()); break;
						// when importing back, "const" is constant8 if value <= 255, and constant16 otherwise
						case 8: writer.String("const"); writer.Uint(pushstream.ReadU8()); break;
						case 9: writer.String("const"); writer.Uint(pushstream.ReadU16()); break;
						default: {
							idStr msg;
							msg.Format("(%d) %s: Unknown value type '%d'.", actionIndex, actionName.c_str(), type);
							errorMsg.Append(msg);
							return false;
						}
						}
						if (useArray) {
							writer.EndArray();
							writer.SetFormatOptions(rapidjson::kFormatDefault);
						}
					}
					writer.EndArray();
				}
				writer.EndArray();
				break;
			}
			case Action_Pop:
			case Action_Add:
			case Action_Subtract:
			case Action_Multiply:
			case Action_Divide:
			case Action_Equals:
			case Action_Less:
			case Action_And:
			case Action_Or:
			case Action_Not:
			case Action_StringEquals:
			case Action_StringLength:
			case Action_StringAdd:
			case Action_StringExtract:
			case Action_StringLess:
			case Action_StringGreater:
			case Action_ToInteger:
			case Action_CharToAscii:
			case Action_AsciiToChar: {
				writer.String(actionName.c_str());
				break;
			}
			case Action_Jump:
			case Action_If: {
				int16 offset = bitstream.ReadS16();
				// bitstream is pointing to the next action right now
				int64 destinationPos = (int64)bitstream.Tell() + offset;
				if (destinationPos < (int64)initialStreamPos || destinationPos > (int64)bitstream.Length()) {
					// already tested; should never get here
					idStr msg;
					msg.Format("(%d) %s: Jump destination isn't reachable.", actionIndex, actionName.c_str());
					errorMsg.Append(msg);
					return false;
				}
				// make sure the destination corresponds to the beginning of an action
				if (actionPos.FindIndex((uint32)destinationPos) < 0) {
					idStr msg;
					msg.Format("(%d) %s: Invalid jump destination.", actionIndex, actionName.c_str());
					errorMsg.Append(msg);
					return false;
				}
				int jumpIndex = jumpPos.FindIndex((uint32)destinationPos);
				if (jumpIndex < 0) {
					// should never get here
					idStr msg;
					msg.Format("(%d) %s: Couldn't find jump label.", actionIndex, actionName.c_str());
					errorMsg.Append(msg);
					return false;
				}
				writer.StartArray();
				writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
				{
					writer.String(actionName.c_str());
					writer.String(local::GetJumpLabelName(jumpIndex).c_str());
				}
				writer.EndArray();
				writer.SetFormatOptions(rapidjson::kFormatDefault);
				break;
			}
			case Action_GetVariable:
			case Action_SetVariable: {
				writer.String(actionName.c_str());
				break;
			}
			case Action_GotoFrame2: {
				uint8 flags = bitstream.ReadU8();
				bool playFlag = (flags & 1);
				bool biasFlag = (flags & 2);
				uint16 bias = 0;
				if (biasFlag) {
					bias = bitstream.ReadU16();
				}
				writer.StartArray();
				writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
				{
					writer.String(actionName.c_str());
					writer.Bool(playFlag);
					if (biasFlag) {
						writer.Uint(bias);
					}
				}
				writer.EndArray();
				writer.SetFormatOptions(rapidjson::kFormatDefault);
				break;
			}
			case Action_GetProperty:
			case Action_SetProperty:
			case Action_Trace:
			case Action_GetTime:
			case Action_RandomNumber:
			case Action_CallFunction:
			case Action_CallMethod: {
				writer.String(actionName.c_str());
				break;
			}
			case Action_ConstantPool: {
				uint16 numConstants = bitstream.ReadU16();
				writer.StartArray();
				{
					writer.String(actionName.c_str());
					writer.StartArray();
					for (int i = 0; i < numConstants; ++i) {
						const char *constant = bitstream.ReadString();
						writer.String(constant);
					}
					writer.EndArray();
				}
				writer.EndArray();
				break;
			}
			case Action_DefineFunction: {
				writer.StartArray();
				{
					writer.String(actionName.c_str());
					writer.StartObject();
					{
						const char *functionName = bitstream.ReadString();
						writer.Key("functionName");
						writer.String(functionName);

						uint16 numParms = bitstream.ReadU16();
						writer.Key("parameters");
						writer.StartArray();
						writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
						for (int i = 0; i < numParms; ++i) {
							const char *parmName = bitstream.ReadString();
							writer.String(parmName);
						}
						writer.EndArray();
						writer.SetFormatOptions(rapidjson::kFormatDefault);

						uint16 codeSize = bitstream.ReadU16();
						idSWFBitStream codeStream(bitstream.ReadData(codeSize), codeSize, false);
						writer.Key("actions");
						if (!CstWriteJson_Actions(writer, codeStream, errorMsg)) {
							idStr msg;
							msg.Format("(%d) %s", actionIndex, actionName.c_str());
							errorMsg.Append(msg);
							return false;
						}
					}
					writer.EndObject();
				}
				writer.EndArray();
				break;
			}
			case Action_DefineFunction2: {
				writer.StartArray();
				{
					writer.String(actionName.c_str());
					writer.StartObject();
					{
						const char *functionName = bitstream.ReadString();
						uint16 numParms = bitstream.ReadU16();
						uint8 numRegs = bitstream.ReadU8();
						uint16 flags = bitstream.ReadU16();

						writer.Key("functionName");
						writer.String(functionName);
						writer.Key("registerCount");
						writer.Uint(numRegs);

						writer.Key("flags");
						writer.StartArray();
						writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
						{
							// see idSWFScriptFunction_Script::Call
							if (flags & BIT(0)) {
								writer.String("PreloadThisFlag");
							}
							if (flags & BIT(1)) {
								writer.String("SuppressThisFlag");
							}
							if (flags & BIT(2)) {
								writer.String("PreloadArgumentsFlag");
							}
							if (flags & BIT(3)) {
								writer.String("SuppressArgumentsFlag");
							}
							if (flags & BIT(4)) {
								writer.String("PreloadSuperFlag");
							}
							if (flags & BIT(5)) {
								writer.String("SuppressSuperFlag");
							}
							if (flags & BIT(6)) {
								writer.String("PreloadRootFlag");
							}
							if (flags & BIT(7)) {
								writer.String("PreloadParentFlag");
							}
							if (flags & BIT(8)) {
								writer.String("PreloadGlobalFlag");
							}
						}
						writer.EndArray();
						writer.SetFormatOptions(rapidjson::kFormatDefault);

						writer.Key("parameters");
						writer.StartArray();
						for (int i = 0; i < numParms; ++i) {
							writer.StartArray();
							writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
							{
								uint8 reg = bitstream.ReadU8();
								const char *parmName = bitstream.ReadString();
								writer.Uint(reg);
								writer.String(parmName);
							}
							writer.EndArray();
							writer.SetFormatOptions(rapidjson::kFormatDefault);
						}
						writer.EndArray();

						uint16 codeSize = bitstream.ReadU16();
						idSWFBitStream codeStream(bitstream.ReadData(codeSize), codeSize, false);
						writer.Key("actions");
						if (!CstWriteJson_Actions(writer, codeStream, errorMsg)) {
							idStr msg;
							msg.Format("(%d) %s", actionIndex, actionName.c_str());
							errorMsg.Append(msg);
							return false;
						}
					}
					writer.EndObject();
				}
				writer.EndArray();
				break;
			}
			case Action_Enumerate:
			case Action_Enumerate2:
			case Action_Equals2:
			case Action_StrictEquals:
			case Action_GetMember:
			case Action_SetMember:
			case Action_InitArray:
			case Action_InitObject:
			case Action_NewObject:
			case Action_Extends:
			case Action_TargetPath: {
				writer.String(actionName.c_str());
				break;
			}
			case Action_With: {
				writer.StartArray();
				{
					writer.String(actionName.c_str());
					int withSize = bitstream.ReadU16();
					idSWFBitStream codeStream(bitstream.ReadData(withSize), withSize, false);
					if (!CstWriteJson_Actions(writer, codeStream, errorMsg)) {
						idStr msg;
						msg.Format("(%d) %s", actionIndex, actionName.c_str());
						errorMsg.Append(msg);
						return false;
					}
				}
				writer.EndArray();
				break;
			}
			case Action_ToNumber:
			case Action_ToString:
			case Action_TypeOf:
			case Action_Add2:
			case Action_Less2:
			case Action_Greater:
			case Action_Modulo:
			case Action_BitAnd:
			case Action_BitLShift:
			case Action_BitOr:
			case Action_BitRShift:
			case Action_BitURShift:
			case Action_BitXor:
			case Action_Decrement:
			case Action_Increment:
			case Action_PushDuplicate:
			case Action_StackSwap: {
				writer.String(actionName.c_str());
				break;
			}
			case Action_StoreRegister: {
				writer.StartArray();
				writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
				{
					uint8 registerNumber = bitstream.ReadU8();
					writer.String(actionName.c_str());
					writer.Uint(registerNumber);
				}
				writer.EndArray();
				writer.SetFormatOptions(rapidjson::kFormatDefault);
				break;
			}
			case Action_DefineLocal:
			case Action_DefineLocal2:
			case Action_Delete: // pops stack, but doesn't delete
			case Action_Delete2: { // pops stack, but doesn't delete
				writer.String(actionName.c_str());
				break;
			}
			// not supported
			case Action_CloneSprite:
			case Action_RemoveSprite:
			case Action_Call:
			case Action_SetTarget2:
			case Action_NewMethod:
			default: {
				// already tested; should never get here
				idStr msg;
				msg.Format("(%d) %s: This action isn't supported.", actionIndex, actionName.c_str());
				errorMsg.Append(msg);
				return false;
			}
			}
		}
		// if a jump to the end of the stream exists, insert a jump label
		{
			int jumpIndex = jumpPos.FindIndex(bitstream.Length());
			if (jumpIndex >= 0) {
				writer.StartArray();
				writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
				{
					writer.String("CstJumpLabel");
					writer.String(local::GetJumpLabelName(jumpIndex).c_str());
				}
				writer.EndArray();
				writer.SetFormatOptions(rapidjson::kFormatDefault);
			}
		}
	}
	writer.EndArray();
	return true;
}

// ===============
void idSWFScriptFunction_Script::CstLoadJson_DoAction(JsonValue &objDoAction, CstSWFBitstream &cstBitstream, idList<idStr> &errorMsg) {
	// may throw CstJson::Except
	assert(objDoAction.IsObject());
	JsonValue &arrayActions = CstJson::Object_GetArray(objDoAction, "actions", errorMsg);
	if (arrayActions.Size() == 0) {
		return; // the array of actions is allowed to be empty
	}
	CstLoadJson_Actions(arrayActions, cstBitstream, errorMsg);
}

// ===============
void idSWFScriptFunction_Script::CstLoadJson_DoInitAction(JsonValue &arrayDoInitAction, CstSWFBitstream &cstBitstream, idList<idStr> &errorMsg) {
	// may throw CstJson::Except
	assert(arrayDoInitAction.IsArray());
	if (arrayDoInitAction.Size() == 0) {
		return; // the array of init actions is allowed to be empty
	}
	CstLoadJson_Actions(arrayDoInitAction, cstBitstream, errorMsg);
}

// ===============
static int CstGetSwfActionCode(idStr actionName) {
	// see idSWF::GetActionName
#define SWF_ACTION_CODE(x) if (idStr::Cmp(#x, actionName.c_str()) == 0) { return Action_##x; }
	if (!actionName.StripLeadingOnce("Action")) {
		return -1;
	}
	SWF_ACTION_CODE(End);
	SWF_ACTION_CODE(NextFrame);
	SWF_ACTION_CODE(PrevFrame);
	SWF_ACTION_CODE(Play);
	SWF_ACTION_CODE(Stop);
	SWF_ACTION_CODE(ToggleQuality);
	SWF_ACTION_CODE(StopSounds);
	SWF_ACTION_CODE(GotoFrame);
	SWF_ACTION_CODE(GetURL);
	SWF_ACTION_CODE(WaitForFrame);
	SWF_ACTION_CODE(SetTarget);
	SWF_ACTION_CODE(GoToLabel);
	SWF_ACTION_CODE(Add);
	SWF_ACTION_CODE(Subtract);
	SWF_ACTION_CODE(Multiply);
	SWF_ACTION_CODE(Divide);
	SWF_ACTION_CODE(Equals);
	SWF_ACTION_CODE(Less);
	SWF_ACTION_CODE(And);
	SWF_ACTION_CODE(Or);
	SWF_ACTION_CODE(Not);
	SWF_ACTION_CODE(StringEquals);
	SWF_ACTION_CODE(StringLength);
	SWF_ACTION_CODE(StringExtract);
	SWF_ACTION_CODE(Pop);
	SWF_ACTION_CODE(ToInteger);
	SWF_ACTION_CODE(GetVariable);
	SWF_ACTION_CODE(SetVariable);
	SWF_ACTION_CODE(SetTarget2);
	SWF_ACTION_CODE(StringAdd);
	SWF_ACTION_CODE(GetProperty);
	SWF_ACTION_CODE(SetProperty);
	SWF_ACTION_CODE(CloneSprite);
	SWF_ACTION_CODE(RemoveSprite);
	SWF_ACTION_CODE(Trace);
	SWF_ACTION_CODE(StartDrag);
	SWF_ACTION_CODE(EndDrag);
	SWF_ACTION_CODE(StringLess);
	SWF_ACTION_CODE(RandomNumber);
	SWF_ACTION_CODE(MBStringLength);
	SWF_ACTION_CODE(CharToAscii);
	SWF_ACTION_CODE(AsciiToChar);
	SWF_ACTION_CODE(GetTime);
	SWF_ACTION_CODE(MBStringExtract);
	SWF_ACTION_CODE(MBCharToAscii);
	SWF_ACTION_CODE(MBAsciiToChar);
	SWF_ACTION_CODE(WaitForFrame2);
	SWF_ACTION_CODE(Push);
	SWF_ACTION_CODE(Jump);
	SWF_ACTION_CODE(GetURL2);
	SWF_ACTION_CODE(If);
	SWF_ACTION_CODE(Call);
	SWF_ACTION_CODE(GotoFrame2);
	SWF_ACTION_CODE(Delete);
	SWF_ACTION_CODE(Delete2);
	SWF_ACTION_CODE(DefineLocal);
	SWF_ACTION_CODE(CallFunction);
	SWF_ACTION_CODE(Return);
	SWF_ACTION_CODE(Modulo);
	SWF_ACTION_CODE(NewObject);
	SWF_ACTION_CODE(DefineLocal2);
	SWF_ACTION_CODE(InitArray);
	SWF_ACTION_CODE(InitObject);
	SWF_ACTION_CODE(TypeOf);
	SWF_ACTION_CODE(TargetPath);
	SWF_ACTION_CODE(Enumerate);
	SWF_ACTION_CODE(Add2);
	SWF_ACTION_CODE(Less2);
	SWF_ACTION_CODE(Equals2);
	SWF_ACTION_CODE(ToNumber);
	SWF_ACTION_CODE(ToString);
	SWF_ACTION_CODE(PushDuplicate);
	SWF_ACTION_CODE(StackSwap);
	SWF_ACTION_CODE(GetMember);
	SWF_ACTION_CODE(SetMember);
	SWF_ACTION_CODE(Increment);
	SWF_ACTION_CODE(Decrement);
	SWF_ACTION_CODE(CallMethod);
	SWF_ACTION_CODE(NewMethod);
	SWF_ACTION_CODE(BitAnd);
	SWF_ACTION_CODE(BitOr);
	SWF_ACTION_CODE(BitXor);
	SWF_ACTION_CODE(BitLShift);
	SWF_ACTION_CODE(BitRShift);
	SWF_ACTION_CODE(BitURShift);
	SWF_ACTION_CODE(StoreRegister);
	SWF_ACTION_CODE(ConstantPool);
	SWF_ACTION_CODE(With);
	SWF_ACTION_CODE(DefineFunction);
	SWF_ACTION_CODE(InstanceOf);
	SWF_ACTION_CODE(Enumerate2);
	SWF_ACTION_CODE(StrictEquals);
	SWF_ACTION_CODE(Greater);
	SWF_ACTION_CODE(StringGreater);
	SWF_ACTION_CODE(Extends);
	SWF_ACTION_CODE(CastOp);
	SWF_ACTION_CODE(ImplementsOp);
	SWF_ACTION_CODE(Throw);
	SWF_ACTION_CODE(Try);
	SWF_ACTION_CODE(DefineFunction2);
	return -1;
#undef SWF_ACTION_CODE
}

// ===============
void idSWFScriptFunction_Script::CstLoadJson_Actions(JsonValue &arrayActions, CstSWFBitstream &cstBitstream, idList<idStr> &errorMsg) {
	// may throw CstJson::Except
	assert(arrayActions.IsArray());

	struct infoJumpDest_t {
		idStr label;
		int byteIndex;
	};
	struct infoJumpOrg_t {
		idStr label;
		int byteIndex;
		JsonSizeType actionIndex;
		idStr actionName;
	};
	idList<infoJumpDest_t> jumpDest; // jump destination in the bitstream
	idList<infoJumpOrg_t> jumpOrg; // jump origin in the bitstream

	// First pass: Find the destination jump labels.
	for (JsonSizeType i = 0; i < arrayActions.Size(); ++i) {
		JsonValue &valueAction = arrayActions[i];
		const char *actionName = NULL;
		if (valueAction.IsString()) {
			actionName = valueAction.GetString();
		} else if (valueAction.IsArray() && valueAction.Size() > 0 && valueAction[0].IsString()) {
			actionName = valueAction[0].GetString();
		}
		if (actionName && idStr::Cmp("CstJumpLabel", actionName) == 0) {
			if (valueAction.IsString() || (valueAction.IsArray() && valueAction.Size() != 2)) {
				errorMsg.Append("Expected 1 argument.");
				idStr msg; msg.Format("(%u) CstJumpLable", i); errorMsg.Append(msg);
				throw CstJson::Except();
			}
			if (!valueAction[1].IsString()) {
				errorMsg.Append("The label must be a string.");
				idStr msg; msg.Format("(%u) CstJumpLable", i); errorMsg.Append(msg);
				throw CstJson::Except();
			}
			const char *label = valueAction[1].GetString();
			for (int j = 0; j < jumpDest.Num(); ++j) {
				if (idStr::Cmp(label, jumpDest[j].label) == 0) {
					errorMsg.Append("Not unique.");
					idStr msg; msg.Format("(%u) CstJumpLable", i); errorMsg.Append(msg);
					throw CstJson::Except();
				}
			}
			jumpDest.Append({ label, 0 }); // the real byte index is added in the second pass
		}
	}

	// Second pass: Build the bitstream and complete the jump tables. The jump offsets in the bitstream are adjusted later.
	int jumpDestCount = 0;
	for (JsonSizeType i = 0; i < arrayActions.Size(); ++i) {
		JsonValue &valueAction = arrayActions[i];
		const char *actionName = NULL;
		if (valueAction.IsString()) {
			actionName = valueAction.GetString();
		} else if (valueAction.IsArray() && valueAction.Size() > 0 && valueAction[0].IsString()) {
			actionName = valueAction[0].GetString();
		}
		if (!actionName) {
			errorMsg.Append("Expected an action.");
			idStr msg; msg.Format("(%u)", i); errorMsg.Append(msg);
			throw CstJson::Except();
		}
		// at this point, valueAction is either a string or a non-empty array

		// add the byte index to the table of jump destinations
		if (idStr::Cmp("CstJumpLabel", actionName) == 0) {
			// validation was done in the first pass
			jumpDest[jumpDestCount++].byteIndex = cstBitstream.Tell(); // points to the beginning of the next action
			continue;
		}

		int code = CstGetSwfActionCode(actionName);
		if (code == -1) {
			idStr msg;
			msg.Format("Unknown action '%s'", actionName); errorMsg.Append(msg);
			msg.Format("(%u)", i); errorMsg.Append(msg);
			throw CstJson::Except();
		}

		switch (code) {
		case Action_Return:
		case Action_End: // end of actions
		case Action_NextFrame:
		case Action_PrevFrame:
		case Action_Play:
		case Action_Stop:
		case Action_ToggleQuality: // ignored when executed
		case Action_StopSounds: { // ignored when executed
			if (valueAction.IsArray() && valueAction.Size() != 1) {
				errorMsg.Append("Expected 0 arguments.");
				idStr msg; msg.Format("(%u) %s", i, actionName); errorMsg.Append(msg);
				throw CstJson::Except();
			}
			cstBitstream.WriteU8(code);
			break;
		}
		case Action_GotoFrame: {
			uint32 frameNum;
			try {
				if (!valueAction.IsArray() || valueAction.Size() != 2) {
					errorMsg.Append("Expected 1 argument.");
					throw CstJson::Except();
				}
				try {
					frameNum = CstJson::Array_GetUint(valueAction, 1, errorMsg, 0 + 1, (1uLL << 16) - 1 + 1); // 1 based
					--frameNum; // but encoded 0 based
				}
				catch (CstJson::Except) {
					errorMsg.Append("Arg 0");
					throw;
				}
			}
			catch (CstJson::Except) {
				idStr msg; msg.Format("(%u) %s", i, actionName); errorMsg.Append(msg);
				throw;
			}
			cstBitstream.WriteU8(code);
			cstBitstream.WriteU16(2); // record length
			cstBitstream.WriteU16(frameNum);
			break;
		}
		case Action_SetTarget:
		case Action_GoToLabel: {
			idStr targetName;
			try {
				if (!valueAction.IsArray() || valueAction.Size() != 2) {
					errorMsg.Append("Expected 1 argument.");
					throw CstJson::Except();
				}
				try {
					targetName = CstJson::Array_GetString(valueAction, 1, errorMsg);
					if (targetName.Length() + 1 > (1LL << 16) - 1) {
						errorMsg.Append("Too large string."); // the size must fit in an uint16
						throw CstJson::Except();
					}
				}
				catch (CstJson::Except) {
					errorMsg.Append("Arg 0");
					throw;
				}
			}
			catch (CstJson::Except) {
				idStr msg; msg.Format("(%u) %s", i, actionName); errorMsg.Append(msg);
				throw;
			}
			cstBitstream.WriteU8(code);
			cstBitstream.WriteU16(targetName.Length() + 1); // record length; +1 for the terminating null char
			cstBitstream.WriteString(targetName.c_str());
			break;
		}
		case Action_Push: {
			cstBitstream.WriteU8(code);
			int posRecordLength = cstBitstream.Tell();
			cstBitstream.WriteU16(0); // record length; using a dummy value for now
			try {
				if (!valueAction.IsArray() || valueAction.Size() != 2) {
					errorMsg.Append("Expected 1 argument.");
					throw CstJson::Except();
				}
				try {
					JsonValue &arrayArg0 = CstJson::Array_GetArray(valueAction, 1, errorMsg);
					for (JsonSizeType j = 0; j < arrayArg0.Size(); ++j) {
						try {
							JsonValue &valuePushEntry = arrayArg0[j];
							const char *typeName = NULL;
							if (valuePushEntry.IsString()) {
								typeName = valuePushEntry.GetString();
							} else if (valuePushEntry.IsArray() && valuePushEntry.Size() > 0 && valuePushEntry[0].IsString()) {
								typeName = valuePushEntry[0].GetString();
							}
							if (!typeName) {
								errorMsg.Append("Invalid entry.");
								throw CstJson::Except();
							}
							// at this point, valuePushEntry is either a string or a non-empty array
							struct local {
								static int GetTypeCode(const char *typeName) {
									if (idStr::Cmp("string", typeName) == 0) { return 0; }
									if (idStr::Cmp("float", typeName) == 0) { return 1; }
									if (idStr::Cmp("null", typeName) == 0) { return 2; }
									if (idStr::Cmp("undefined", typeName) == 0) { return 3; }
									if (idStr::Cmp("reg", typeName) == 0) { return 4; }
									if (idStr::Cmp("bool", typeName) == 0) { return 5; }
									if (idStr::Cmp("double", typeName) == 0) { return 6; }
									if (idStr::Cmp("int", typeName) == 0) { return 7; }
									if (idStr::Cmp("const", typeName) == 0) { return 8; }
									return -1;
								}
							};
							int typeCode = local::GetTypeCode(typeName);
							if (typeCode == -1) {
								idStr msg; msg.Format("Unknown type '%s'.", typeName); errorMsg.Append(msg);
								throw CstJson::Except();
							}
							// null or undefined
							if (typeCode == 2 || typeCode == 3) {
								if (valuePushEntry.IsArray() && valuePushEntry.Size() != 1) {
									idStr msg; msg.Format("Expected no values for type '%s'.", typeName); errorMsg.Append(msg);
									throw CstJson::Except();
								}
								cstBitstream.WriteU8(typeCode);
								continue;
							}
							// other types
							if (!valuePushEntry.IsArray() || valuePushEntry.Size() != 2) {
								idStr msg; msg.Format("Type '%s' requires 1 value.", typeName); errorMsg.Append(msg);
								throw CstJson::Except();
							}
							try {
								switch (typeCode) {
								case 0: { // string
									const char *str = CstJson::Array_GetString(valuePushEntry, 1, errorMsg);
									cstBitstream.WriteU8(0); // type
									cstBitstream.WriteString(str);
									break;
								}
								case 1: { // float
									float value = CstJson::Array_GetFloat(valuePushEntry, 1, errorMsg);
									cstBitstream.WriteU8(1); // type
									cstBitstream.WriteFloat(value);
									break;
								}
								case 4: { // reg
									uint8 reg = CstJson::Array_GetUint8(valuePushEntry, 1, errorMsg);
									cstBitstream.WriteU8(4); // type
									cstBitstream.WriteU8(reg);
									break;
								}
								case 5: { // bool
									bool value = CstJson::Array_GetBool(valuePushEntry, 1, errorMsg);
									cstBitstream.WriteU8(5); // type
									cstBitstream.WriteU8(value ? 1 : 0);
									break;
								}
								case 6: { // double
									double value = CstJson::Array_GetDouble(valuePushEntry, 1, errorMsg);
									cstBitstream.WriteU8(6); // type
									cstBitstream.WriteDouble(value);
									break;
								}
								case 7: { // int32
									int32 value = CstJson::Array_GetInt32(valuePushEntry, 1, errorMsg);
									cstBitstream.WriteU8(7); // type
									cstBitstream.WriteS32(value);
									break;
								}
								case 8: { // const8 or const16
									uint16 value = CstJson::Array_GetUint16(valuePushEntry, 1, errorMsg);
									if (value < 256) {
										cstBitstream.WriteU8(8); // type: const8
										cstBitstream.WriteU8(value);
									} else {
										cstBitstream.WriteU8(9); // type: const16
										cstBitstream.WriteU16(value);
									}
									break;
								}
								}
							}
							catch (CstJson::Except) {
								errorMsg.Append("(1)");
								throw;
							}
						}
						catch (CstJson::Except) {
							idStr msg; msg.Format("(%u)", j); errorMsg.Append(msg);
							throw;
						}
					}
					// adjust the record length
					int posCurrent = cstBitstream.Tell();
					int recordLength = posCurrent - (posRecordLength + 2); // +2 to skip the record length field
					if (recordLength > (1 << 16) - 1) {
						errorMsg.Append("Maximum size exceeded."); // the record length must fit in an uint16
						throw CstJson::Except();
					}
					cstBitstream.Seek(posRecordLength);
					cstBitstream.WriteU16(recordLength);
					cstBitstream.Seek(posCurrent);
				}
				catch (CstJson::Except) {
					errorMsg.Append("Arg 0");
					throw;
				}
			}
			catch (CstJson::Except) {
				idStr msg; msg.Format("(%u) %s", i, actionName); errorMsg.Append(msg);
				throw;
			}
			break;
		}
		case Action_Pop:
		case Action_Add:
		case Action_Subtract:
		case Action_Multiply:
		case Action_Divide:
		case Action_Equals:
		case Action_Less:
		case Action_And:
		case Action_Or:
		case Action_Not:
		case Action_StringEquals:
		case Action_StringLength:
		case Action_StringAdd:
		case Action_StringExtract:
		case Action_StringLess:
		case Action_StringGreater:
		case Action_ToInteger:
		case Action_CharToAscii:
		case Action_AsciiToChar: {
			if (valueAction.IsArray() && valueAction.Size() != 1) {
				errorMsg.Append("Expected 0 arguments.");
				idStr msg; msg.Format("(%u) %s", i, actionName); errorMsg.Append(msg);
				throw CstJson::Except();
			}
			cstBitstream.WriteU8(code);
			break;
		}
		case Action_Jump:
		case Action_If: {
			idStr jumpLabel;
			try {
				if (!valueAction.IsArray() || valueAction.Size() != 2) {
					errorMsg.Append("Expected 1 argument.");
					throw CstJson::Except();
				}
				try {
					jumpLabel = CstJson::Array_GetString(valueAction, 1, errorMsg);
					// check this label against the destination labels to make sure it is valid
					bool found = false;
					for (int j = 0; j < jumpDest.Num(); ++j) {
						if (idStr::Cmp(jumpDest[j].label.c_str(), jumpLabel.c_str()) == 0) {
							found = true;
							break;
						}
					}
					if (!found) {
						idStr msg; msg.Format("Label '%s' not found.", jumpLabel.c_str()); errorMsg.Append(msg);
						throw CstJson::Except();
					}
				}
				catch (CstJson::Except) {
					errorMsg.Append("Arg 0");
					throw;
				}
			}
			catch (CstJson::Except) {
				idStr msg; msg.Format("(%u) %s", i, actionName); errorMsg.Append(msg);
				throw;
			}
			cstBitstream.WriteU8(code);
			cstBitstream.WriteU16(2); // record length
			int posJumpOffset = cstBitstream.Tell();
			cstBitstream.WriteS16(0); // using a dummy offset for now; this is adjusted after the second pass
			// update the table of jump origins
			jumpOrg.Append({ jumpLabel, posJumpOffset, i, actionName });
			break;
		}
		case Action_GetVariable:
		case Action_SetVariable: {
			if (valueAction.IsArray() && valueAction.Size() != 1) {
				errorMsg.Append("Expected 0 arguments.");
				idStr msg; msg.Format("(%u) %s", i, actionName); errorMsg.Append(msg);
				throw CstJson::Except();
			}
			cstBitstream.WriteU8(code);
			break;
		}
		case Action_GotoFrame2: {
			bool playFlag = false;
			bool biasFlag = false;
			uint16 bias = 0;
			try {
				if (!valueAction.IsArray() || (valueAction.Size() != 2 && valueAction.Size() != 3)) {
					errorMsg.Append("Expected 1 or 2 arguments.");
					throw CstJson::Except();
				}
				unsigned index;
				try {
					playFlag = CstJson::Array_GetBool(valueAction, index = 1, errorMsg);
					if (valueAction.Size() == 3) {
						bias = CstJson::Array_GetUint16(valueAction, index = 2, errorMsg);
						if (bias > 0) {
							biasFlag = true;
						}
					}
				}
				catch (CstJson::Except) {
					idStr msg; msg.Format("Arg %u", index - 1); errorMsg.Append(msg);
					throw;
				}
			}
			catch (CstJson::Except) {
				idStr msg; msg.Format("(%u) %s", i, actionName); errorMsg.Append(msg);
				throw;
			}
			uint16 recordLength = 1;
			if (biasFlag) {
				recordLength = 3;
			}
			uint8 flags = (playFlag ? 1 : 0) | (biasFlag ? 2 : 0);
			cstBitstream.WriteU8(code);
			cstBitstream.WriteU16(recordLength);
			cstBitstream.WriteU8(flags);
			if (biasFlag) {
				cstBitstream.WriteU16(bias);
			}
			break;
		}
		case Action_GetProperty:
		case Action_SetProperty:
		case Action_Trace:
		case Action_GetTime:
		case Action_RandomNumber:
		case Action_CallFunction:
		case Action_CallMethod: {
			if (valueAction.IsArray() && valueAction.Size() != 1) {
				errorMsg.Append("Expected 0 arguments.");
				idStr msg; msg.Format("(%u) %s", i, actionName); errorMsg.Append(msg);
				throw CstJson::Except();
			}
			cstBitstream.WriteU8(code);
			break;
		}
		case Action_ConstantPool: {
			cstBitstream.WriteU8(code);
			int posRecordLength = cstBitstream.Tell();
			cstBitstream.WriteU16(0); // record length; using a dummy value for now
			try {
				if (!valueAction.IsArray() || valueAction.Size() != 2) {
					errorMsg.Append("Expected 1 argument.");
					throw CstJson::Except();
				}
				try {
					JsonValue &arrayConstants = CstJson::Array_GetArray(valueAction, 1, errorMsg);
					if (arrayConstants.Size() > (1uLL << 16) - 1) {
						errorMsg.Append("Too many entries."); // the number of constants must fit in an uint16
						throw CstJson::Except();
					}
					cstBitstream.WriteU16(arrayConstants.Size());
					for (JsonSizeType j = 0; j < arrayConstants.Size(); ++j) {
						try {
							const char *constant = CstJson::Array_GetString(arrayConstants, j, errorMsg);
							cstBitstream.WriteString(constant);
						}
						catch (CstJson::Except) {
							idStr msg; msg.Format("(%u)", j); errorMsg.Append(msg);
							throw;
						}
					}
					// adjust the record length
					int posCurrent = cstBitstream.Tell();
					int recordLength = posCurrent - (posRecordLength + 2); // +2 to skip the record length field
					if (recordLength > (1 << 16) - 1) {
						errorMsg.Append("Maximum size exceeded."); // the record length must fit in an uint16
						throw CstJson::Except();
					}
					cstBitstream.Seek(posRecordLength);
					cstBitstream.WriteU16(recordLength);
					cstBitstream.Seek(posCurrent);
				}
				catch (CstJson::Except) {
					errorMsg.Append("Arg 0");
					throw;
				}
			}
			catch (CstJson::Except) {
				idStr msg; msg.Format("(%u) %s", i, actionName); errorMsg.Append(msg);
				throw;
			}
			break;
		}
		case Action_DefineFunction: {
			cstBitstream.WriteU8(code);
			int posRecordLength = cstBitstream.Tell();
			cstBitstream.WriteU16(0); // record length; using a dummy value for now
			try {
				if (!valueAction.IsArray() || valueAction.Size() != 2) {
					errorMsg.Append("Expected 1 argument.");
					throw CstJson::Except();
				}
				try {
					JsonValue &objArg0 = CstJson::Array_GetObject(valueAction, 1, errorMsg);

					const char *functionName = CstJson::Object_GetString(objArg0, "functionName", errorMsg);
					cstBitstream.WriteString(functionName);

					const char keyParms[] = "parameters";
					JsonValue &arrayParms = CstJson::Object_GetArray(objArg0, keyParms, errorMsg);
					if (arrayParms.Size() > (1uLL << 16) - 1) {
						errorMsg.Append("Too many parameters."); // the number of parmameters must fit in an uint16
						errorMsg.Append(keyParms);
						throw CstJson::Except();
					}
					uint16 numParms = arrayParms.Size();
					cstBitstream.WriteU16(numParms);
					for (uint16 j = 0; j < numParms; ++j) {
						try {
							const char *parmName = CstJson::Array_GetString(arrayParms, j, errorMsg);
							cstBitstream.WriteString(parmName);
						}
						catch (CstJson::Except) {
							idStr msg; msg.Format("%s (%u)", keyParms, j); errorMsg.Append(msg);
							throw;
						}
					}

					const char keyActions[] = "actions";
					JsonValue &_arrayActions = CstJson::Object_GetArray(objArg0, keyActions, errorMsg);
					int posCodeSize = cstBitstream.Tell();
					cstBitstream.WriteU16(0); // code size; using a dummy value for now

					// adjust the record length
					int posCurrent = cstBitstream.Tell();
					int recordLength = posCurrent - (posRecordLength + 2); // +2 to skip the record length field
					if (recordLength > (1LL << 16) - 1) {
						errorMsg.Append("Maximum size exceeded."); // the record length must fit in an uint16
						throw CstJson::Except();
					}
					cstBitstream.Seek(posRecordLength);
					cstBitstream.WriteU16(recordLength);
					cstBitstream.Seek(posCurrent);

					// write the code
					if (_arrayActions.Size() > 0) {
						try {
							CstLoadJson_Actions(_arrayActions, cstBitstream, errorMsg);
						}
						catch (CstJson::Except) {
							errorMsg.Append(keyActions);
							throw;
						}
					}
					// adjust the code size
					posCurrent = cstBitstream.Tell();
					int codeSize = posCurrent - (posCodeSize + 2); // +2 to skip the code size field
					if (codeSize > (1LL << 16) - 1) {
						errorMsg.Append("Maximum size exceeded."); // the code size must fit in an uint16
						throw CstJson::Except();
					}
					cstBitstream.Seek(posCodeSize);
					cstBitstream.WriteU16(codeSize);
					cstBitstream.Seek(posCurrent);
				}
				catch (CstJson::Except) {
					errorMsg.Append("Arg 0");
					throw;
				}
			}
			catch (CstJson::Except) {
				idStr msg; msg.Format("(%u) %s", i, actionName); errorMsg.Append(msg);
				throw;
			}
			break;
		}
		case Action_DefineFunction2: {
			cstBitstream.WriteU8(code);
			int posRecordLength = cstBitstream.Tell();
			cstBitstream.WriteU16(0); // record length; using a dummy value for now
			try {
				if (!valueAction.IsArray() || valueAction.Size() != 2) {
					errorMsg.Append("Expected 1 argument.");
					throw CstJson::Except();
				}
				try {
					JsonValue &objArg0 = CstJson::Array_GetObject(valueAction, 1, errorMsg);

					const char *functionName = CstJson::Object_GetString(objArg0, "functionName", errorMsg);
					cstBitstream.WriteString(functionName);

					const char keyParms[] = "parameters";
					JsonValue &arrayParms = CstJson::Object_GetArray(objArg0, keyParms, errorMsg);
					if (arrayParms.Size() > (1uLL << 16) - 1) {
						errorMsg.Append("Too many parameters."); // the number of parmameters must fit in an uint16
						errorMsg.Append(keyParms);
						throw CstJson::Except();
					}
					uint16 numParms = arrayParms.Size();
					cstBitstream.WriteU16(numParms);

					const char keyRegisterCount[] = "registerCount";
					uint8 numRegs = CstJson::Object_GetUint8(objArg0, keyRegisterCount, errorMsg);
					cstBitstream.WriteU8(numRegs);

					const char keyFlags[] = "flags";
					uint16 flags = 0;
					unsigned numPreloadedRegs = 0;
					JsonValue &arrayFlags = CstJson::Object_GetArray(objArg0, keyFlags, errorMsg);
					for (JsonSizeType j = 0; j < arrayFlags.Size(); ++j) {
						try {
							const char *flagName = CstJson::Array_GetString(arrayFlags, j, errorMsg);
							// see idSWFScriptFunction_Script::Call
							if (idStr::Cmp("PreloadThisFlag", flagName) == 0) {
								flags |= BIT(0); ++numPreloadedRegs;
							}
							else if (idStr::Cmp("SuppressThisFlag", flagName) == 0) {
								flags |= BIT(1);
							}
							else if (idStr::Cmp("PreloadArgumentsFlag", flagName) == 0) {
								flags |= BIT(2); ++numPreloadedRegs;
							}
							else if (idStr::Cmp("SuppressArgumentsFlag", flagName) == 0) {
								flags |= BIT(3);
							}
							else if (idStr::Cmp("PreloadSuperFlag", flagName) == 0) {
								flags |= BIT(4); ++numPreloadedRegs;
							}
							else if (idStr::Cmp("SuppressSuperFlag", flagName) == 0) {
								flags |= BIT(5);
							}
							else if (idStr::Cmp("PreloadRootFlag", flagName) == 0) {
								flags |= BIT(6); ++numPreloadedRegs;
							}
							else if (idStr::Cmp("PreloadParentFlag", flagName) == 0) {
								flags |= BIT(7); ++numPreloadedRegs;
							}
							else if (idStr::Cmp("PreloadGlobalFlag", flagName) == 0) {
								flags |= BIT(8); ++numPreloadedRegs;
							}
							else {
								idStr msg; msg.Format("Unknown flag '%s'.", flagName); errorMsg.Append(msg);
								throw CstJson::Except();
							}
						}
						catch (CstJson::Except) {
							idStr msg; msg.Format("%s (%u)", keyFlags, j); errorMsg.Append(msg);
							throw;
						}
					}
					// check if flags are ok
					if ( (flags & 0x3) == 0x3 || (flags & 0xC) == 0xC || (flags & 0x30) == 0x30) {
						errorMsg.Append("Cannot both preload and suppress a variable.");
						errorMsg.Append(keyFlags);
						throw CstJson::Except();
					}
					cstBitstream.WriteU16(flags);

					// write parms
					unsigned numRegMappedParms = 0;
					for (uint16 j = 0; j < numParms; ++j) {
						try {
							JsonValue &arrayParmPair = CstJson::Array_GetArray(arrayParms, j, errorMsg, 2);
							unsigned index;
							try {
								uint8 reg = CstJson::Array_GetUint8(arrayParmPair, index = 0, errorMsg);
								if (reg > 0) {
									++numRegMappedParms;
								}
								const char *parmName = CstJson::Array_GetString(arrayParmPair, index = 1, errorMsg);
								cstBitstream.WriteU8(reg);
								cstBitstream.WriteString(parmName);
							}
							catch (CstJson::Except) {
								idStr msg; msg.Format("(%u)", index); errorMsg.Append(msg);
								throw;
							}
						}
						catch (CstJson::Except) {
							idStr msg; msg.Format("%s (%u)", keyParms, j); errorMsg.Append(msg);
							throw;
						}
					}

					// check if the register count is ok
					if (numRegs < 1 + numPreloadedRegs + numRegMappedParms) {
						errorMsg.Append("Value can't be less than (1 + preloaded regs + reg parms).");
						errorMsg.Append(keyRegisterCount);
						throw CstJson::Except();
					}

					const char keyActions[] = "actions";
					JsonValue &_arrayActions = CstJson::Object_GetArray(objArg0, keyActions, errorMsg);
					int posCodeSize = cstBitstream.Tell();
					cstBitstream.WriteU16(0); // code size; using a dummy value for now

					// adjust the record length
					int posCurrent = cstBitstream.Tell();
					int recordLength = posCurrent - (posRecordLength + 2); // +2 to skip the record length field
					if (recordLength > (1LL << 16) - 1) {
						errorMsg.Append("Maximum size exceeded."); // the record length must fit in an uint16
						throw CstJson::Except();
					}
					cstBitstream.Seek(posRecordLength);
					cstBitstream.WriteU16(recordLength);
					cstBitstream.Seek(posCurrent);

					// write the code
					if (_arrayActions.Size() > 0) {
						try {
							CstLoadJson_Actions(_arrayActions, cstBitstream, errorMsg);
						}
						catch (CstJson::Except) {
							errorMsg.Append(keyActions);
							throw;
						}
					}
					// adjust the code size
					posCurrent = cstBitstream.Tell();
					int codeSize = posCurrent - (posCodeSize + 2); // +2 to skip the code size field
					if (codeSize > (1LL << 16) - 1) {
						errorMsg.Append("Maximum size exceeded."); // the code size must fit in an uint16
						throw CstJson::Except();
					}
					cstBitstream.Seek(posCodeSize);
					cstBitstream.WriteU16(codeSize);
					cstBitstream.Seek(posCurrent);
				}
				catch (CstJson::Except) {
					errorMsg.Append("Arg 0");
					throw;
				}
			}
			catch (CstJson::Except) {
				idStr msg; msg.Format("(%u) %s", i, actionName); errorMsg.Append(msg);
				throw;
			}
			break;
		}
		case Action_Enumerate:
		case Action_Enumerate2:
		case Action_Equals2:
		case Action_StrictEquals:
		case Action_GetMember:
		case Action_SetMember:
		case Action_InitArray:
		case Action_InitObject:
		case Action_NewObject:
		case Action_Extends:
		case Action_TargetPath: {
			if (valueAction.IsArray() && valueAction.Size() != 1) {
				errorMsg.Append("Expected 0 arguments.");
				idStr msg; msg.Format("(%u) %s", i, actionName); errorMsg.Append(msg);
				throw CstJson::Except();
			}
			cstBitstream.WriteU8(code);
			break;
		}
		case Action_With: {
			cstBitstream.WriteU8(code);
			cstBitstream.WriteU16(2); // record length; includes only the size of the code size field
			try {
				if (!valueAction.IsArray() || valueAction.Size() != 2) {
					errorMsg.Append("Expected 1 argument.");
					throw CstJson::Except();
				}
				try {
					JsonValue &_arrayActions = CstJson::Array_GetArray(valueAction, 1, errorMsg);
					int posCodeSize = cstBitstream.Tell();
					cstBitstream.WriteU16(0); // code size; using a dummy value for now
					if (_arrayActions.Size() > 0) {
						CstLoadJson_Actions(_arrayActions, cstBitstream, errorMsg);
					}
					// adjust the code size
					int posCurrent = cstBitstream.Tell();
					int codeSize = posCurrent - (posCodeSize + 2); // +2 to skip the code size field
					if (codeSize > (1LL << 16) - 1) {
						errorMsg.Append("Maximum size exceeded."); // the code size must fit in an uint16
						throw CstJson::Except();
					}
					cstBitstream.Seek(posCodeSize);
					cstBitstream.WriteU16(codeSize);
					cstBitstream.Seek(posCurrent);
				}
				catch (CstJson::Except) {
					errorMsg.Append("Arg 0");
					throw;
				}
			}
			catch (CstJson::Except) {
				idStr msg; msg.Format("(%u) %s", i, actionName); errorMsg.Append(msg);
				throw;
			}
			break;
		}
		case Action_ToNumber:
		case Action_ToString:
		case Action_TypeOf:
		case Action_Add2:
		case Action_Less2:
		case Action_Greater:
		case Action_Modulo:
		case Action_BitAnd:
		case Action_BitLShift:
		case Action_BitOr:
		case Action_BitRShift:
		case Action_BitURShift:
		case Action_BitXor:
		case Action_Decrement:
		case Action_Increment:
		case Action_PushDuplicate:
		case Action_StackSwap: {
			if (valueAction.IsArray() && valueAction.Size() != 1) {
				errorMsg.Append("Expected 0 arguments.");
				idStr msg; msg.Format("(%u) %s", i, actionName); errorMsg.Append(msg);
				throw CstJson::Except();
			}
			cstBitstream.WriteU8(code);
			break;
		}
		case Action_StoreRegister: {
			uint8 reg;
			try {
				if (!valueAction.IsArray() || valueAction.Size() != 2) {
					errorMsg.Append("Expected 1 argument.");
					throw CstJson::Except();
				}
				try {
					reg = CstJson::Array_GetUint8(valueAction, 1, errorMsg);
				}
				catch (CstJson::Except) {
					errorMsg.Append("Arg 0");
					throw;
				}
			}
			catch (CstJson::Except) {
				idStr msg; msg.Format("(%u) %s", i, actionName); errorMsg.Append(msg);
				throw;
			}
			cstBitstream.WriteU8(code);
			cstBitstream.WriteU16(1); // record length
			cstBitstream.WriteU8(reg);
			break;
		}
		case Action_DefineLocal:
		case Action_DefineLocal2:
		case Action_Delete: // pops stack, but doesn't delete
		case Action_Delete2: { // pops stack, but doesn't delete
			if (valueAction.IsArray() && valueAction.Size() != 1) {
				errorMsg.Append("Expected 0 arguments.");
				idStr msg; msg.Format("(%u) %s", i, actionName); errorMsg.Append(msg);
				throw CstJson::Except();
			}
			cstBitstream.WriteU8(code);
			break;
		}
		// not supported
		case Action_CloneSprite:
		case Action_RemoveSprite:
		case Action_Call:
		case Action_SetTarget2:
		case Action_NewMethod:
		default: {
			errorMsg.Append("This action isn't supported.");
			idStr msg; msg.Format("(%u) %s", i, actionName); errorMsg.Append(msg);
			throw CstJson::Except();
		}
		}
	}

	// Adjust the jump offsets in the bitstream
	int posCurrent = cstBitstream.Tell();
	for (int i = 0; i < jumpOrg.Num(); ++i) {
		const infoJumpOrg_t &infoOrg = jumpOrg[i];
		for (int j = 0; j < jumpDest.Num(); ++j) {
			const infoJumpDest_t &infoDest = jumpDest[j];
			if (idStr::Cmp(infoOrg.label.c_str(), infoDest.label.c_str()) == 0) {
				int offset = infoDest.byteIndex - (infoOrg.byteIndex + 2); // +2 to skip the offset field
				if (offset < -(1LL << 15) || offset > (1LL << 15) - 1) {
					// the offset must fit in an int16
					errorMsg.Append("Maximum jump offset exceeded.");
					idStr msg; msg.Format("(%u) %s", infoOrg.actionIndex, infoOrg.actionName.c_str()); errorMsg.Append(msg);
					throw CstJson::Except();
				}
				cstBitstream.Seek(infoOrg.byteIndex);
				cstBitstream.WriteS16(offset);
				break; // inner for
			}
		}
	}
	cstBitstream.Seek(posCurrent);
}
//#modified-fva; END

/*
========================
idSWF::Invoke
========================
*/
void idSWF::Invoke( const char * functionName, const idSWFParmList & parms ) {
	idSWFScriptObject * obj = mainspriteInstance->GetScriptObject();
	idSWFScriptVar scriptVar = obj->Get( functionName );

	if ( swf_debugInvoke.GetBool() ) {
		idLib::Printf( "SWF: Invoke %s with %d parms (%s)\n", functionName, parms.Num(), GetName() );
	}

	if ( scriptVar.IsFunction() ) {
		scriptVar.GetFunction()->Call( NULL, parms );
	}
}

/*
========================
idSWF::Invoke
========================
*/
void idSWF::Invoke( const char * functionName, const idSWFParmList & parms, idSWFScriptVar & scriptVar ) {

	if ( scriptVar.IsFunction() ) {
		scriptVar.GetFunction()->Call( NULL, parms );
	} else {
		idSWFScriptObject * obj = mainspriteInstance->GetScriptObject();
		scriptVar = obj->Get( functionName );

		if ( scriptVar.IsFunction() ) {
			scriptVar.GetFunction()->Call( NULL, parms );
		}
	}
}

/*
========================
idSWF::Invoke
========================
*/
void idSWF::Invoke( const char *  functionName, const idSWFParmList & parms, bool & functionExists ) {
	idSWFScriptObject * obj = mainspriteInstance->GetScriptObject();
	idSWFScriptVar scriptVar = obj->Get( functionName );

	if ( swf_debugInvoke.GetBool() ) {
		idLib::Printf( "SWF: Invoke %s with %d parms (%s)\n", functionName, parms.Num(), GetName() );
	}

	if ( scriptVar.IsFunction() ) {
		scriptVar.GetFunction()->Call( NULL, parms );
		functionExists = true;
	} else {
		functionExists = false;
	}
}
