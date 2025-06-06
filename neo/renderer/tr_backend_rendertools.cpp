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

#include "tr_local.h"
#include "simplex.h"	// line font definition

idCVar r_showCenterOfProjection( "r_showCenterOfProjection", "0", CVAR_RENDERER | CVAR_BOOL, "Draw a cross to show the center of projection" );
idCVar r_showLines( "r_showLines", "0", CVAR_RENDERER | CVAR_INTEGER, "1 = draw alternate horizontal lines, 2 = draw alternate vertical lines" );

#define MAX_DEBUG_LINES			16384

typedef struct debugLine_s {
	idVec4		rgb;
	idVec3		start;
	idVec3		end;
	bool		depthTest;
	int			lifeTime;
} debugLine_t;

debugLine_t		rb_debugLines[ MAX_DEBUG_LINES ];
int				rb_numDebugLines = 0;
int				rb_debugLineTime = 0;

#define MAX_DEBUG_TEXT			512

typedef struct debugText_s {
	idStr		text;
	idVec3		origin;
	float		scale;
	idVec4		color;
	idMat3		viewAxis;
	int			align;
	int			lifeTime;
	bool		depthTest;
} debugText_t;

debugText_t		rb_debugText[ MAX_DEBUG_TEXT ];
int				rb_numDebugText = 0;
int				rb_debugTextTime = 0;

#define MAX_DEBUG_POLYGONS		8192

typedef struct debugPolygon_s {
	idVec4		rgb;
	idWinding	winding;
	bool		depthTest;
	int			lifeTime;
} debugPolygon_t;

debugPolygon_t	rb_debugPolygons[ MAX_DEBUG_POLYGONS ];
int				rb_numDebugPolygons = 0;
int				rb_debugPolygonTime = 0;

//#modified-fva; BEGIN
//static void RB_DrawText( const char *text, const idVec3 &origin, float scale, const idVec4 &color, const idMat3 &viewAxis, const int align );
//#modified-fva; END

void RB_SetMVP( const idRenderMatrix & mvp );

//#modified-fva; BEGIN
namespace {
	struct CstVertPosColor {
		idVec3 position;
		idVec4 color;
	};
}

static int RB_Cst_DebugText_GetNumVerts(const idStr &str);
static int RB_Cst_DebugText_AddVertsToBuffer(CstVertPosColor *vertsBuffer, int vertsBufferOffset, idStr &str, const idVec3 &origin, float scale, const idVec4 &color, const idMat3 &viewAxis, const int align);

// -----------
void RB_Cst_ClearBackendTextureBindingCaches() {
	backEnd.glState.currenttmu = 0;

	// see idImage::PurgeImage
	for (int i = 0; i < MAX_MULTITEXTURE_UNITS; ++i) {
		backEnd.glState.tmu[i].current2DMap = idImage::TEXTURE_NOT_LOADED;
		backEnd.glState.tmu[i].currentCubeMap = idImage::TEXTURE_NOT_LOADED;
	}
}

// -----------
static void RB_Cst_SetupViewDefScissor() {
	if (!backEnd.currentScissor.Equals(backEnd.viewDef->scissor) && r_useScissor.GetBool()) {
		GL_Scissor(backEnd.viewDef->viewport.x1 + backEnd.viewDef->scissor.x1,
			backEnd.viewDef->viewport.y1 + backEnd.viewDef->scissor.y1,
			backEnd.viewDef->scissor.x2 + 1 - backEnd.viewDef->scissor.x1,
			backEnd.viewDef->scissor.y2 + 1 - backEnd.viewDef->scissor.y1);
		backEnd.currentScissor = backEnd.viewDef->scissor;
	}
}

// -----------
static void RB_Cst_SetupVertPosColorLayout() {
	// note: make sure to bind the vertex buffer object before calling this function

	qglVertexAttribPointerARB(PC_ATTRIB_INDEX_VERTEX, 3, GL_FLOAT, GL_FALSE, sizeof(CstVertPosColor), (void *)0);
	qglVertexAttribPointerARB(PC_ATTRIB_INDEX_COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(CstVertPosColor), (void *)offsetof(CstVertPosColor, color));

	qglEnableVertexAttribArrayARB(PC_ATTRIB_INDEX_VERTEX);
	qglDisableVertexAttribArrayARB(PC_ATTRIB_INDEX_NORMAL);
	qglEnableVertexAttribArrayARB(PC_ATTRIB_INDEX_COLOR);
	qglDisableVertexAttribArrayARB(PC_ATTRIB_INDEX_COLOR2);
	qglDisableVertexAttribArrayARB(PC_ATTRIB_INDEX_ST);
	qglDisableVertexAttribArrayARB(PC_ATTRIB_INDEX_TANGENT);

	backEnd.glState.vertexLayout = LAYOUT_UNKNOWN; // mark as not set
}

// -----------
static void RB_Cst_SetupShadowVertSkinnedLayout() {
	// note: make sure to bind the vertex buffer object before calling this function

	// see RB_StencilShadowPass
	qglEnableVertexAttribArrayARB(PC_ATTRIB_INDEX_VERTEX);
	qglDisableVertexAttribArrayARB(PC_ATTRIB_INDEX_NORMAL);
	qglEnableVertexAttribArrayARB(PC_ATTRIB_INDEX_COLOR);
	qglEnableVertexAttribArrayARB(PC_ATTRIB_INDEX_COLOR2);
	qglDisableVertexAttribArrayARB(PC_ATTRIB_INDEX_ST);
	qglDisableVertexAttribArrayARB(PC_ATTRIB_INDEX_TANGENT);

	qglVertexAttribPointerARB(PC_ATTRIB_INDEX_VERTEX, 4, GL_FLOAT, GL_FALSE, sizeof(idShadowVertSkinned), (void *)(SHADOWVERTSKINNED_XYZW_OFFSET));
	qglVertexAttribPointerARB(PC_ATTRIB_INDEX_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(idShadowVertSkinned), (void *)(SHADOWVERTSKINNED_COLOR_OFFSET));
	qglVertexAttribPointerARB(PC_ATTRIB_INDEX_COLOR2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(idShadowVertSkinned), (void *)(SHADOWVERTSKINNED_COLOR2_OFFSET));

	backEnd.glState.vertexLayout = LAYOUT_UNKNOWN; // mark as not set
}

// -----------
static void RB_Cst_SetupShadowVertLayout() {
	// note: make sure to bind the vertex buffer object before calling this function

	// see RB_StencilShadowPass
	qglEnableVertexAttribArrayARB(PC_ATTRIB_INDEX_VERTEX);
	qglDisableVertexAttribArrayARB(PC_ATTRIB_INDEX_NORMAL);
	qglDisableVertexAttribArrayARB(PC_ATTRIB_INDEX_COLOR);
	qglDisableVertexAttribArrayARB(PC_ATTRIB_INDEX_COLOR2);
	qglDisableVertexAttribArrayARB(PC_ATTRIB_INDEX_ST);
	qglDisableVertexAttribArrayARB(PC_ATTRIB_INDEX_TANGENT);

	qglVertexAttribPointerARB(PC_ATTRIB_INDEX_VERTEX, 4, GL_FLOAT, GL_FALSE, sizeof(idShadowVert), (void *)(SHADOWVERT_XYZW_OFFSET));

	backEnd.glState.vertexLayout = LAYOUT_UNKNOWN; // mark as not set
}

// -----------
static void RB_Cst_Simple_MVP_Setup(const idRenderMatrix & mvp) {
	// change the matrix
	RB_SetMVP(mvp);
	backEnd.currentSpace = NULL; // mark as not set

	// change the scissor if needed
	RB_Cst_SetupViewDefScissor();
}

// -----------
static void RB_Cst_SimpleViewEntitySetup(const viewEntity_t &viewEntity) {
	// change the matrix
	RB_SetMVP(viewEntity.mvp);
	backEnd.currentSpace = NULL; // mark as not set

	// change the scissor if needed
	RB_Cst_SetupViewDefScissor();
}

// -----------
static int RB_Cst_AddBoundsToBuffer(const idBounds &bounds, CstVertPosColor * vertsBuffer, int vertsBufferOffset, const idVec4 &color) {
	// see RB_DrawBounds

	if (bounds.IsCleared()) {
		return vertsBufferOffset;
	}

	CstVertPosColor p1, p2, p3, p4;
	p1.color = p2.color = p3.color = p4.color = color;

	// ------------
	p1.position.Set(bounds[0][0], bounds[0][1], bounds[0][2]);
	p2.position.Set(bounds[0][0], bounds[1][1], bounds[0][2]);
	p3.position.Set(bounds[1][0], bounds[1][1], bounds[0][2]);
	p4.position.Set(bounds[1][0], bounds[0][1], bounds[0][2]);

	vertsBuffer[vertsBufferOffset++] = p1;
	vertsBuffer[vertsBufferOffset++] = p2;

	vertsBuffer[vertsBufferOffset++] = p2;
	vertsBuffer[vertsBufferOffset++] = p3;

	vertsBuffer[vertsBufferOffset++] = p3;
	vertsBuffer[vertsBufferOffset++] = p4;

	vertsBuffer[vertsBufferOffset++] = p4;
	vertsBuffer[vertsBufferOffset++] = p1;

	// ------------
	p1.position.Set(bounds[0][0], bounds[0][1], bounds[1][2]);
	p2.position.Set(bounds[0][0], bounds[1][1], bounds[1][2]);
	p3.position.Set(bounds[1][0], bounds[1][1], bounds[1][2]);
	p4.position.Set(bounds[1][0], bounds[0][1], bounds[1][2]);

	vertsBuffer[vertsBufferOffset++] = p1;
	vertsBuffer[vertsBufferOffset++] = p2;

	vertsBuffer[vertsBufferOffset++] = p2;
	vertsBuffer[vertsBufferOffset++] = p3;

	vertsBuffer[vertsBufferOffset++] = p3;
	vertsBuffer[vertsBufferOffset++] = p4;

	vertsBuffer[vertsBufferOffset++] = p4;
	vertsBuffer[vertsBufferOffset++] = p1;

	// ------------
	p1.position.Set(bounds[0][0], bounds[0][1], bounds[0][2]);
	p2.position.Set(bounds[0][0], bounds[0][1], bounds[1][2]);
	vertsBuffer[vertsBufferOffset++] = p1;
	vertsBuffer[vertsBufferOffset++] = p2;

	p1.position.Set(bounds[0][0], bounds[1][1], bounds[0][2]);
	p2.position.Set(bounds[0][0], bounds[1][1], bounds[1][2]);
	vertsBuffer[vertsBufferOffset++] = p1;
	vertsBuffer[vertsBufferOffset++] = p2;

	p1.position.Set(bounds[1][0], bounds[0][1], bounds[0][2]);
	p2.position.Set(bounds[1][0], bounds[0][1], bounds[1][2]);
	vertsBuffer[vertsBufferOffset++] = p1;
	vertsBuffer[vertsBufferOffset++] = p2;

	p1.position.Set(bounds[1][0], bounds[1][1], bounds[0][2]);
	p2.position.Set(bounds[1][0], bounds[1][1], bounds[1][2]);
	vertsBuffer[vertsBufferOffset++] = p1;
	vertsBuffer[vertsBufferOffset++] = p2;

	return vertsBufferOffset;
}

// -----------
static int RB_Cst_AddExpandedTrianglesToBuffer(const srfTriangles_t *tri, const float radius, const idVec3 &vieworg, CstVertPosColor * vertsBuffer, int vertsBufferOffset, const idVec4 &color) {
	// see RB_DrawExpandedTriangles

	for (int i = 0; i < tri->numIndexes; i += 3) {

		idVec3 p[3] = { tri->verts[tri->indexes[i + 0]].xyz, tri->verts[tri->indexes[i + 1]].xyz, tri->verts[tri->indexes[i + 2]].xyz };

		idVec3 dir[6];
		dir[0] = p[0] - p[1];
		dir[1] = p[1] - p[2];
		dir[2] = p[2] - p[0];

		idVec3 normal = dir[0].Cross(dir[1]);

		if (normal * p[0] < normal * vieworg) {
			continue;
		}

		dir[0] = normal.Cross(dir[0]);
		dir[1] = normal.Cross(dir[1]);
		dir[2] = normal.Cross(dir[2]);

		dir[0].Normalize();
		dir[1].Normalize();
		dir[2].Normalize();

		// add points as line loop vertices
		for (int j = 0; j < 3; ++j) {
			int k = (j + 1) % 3;

			dir[4] = (dir[j] + dir[k]) * 0.5f;
			dir[4].Normalize();

			dir[3] = (dir[j] + dir[4]) * 0.5f;
			dir[3].Normalize();

			dir[5] = (dir[4] + dir[k]) * 0.5f;
			dir[5].Normalize();

			CstVertPosColor vert;
			vert.color = color;

			idVec3 point = p[k] + dir[j] * radius;
			vert.position = point;
			vertsBuffer[vertsBufferOffset++] = vert;

			point = p[k] + dir[3] * radius;
			vert.position = point;
			vertsBuffer[vertsBufferOffset++] = vert;

			point = p[k] + dir[4] * radius;
			vert.position = point;
			vertsBuffer[vertsBufferOffset++] = vert;

			point = p[k] + dir[5] * radius;
			vert.position = point;
			vertsBuffer[vertsBufferOffset++] = vert;

			point = p[k] + dir[k] * radius;
			vert.position = point;
			vertsBuffer[vertsBufferOffset++] = vert;
		}
	}

	return vertsBufferOffset;
}
//#modified-fva; END

/*
================
RB_DrawBounds
================
*/
//#modified-fva; BEGIN
/*
void RB_DrawBounds( const idBounds &bounds ) {
	if ( bounds.IsCleared() ) {
		return;
	}
	qglBegin( GL_LINE_LOOP );
	qglVertex3f( bounds[0][0], bounds[0][1], bounds[0][2] );
	qglVertex3f( bounds[0][0], bounds[1][1], bounds[0][2] );
	qglVertex3f( bounds[1][0], bounds[1][1], bounds[0][2] );
	qglVertex3f( bounds[1][0], bounds[0][1], bounds[0][2] );
	qglEnd();
	qglBegin( GL_LINE_LOOP );
	qglVertex3f( bounds[0][0], bounds[0][1], bounds[1][2] );
	qglVertex3f( bounds[0][0], bounds[1][1], bounds[1][2] );
	qglVertex3f( bounds[1][0], bounds[1][1], bounds[1][2] );
	qglVertex3f( bounds[1][0], bounds[0][1], bounds[1][2] );
	qglEnd();

	qglBegin( GL_LINES );
	qglVertex3f( bounds[0][0], bounds[0][1], bounds[0][2] );
	qglVertex3f( bounds[0][0], bounds[0][1], bounds[1][2] );

	qglVertex3f( bounds[0][0], bounds[1][1], bounds[0][2] );
	qglVertex3f( bounds[0][0], bounds[1][1], bounds[1][2] );

	qglVertex3f( bounds[1][0], bounds[0][1], bounds[0][2] );
	qglVertex3f( bounds[1][0], bounds[0][1], bounds[1][2] );

	qglVertex3f( bounds[1][0], bounds[1][1], bounds[0][2] );
	qglVertex3f( bounds[1][0], bounds[1][1], bounds[1][2] );
	qglEnd();
}
*/
//#modified-fva; END


/*
================
RB_SimpleSurfaceSetup
================
*/
static void RB_SimpleSurfaceSetup( const drawSurf_t *drawSurf ) {
	//#modified-fva; BEGIN
	/*
	// change the matrix if needed
	if ( drawSurf->space != backEnd.currentSpace ) {
		qglLoadMatrixf( drawSurf->space->modelViewMatrix );
		backEnd.currentSpace = drawSurf->space;
	}
	*/
	// change the matrix
	RB_SetMVP(drawSurf->space->mvp);
	backEnd.currentSpace = NULL; // mark as not set
	//#modified-fva; END

	// change the scissor if needed
	if ( !backEnd.currentScissor.Equals( drawSurf->scissorRect ) && r_useScissor.GetBool() ) {
		GL_Scissor( backEnd.viewDef->viewport.x1 + drawSurf->scissorRect.x1,
					backEnd.viewDef->viewport.y1 + drawSurf->scissorRect.y1,
					drawSurf->scissorRect.x2 + 1 - drawSurf->scissorRect.x1,
					drawSurf->scissorRect.y2 + 1 - drawSurf->scissorRect.y1 );
		backEnd.currentScissor = drawSurf->scissorRect;
	}
}

/*
================
RB_SimpleWorldSetup
================
*/
//#modified-fva; BEGIN
/*
static void RB_SimpleWorldSetup() {
	backEnd.currentSpace = &backEnd.viewDef->worldSpace;


	qglLoadMatrixf( backEnd.viewDef->worldSpace.modelViewMatrix );

	GL_Scissor( backEnd.viewDef->viewport.x1 + backEnd.viewDef->scissor.x1,
				backEnd.viewDef->viewport.y1 + backEnd.viewDef->scissor.y1,
				backEnd.viewDef->scissor.x2 + 1 - backEnd.viewDef->scissor.x1,
				backEnd.viewDef->scissor.y2 + 1 - backEnd.viewDef->scissor.y1 );
	backEnd.currentScissor = backEnd.viewDef->scissor;
}
*/
static void RB_SimpleWorldSetup() {
	// change the matrix
	RB_SetMVP(backEnd.viewDef->worldSpace.mvp);
	backEnd.currentSpace = NULL; // mark as not set

	// change the scissor if needed
	RB_Cst_SetupViewDefScissor();
}
//#modified-fva; END

/*
=================
RB_PolygonClear

This will cover the entire screen with normal rasterization.
Texturing is disabled, but the existing glColor, glDepthMask,
glColorMask, and the enabled state of depth buffering and
stenciling will matter.
=================
*/
//#modified-fva; BEGIN
/*
void RB_PolygonClear() {
	qglPushMatrix();
	qglPushAttrib( GL_ALL_ATTRIB_BITS  );
	qglLoadIdentity();
	qglDisable( GL_TEXTURE_2D );
	qglDisable( GL_DEPTH_TEST );
	qglDisable( GL_CULL_FACE );
	qglDisable( GL_SCISSOR_TEST );
	qglBegin( GL_POLYGON );
	qglVertex3f( -20, -20, -10 );
	qglVertex3f( 20, -20, -10 );
	qglVertex3f( 20, 20, -10 );
	qglVertex3f( -20, 20, -10 );
	qglEnd();
	qglPopAttrib();
	qglPopMatrix();
}
*/
//#modified-fva; END

/*
====================
RB_ShowDestinationAlpha
====================
*/
//#modified-fva; BEGIN
/*
void RB_ShowDestinationAlpha() {
	GL_State( GLS_SRCBLEND_DST_ALPHA | GLS_DSTBLEND_ZERO | GLS_DEPTHMASK | GLS_DEPTHFUNC_ALWAYS );
	GL_Color( 1, 1, 1 );
	RB_PolygonClear();
}
*/
//#modified-fva; END

/*
===================
RB_ScanStencilBuffer

Debugging tool to see what values are in the stencil buffer
===================
*/
//#modified-fva; BEGIN
/*
void RB_ScanStencilBuffer() {
	int		counts[256];
	int		i;
	byte	*stencilReadback;

	memset( counts, 0, sizeof( counts ) );

	stencilReadback = (byte *)R_StaticAlloc( renderSystem->GetWidth() * renderSystem->GetHeight(), TAG_RENDER_TOOLS );
	qglReadPixels( 0, 0, renderSystem->GetWidth(), renderSystem->GetHeight(), GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, stencilReadback );

	for ( i = 0; i < renderSystem->GetWidth() * renderSystem->GetHeight(); i++ ) {
		counts[ stencilReadback[i] ]++;
	}

	R_StaticFree( stencilReadback );

	// print some stats (not supposed to do from back end in SMP...)
	common->Printf( "stencil values:\n" );
	for ( i = 0; i < 255; i++ ) {
		if ( counts[i] ) {
			common->Printf( "%i: %i\n", i, counts[i] );
		}
	}
}
*/
//#modified-fva; END


/*
===================
RB_CountStencilBuffer

Print an overdraw count based on stencil index values
===================
*/
static void RB_CountStencilBuffer() {
	int		count;
	int		i;
	byte	*stencilReadback;


	stencilReadback = (byte *)R_StaticAlloc( renderSystem->GetWidth() * renderSystem->GetHeight(), TAG_RENDER_TOOLS );
	qglReadPixels( 0, 0, renderSystem->GetWidth(), renderSystem->GetHeight(), GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, stencilReadback );

	count = 0;
	for ( i = 0; i < renderSystem->GetWidth() * renderSystem->GetHeight(); i++ ) {
		count += stencilReadback[i];
	}

	R_StaticFree( stencilReadback );

	// print some stats (not supposed to do from back end in SMP...)
	common->Printf( "overdraw: %5.1f\n", (float)count/(renderSystem->GetWidth() * renderSystem->GetHeight())  );
}

/*
===================
R_ColorByStencilBuffer

Sets the screen colors based on the contents of the
stencil buffer.  Stencil of 0 = black, 1 = red, 2 = green,
3 = blue, ..., 7+ = white
===================
*/
//#modified-fva; BEGIN
/*
static void R_ColorByStencilBuffer() {
	int		i;
	static float	colors[8][3] = {
		{0,0,0},
		{1,0,0},
		{0,1,0},
		{0,0,1},
		{0,1,1},
		{1,0,1},
		{1,1,0},
		{1,1,1},
	};

	// clear color buffer to white (>6 passes)
	GL_Clear( true, false, false, 0, 1.0f, 1.0f, 1.0f, 1.0f );

	// now draw color for each stencil value
	qglStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
	for ( i = 0; i < 6; i++ ) {
		GL_Color( colors[i] );
		renderProgManager.BindShader_Color();
		qglStencilFunc( GL_EQUAL, i, 255 );
		RB_PolygonClear();
	}

	qglStencilFunc( GL_ALWAYS, 0, 255 );
}
*/
static void R_ColorByStencilBuffer() {
	int		i;
	static float	colors[8][3] = {
		{0,0,0},
		{1,0,0},
		{0,1,0},
		{0,0,1},
		{0,1,1},
		{1,0,1},
		{1,1,0},
		{1,1,1},
	};

	GL_State(GLS_DEFAULT);
	RB_Cst_Simple_MVP_Setup(renderMatrix_identity);
	renderProgManager.BindShader_Color();

	// clear color buffer to white (>6 passes)
	GL_Clear(true, false, false, 0, 1.0f, 1.0f, 1.0f, 1.0f);

	// now draw color for each stencil value
	uint64 baseState = GLS_DEPTHMASK | GLS_DEPTHFUNC_ALWAYS | GLS_STENCIL_FUNC_EQUAL | GLS_STENCIL_OP_FAIL_KEEP | GLS_STENCIL_OP_ZFAIL_KEEP | GLS_STENCIL_OP_PASS_KEEP;
	GL_Cull(CT_TWO_SIDED);

	for (i = 0; i < 7; ++i) {
		GL_Color(colors[i]);
		GL_State(baseState | GLS_STENCIL_MAKE_REF(i) | GLS_STENCIL_MAKE_MASK(255));
		RB_DrawElementsWithCounters(&backEnd.unitSquareSurface);
	}

	GL_Cull(CT_FRONT_SIDED);
	GL_State(GLS_DEFAULT);
}
//#modified-fva; END

//======================================================================

/*
==================
RB_ShowOverdraw
==================
*/
//#modified-fva; BEGIN
/*
void RB_ShowOverdraw() {
	const idMaterial *	material;
	int					i;
	drawSurf_t * *		drawSurfs;
	const drawSurf_t *	surf;
	int					numDrawSurfs;
	viewLight_t *		vLight;

	if ( r_showOverDraw.GetInteger() == 0 ) {
		return;
	}

	material = declManager->FindMaterial( "textures/common/overdrawtest", false );
	if ( material == NULL ) {
		return;
	}

	drawSurfs = backEnd.viewDef->drawSurfs;
	numDrawSurfs = backEnd.viewDef->numDrawSurfs;

	int interactions = 0;
	for ( vLight = backEnd.viewDef->viewLights; vLight; vLight = vLight->next ) {
		for ( surf = vLight->localInteractions; surf; surf = surf->nextOnLight ) {
			interactions++;
		}
		for ( surf = vLight->globalInteractions; surf; surf = surf->nextOnLight ) {
			interactions++;
		}
	}

	// FIXME: can't frame alloc from the renderer back-end
	drawSurf_t **newDrawSurfs = (drawSurf_t **)R_FrameAlloc( numDrawSurfs + interactions * sizeof( newDrawSurfs[0] ), FRAME_ALLOC_DRAW_SURFACE_POINTER );

	for ( i = 0; i < numDrawSurfs; i++ ) {
		surf = drawSurfs[i];
		if ( surf->material ) {
			const_cast<drawSurf_t *>(surf)->material = material;
		}
		newDrawSurfs[i] = const_cast<drawSurf_t *>(surf);
	}

	for ( vLight = backEnd.viewDef->viewLights; vLight; vLight = vLight->next ) {
		for ( surf = vLight->localInteractions; surf; surf = surf->nextOnLight ) {
			const_cast<drawSurf_t *>(surf)->material = material;
			newDrawSurfs[i++] = const_cast<drawSurf_t *>(surf);
		}
		for ( surf = vLight->globalInteractions; surf; surf = surf->nextOnLight ) {
			const_cast<drawSurf_t *>(surf)->material = material;
			newDrawSurfs[i++] = const_cast<drawSurf_t *>(surf);
		}
		vLight->localInteractions = NULL;
		vLight->globalInteractions = NULL;
	}

	switch( r_showOverDraw.GetInteger() ) {
		case 1: // geometry overdraw
			const_cast<viewDef_t *>(backEnd.viewDef)->drawSurfs = newDrawSurfs;
			const_cast<viewDef_t *>(backEnd.viewDef)->numDrawSurfs = numDrawSurfs;
			break;
		case 2: // light interaction overdraw
			const_cast<viewDef_t *>(backEnd.viewDef)->drawSurfs = &newDrawSurfs[numDrawSurfs];
			const_cast<viewDef_t *>(backEnd.viewDef)->numDrawSurfs = interactions;
			break;
		case 3: // geometry + light interaction overdraw
			const_cast<viewDef_t *>(backEnd.viewDef)->drawSurfs = newDrawSurfs;
			const_cast<viewDef_t *>(backEnd.viewDef)->numDrawSurfs += interactions;
			break;
	}
}
*/
void RB_ShowOverdraw() {
	if (r_showOverDraw.GetInteger() == 0) {
		return;
	}
	common->CstBrtRequestEnable();
	if (!common->CstBrtAreEnabled()) {
		return;
	}

	int option = idMath::ClampInt(-3, 3, r_showOverDraw.GetInteger());
	bool printCount = false;
	if (option < 0) {
		option = -option;
		printCount = true;
	}

	GL_State(GLS_DEFAULT);
	RB_Cst_SetupViewDefScissor();

	GL_Clear(false, false, true, 0, 0.0f, 0.0f, 0.0f, 0.0f);

	GL_State(GLS_DEPTHFUNC_EQUAL | GLS_STENCIL_OP_FAIL_KEEP | GLS_STENCIL_OP_ZFAIL_INCR | GLS_STENCIL_OP_PASS_INCR);

	GL_Color(0.0f, 0.0f, 0.0f, 1.0f);

	if (option == 1 || option == 3) {
		drawSurf_t ** drawSurfs = backEnd.viewDef->drawSurfs;
		const int numDrawSurfs = backEnd.viewDef->numDrawSurfs;

		for (int i = 0; i < numDrawSurfs; ++i) {
			const drawSurf_t * surf = drawSurfs[i];
			if (!surf) {
				continue;
			}
			if (surf->numIndexes == 0) {
				continue;
			}
			if (!surf->space) {
				continue;
			}
			if (backEnd.viewDef->isXraySubview && surf->space->entityDef) { // see RB_DrawShaderPasses
				if (surf->space->entityDef->parms.xrayIndex != 2) {
					continue;
				}
			}
			RB_SimpleSurfaceSetup(surf);
			if (surf->jointCache) {
				renderProgManager.BindShader_CstDebugColorSkinned();
			} else {
				renderProgManager.BindShader_Color();
			}
			RB_DrawElementsWithCounters(surf);
		}
	}

	if (option == 2 || option == 3) {
		// same as in RB_ShowLightCount
		for (viewLight_t * vLight = backEnd.viewDef->viewLights; vLight; vLight = vLight->next) {
			for (int i = 0; i < 2; i++) {
				for (drawSurf_t * surf = i ? vLight->localInteractions : vLight->globalInteractions; surf; surf = (drawSurf_t *)surf->nextOnLight) {
					RB_SimpleSurfaceSetup(surf);
					if (surf->jointCache) {
						renderProgManager.BindShader_CstDebugColorSkinned();
					} else {
						renderProgManager.BindShader_Color();
					}
					RB_DrawElementsWithCounters(surf);
				}
			}
		}
	}

	R_ColorByStencilBuffer();

	if (printCount) {
		RB_CountStencilBuffer();
	}

	renderProgManager.Unbind();
	GL_State(GLS_DEFAULT);
}
//#modified-fva; END

/*
===================
RB_ShowIntensity

Debugging tool to see how much dynamic range a scene is using.
The greatest of the rgb values at each pixel will be used, with
the resulting color shading from red at 0 to green at 128 to blue at 255
===================
*/
//#modified-fva; BEGIN
/*
static void RB_ShowIntensity() {
	byte	*colorReadback;
	int		i, j, c;

	if ( !r_showIntensity.GetBool() ) {
		return;
	}

	colorReadback = (byte *)R_StaticAlloc( renderSystem->GetWidth() * renderSystem->GetHeight() * 4, TAG_RENDER_TOOLS );
	qglReadPixels( 0, 0, renderSystem->GetWidth(), renderSystem->GetHeight(), GL_RGBA, GL_UNSIGNED_BYTE, colorReadback );

	c = renderSystem->GetWidth() * renderSystem->GetHeight() * 4;
	for ( i = 0; i < c; i+=4 ) {
		j = colorReadback[i];
		if ( colorReadback[i+1] > j ) {
			j = colorReadback[i+1];
		}
		if ( colorReadback[i+2] > j ) {
			j = colorReadback[i+2];
		}
		if ( j < 128 ) {
			colorReadback[i+0] = 2*(128-j);
			colorReadback[i+1] = 2*j;
			colorReadback[i+2] = 0;
		} else {
			colorReadback[i+0] = 0;
			colorReadback[i+1] = 2*(255-j);
			colorReadback[i+2] = 2*(j-128);
		}
	}

	// draw it back to the screen
	qglLoadIdentity();
	qglMatrixMode( GL_PROJECTION );
	GL_State( GLS_DEPTHFUNC_ALWAYS );
	qglPushMatrix();
	qglLoadIdentity();
    qglOrtho( 0, 1, 0, 1, -1, 1 );
	qglRasterPos2f( 0, 0 );
	qglPopMatrix();
	GL_Color( 1, 1, 1 );
	globalImages->BindNull();
	qglMatrixMode( GL_MODELVIEW );

	qglDrawPixels( renderSystem->GetWidth(), renderSystem->GetHeight(), GL_RGBA , GL_UNSIGNED_BYTE, colorReadback );

	R_StaticFree( colorReadback );
}
*/
static void RB_ShowIntensity() {
	if (!r_showIntensity.GetBool()) {
		return;
	}
	common->CstBrtRequestEnable();
	if (!common->CstBrtAreEnabled()) {
		return;
	}

	const int screenWidth = renderSystem->GetWidth();
	const int screenHeight = renderSystem->GetHeight();

	byte * colorReadback = (byte *)Mem_Alloc(screenWidth * screenHeight * 4, TAG_RENDER_TOOLS);
	qglReadPixels(0, 0, screenWidth, screenHeight, GL_RGBA, GL_UNSIGNED_BYTE, colorReadback);

	int c = screenWidth * screenHeight * 4;
	for (int i = 0; i < c; i += 4) {
		int j = colorReadback[i];
		if (colorReadback[i + 1] > j) {
			j = colorReadback[i + 1];
		}
		if (colorReadback[i + 2] > j) {
			j = colorReadback[i + 2];
		}
		if (j < 128) {
			colorReadback[i + 0] = 2 * (128 - j);
			colorReadback[i + 1] = 2 * j;
			colorReadback[i + 2] = 0;
		} else {
			colorReadback[i + 0] = 0;
			colorReadback[i + 1] = 2 * (255 - j);
			colorReadback[i + 2] = 2 * (j - 128);
		}
	}

	GLuint texId;
	qglGenTextures(1, &texId);
	qglBindMultiTextureEXT(GL_TEXTURE0_ARB, GL_TEXTURE_2D, texId);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, colorReadback);

	GL_State(GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO | GLS_DEPTHMASK | GLS_DEPTHFUNC_ALWAYS);
	GL_Cull(CT_TWO_SIDED);

	float texS[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
	float texT[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
	renderProgManager.SetRenderParm(RENDERPARM_TEXTUREMATRIX_S, texS);
	renderProgManager.SetRenderParm(RENDERPARM_TEXTUREMATRIX_T, texT);

	float texGenEnabled[4] = { 0, 0, 0, 0 };
	renderProgManager.SetRenderParm(RENDERPARM_TEXGEN_0_ENABLED, texGenEnabled);

	GL_Color(1.0f, 1.0f, 1.0f, 1.0f);

	RB_Cst_Simple_MVP_Setup(renderMatrix_identity);
	renderProgManager.BindShader_Texture();

	RB_DrawElementsWithCounters(&backEnd.unitSquareSurface);

	qglBindMultiTextureEXT(GL_TEXTURE0_ARB, GL_TEXTURE_2D, 0);
	qglDeleteTextures(1, &texId);
	RB_Cst_ClearBackendTextureBindingCaches();
	renderProgManager.Unbind();
	if (colorReadback) {
		Mem_Free(colorReadback);
		colorReadback = NULL;
	}
	GL_State(GLS_DEFAULT);
	GL_Cull(CT_FRONT_SIDED);
}
//#modified-fva; END


/*
===================
RB_ShowDepthBuffer

Draw the depth buffer as colors
===================
*/
//#modified-fva; BEGIN
/*
static void RB_ShowDepthBuffer() {
	void	*depthReadback;

	if ( !r_showDepth.GetBool() ) {
		return;
	}

	qglPushMatrix();
	qglLoadIdentity();
	qglMatrixMode( GL_PROJECTION );
	qglPushMatrix();
	qglLoadIdentity();
    qglOrtho( 0, 1, 0, 1, -1, 1 );
	qglRasterPos2f( 0, 0 );
	qglPopMatrix();
	qglMatrixMode( GL_MODELVIEW );
	qglPopMatrix();

	GL_State( GLS_DEPTHFUNC_ALWAYS );
	GL_Color( 1, 1, 1 );
	globalImages->BindNull();

	depthReadback = R_StaticAlloc( renderSystem->GetWidth() * renderSystem->GetHeight()*4, TAG_RENDER_TOOLS );
	memset( depthReadback, 0, renderSystem->GetWidth() * renderSystem->GetHeight()*4 );

	qglReadPixels( 0, 0, renderSystem->GetWidth(), renderSystem->GetHeight(), GL_DEPTH_COMPONENT , GL_FLOAT, depthReadback );

#if 0
	for ( i = 0; i < renderSystem->GetWidth() * renderSystem->GetHeight(); i++ ) {
		((byte *)depthReadback)[i*4] =
		((byte *)depthReadback)[i*4+1] =
		((byte *)depthReadback)[i*4+2] = 255 * ((float *)depthReadback)[i];
		((byte *)depthReadback)[i*4+3] = 1;
	}
#endif

	qglDrawPixels( renderSystem->GetWidth(), renderSystem->GetHeight(), GL_RGBA , GL_UNSIGNED_BYTE, depthReadback );
	R_StaticFree( depthReadback );
}
*/
static void RB_ShowDepthBuffer() {
	if (r_showDepth.GetInteger() == 0) {
		return;
	}
	common->CstBrtRequestEnable();
	if (!common->CstBrtAreEnabled()) {
		return;
	}

	int option = idMath::ClampInt(0, 2, r_showDepth.GetInteger() - 1);

	RB_Cst_SetupViewDefScissor();

	int screenWidth = renderSystem->GetWidth();
	int screenHeight = renderSystem->GetHeight();

	GLvoid * depthReadback = NULL;

	if (option == 0) {
		depthReadback = Mem_Alloc(screenWidth * screenHeight * 4, TAG_RENDER_TOOLS);
		memset(depthReadback, 0, screenWidth * screenHeight * 4);
		qglReadPixels(0, 0, screenWidth, screenHeight, GL_DEPTH_COMPONENT, GL_FLOAT, depthReadback);
	}

	GLuint texId;
	qglGenTextures(1, &texId);
	qglBindMultiTextureEXT(GL_TEXTURE0_ARB, GL_TEXTURE_2D, texId);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (option == 0) {
		qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, depthReadback);
	} else {
		qglCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 0, 0, screenWidth, screenHeight, 0);
	}

	GL_State(GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO | GLS_DEPTHMASK | GLS_DEPTHFUNC_ALWAYS);
	GL_Cull(CT_TWO_SIDED);

	renderProgManager.SetRenderParm((renderParm_t)(RENDERPARM_USER + 0), idVec4((float)option).ToFloatPtr());

	renderProgManager.BindShader_CstDebugDepth();
	RB_DrawElementsWithCounters(&backEnd.unitSquareSurface);

	qglBindMultiTextureEXT(GL_TEXTURE0_ARB, GL_TEXTURE_2D, 0);
	qglDeleteTextures(1, &texId);
	RB_Cst_ClearBackendTextureBindingCaches();
	renderProgManager.Unbind();
	if (depthReadback) {
		Mem_Free(depthReadback);
		depthReadback = NULL;
	}
	GL_State(GLS_DEFAULT);
	GL_Cull(CT_FRONT_SIDED);
}
//#modified-fva; END

/*
=================
RB_ShowLightCount

This is a debugging tool that will draw each surface with a color
based on how many lights are effecting it
=================
*/
//#modified-fva; BEGIN
/*
static void RB_ShowLightCount() {
	int		i;
	const drawSurf_t	*surf;
	const viewLight_t	*vLight;

	if ( !r_showLightCount.GetBool() ) {
		return;
	}

	RB_SimpleWorldSetup();

	GL_Clear( false, false, true, 0, 0.0f, 0.0f, 0.0f, 0.0f );

	// optionally count everything through walls
	if ( r_showLightCount.GetInteger() >= 2 ) {
		GL_State( GLS_DEPTHFUNC_EQUAL | GLS_STENCIL_OP_FAIL_KEEP | GLS_STENCIL_OP_ZFAIL_INCR | GLS_STENCIL_OP_PASS_INCR );
	} else {
		GL_State( GLS_DEPTHFUNC_EQUAL | GLS_STENCIL_OP_FAIL_KEEP | GLS_STENCIL_OP_ZFAIL_KEEP | GLS_STENCIL_OP_PASS_INCR );
	}

	globalImages->defaultImage->Bind();

	for ( vLight = backEnd.viewDef->viewLights; vLight; vLight = vLight->next ) {
		for ( i = 0; i < 2; i++ ) {
			for ( surf = i ? vLight->localInteractions: vLight->globalInteractions; surf; surf = (drawSurf_t *)surf->nextOnLight ) {
				RB_SimpleSurfaceSetup( surf );
				RB_DrawElementsWithCounters( surf );
			}
		}
	}

	// display the results
	R_ColorByStencilBuffer();

	if ( r_showLightCount.GetInteger() > 2 ) {
		RB_CountStencilBuffer();
	}
}
*/
static void RB_ShowLightCount() {
	if (!r_showLightCount.GetBool()) {
		return;
	}
	common->CstBrtRequestEnable();
	if (!common->CstBrtAreEnabled()) {
		return;
	}

	GL_State(GLS_DEFAULT);
	RB_Cst_SetupViewDefScissor();

	GL_Clear(false, false, true, 0, 0.0f, 0.0f, 0.0f, 0.0f);

	// optionally count everything through walls
	if (r_showLightCount.GetInteger() >= 2) {
		GL_State(GLS_DEPTHFUNC_EQUAL | GLS_STENCIL_OP_FAIL_KEEP | GLS_STENCIL_OP_ZFAIL_INCR | GLS_STENCIL_OP_PASS_INCR);
	} else {
		GL_State(GLS_DEPTHFUNC_EQUAL | GLS_STENCIL_OP_FAIL_KEEP | GLS_STENCIL_OP_ZFAIL_KEEP | GLS_STENCIL_OP_PASS_INCR);
	}

	GL_Color(0.0f, 0.0f, 0.0f, 1.0f);
	for (viewLight_t * vLight = backEnd.viewDef->viewLights; vLight; vLight = vLight->next) {
		for (int i = 0; i < 2; i++) {
			for (drawSurf_t * surf = i ? vLight->localInteractions : vLight->globalInteractions; surf; surf = (drawSurf_t *)surf->nextOnLight) {
				RB_SimpleSurfaceSetup(surf);
				if (surf->jointCache) {
					renderProgManager.BindShader_CstDebugColorSkinned();
				} else {
					renderProgManager.BindShader_Color();
				}
				RB_DrawElementsWithCounters(surf);
			}
		}
	}

	// display the results
	R_ColorByStencilBuffer();

	if (r_showLightCount.GetInteger() > 2) {
		RB_CountStencilBuffer();
	}

	renderProgManager.Unbind();
	GL_State(GLS_DEFAULT);
}
//#modified-fva; END

/*
===============
RB_SetWeaponDepthHack
===============
*/
//#modified-fva; BEGIN
/*
static void RB_SetWeaponDepthHack() {
}
*/
//#modified-fva; END

/*
===============
RB_SetModelDepthHack
===============
*/
//#modified-fva; BEGIN
/*
static void RB_SetModelDepthHack( float depth ) {
}
*/
//#modified-fva; END

/*
===============
RB_EnterWeaponDepthHack
===============
*/
//#modified-fva; BEGIN
/*
static void RB_EnterWeaponDepthHack() {
	float	matrix[16];

	memcpy( matrix, backEnd.viewDef->projectionMatrix, sizeof( matrix ) );

	const float modelDepthHack = 0.25f;
	matrix[2] *= modelDepthHack;
	matrix[6] *= modelDepthHack;
	matrix[10] *= modelDepthHack;
	matrix[14] *= modelDepthHack;

	qglMatrixMode( GL_PROJECTION );
	qglLoadMatrixf( matrix );
	qglMatrixMode( GL_MODELVIEW );
}
*/
//#modified-fva; END

/*
===============
RB_EnterModelDepthHack
===============
*/
//#modified-fva; BEGIN
/*
static void RB_EnterModelDepthHack( float depth ) {
	float matrix[16];

	memcpy( matrix, backEnd.viewDef->projectionMatrix, sizeof( matrix ) );

	matrix[14] -= depth;

	qglMatrixMode( GL_PROJECTION );
	qglLoadMatrixf( matrix );
	qglMatrixMode( GL_MODELVIEW );
}
*/
//#modified-fva; END

/*
===============
RB_LeaveDepthHack
===============
*/
//#modified-fva; BEGIN
/*
static void RB_LeaveDepthHack() {
	qglMatrixMode( GL_PROJECTION );
	qglLoadMatrixf( backEnd.viewDef->projectionMatrix );
	qglMatrixMode( GL_MODELVIEW );
}
*/
//#modified-fva; END

/*
=============
RB_LoadMatrixWithBypass

does a glLoadMatrixf after optionally applying the low-latency bypass matrix
=============
*/
//#modified-fva; BEGIN
/*
static void RB_LoadMatrixWithBypass( const float m[16] ) {
	glLoadMatrixf( m );
}
*/
//#modified-fva; END

/*
====================
RB_RenderDrawSurfListWithFunction

The triangle functions can check backEnd.currentSpace != surf->space
to see if they need to perform any new matrix setup.  The modelview
matrix will already have been loaded, and backEnd.currentSpace will
be updated after the triangle function completes.
====================
*/
//#modified-fva; BEGIN
/*
static void RB_RenderDrawSurfListWithFunction( drawSurf_t **drawSurfs, int numDrawSurfs, void (*triFunc_)( const drawSurf_t *) ) {
	backEnd.currentSpace = NULL;

	for ( int i = 0 ; i < numDrawSurfs ; i++ ) {
		const drawSurf_t * drawSurf = drawSurfs[i];
		if ( drawSurf == NULL ) {
			continue;
		}
		assert( drawSurf->space != NULL );
		if ( drawSurf->space != NULL ) {	// is it ever NULL?  Do we need to check?
			// Set these values ahead of time so we don't have to reconstruct the matrices on the consoles
			if ( drawSurf->space->weaponDepthHack ) {
				RB_SetWeaponDepthHack();
			}

			if ( drawSurf->space->modelDepthHack != 0.0f ) {
				RB_SetModelDepthHack( drawSurf->space->modelDepthHack );
			}

			// change the matrix if needed
			if ( drawSurf->space != backEnd.currentSpace ) {
				RB_LoadMatrixWithBypass( drawSurf->space->modelViewMatrix );
			}

			if ( drawSurf->space->weaponDepthHack ) {
				RB_EnterWeaponDepthHack();
			}

			if ( drawSurf->space->modelDepthHack != 0.0f ) {
				RB_EnterModelDepthHack( drawSurf->space->modelDepthHack );
			}
		}

		// change the scissor if needed
		if ( r_useScissor.GetBool() && !backEnd.currentScissor.Equals( drawSurf->scissorRect ) ) {
			backEnd.currentScissor = drawSurf->scissorRect;
			GL_Scissor( backEnd.viewDef->viewport.x1 + backEnd.currentScissor.x1,
				backEnd.viewDef->viewport.y1 + backEnd.currentScissor.y1,
				backEnd.currentScissor.x2 + 1 - backEnd.currentScissor.x1,
				backEnd.currentScissor.y2 + 1 - backEnd.currentScissor.y1 );
		}

		// render it
		triFunc_( drawSurf );

		if ( drawSurf->space != NULL && ( drawSurf->space->weaponDepthHack || drawSurf->space->modelDepthHack != 0.0f ) ) {
			RB_LeaveDepthHack();
		}

		backEnd.currentSpace = drawSurf->space;
	}
}
*/
//#modified-fva; END

/*
=================
RB_ShowSilhouette

Blacks out all edges, then adds color for each edge that a shadow
plane extends from, allowing you to see doubled edges

FIXME: not thread safe!
=================
*/
//#modified-fva; BEGIN
/*
static void RB_ShowSilhouette() {
	int		i;
	const drawSurf_t	*surf;
	const viewLight_t	*vLight;

	if ( !r_showSilhouette.GetBool() ) {
		return;
	}

	//
	// clear all triangle edges to black
	//
	globalImages->BindNull();
	qglDisable( GL_TEXTURE_2D );

	GL_Color( 0, 0, 0 );

	GL_State( GLS_DEPTHFUNC_ALWAYS | GLS_POLYMODE_LINE );

	GL_Cull( CT_TWO_SIDED );

	RB_RenderDrawSurfListWithFunction( backEnd.viewDef->drawSurfs, backEnd.viewDef->numDrawSurfs,
		RB_DrawElementsWithCounters );


	//
	// now blend in edges that cast silhouettes
	//
	RB_SimpleWorldSetup();
	GL_Color( 0.5, 0, 0 );
	GL_State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );

	for ( vLight = backEnd.viewDef->viewLights; vLight; vLight = vLight->next ) {
		for ( i = 0; i < 2; i++ ) {
			for ( surf = i ? vLight->localShadows : vLight->globalShadows
				; surf; surf = (drawSurf_t *)surf->nextOnLight ) {
				RB_SimpleSurfaceSetup( surf );

				const srfTriangles_t * tri = surf->frontEndGeo;

				idVertexBuffer vertexBuffer;
				if ( !vertexCache.GetVertexBuffer( tri->shadowCache, &vertexBuffer ) ) {
					continue;
				}

				qglBindBufferARB( GL_ARRAY_BUFFER_ARB, (GLuint)vertexBuffer.GetAPIObject() );
				int vertOffset = vertexBuffer.GetOffset();

				qglVertexPointer( 3, GL_FLOAT, sizeof( idShadowVert ), (void *)vertOffset );
				qglBegin( GL_LINES );

				for ( int j = 0; j < tri->numIndexes; j+=3 ) {
					int		i1 = tri->indexes[j+0];
					int		i2 = tri->indexes[j+1];
					int		i3 = tri->indexes[j+2];

					if ( (i1 & 1) + (i2 & 1) + (i3 & 1) == 1 ) {
						if ( (i1 & 1) + (i2 & 1) == 0 ) {
							qglArrayElement( i1 );
							qglArrayElement( i2 );
						} else if ( (i1 & 1 ) + (i3 & 1) == 0 ) {
							qglArrayElement( i1 );
							qglArrayElement( i3 );
						}
					}
				}
				qglEnd();

			}
		}
	}

	GL_State( GLS_DEFAULT );
	GL_Color( 1,1,1 );
	GL_Cull( CT_FRONT_SIDED );
}
*/
static void RB_ShowSilhouette() {
	if (!r_showSilhouette.GetBool()) {
		return;
	}
	common->CstBrtRequestEnable();
	if (!common->CstBrtAreEnabled()) {
		return;
	}

	int width = r_debugLineWidth.GetInteger();
	if (width < 1) {
		width = 1;
	} else if (width > 10) {
		width = 10;
	}
	qglLineWidth(width);

	//
	// clear all triangle edges to black
	//
	GL_State(GLS_DEPTHFUNC_ALWAYS | GLS_POLYMODE_LINE);
	GL_Cull(CT_TWO_SIDED);

	GL_Color(0.0f, 0.0f, 0.0f);
	drawSurf_t ** drawSurfs = backEnd.viewDef->drawSurfs;
	int numDrawSurfs = backEnd.viewDef->numDrawSurfs;

	for (int i = 0; i < numDrawSurfs; ++i) {
		const drawSurf_t * surf = drawSurfs[i];
		if (!surf) {
			continue;
		}
		if (!surf->space) {
			continue;
		}
		RB_SimpleSurfaceSetup(surf);
		if (surf->jointCache) {
			renderProgManager.BindShader_CstDebugColorSkinned();
		} else {
			renderProgManager.BindShader_Color();
		}
		RB_DrawElementsWithCounters(surf);
	}

	//
	// now blend in edges that cast silhouettes
	//
	GL_Color(0.5f, 0.0f, 0.0f, 1.0f);

	GLuint iBuffer = 0;
	qglGenBuffersARB(1, &iBuffer);
	qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, iBuffer);

	GLuint cpyBuffer = 0;
	qglGenBuffersARB(1, &cpyBuffer);
	qglBindBufferARB(GL_COPY_WRITE_BUFFER, cpyBuffer);

	const int localIndicesGranularityBytes = 512 * 1024;
	const int localIndicesGranularity = localIndicesGranularityBytes / sizeof(triIndex_t);
	idList<triIndex_t> localIndices(localIndicesGranularity);

	for (const viewLight_t * vLight = backEnd.viewDef->viewLights; vLight; vLight = vLight->next) {
		for (int i = 0; i < 2; ++i) {
			// contains code from RB_StencilShadowPass (with changes)

			for (const drawSurf_t * drawSurf = i ? vLight->localShadows : vLight->globalShadows; drawSurf; drawSurf = (drawSurf_t *)drawSurf->nextOnLight) {
				if (drawSurf->scissorRect.IsEmpty()) {
					continue;	// !@# FIXME: find out why this is sometimes being hit!
								// temporarily jump over the scissor and draw so the gl error callback doesn't get hit
				}

				// make sure the shadow volume is done
				if (drawSurf->shadowVolumeState != SHADOWVOLUME_DONE) {
					assert(drawSurf->shadowVolumeState == SHADOWVOLUME_UNFINISHED || drawSurf->shadowVolumeState == SHADOWVOLUME_DONE);
					while (drawSurf->shadowVolumeState == SHADOWVOLUME_UNFINISHED) {
						Sys_Yield();
					}
				}

				if (drawSurf->numIndexes == 0) {
					continue;	// a job may have created an empty shadow volume
				}

				// get vertex buffer
				const vertCacheHandle_t vbHandle = drawSurf->shadowCache;
				idVertexBuffer * vertexBuffer;
				if (vertexCache.CacheIsStatic(vbHandle)) {
					vertexBuffer = &vertexCache.staticData.vertexBuffer;
				} else {
					const uint64 frameNum = (int)(vbHandle >> VERTCACHE_FRAME_SHIFT) & VERTCACHE_FRAME_MASK;
					if (frameNum != ((vertexCache.currentFrame - 1) & VERTCACHE_FRAME_MASK)) {
						continue;
					}
					vertexBuffer = &vertexCache.frameData[vertexCache.drawListNum].vertexBuffer;
				}
				const int vertOffset = (int)(vbHandle >> VERTCACHE_OFFSET_SHIFT) & VERTCACHE_OFFSET_MASK;

				// get index buffer
				const vertCacheHandle_t ibHandle = drawSurf->indexCache;
				idIndexBuffer * indexBuffer;
				if (vertexCache.CacheIsStatic(ibHandle)) {
					indexBuffer = &vertexCache.staticData.indexBuffer;
				} else {
					const uint64 frameNum = (int)(ibHandle >> VERTCACHE_FRAME_SHIFT) & VERTCACHE_FRAME_MASK;
					if (frameNum != ((vertexCache.currentFrame - 1) & VERTCACHE_FRAME_MASK)) {
						continue;
					}
					indexBuffer = &vertexCache.frameData[vertexCache.drawListNum].indexBuffer;
				}
				const uint64 indexOffset = (int)(ibHandle >> VERTCACHE_OFFSET_SHIFT) & VERTCACHE_OFFSET_MASK;

				// -----------------
				const int numShadowIndices = drawSurf->numIndexes;
				if (numShadowIndices == 0) {
					continue;
				}

				// read the index buffer while trying to keep it write-combined: copy it to another gl buffer and then read the copy
				qglBindBufferARB(GL_COPY_READ_BUFFER, (GLintptrARB)indexBuffer->GetAPIObject());
				qglBufferDataARB(GL_COPY_WRITE_BUFFER, numShadowIndices * sizeof(triIndex_t), NULL, GL_STREAM_READ);
				cst_qglCopyBufferSubDataARB(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, indexOffset, 0, numShadowIndices * sizeof(triIndex_t));

				triIndex_t * shadowIndices = (triIndex_t *)qglMapBufferARB(GL_COPY_WRITE_BUFFER, GL_READ_ONLY);
				if (!shadowIndices) {
					continue;
				}

				localIndices.SetNum(0);
				for (int j = 0; j < numShadowIndices; j += 3) {
					triIndex_t i1 = shadowIndices[j + 0];
					triIndex_t i2 = shadowIndices[j + 1];
					triIndex_t i3 = shadowIndices[j + 2];

					if ((i1 & 1) + (i2 & 1) + (i3 & 1) == 1) {
						if ((i1 & 1) + (i2 & 1) == 0) {
							localIndices.Append(i1);
							localIndices.Append(i2);
						} else if ((i1 & 1) + (i3 & 1) == 0) {
							localIndices.Append(i1);
							localIndices.Append(i3);
						}
					}
				}

				qglUnmapBufferARB(GL_COPY_WRITE_BUFFER);
				if (localIndices.Num() == 0) {
					continue;
				}

				// -----------------
				qglBindBufferARB(GL_ARRAY_BUFFER, (GLintptrARB)vertexBuffer->GetAPIObject());
				qglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, localIndices.Num() * sizeof(triIndex_t), localIndices.Ptr(), GL_STREAM_DRAW);

				RB_SimpleSurfaceSetup(drawSurf);
				if (drawSurf->jointCache) {
					renderProgManager.BindShader_ShadowDebugSkinned();
				} else {
					renderProgManager.BindShader_ShadowDebug();
				}

				idVec4 localLight(0.0f);
				R_GlobalPointToLocal(drawSurf->space->modelMatrix, vLight->globalLightOrigin, localLight.ToVec3());
				renderProgManager.SetUniformValue(RENDERPARM_LOCALLIGHTORIGIN, localLight.ToFloatPtr());

				if (drawSurf->jointCache) {
					idJointBuffer jointBuffer;
					if (!vertexCache.GetJointBuffer(drawSurf->jointCache, &jointBuffer)) {
						continue;
					}

					const GLintptrARB ubo = reinterpret_cast<GLintptrARB>(jointBuffer.GetAPIObject());
					qglBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo, jointBuffer.GetOffset(), jointBuffer.GetNumJoints() * sizeof(idJointMat));

					RB_Cst_SetupShadowVertSkinnedLayout();
				} else {
					RB_Cst_SetupShadowVertLayout();
				}

				renderProgManager.CommitUniforms();

				if (drawSurf->jointCache) {
					qglDrawElementsBaseVertex(GL_LINES, localIndices.Num(), GL_INDEX_TYPE, 0, vertOffset / sizeof(idShadowVertSkinned));
				} else {
					qglDrawElementsBaseVertex(GL_LINES, localIndices.Num(), GL_INDEX_TYPE, 0, vertOffset / sizeof(idShadowVert));
				}
			}
		}
	}

	backEnd.glState.currentVertexBuffer = 0; // mark as not set
	backEnd.glState.currentIndexBuffer = 0; // mark as not set

	qglBindBufferARB(GL_ARRAY_BUFFER, 0);
	qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);
	qglDeleteBuffersARB(1, &iBuffer);
	qglBindBufferARB(GL_COPY_READ_BUFFER, 0);
	qglBindBufferARB(GL_COPY_WRITE_BUFFER, 0);
	qglDeleteBuffersARB(1, &cpyBuffer);
	renderProgManager.Unbind();
	qglLineWidth(1);
	GL_State(GLS_DEFAULT);
	GL_Cull(CT_FRONT_SIDED);
}
//#modified-fva; END

/*
=====================
RB_ShowTris

Debugging tool
=====================
*/
//#modified-fva; BEGIN
/*
static void RB_ShowTris( drawSurf_t **drawSurfs, int numDrawSurfs ) {

	modelTrace_t mt;
	idVec3 end;

	if ( r_showTris.GetInteger() == 0 ) {
		return;
	}

	float color[4] = { 1, 1, 1, 1 };

	GL_PolygonOffset( -1.0f, -2.0f );

	switch ( r_showTris.GetInteger() ) {
		case 1: // only draw visible ones
			GL_State( GLS_DEPTHMASK | GLS_ALPHAMASK | GLS_POLYMODE_LINE | GLS_POLYGON_OFFSET );
			break;
		case 2:	// draw all front facing
		case 3: // draw all
			GL_State( GLS_DEPTHMASK | GLS_ALPHAMASK | GLS_POLYMODE_LINE | GLS_POLYGON_OFFSET | GLS_DEPTHFUNC_ALWAYS );
			break;
		case 4: // only draw visible ones with blended lines
			GL_State( GLS_DEPTHMASK | GLS_ALPHAMASK | GLS_POLYMODE_LINE | GLS_POLYGON_OFFSET | GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );
			color[3] = 0.4f;
			break;
	}

	if ( r_showTris.GetInteger() == 3 ) {
		GL_Cull( CT_TWO_SIDED );
	}

	GL_Color( color );
	renderProgManager.BindShader_Color();

	RB_RenderDrawSurfListWithFunction( drawSurfs, numDrawSurfs, RB_DrawElementsWithCounters );

	if ( r_showTris.GetInteger() == 3 ) {
		GL_Cull( CT_FRONT_SIDED );
	}
}
*/
static void RB_ShowTris(drawSurf_t **drawSurfs, int numDrawSurfs) {
	if (r_showTris.GetInteger() == 0) {
		return;
	}
	common->CstBrtRequestEnable();
	if (!common->CstBrtAreEnabled()) {
		return;
	}

	int width = r_debugLineWidth.GetInteger();
	if (width < 1) {
		width = 1;
	} else if (width > 10) {
		width = 10;
	}
	qglLineWidth(width);

	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	GL_PolygonOffset(-1.0f, -2.0f);

	switch (r_showTris.GetInteger()) {
	case 1: // only draw visible ones
		GL_State(GLS_DEPTHMASK | GLS_ALPHAMASK | GLS_POLYMODE_LINE | GLS_POLYGON_OFFSET);
		break;
	case 2:	// draw all front facing
	case 3: // draw all
		GL_State(GLS_DEPTHMASK | GLS_ALPHAMASK | GLS_POLYMODE_LINE | GLS_POLYGON_OFFSET | GLS_DEPTHFUNC_ALWAYS);
		break;
	case 4: // only draw visible ones with blended lines
		GL_State(GLS_DEPTHMASK | GLS_ALPHAMASK | GLS_POLYMODE_LINE | GLS_POLYGON_OFFSET | GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
		color[3] = 0.4f;
		break;
	}

	if (r_showTris.GetInteger() == 3) {
		GL_Cull(CT_TWO_SIDED);
	}

	GL_Color(color);
	for (int i = 0; i < numDrawSurfs; ++i) {
		const drawSurf_t * surf = drawSurfs[i];
		if (!surf) {
			continue;
		}
		if (!surf->space) {
			continue;
		}
		RB_SimpleSurfaceSetup(surf);
		if (surf->jointCache) {
			renderProgManager.BindShader_CstDebugColorSkinned();
		} else {
			renderProgManager.BindShader_Color();
		}
		RB_DrawElementsWithCounters(surf);
	}

	if (r_showTris.GetInteger() == 3) {
		GL_Cull(CT_FRONT_SIDED);
	}
	renderProgManager.Unbind();
	qglLineWidth(1);
	GL_State(GLS_DEFAULT);
}
//#modified-fva; END

/*
=====================
RB_ShowSurfaceInfo

Debugging tool
=====================
*/
//#modified-fva; BEGIN
/*
static void RB_ShowSurfaceInfo( drawSurf_t **drawSurfs, int numDrawSurfs ) {
	modelTrace_t mt;
	idVec3 start, end;

	if ( !r_showSurfaceInfo.GetBool() ) {
		return;
	}

	// start far enough away that we don't hit the player model
	start = tr.primaryView->renderView.vieworg + tr.primaryView->renderView.viewaxis[0] * 16;
	end = start + tr.primaryView->renderView.viewaxis[0] * 1000.0f;
	if ( !tr.primaryWorld->Trace( mt, start, end, 0.0f, false ) ) {
		return;
	}

	globalImages->BindNull();
	qglDisable( GL_TEXTURE_2D );

	GL_Color( 1, 1, 1 );

	static float scale = -1;
	static float bias = -2;

	GL_PolygonOffset( scale, bias );
	GL_State( GLS_DEPTHFUNC_ALWAYS | GLS_POLYMODE_LINE | GLS_POLYGON_OFFSET );

	idVec3	trans[3];
	float	matrix[16];

	// transform the object verts into global space
	R_AxisToModelMatrix( mt.entity->axis, mt.entity->origin, matrix );

	tr.primaryWorld->DrawText( mt.entity->hModel->Name(), mt.point + tr.primaryView->renderView.viewaxis[2] * 12,
		0.35f, colorRed, tr.primaryView->renderView.viewaxis );
	tr.primaryWorld->DrawText( mt.material->GetName(), mt.point,
		0.35f, colorBlue, tr.primaryView->renderView.viewaxis );
}
*/
static void RB_ShowSurfaceInfo(drawSurf_t **drawSurfs, int numDrawSurfs) {
	if (!r_showSurfaceInfo.GetBool()) {
		return;
	}
	common->CstBrtRequestEnable();
	if (!common->CstBrtAreEnabled()) {
		return;
	}

	// start far enough away that we don't hit the player model
	modelTrace_t mt;
	idVec3 start = tr.primaryView->renderView.vieworg + tr.primaryView->renderView.viewaxis[0] * 16;
	idVec3 end = start + tr.primaryView->renderView.viewaxis[0] * 1000.0f;
	if (!tr.primaryWorld->Trace(mt, start, end, 0.0f, false)) {
		return;
	}

	// transform the object verts into global space
	float matrix[16];
	R_AxisToModelMatrix(mt.entity->axis, mt.entity->origin, matrix);

	tr.primaryWorld->DrawText(mt.entity->hModel->Name(), mt.point + tr.primaryView->renderView.viewaxis[2] * 12,
		0.35f, colorRed, tr.primaryView->renderView.viewaxis);
	tr.primaryWorld->DrawText(mt.material->GetName(), mt.point,
		0.35f, colorBlue, tr.primaryView->renderView.viewaxis);
}
//#modified-fva; END

/*
=====================
RB_ShowViewEntitys

Debugging tool
=====================
*/
//#modified-fva; BEGIN
/*
static void RB_ShowViewEntitys( viewEntity_t *vModels ) {
	if ( !r_showViewEntitys.GetBool() ) {
		return;
	}
	if ( r_showViewEntitys.GetInteger() >= 2 ) {
		common->Printf( "view entities: " );
		for ( const viewEntity_t * vModel = vModels; vModel; vModel = vModel->next ) {
			if ( vModel->entityDef->IsDirectlyVisible() ) {
				common->Printf( "<%i> ", vModel->entityDef->index );
			} else {
				common->Printf( "%i ", vModel->entityDef->index );
			}
		}
		common->Printf( "\n" );
	}

	globalImages->BindNull();

	renderProgManager.BindShader_Color();

	GL_Color( 1, 1, 1 );
	GL_State( GLS_DEPTHFUNC_ALWAYS | GLS_POLYMODE_LINE );
	GL_Cull( CT_TWO_SIDED );

	for ( const viewEntity_t * vModel = vModels; vModel; vModel = vModel->next ) {
		idBounds	b;

		qglLoadMatrixf( vModel->modelViewMatrix );

		const idRenderEntityLocal * edef = vModel->entityDef;
		if ( !edef ) {
			continue;
		}



		// draw the model bounds in white if directly visible,
		// or, blue if it is only-for-sahdow
		idVec4	color;
		if ( edef->IsDirectlyVisible() ) {
			color.Set( 1, 1, 1, 1 );
		} else {
			color.Set( 0, 0, 1, 1 );
		}
		GL_Color( color[0], color[1], color[2] );
		RB_DrawBounds( edef->localReferenceBounds );

		// transform the upper bounds corner into global space
		if ( r_showViewEntitys.GetInteger() >= 2 ) {
			idVec3 corner;
			R_LocalPointToGlobal( vModel->modelMatrix, edef->localReferenceBounds[1], corner );

			tr.primaryWorld->DrawText(
				va( "%i:%s", edef->index, edef->parms.hModel->Name() ),
				corner,
				0.25f, color,
				tr.primaryView->renderView.viewaxis );
		}

		// draw the actual bounds in yellow if different
		if ( r_showViewEntitys.GetInteger() >= 3 ) {
			GL_Color( 1, 1, 0 );
			// FIXME: cannot instantiate a dynamic model from the renderer back-end
			idRenderModel *model = R_EntityDefDynamicModel( vModel->entityDef );
			if ( !model ) {
				continue;	// particles won't instantiate without a current view
			}
			b = model->Bounds( &vModel->entityDef->parms );
			if ( b != vModel->entityDef->localReferenceBounds ) {
				RB_DrawBounds( b );
			}
		}
	}
}
*/
static void RB_ShowViewEntitys(viewEntity_t *vModels) {
	int showViewEntities = r_showViewEntitys.GetInteger();

	if (showViewEntities <= 0) {
		return;
	}
	common->CstBrtRequestEnable();
	if (!common->CstBrtAreEnabled()) {
		return;
	}

	if (showViewEntities == 2) {
		common->Printf("view entities: ");
		for (const viewEntity_t * vModel = vModels; vModel; vModel = vModel->next) {
			if (!vModel->entityDef) {
				continue;
			}
			if (vModel->entityDef->IsDirectlyVisible()) {
				common->Printf("<%i> ", vModel->entityDef->index);
			} else {
				common->Printf("%i ", vModel->entityDef->index);
			}
		}
		common->Printf("\n");
		return;
	}

	int width = r_debugLineWidth.GetInteger();
	if (width < 1) {
		width = 1;
	} else if (width > 10) {
		width = 10;
	}
	qglLineWidth(width);

	GL_State(GLS_DEPTHFUNC_ALWAYS);

	renderProgManager.BindShader_CstDebugPosColor();

	GLuint vBuffer = 0;
	qglGenBuffersARB(1, &vBuffer);
	qglBindBufferARB(GL_ARRAY_BUFFER, vBuffer);
	backEnd.glState.currentVertexBuffer = 0; // mark as not set

	for (const viewEntity_t * vModel = vModels; vModel; vModel = vModel->next) {
		const idRenderEntityLocal * edef = vModel->entityDef;
		if (!edef) {
			continue;
		}

		const int maxBuffVerts = (showViewEntities >= 3) ? 24 * 2 : 24;

		qglBufferDataARB(GL_ARRAY_BUFFER, maxBuffVerts * sizeof(CstVertPosColor), NULL, GL_STREAM_DRAW);
		CstVertPosColor * vertsPosColor = (CstVertPosColor *)qglMapBufferARB(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		if (!vertsPosColor) {
			qglBindBufferARB(GL_ARRAY_BUFFER, 0);
			qglDeleteBuffersARB(1, &vBuffer);
			renderProgManager.Unbind();
			qglLineWidth(1);
			GL_State(GLS_DEFAULT);
			return;
		}

		// draw the model bounds in white if directly visible or blue if it is only-for-shadow
		idVec4	color;
		if (edef->IsDirectlyVisible()) {
			color.Set(1.0f, 1.0f, 1.0f, 1.0f);
		} else {
			color.Set(0.0f, 0.0f, 1.0f, 1.0f);
		}

		int vertsBufferOffset = 0;
		vertsBufferOffset = RB_Cst_AddBoundsToBuffer(edef->localReferenceBounds, vertsPosColor, vertsBufferOffset, color);

		// transform the upper bounds corner into global space
		if (showViewEntities >= 4) {
			idVec3 corner;
			R_LocalPointToGlobal(vModel->modelMatrix, edef->localReferenceBounds[1], corner);
			tr.primaryWorld->DrawText(va("%i:%s", edef->index, edef->parms.hModel->Name()), corner, 0.25f, color, tr.primaryView->renderView.viewaxis);
		}

		// draw the actual bounds in yellow if different
		if (showViewEntities >= 3) {
			idRenderModel * model = edef->dynamicModel;
			if (model) {
				idBounds b = model->Bounds(&edef->parms);
				if (b != edef->localReferenceBounds) {
					color.Set(1.0f, 1.0f, 0.0f, 1.0f);
					vertsBufferOffset = RB_Cst_AddBoundsToBuffer(b, vertsPosColor, vertsBufferOffset, color);
				}
			}
		}

		qglUnmapBufferARB(GL_ARRAY_BUFFER);
		RB_Cst_SetupVertPosColorLayout();

		RB_Cst_SimpleViewEntitySetup(*vModel);
		renderProgManager.CommitUniforms();

		qglDrawArrays(GL_LINES, 0, vertsBufferOffset);
	}

	qglBindBufferARB(GL_ARRAY_BUFFER, 0);
	qglDeleteBuffersARB(1, &vBuffer);
	renderProgManager.Unbind();
	qglLineWidth(1);
	GL_State(GLS_DEFAULT);
}
//#modified-fva; END

/*
=====================
RB_ShowTexturePolarity

Shade triangle red if they have a positive texture area
green if they have a negative texture area, or blue if degenerate area
=====================
*/
//#modified-fva; BEGIN
/*
static void RB_ShowTexturePolarity( drawSurf_t **drawSurfs, int numDrawSurfs ) {
	int		i, j;
	drawSurf_t	*drawSurf;
	const srfTriangles_t	*tri;

	if ( !r_showTexturePolarity.GetBool() ) {
		return;
	}
	globalImages->BindNull();

	GL_State( GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );

	GL_Color( 1, 1, 1 );

	for ( i = 0; i < numDrawSurfs; i++ ) {
		drawSurf = drawSurfs[i];
		tri = drawSurf->frontEndGeo;
		if ( !tri->verts ) {
			continue;
		}

		RB_SimpleSurfaceSetup( drawSurf );

		qglBegin( GL_TRIANGLES );
		for ( j = 0; j < tri->numIndexes; j+=3 ) {
			idDrawVert	*a, *b, *c;
			float		d0[5], d1[5];
			float		area;

			a = tri->verts + tri->indexes[j];
			b = tri->verts + tri->indexes[j+1];
			c = tri->verts + tri->indexes[j+2];

			const idVec2 aST = a->GetTexCoord();
			const idVec2 bST = b->GetTexCoord();
			const idVec2 cST = c->GetTexCoord();

			d0[3] = bST[0] - aST[0];
			d0[4] = bST[1] - aST[1];

			d1[3] = cST[0] - aST[0];
			d1[4] = cST[1] - aST[1];

			area = d0[3] * d1[4] - d0[4] * d1[3];

			if ( idMath::Fabs( area ) < 0.0001 ) {
				GL_Color( 0, 0, 1, 0.5 );
			} else  if ( area < 0 ) {
				GL_Color( 1, 0, 0, 0.5 );
			} else {
				GL_Color( 0, 1, 0, 0.5 );
			}
			qglVertex3fv( a->xyz.ToFloatPtr() );
			qglVertex3fv( b->xyz.ToFloatPtr() );
			qglVertex3fv( c->xyz.ToFloatPtr() );
		}
		qglEnd();
	}

	GL_State( GLS_DEFAULT );
}
*/
static void RB_ShowTexturePolarity(drawSurf_t **drawSurfs, int numDrawSurfs) {
	if (!r_showTexturePolarity.GetBool()) {
		return;
	}
	common->CstBrtRequestEnable();
	if (!common->CstBrtAreEnabled()) {
		return;
	}

	GL_State(GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);

	renderProgManager.BindShader_CstDebugPosColor();

	GLuint vBuffer = 0;
	qglGenBuffersARB(1, &vBuffer);
	qglBindBufferARB(GL_ARRAY_BUFFER, vBuffer);
	backEnd.glState.currentVertexBuffer = 0; // mark as not set

	for (int i = 0; i < numDrawSurfs; ++i) {
		drawSurf_t * drawSurf = drawSurfs[i];
		const srfTriangles_t * tri = drawSurf->frontEndGeo;
		if (!tri || !tri->verts) {
			continue;
		}
		if (tri->numVerts == 0 || tri->numIndexes == 0) {
			continue;
		}

		const int numBuffVerts = tri->numIndexes; // (tri->numIndexes / 3) * 3
		qglBufferDataARB(GL_ARRAY_BUFFER, numBuffVerts * sizeof(CstVertPosColor), NULL, GL_STREAM_DRAW);
		CstVertPosColor * vertsPosColor = (CstVertPosColor *)qglMapBufferARB(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		if (!vertsPosColor) {
			qglBindBufferARB(GL_ARRAY_BUFFER, 0);
			qglDeleteBuffersARB(1, &vBuffer);
			renderProgManager.Unbind();
			GL_State(GLS_DEFAULT);
			return;
		}

		int vertsPosColorIndex = 0;
		for (int j = 0; j < tri->numIndexes; j += 3) {
			idDrawVert	*a, *b, *c;
			float		d0[5], d1[5];
			float		area;

			a = tri->verts + tri->indexes[j];
			b = tri->verts + tri->indexes[j + 1];
			c = tri->verts + tri->indexes[j + 2];

			const idVec2 aST = a->GetTexCoord();
			const idVec2 bST = b->GetTexCoord();
			const idVec2 cST = c->GetTexCoord();

			d0[3] = bST[0] - aST[0];
			d0[4] = bST[1] - aST[1];

			d1[3] = cST[0] - aST[0];
			d1[4] = cST[1] - aST[1];

			area = d0[3] * d1[4] - d0[4] * d1[3];

			idVec4 color;
			if (idMath::Fabs(area) < 0.0001) {
				color.Set(0.0f, 0.0f, 1.0f, 0.5f);
			} else if (area < 0) {
				color.Set(1.0f, 0.0f, 0.0f, 0.5f);
			} else {
				color.Set(0.0f, 1.0f, 0.0f, 0.5f);
			}

			CstVertPosColor v1, v2, v3;
			v1.position = a->xyz;
			v2.position = b->xyz;
			v3.position = c->xyz;
			v1.color = v2.color = v3.color = color;

			vertsPosColor[vertsPosColorIndex++] = v1;
			vertsPosColor[vertsPosColorIndex++] = v2;
			vertsPosColor[vertsPosColorIndex++] = v3;
		}

		qglUnmapBufferARB(GL_ARRAY_BUFFER);
		RB_Cst_SetupVertPosColorLayout();

		RB_SimpleSurfaceSetup(drawSurf);
		renderProgManager.CommitUniforms();

		qglDrawArrays(GL_TRIANGLES, 0, numBuffVerts);
	}

	qglBindBufferARB(GL_ARRAY_BUFFER, 0);
	qglDeleteBuffersARB(1, &vBuffer);
	renderProgManager.Unbind();
	GL_State(GLS_DEFAULT);
}
//#modified-fva; END

/*
=====================
RB_ShowUnsmoothedTangents

Shade materials that are using unsmoothed tangents
=====================
*/
//#modified-fva; BEGIN
/*
static void RB_ShowUnsmoothedTangents( drawSurf_t **drawSurfs, int numDrawSurfs ) {
	int		i, j;
	drawSurf_t	*drawSurf;
	const srfTriangles_t	*tri;

	if ( !r_showUnsmoothedTangents.GetBool() ) {
		return;
	}
	globalImages->BindNull();

	GL_State( GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );

	GL_Color( 0, 1, 0, 0.5 );

	for ( i = 0; i < numDrawSurfs; i++ ) {
		drawSurf = drawSurfs[i];

		if ( !drawSurf->material->UseUnsmoothedTangents() ) {
			continue;
		}

		RB_SimpleSurfaceSetup( drawSurf );

		tri = drawSurf->frontEndGeo;
		qglBegin( GL_TRIANGLES );
		for ( j = 0; j < tri->numIndexes; j+=3 ) {
			idDrawVert	*a, *b, *c;

			a = tri->verts + tri->indexes[j];
			b = tri->verts + tri->indexes[j+1];
			c = tri->verts + tri->indexes[j+2];

			qglVertex3fv( a->xyz.ToFloatPtr() );
			qglVertex3fv( b->xyz.ToFloatPtr() );
			qglVertex3fv( c->xyz.ToFloatPtr() );
		}
		qglEnd();
	}

	GL_State( GLS_DEFAULT );
}
*/
static void RB_ShowUnsmoothedTangents(drawSurf_t **drawSurfs, int numDrawSurfs) {
	if (!r_showUnsmoothedTangents.GetBool()) {
		return;
	}
	common->CstBrtRequestEnable();
	if (!common->CstBrtAreEnabled()) {
		return;
	}

	GL_State(GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);

	renderProgManager.BindShader_CstDebugPosColor();

	GLuint vBuffer = 0;
	qglGenBuffersARB(1, &vBuffer);
	qglBindBufferARB(GL_ARRAY_BUFFER, vBuffer);
	backEnd.glState.currentVertexBuffer = 0; // mark as not set

	for (int i = 0; i < numDrawSurfs; ++i) {
		drawSurf_t * drawSurf = drawSurfs[i];

		if (!drawSurf->material->UseUnsmoothedTangents()) {
			continue;
		}

		const srfTriangles_t * tri = drawSurf->frontEndGeo;
		if (!tri || !tri->verts) {
			continue;
		}
		if (tri->numVerts == 0 || tri->numIndexes == 0) {
			continue;
		}

		const int numBuffVerts = tri->numIndexes; // (tri->numIndexes / 3) * 3
		qglBufferDataARB(GL_ARRAY_BUFFER, numBuffVerts * sizeof(CstVertPosColor), NULL, GL_STREAM_DRAW);
		CstVertPosColor * vertsPosColor = (CstVertPosColor *)qglMapBufferARB(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		if (!vertsPosColor) {
			qglBindBufferARB(GL_ARRAY_BUFFER, 0);
			qglDeleteBuffersARB(1, &vBuffer);
			renderProgManager.Unbind();
			GL_State(GLS_DEFAULT);
			return;
		}

		int vertsPosColorIndex = 0;
		for (int j = 0; j < tri->numIndexes; j += 3) {
			idDrawVert	*a, *b, *c;
			a = tri->verts + tri->indexes[j];
			b = tri->verts + tri->indexes[j + 1];
			c = tri->verts + tri->indexes[j + 2];

			CstVertPosColor v1, v2, v3;
			v1.position = a->xyz;
			v2.position = b->xyz;
			v3.position = c->xyz;
			v1.color = v2.color = v3.color = idVec4(0.0f, 1.0f, 0.0f, 0.5f);

			vertsPosColor[vertsPosColorIndex++] = v1;
			vertsPosColor[vertsPosColorIndex++] = v2;
			vertsPosColor[vertsPosColorIndex++] = v3;
		}

		qglUnmapBufferARB(GL_ARRAY_BUFFER);
		RB_Cst_SetupVertPosColorLayout();

		RB_SimpleSurfaceSetup(drawSurf);
		renderProgManager.CommitUniforms();

		qglDrawArrays(GL_TRIANGLES, 0, numBuffVerts);
	}

	qglBindBufferARB(GL_ARRAY_BUFFER, 0);
	qglDeleteBuffersARB(1, &vBuffer);
	renderProgManager.Unbind();
	GL_State(GLS_DEFAULT);
}
//#modified-fva; END

/*
=====================
RB_ShowTangentSpace

Shade a triangle by the RGB colors of its tangent space
1 = tangents[0]
2 = tangents[1]
3 = normal
=====================
*/
//#modified-fva; BEGIN
/*
static void RB_ShowTangentSpace( drawSurf_t **drawSurfs, int numDrawSurfs ) {
	int		i, j;
	drawSurf_t	*drawSurf;
	const srfTriangles_t	*tri;

	if ( !r_showTangentSpace.GetInteger() ) {
		return;
	}
	globalImages->BindNull();

	GL_State( GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );

	for ( i = 0; i < numDrawSurfs; i++ ) {
		drawSurf = drawSurfs[i];

		RB_SimpleSurfaceSetup( drawSurf );

		tri = drawSurf->frontEndGeo;
		if ( !tri->verts ) {
			continue;
		}
		qglBegin( GL_TRIANGLES );
		for ( j = 0; j < tri->numIndexes; j++ ) {
			const idDrawVert *v;

			v = &tri->verts[tri->indexes[j]];

			if ( r_showTangentSpace.GetInteger() == 1 ) {
				const idVec3 vertexTangent = v->GetTangent();
				GL_Color( 0.5 + 0.5 * vertexTangent[0],  0.5 + 0.5 * vertexTangent[1],
					0.5 + 0.5 * vertexTangent[2], 0.5 );
			} else if ( r_showTangentSpace.GetInteger() == 2 ) {
				const idVec3 vertexBiTangent = v->GetBiTangent();
				GL_Color( 0.5 + 0.5 *vertexBiTangent[0],  0.5 + 0.5 * vertexBiTangent[1],
					0.5 + 0.5 * vertexBiTangent[2], 0.5 );
			} else {
				const idVec3 vertexNormal = v->GetNormal();
				GL_Color( 0.5 + 0.5 * vertexNormal[0],  0.5 + 0.5 * vertexNormal[1],
					0.5 + 0.5 * vertexNormal[2], 0.5 );
			}
			qglVertex3fv( v->xyz.ToFloatPtr() );
		}
		qglEnd();
	}

	GL_State( GLS_DEFAULT );
}
*/
static void RB_ShowTangentSpace(drawSurf_t **drawSurfs, int numDrawSurfs) {
	if (!r_showTangentSpace.GetInteger()) {
		return;
	}
	common->CstBrtRequestEnable();
	if (!common->CstBrtAreEnabled()) {
		return;
	}

	GL_State(GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);

	renderProgManager.BindShader_CstDebugPosColor();

	GLuint vBuffer = 0;
	qglGenBuffersARB(1, &vBuffer);
	qglBindBufferARB(GL_ARRAY_BUFFER, vBuffer);
	backEnd.glState.currentVertexBuffer = 0; // mark as not set

	for (int i = 0; i < numDrawSurfs; ++i) {
		drawSurf_t * drawSurf = drawSurfs[i];
		const srfTriangles_t * tri = drawSurf->frontEndGeo;
		if (!tri || !tri->verts) {
			continue;
		}
		if (tri->numVerts == 0 || tri->numIndexes == 0) {
			continue;
		}

		const int numBuffVerts = tri->numIndexes; // (tri->numIndexes / 3) * 3
		qglBufferDataARB(GL_ARRAY_BUFFER, numBuffVerts * sizeof(CstVertPosColor), NULL, GL_STREAM_DRAW);
		CstVertPosColor * vertsPosColor = (CstVertPosColor *)qglMapBufferARB(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		if (!vertsPosColor) {
			qglBindBufferARB(GL_ARRAY_BUFFER, 0);
			qglDeleteBuffersARB(1, &vBuffer);
			renderProgManager.Unbind();
			GL_State(GLS_DEFAULT);
			return;
		}

		int vertsPosColorIndex = 0;
		for (int j = 0; j < tri->numIndexes; ++j) {
			const idDrawVert * v = &tri->verts[tri->indexes[j]];

			CstVertPosColor vPosColor;
			vPosColor.position = v->xyz;
			if (r_showTangentSpace.GetInteger() == 1) {
				const idVec3 vertexTangent = v->GetTangent();
				vPosColor.color.Set(0.5f + 0.5f * vertexTangent[0], 0.5f + 0.5f * vertexTangent[1], 0.5f + 0.5f * vertexTangent[2], 0.5f);
			}
			else if (r_showTangentSpace.GetInteger() == 2) {
				const idVec3 vertexBiTangent = v->GetBiTangent();
				vPosColor.color.Set(0.5f + 0.5f * vertexBiTangent[0], 0.5f + 0.5f * vertexBiTangent[1], 0.5f + 0.5f * vertexBiTangent[2], 0.5f);
			} else {
				const idVec3 vertexNormal = v->GetNormal();
				vPosColor.color.Set(0.5f + 0.5f * vertexNormal[0], 0.5f + 0.5f * vertexNormal[1], 0.5f + 0.5f * vertexNormal[2], 0.5f);
			}
			vertsPosColor[vertsPosColorIndex++] = vPosColor;
		}

		qglUnmapBufferARB(GL_ARRAY_BUFFER);
		RB_Cst_SetupVertPosColorLayout();

		RB_SimpleSurfaceSetup(drawSurf);
		renderProgManager.CommitUniforms();

		qglDrawArrays(GL_TRIANGLES, 0, numBuffVerts);
	}

	qglBindBufferARB(GL_ARRAY_BUFFER, 0);
	qglDeleteBuffersARB(1, &vBuffer);
	renderProgManager.Unbind();
	GL_State(GLS_DEFAULT);
}
//#modified-fva; END

/*
=====================
RB_ShowVertexColor

Draw each triangle with the solid vertex colors
=====================
*/
//#modified-fva; BEGIN
/*
static void RB_ShowVertexColor( drawSurf_t **drawSurfs, int numDrawSurfs ) {
	int		i, j;
	drawSurf_t	*drawSurf;
	const srfTriangles_t	*tri;

	if ( !r_showVertexColor.GetBool() ) {
		return;
	}
	globalImages->BindNull();

	GL_State( GLS_DEPTHFUNC_LESS );

	for ( i = 0; i < numDrawSurfs; i++ ) {
		drawSurf = drawSurfs[i];

		RB_SimpleSurfaceSetup( drawSurf );

		tri = drawSurf->frontEndGeo;
		if ( !tri->verts ) {
			continue;
		}
		qglBegin( GL_TRIANGLES );
		for ( j = 0; j < tri->numIndexes; j++ ) {
			const idDrawVert *v;

			v = &tri->verts[tri->indexes[j]];
			qglColor4ubv( v->color );
			qglVertex3fv( v->xyz.ToFloatPtr() );
		}
		qglEnd();
	}

	GL_State( GLS_DEFAULT );
}
*/
static void RB_ShowVertexColor(drawSurf_t **drawSurfs, int numDrawSurfs) {
	if (!r_showVertexColor.GetBool()) {
		return;
	}
	common->CstBrtRequestEnable();
	if (!common->CstBrtAreEnabled()) {
		return;
	}

	GL_State(GLS_DEPTHFUNC_LESS);

	renderProgManager.BindShader_CstDebugPosColor();

	GLuint vBuffer = 0;
	qglGenBuffersARB(1, &vBuffer);
	qglBindBufferARB(GL_ARRAY_BUFFER, vBuffer);
	backEnd.glState.currentVertexBuffer = 0; // mark as not set

	for (int i = 0; i < numDrawSurfs; ++i) {
		drawSurf_t * drawSurf = drawSurfs[i];
		const srfTriangles_t * tri = drawSurf->frontEndGeo;
		if (!tri || !tri->verts) {
			continue;
		}
		if (tri->numVerts == 0 || tri->numIndexes == 0) {
			continue;
		}

		const int numBuffVerts = tri->numIndexes; // (tri->numIndexes / 3) * 3
		qglBufferDataARB(GL_ARRAY_BUFFER, numBuffVerts * sizeof(CstVertPosColor), NULL, GL_STREAM_DRAW);
		CstVertPosColor * vertsPosColor = (CstVertPosColor *)qglMapBufferARB(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		if (!vertsPosColor) {
			qglBindBufferARB(GL_ARRAY_BUFFER, 0);
			qglDeleteBuffersARB(1, &vBuffer);
			renderProgManager.Unbind();
			GL_State(GLS_DEFAULT);
			return;
		}

		int vertsPosColorIndex = 0;
		for (int j = 0; j < tri->numIndexes; ++j) {
			const idDrawVert *v = &tri->verts[tri->indexes[j]];

			const float inv255 = 1.0f / 255.0f;
			CstVertPosColor vPosColor;
			vPosColor.position = v->xyz;
			vPosColor.color.Set(v->color[0] * inv255, v->color[1] * inv255, v->color[2] * inv255, v->color[3] * inv255);

			vertsPosColor[vertsPosColorIndex++] = vPosColor;
		}

		qglUnmapBufferARB(GL_ARRAY_BUFFER);
		RB_Cst_SetupVertPosColorLayout();

		RB_SimpleSurfaceSetup(drawSurf);
		renderProgManager.CommitUniforms();

		qglDrawArrays(GL_TRIANGLES, 0, numBuffVerts);
	}

	qglBindBufferARB(GL_ARRAY_BUFFER, 0);
	qglDeleteBuffersARB(1, &vBuffer);
	renderProgManager.Unbind();
	GL_State(GLS_DEFAULT);
}
//#modified-fva; END

/*
=====================
RB_ShowNormals

Debugging tool
=====================
*/
//#modified-fva; BEGIN
/*
static void RB_ShowNormals( drawSurf_t **drawSurfs, int numDrawSurfs ) {
	int			i, j;
	drawSurf_t	*drawSurf;
	idVec3		end;
	const srfTriangles_t	*tri;
	float		size;
	bool		showNumbers;
	idVec3		pos;

	if ( r_showNormals.GetFloat() == 0.0f ) {
		return;
	}

	globalImages->BindNull();

	if ( !r_debugLineDepthTest.GetBool() ) {
		GL_State( GLS_POLYMODE_LINE | GLS_DEPTHFUNC_ALWAYS );
	} else {
		GL_State( GLS_POLYMODE_LINE );
	}

	size = r_showNormals.GetFloat();
	if ( size < 0.0f ) {
		size = -size;
		showNumbers = true;
	} else {
		showNumbers = false;
	}

	for ( i = 0; i < numDrawSurfs; i++ ) {
		drawSurf = drawSurfs[i];

		RB_SimpleSurfaceSetup( drawSurf );

		tri = drawSurf->frontEndGeo;
		if ( !tri->verts ) {
			continue;
		}

		qglBegin( GL_LINES );
		for ( j = 0; j < tri->numVerts; j++ ) {
			const idVec3 normal = tri->verts[j].GetNormal();
			const idVec3 tangent = tri->verts[j].GetTangent();
			const idVec3 bitangent = tri->verts[j].GetBiTangent();
			GL_Color( 0, 0, 1 );
			qglVertex3fv( tri->verts[j].xyz.ToFloatPtr() );
			VectorMA( tri->verts[j].xyz, size, normal, end );
			qglVertex3fv( end.ToFloatPtr() );

			GL_Color( 1, 0, 0 );
			qglVertex3fv( tri->verts[j].xyz.ToFloatPtr() );
			VectorMA( tri->verts[j].xyz, size, tangent, end );
			qglVertex3fv( end.ToFloatPtr() );

			GL_Color( 0, 1, 0 );
			qglVertex3fv( tri->verts[j].xyz.ToFloatPtr() );
			VectorMA( tri->verts[j].xyz, size, bitangent, end );
			qglVertex3fv( end.ToFloatPtr() );
		}
		qglEnd();
	}

	if ( showNumbers ) {
		RB_SimpleWorldSetup();
		for ( i = 0; i < numDrawSurfs; i++ ) {
			drawSurf = drawSurfs[i];
			tri = drawSurf->frontEndGeo;
			if ( !tri->verts ) {
				continue;
			}

			for ( j = 0; j < tri->numVerts; j++ ) {
				const idVec3 normal = tri->verts[j].GetNormal();
				const idVec3 tangent = tri->verts[j].GetTangent();
				R_LocalPointToGlobal( drawSurf->space->modelMatrix, tri->verts[j].xyz + tangent + normal * 0.2f, pos );
				RB_DrawText( va( "%d", j ), pos, 0.01f, colorWhite, backEnd.viewDef->renderView.viewaxis, 1 );
			}

			for ( j = 0; j < tri->numIndexes; j += 3 ) {
				const idVec3 normal = tri->verts[ tri->indexes[ j + 0 ] ].GetNormal();
				R_LocalPointToGlobal( drawSurf->space->modelMatrix, ( tri->verts[ tri->indexes[ j + 0 ] ].xyz + tri->verts[ tri->indexes[ j + 1 ] ].xyz + tri->verts[ tri->indexes[ j + 2 ] ].xyz ) * ( 1.0f / 3.0f ) + normal * 0.2f, pos );
				RB_DrawText( va( "%d", j / 3 ), pos, 0.01f, colorCyan, backEnd.viewDef->renderView.viewaxis, 1 );
			}
		}
	}
}
*/
static void RB_ShowNormals(drawSurf_t **drawSurfs, int numDrawSurfs) {
	if (r_showNormals.GetFloat() == 0.0f) {
		return;
	}
	common->CstBrtRequestEnable();
	if (!common->CstBrtAreEnabled()) {
		return;
	}

	int width = r_debugLineWidth.GetInteger();
	if (width < 1) {
		width = 1;
	} else if (width > 10) {
		width = 10;
	}
	qglLineWidth(width);

	if (!r_debugLineDepthTest.GetBool()) {
		GL_State(GLS_DEPTHFUNC_ALWAYS);
	} else {
		GL_State(GLS_DEFAULT);
	}

	bool showNumbers = false;
	float size = r_showNormals.GetFloat();
	if (size < 0.0f) {
		size = -size;
		showNumbers = true;
	}

	renderProgManager.BindShader_CstDebugPosColor();

	GLuint vBuffer = 0;
	qglGenBuffersARB(1, &vBuffer);
	qglBindBufferARB(GL_ARRAY_BUFFER, vBuffer);
	backEnd.glState.currentVertexBuffer = 0; // mark as not set

	for (int i = 0; i < numDrawSurfs; ++i) {
		drawSurf_t * drawSurf = drawSurfs[i];

		const srfTriangles_t * tri = drawSurf->frontEndGeo;
		if (!tri || !tri->verts) {
			continue;
		}
		if (tri->numVerts == 0 || tri->numIndexes == 0) {
			continue;
		}

		const int numBuffVerts = tri->numVerts * 2 * 3;
		qglBufferDataARB(GL_ARRAY_BUFFER, numBuffVerts * sizeof(CstVertPosColor), NULL, GL_STREAM_DRAW);
		CstVertPosColor * vertsPosColor = (CstVertPosColor *)qglMapBufferARB(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		if (!vertsPosColor) {
			qglBindBufferARB(GL_ARRAY_BUFFER, 0);
			qglDeleteBuffersARB(1, &vBuffer);
			renderProgManager.Unbind();
			qglLineWidth(1);
			GL_State(GLS_DEFAULT);
			return;
		}

		int vertsPosColorIndex = 0;
		for (int j = 0; j < tri->numVerts; ++j) {
			const idVec3 normal = tri->verts[j].GetNormal();
			const idVec3 tangent = tri->verts[j].GetTangent();
			const idVec3 bitangent = tri->verts[j].GetBiTangent();

			CstVertPosColor v1, v2;
			idVec3 end;

			VectorMA(tri->verts[j].xyz, size, normal, end);
			v1.position = tri->verts[j].xyz;
			v2.position = end;
			v1.color = v2.color = idVec4(0.0f, 0.0f, 1.0f, 1.0f);
			vertsPosColor[vertsPosColorIndex++] = v1;
			vertsPosColor[vertsPosColorIndex++] = v2;

			VectorMA(tri->verts[j].xyz, size, tangent, end);
			// v1.position = same as above
			v2.position = end;
			v1.color = v2.color = idVec4(1.0f, 0.0f, 0.0f, 1.0f);
			vertsPosColor[vertsPosColorIndex++] = v1;
			vertsPosColor[vertsPosColorIndex++] = v2;

			VectorMA(tri->verts[j].xyz, size, bitangent, end);
			// v1.position = same as above
			v2.position = end;
			v1.color = v2.color = idVec4(0.0f, 1.0f, 0.0f, 1.0f);
			vertsPosColor[vertsPosColorIndex++] = v1;
			vertsPosColor[vertsPosColorIndex++] = v2;
		}

		qglUnmapBufferARB(GL_ARRAY_BUFFER);
		RB_Cst_SetupVertPosColorLayout();

		RB_SimpleSurfaceSetup(drawSurf);
		renderProgManager.CommitUniforms();

		qglDrawArrays(GL_LINES, 0, numBuffVerts);
	}

	// -------------
	if (showNumbers) {
		RB_SimpleWorldSetup();
		renderProgManager.CommitUniforms();

		const int maxBufferSize = 1 * 1024 * 1024;
		const int maxBufferVerts = maxBufferSize / sizeof(CstVertPosColor);

		for (int pass = 0; pass < 2; ++pass) {
			int numBufferVerts = 0;
			int firstSurf = 0;
			int firstTri = 0;
			bool breakThisPass = false;
			idStr str;
			idVec3 pos;

			// draw
			for (int surfLoopIndex = 0; surfLoopIndex < numDrawSurfs; ++surfLoopIndex) {
				if (breakThisPass) {
					break;
				}
				bool nullTri = false;
				bool drawNow = false;

				drawSurf_t * _surf = drawSurfs[surfLoopIndex];
				const srfTriangles_t * _tri = _surf->frontEndGeo;
				if (!_tri || !_tri->verts) {
					if (surfLoopIndex < numDrawSurfs - 1 || numBufferVerts == 0) {
						continue;
					}
					nullTri = true;
					drawNow = true;
				}
				const int triLoopMax = (pass == 0) ? (nullTri ? 0 : _tri->numVerts) : (nullTri ? 0 : _tri->numIndexes);
				const int triLoopInc = (pass == 0) ? 1 : 3;
				for (int triLoopIndex = 0; triLoopIndex < triLoopMax || nullTri; triLoopIndex += triLoopInc) {
					if (breakThisPass) {
						break;
					}
					if (pass == 0) {
						str.Format("%d", triLoopIndex);
					} else {
						str.Format("%d", triLoopIndex / 3);
					}
					int numVertsText = nullTri ? 0 : RB_Cst_DebugText_GetNumVerts(str);
					if (numVertsText > maxBufferVerts) {
						// string is too large; skip it and skip all strings after it
						breakThisPass = true;
						if (numBufferVerts == 0) {
							break;
						}
						triLoopIndex -= triLoopInc;
						numVertsText = 0;
						drawNow = true;
					}
					if (numBufferVerts + numVertsText > maxBufferVerts) {
						triLoopIndex -= triLoopInc; // so the current string goes on the next buffer
						drawNow = true;
					} else {
						numBufferVerts += numVertsText;
					}
					if (numBufferVerts > 0 && (drawNow || (surfLoopIndex == numDrawSurfs - 1 && triLoopIndex == triLoopMax - triLoopInc))) {
						qglBufferDataARB(GL_ARRAY_BUFFER, numBufferVerts * sizeof(CstVertPosColor), NULL, GL_STREAM_DRAW);
						CstVertPosColor * vertsPosColor = (CstVertPosColor *)qglMapBufferARB(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
						if (!vertsPosColor) {
							qglBindBufferARB(GL_ARRAY_BUFFER, 0);
							qglDeleteBuffersARB(1, &vBuffer);
							renderProgManager.Unbind();
							qglLineWidth(1);
							GL_State(GLS_DEFAULT);
							return;
						}
						int vertsBufferOffet = 0;
						for (int i = firstSurf; i <= surfLoopIndex; ++i) {
							drawSurf_t * surf = drawSurfs[i];
							const srfTriangles_t * tri = surf->frontEndGeo;
							if (!tri || !tri->verts) {
								continue;
							}
							const int j_ini = (i == firstSurf) ? firstTri : 0;
							const int j_max = (i == surfLoopIndex) ? triLoopIndex + triLoopInc : ((pass == 0) ? tri->numVerts : tri->numIndexes);
							for (int j = j_ini; j < j_max; j += triLoopInc) {
								if (pass == 0) {
									const idVec3 normal = tri->verts[j].GetNormal();
									const idVec3 tangent = tri->verts[j].GetTangent();
									R_LocalPointToGlobal(surf->space->modelMatrix, tri->verts[j].xyz + tangent + normal * 0.2f, pos);
									str.Format("%d", j);
									vertsBufferOffet += RB_Cst_DebugText_AddVertsToBuffer(vertsPosColor, vertsBufferOffet, str, pos, 0.01f, colorWhite, backEnd.viewDef->renderView.viewaxis, 1);
								} else {
									const idVec3 normal = tri->verts[tri->indexes[j + 0]].GetNormal();
									R_LocalPointToGlobal(surf->space->modelMatrix, (tri->verts[tri->indexes[j + 0]].xyz + tri->verts[tri->indexes[j + 1]].xyz + tri->verts[tri->indexes[j + 2]].xyz) * (1.0f / 3.0f) + normal * 0.2f, pos);
									str.Format("%d", j / 3);
									vertsBufferOffet += RB_Cst_DebugText_AddVertsToBuffer(vertsPosColor, vertsBufferOffet, str, pos, 0.01f, colorCyan, backEnd.viewDef->renderView.viewaxis, 1);
								}
							}
						}
						qglUnmapBufferARB(GL_ARRAY_BUFFER);
						RB_Cst_SetupVertPosColorLayout();

						qglDrawArrays(GL_LINES, 0, numBufferVerts);

						numBufferVerts = 0;
						firstSurf = surfLoopIndex;
						firstTri = triLoopIndex + triLoopInc;
						if (firstTri >= ((pass == 0) ? _tri->numVerts : _tri->numIndexes)) {
							++firstSurf;
							firstTri = 0;
						}
						nullTri = false;
						drawNow = false;
					}
				}
			}
		}
	}

	qglBindBufferARB(GL_ARRAY_BUFFER, 0);
	qglDeleteBuffersARB(1, &vBuffer);
	renderProgManager.Unbind();
	qglLineWidth(1);
	GL_State(GLS_DEFAULT);
}
//#modified-fva; END

#if 0 // compiler warning

/*
=====================
RB_ShowNormals

Debugging tool
=====================
*/
static void RB_AltShowNormals( drawSurf_t **drawSurfs, int numDrawSurfs ) {
	if ( r_showNormals.GetFloat() == 0.0f ) {
		return;
	}

	globalImages->BindNull();

	GL_State( GLS_DEPTHFUNC_ALWAYS );

	for ( int i = 0; i < numDrawSurfs; i++ ) {
		drawSurf_t * drawSurf = drawSurfs[i];

		RB_SimpleSurfaceSetup( drawSurf );

		const srfTriangles_t * tri = drawSurf->geo;

		qglBegin( GL_LINES );
		for ( int j = 0; j < tri->numIndexes; j += 3 ) {
			const idDrawVert *v[3] = {
				&tri->verts[tri->indexes[j+0]],
				&tri->verts[tri->indexes[j+1]],
				&tri->verts[tri->indexes[j+2]]
			}

			const idPlane plane( v[0]->xyz, v[1]->xyz, v[2]->xyz );

			// make the midpoint slightly above the triangle
			const idVec3 mid = ( v[0]->xyz + v[1]->xyz + v[2]->xyz ) * ( 1.0f / 3.0f ) + 0.1f * plane.Normal();

			for ( int k = 0; k < 3; k++ ) {
				const idVec3 pos = ( mid + v[k]->xyz * 3.0f ) * 0.25f;
				idVec3 end;

				GL_Color( 0, 0, 1 );
				qglVertex3fv( pos.ToFloatPtr() );
				VectorMA( pos, r_showNormals.GetFloat(), v[k]->normal, end );
				qglVertex3fv( end.ToFloatPtr() );

				GL_Color( 1, 0, 0 );
				qglVertex3fv( pos.ToFloatPtr() );
				VectorMA( pos, r_showNormals.GetFloat(), v[k]->tangents[0], end );
				qglVertex3fv( end.ToFloatPtr() );

				GL_Color( 0, 1, 0 );
				qglVertex3fv( pos.ToFloatPtr() );
				VectorMA( pos, r_showNormals.GetFloat(), v[k]->tangents[1], end );
				qglVertex3fv( end.ToFloatPtr() );

				GL_Color( 1, 1, 1 );
				qglVertex3fv( pos.ToFloatPtr() );
				qglVertex3fv( v[k]->xyz.ToFloatPtr() );
			}
		}
		qglEnd();
	}
}

#endif

/*
=====================
RB_ShowTextureVectors

Draw texture vectors in the center of each triangle
=====================
*/
//#modified-fva; BEGIN
/*
static void RB_ShowTextureVectors( drawSurf_t **drawSurfs, int numDrawSurfs ) {
	if ( r_showTextureVectors.GetFloat() == 0.0f ) {
		return;
	}

	GL_State( GLS_DEPTHFUNC_LESS );

	globalImages->BindNull();

	for ( int i = 0; i < numDrawSurfs; i++ ) {
		drawSurf_t * drawSurf = drawSurfs[i];

		const srfTriangles_t * tri = drawSurf->frontEndGeo;

		if ( tri->verts == NULL ) {
			continue;
		}

		RB_SimpleSurfaceSetup( drawSurf );

		// draw non-shared edges in yellow
		qglBegin( GL_LINES );

		for ( int j = 0; j < tri->numIndexes; j+= 3 ) {
			float d0[5], d1[5];
			idVec3 temp;
			idVec3 tangents[2];

			const idDrawVert *a = &tri->verts[tri->indexes[j+0]];
			const idDrawVert *b = &tri->verts[tri->indexes[j+1]];
			const idDrawVert *c = &tri->verts[tri->indexes[j+2]];

			const idPlane plane( a->xyz, b->xyz, c->xyz );

			// make the midpoint slightly above the triangle
			const idVec3 mid = ( a->xyz + b->xyz + c->xyz ) * ( 1.0f / 3.0f ) + 0.1f * plane.Normal();

			// calculate the texture vectors
			const idVec2 aST = a->GetTexCoord();
			const idVec2 bST = b->GetTexCoord();
			const idVec2 cST = c->GetTexCoord();

			d0[0] = b->xyz[0] - a->xyz[0];
			d0[1] = b->xyz[1] - a->xyz[1];
			d0[2] = b->xyz[2] - a->xyz[2];
			d0[3] = bST[0] - aST[0];
			d0[4] = bST[1] - aST[1];

			d1[0] = c->xyz[0] - a->xyz[0];
			d1[1] = c->xyz[1] - a->xyz[1];
			d1[2] = c->xyz[2] - a->xyz[2];
			d1[3] = cST[0] - aST[0];
			d1[4] = cST[1] - aST[1];

			const float area = d0[3] * d1[4] - d0[4] * d1[3];
			if ( area == 0 ) {
				continue;
			}
			const float inva = 1.0f / area;

			temp[0] = (d0[0] * d1[4] - d0[4] * d1[0]) * inva;
			temp[1] = (d0[1] * d1[4] - d0[4] * d1[1]) * inva;
			temp[2] = (d0[2] * d1[4] - d0[4] * d1[2]) * inva;
			temp.Normalize();
			tangents[0] = temp;

			temp[0] = (d0[3] * d1[0] - d0[0] * d1[3]) * inva;
			temp[1] = (d0[3] * d1[1] - d0[1] * d1[3]) * inva;
			temp[2] = (d0[3] * d1[2] - d0[2] * d1[3]) * inva;
			temp.Normalize();
			tangents[1] = temp;

			// draw the tangents
			tangents[0] = mid + tangents[0] * r_showTextureVectors.GetFloat();
			tangents[1] = mid + tangents[1] * r_showTextureVectors.GetFloat();

			GL_Color( 1, 0, 0 );
			qglVertex3fv( mid.ToFloatPtr() );
			qglVertex3fv( tangents[0].ToFloatPtr() );

			GL_Color( 0, 1, 0 );
			qglVertex3fv( mid.ToFloatPtr() );
			qglVertex3fv( tangents[1].ToFloatPtr() );
		}

		qglEnd();
	}
}
*/
static void RB_ShowTextureVectors(drawSurf_t **drawSurfs, int numDrawSurfs) {
	if (r_showTextureVectors.GetFloat() == 0.0f) {
		return;
	}
	common->CstBrtRequestEnable();
	if (!common->CstBrtAreEnabled()) {
		return;
	}

	int width = r_debugLineWidth.GetInteger();
	if (width < 1) {
		width = 1;
	} else if (width > 10) {
		width = 10;
	}
	qglLineWidth(width);

	GL_State(GLS_DEPTHFUNC_LESS);

	renderProgManager.BindShader_CstDebugPosColor();

	GLuint vBuffer = 0;
	qglGenBuffersARB(1, &vBuffer);
	qglBindBufferARB(GL_ARRAY_BUFFER, vBuffer);
	backEnd.glState.currentVertexBuffer = 0; // mark as not set

	for (int i = 0; i < numDrawSurfs; ++i) {
		drawSurf_t * drawSurf = drawSurfs[i];

		const srfTriangles_t * tri = drawSurf->frontEndGeo;
		if (!tri || !tri->verts) {
			continue;
		}
		if (tri->numVerts == 0 || tri->numIndexes == 0) {
			continue;
		}

		const int maxBuffVerts = (tri->numIndexes / 3) * 2 * 2;
		qglBufferDataARB(GL_ARRAY_BUFFER, maxBuffVerts * sizeof(CstVertPosColor), NULL, GL_STREAM_DRAW);
		CstVertPosColor * vertsPosColor = (CstVertPosColor *)qglMapBufferARB(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		if (!vertsPosColor) {
			qglBindBufferARB(GL_ARRAY_BUFFER, 0);
			qglDeleteBuffersARB(1, &vBuffer);
			renderProgManager.Unbind();
			qglLineWidth(1);
			GL_State(GLS_DEFAULT);
			return;
		}

		int vertsPosColorIndex = 0;
		for (int j = 0; j < tri->numIndexes; j += 3) {
			float d0[5], d1[5];
			idVec3 temp;
			idVec3 tangents[2];

			const idDrawVert *a = &tri->verts[tri->indexes[j + 0]];
			const idDrawVert *b = &tri->verts[tri->indexes[j + 1]];
			const idDrawVert *c = &tri->verts[tri->indexes[j + 2]];

			const idPlane plane(a->xyz, b->xyz, c->xyz);

			// make the midpoint slightly above the triangle
			const idVec3 mid = (a->xyz + b->xyz + c->xyz) * (1.0f / 3.0f) + 0.1f * plane.Normal();

			// calculate the texture vectors
			const idVec2 aST = a->GetTexCoord();
			const idVec2 bST = b->GetTexCoord();
			const idVec2 cST = c->GetTexCoord();

			d0[0] = b->xyz[0] - a->xyz[0];
			d0[1] = b->xyz[1] - a->xyz[1];
			d0[2] = b->xyz[2] - a->xyz[2];
			d0[3] = bST[0] - aST[0];
			d0[4] = bST[1] - aST[1];

			d1[0] = c->xyz[0] - a->xyz[0];
			d1[1] = c->xyz[1] - a->xyz[1];
			d1[2] = c->xyz[2] - a->xyz[2];
			d1[3] = cST[0] - aST[0];
			d1[4] = cST[1] - aST[1];

			const float area = d0[3] * d1[4] - d0[4] * d1[3];
			if (area == 0) {
				continue;
			}
			const float inva = 1.0f / area;

			temp[0] = (d0[0] * d1[4] - d0[4] * d1[0]) * inva;
			temp[1] = (d0[1] * d1[4] - d0[4] * d1[1]) * inva;
			temp[2] = (d0[2] * d1[4] - d0[4] * d1[2]) * inva;
			temp.Normalize();
			tangents[0] = temp;

			temp[0] = (d0[3] * d1[0] - d0[0] * d1[3]) * inva;
			temp[1] = (d0[3] * d1[1] - d0[1] * d1[3]) * inva;
			temp[2] = (d0[3] * d1[2] - d0[2] * d1[3]) * inva;
			temp.Normalize();
			tangents[1] = temp;

			// draw the tangents
			tangents[0] = mid + tangents[0] * r_showTextureVectors.GetFloat();
			tangents[1] = mid + tangents[1] * r_showTextureVectors.GetFloat();

			CstVertPosColor v1, v2;

			v1.position = mid;
			v2.position = tangents[0];
			v1.color = v2.color = idVec4(1.0f, 0.0f, 0.0, 1.0f);
			vertsPosColor[vertsPosColorIndex++] = v1;
			vertsPosColor[vertsPosColorIndex++] = v2;

			//v1.position = same as above
			v2.position = tangents[1];
			v1.color = v2.color = idVec4(0.0f, 1.0f, 0.0, 1.0f);
			vertsPosColor[vertsPosColorIndex++] = v1;
			vertsPosColor[vertsPosColorIndex++] = v2;
		}

		qglUnmapBufferARB(GL_ARRAY_BUFFER);
		if (vertsPosColorIndex > 0) {
			RB_Cst_SetupVertPosColorLayout();

			RB_SimpleSurfaceSetup(drawSurf);
			renderProgManager.CommitUniforms();

			qglDrawArrays(GL_LINES, 0, vertsPosColorIndex);
		}
	}

	qglBindBufferARB(GL_ARRAY_BUFFER, 0);
	qglDeleteBuffersARB(1, &vBuffer);
	renderProgManager.Unbind();
	qglLineWidth(1);
	GL_State(GLS_DEFAULT);
}
//#modified-fva; END

/*
=====================
RB_ShowDominantTris

Draw lines from each vertex to the dominant triangle center
=====================
*/
//#modified-fva; BEGIN
/*
static void RB_ShowDominantTris( drawSurf_t **drawSurfs, int numDrawSurfs ) {
	int			i, j;
	drawSurf_t	*drawSurf;
	const srfTriangles_t	*tri;

	if ( !r_showDominantTri.GetBool() ) {
		return;
	}

	GL_State( GLS_DEPTHFUNC_LESS );

	GL_PolygonOffset( -1, -2 );
	qglEnable( GL_POLYGON_OFFSET_LINE );

	globalImages->BindNull();

	for ( i = 0; i < numDrawSurfs; i++ ) {
		drawSurf = drawSurfs[i];

		tri = drawSurf->frontEndGeo;

		if ( !tri->verts ) {
			continue;
		}
		if ( !tri->dominantTris ) {
			continue;
		}
		RB_SimpleSurfaceSetup( drawSurf );

		GL_Color( 1, 1, 0 );
		qglBegin( GL_LINES );

		for ( j = 0; j < tri->numVerts; j++ ) {
			const idDrawVert *a, *b, *c;
			idVec3		mid;

			// find the midpoint of the dominant tri

			a = &tri->verts[j];
			b = &tri->verts[tri->dominantTris[j].v2];
			c = &tri->verts[tri->dominantTris[j].v3];

			mid = ( a->xyz + b->xyz + c->xyz ) * ( 1.0f / 3.0f );

			qglVertex3fv( mid.ToFloatPtr() );
			qglVertex3fv( a->xyz.ToFloatPtr() );
		}

		qglEnd();
	}
	qglDisable( GL_POLYGON_OFFSET_LINE );
}
*/
static void RB_ShowDominantTris(drawSurf_t **drawSurfs, int numDrawSurfs) {
	if (!r_showDominantTri.GetBool()) {
		return;
	}
	common->CstBrtRequestEnable();
	if (!common->CstBrtAreEnabled()) {
		return;
	}

	int width = r_debugLineWidth.GetInteger();
	if (width < 1) {
		width = 1;
	} else if (width > 10) {
		width = 10;
	}
	qglLineWidth(width);

	GL_State(GLS_DEPTHFUNC_LESS);

	renderProgManager.BindShader_CstDebugPosColor();

	GLuint vBuffer = 0;
	qglGenBuffersARB(1, &vBuffer);
	qglBindBufferARB(GL_ARRAY_BUFFER, vBuffer);
	backEnd.glState.currentVertexBuffer = 0; // mark as not set

	for (int i = 0; i < numDrawSurfs; ++i) {
		drawSurf_t * drawSurf = drawSurfs[i];

		const srfTriangles_t * tri = drawSurf->frontEndGeo;
		if (!tri || !tri->verts) {
			continue;
		}
		if (tri->numVerts == 0 || tri->numIndexes == 0) {
			continue;
		}
		if (!tri->dominantTris) {
			continue;
		}

		const int numBuffVerts = tri->numVerts * 2;
		qglBufferDataARB(GL_ARRAY_BUFFER, numBuffVerts * sizeof(CstVertPosColor), NULL, GL_STREAM_DRAW);
		CstVertPosColor * vertsPosColor = (CstVertPosColor *)qglMapBufferARB(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		if (!vertsPosColor) {
			qglBindBufferARB(GL_ARRAY_BUFFER, 0);
			qglDeleteBuffersARB(1, &vBuffer);
			renderProgManager.Unbind();
			qglLineWidth(1);
			GL_State(GLS_DEFAULT);
			return;
		}

		int vertsPosColorIndex = 0;
		for (int j = 0; j < tri->numVerts; ++j) {
			// find the midpoint of the dominant tri

			const idDrawVert * a = &tri->verts[j];
			const idDrawVert * b = &tri->verts[tri->dominantTris[j].v2];
			const idDrawVert * c = &tri->verts[tri->dominantTris[j].v3];

			// make the midpoint slightly above the triangle (as in RB_ShowTextureVectors)
			const idPlane plane(a->xyz, b->xyz, c->xyz);
			const idVec3 mid = (a->xyz + b->xyz + c->xyz) * (1.0f / 3.0f) + 0.1f * plane.Normal();

			CstVertPosColor v1, v2;
			v1.position = mid;
			v2.position = a->xyz;
			v1.color = v2.color = idVec4(1.0f, 1.0f, 0.0f, 1.0f);

			vertsPosColor[vertsPosColorIndex++] = v1;
			vertsPosColor[vertsPosColorIndex++] = v2;
		}

		qglUnmapBufferARB(GL_ARRAY_BUFFER);
		RB_Cst_SetupVertPosColorLayout();

		RB_SimpleSurfaceSetup(drawSurf);
		renderProgManager.CommitUniforms();

		qglDrawArrays(GL_LINES, 0, numBuffVerts);
	}

	qglBindBufferARB(GL_ARRAY_BUFFER, 0);
	qglDeleteBuffersARB(1, &vBuffer);
	renderProgManager.Unbind();
	qglLineWidth(1);
	GL_State(GLS_DEFAULT);
}
//#modified-fva; END

/*
=====================
RB_ShowEdges

Debugging tool
=====================
*/
//#modified-fva; BEGIN
/*
static void RB_ShowEdges( drawSurf_t **drawSurfs, int numDrawSurfs ) {
	int			i, j, k, m, n, o;
	drawSurf_t	*drawSurf;
	const srfTriangles_t	*tri;
	const silEdge_t			*edge;
	int			danglePlane;

	if ( !r_showEdges.GetBool() ) {
		return;
	}

	globalImages->BindNull();

	GL_State( GLS_DEPTHFUNC_ALWAYS );

	for ( i = 0; i < numDrawSurfs; i++ ) {
		drawSurf = drawSurfs[i];

		tri = drawSurf->frontEndGeo;

		idDrawVert *ac = (idDrawVert *)tri->verts;
		if ( !ac ) {
			continue;
		}

		RB_SimpleSurfaceSetup( drawSurf );

		// draw non-shared edges in yellow
		GL_Color( 1, 1, 0 );
		qglBegin( GL_LINES );

		for ( j = 0; j < tri->numIndexes; j+= 3 ) {
			for ( k = 0; k < 3; k++ ) {
				int		l, i1, i2;
				l = ( k == 2 ) ? 0 : k + 1;
				i1 = tri->indexes[j+k];
				i2 = tri->indexes[j+l];

				// if these are used backwards, the edge is shared
				for ( m = 0; m < tri->numIndexes; m += 3 ) {
					for ( n = 0; n < 3; n++ ) {
						o = ( n == 2 ) ? 0 : n + 1;
						if ( tri->indexes[m+n] == i2 && tri->indexes[m+o] == i1 ) {
							break;
						}
					}
					if ( n != 3 ) {
						break;
					}
				}

				// if we didn't find a backwards listing, draw it in yellow
				if ( m == tri->numIndexes ) {
					qglVertex3fv( ac[ i1 ].xyz.ToFloatPtr() );
					qglVertex3fv( ac[ i2 ].xyz.ToFloatPtr() );
				}

			}
		}

		qglEnd();

		// draw dangling sil edges in red
		if ( !tri->silEdges ) {
			continue;
		}

		// the plane number after all real planes
		// is the dangling edge
		danglePlane = tri->numIndexes / 3;

		GL_Color( 1, 0, 0 );

		qglBegin( GL_LINES );
		for ( j = 0; j < tri->numSilEdges; j++ ) {
			edge = tri->silEdges + j;

			if ( edge->p1 != danglePlane && edge->p2 != danglePlane ) {
				continue;
			}

			qglVertex3fv( ac[ edge->v1 ].xyz.ToFloatPtr() );
			qglVertex3fv( ac[ edge->v2 ].xyz.ToFloatPtr() );
		}
		qglEnd();
	}
}
*/
static void RB_ShowEdges(drawSurf_t **drawSurfs, int numDrawSurfs) {
	if (!r_showEdges.GetBool()) {
		return;
	}
	common->CstBrtRequestEnable();
	if (!common->CstBrtAreEnabled()) {
		return;
	}

	int width = r_debugLineWidth.GetInteger();
	if (width < 1) {
		width = 1;
	} else if (width > 10) {
		width = 10;
	}
	qglLineWidth(width);

	GL_State(GLS_DEPTHFUNC_ALWAYS);

	renderProgManager.BindShader_CstDebugPosColor();

	GLuint vBuffer = 0;
	qglGenBuffersARB(1, &vBuffer);
	qglBindBufferARB(GL_ARRAY_BUFFER, vBuffer);
	backEnd.glState.currentVertexBuffer = 0; // mark as not set

	const int listGranuralityBytes = 512 * 1024;
	const int listGranularity = listGranuralityBytes / sizeof(CstVertPosColor);
	idList<CstVertPosColor> vertsToDraw(listGranularity);

	for (int i = 0; i < numDrawSurfs; ++i) {
		drawSurf_t * drawSurf = drawSurfs[i];
		const srfTriangles_t * tri = drawSurf->frontEndGeo;
		if (!tri) {
			continue;
		}
		if (tri->numVerts == 0 || tri->numIndexes == 0) {
			continue;
		}
		idDrawVert *ac = (idDrawVert *)tri->verts;
		if (!ac) {
			continue;
		}
		vertsToDraw.SetNum(0);

		// draw non-shared edges in yellow
		for (int j = 0; j < tri->numIndexes; j += 3) {
			for (int k = 0; k < 3; ++k) {
				int		l, i1, i2;
				l = (k == 2) ? 0 : k + 1;
				i1 = tri->indexes[j + k];
				i2 = tri->indexes[j + l];

				// if these are used backwards, the edge is shared
				int m = 0;
				for (; m < tri->numIndexes; m += 3) {
					int n = 0;
					for (; n < 3; n++) {
						int o = (n == 2) ? 0 : n + 1;
						if (tri->indexes[m + n] == i2 && tri->indexes[m + o] == i1) {
							break;
						}
					}
					if (n != 3) {
						break;
					}
				}

				// if we didn't find a backwards listing, draw it in yellow
				if (m == tri->numIndexes) {
					CstVertPosColor &v1 = vertsToDraw.Alloc();
					CstVertPosColor &v2 = vertsToDraw.Alloc();
					v1.position = ac[i1].xyz;
					v2.position = ac[i2].xyz;
					v1.color = v2.color = idVec4(1.0f, 1.0f, 0.0f, 1.0f);
				}
			}
		}

		// draw dangling sil edges in red
		if (tri->silEdges) {
			// the plane number after all real planes is the dangling edge
			int danglePlane = tri->numIndexes / 3;
			for (int j = 0; j < tri->numSilEdges; ++j) {
				const silEdge_t * edge = tri->silEdges + j;
				if (edge->p1 != danglePlane && edge->p2 != danglePlane) {
					continue;
				}
				CstVertPosColor &v1 = vertsToDraw.Alloc();
				CstVertPosColor &v2 = vertsToDraw.Alloc();
				v1.position = ac[edge->v1].xyz;
				v2.position = ac[edge->v2].xyz;
				v1.color = v2.color = idVec4(1.0f, 0.0f, 0.0f, 1.0f);
			}
		}

		if (vertsToDraw.Num() > 0) {
			qglBufferDataARB(GL_ARRAY_BUFFER, vertsToDraw.Num() * sizeof(CstVertPosColor), vertsToDraw.Ptr(), GL_STREAM_DRAW);
			RB_Cst_SetupVertPosColorLayout();
			RB_SimpleSurfaceSetup(drawSurf);
			renderProgManager.CommitUniforms();
			qglDrawArrays(GL_LINES, 0, vertsToDraw.Num());
		}
	}

	qglBindBufferARB(GL_ARRAY_BUFFER, 0);
	qglDeleteBuffersARB(1, &vBuffer);
	renderProgManager.Unbind();
	qglLineWidth(1);
	GL_State(GLS_DEFAULT);
}
//#modified-fva; END

/*
==============
RB_ShowLights

Visualize all light volumes used in the current scene
r_showLights 1	: just print volumes numbers, highlighting ones covering the view
r_showLights 2	: also draw planes of each volume
r_showLights 3	: also draw edges of each volume
==============
*/
//#modified-fva; BEGIN
/*
static void RB_ShowLights() {
	if ( !r_showLights.GetInteger() ) {
		return;
	}

	GL_State( GLS_DEFAULT );

	// we use the 'vLight->invProjectMVPMatrix'
	qglMatrixMode( GL_PROJECTION );
	qglLoadIdentity();

	globalImages->BindNull();

	renderProgManager.BindShader_Color();

	GL_Cull( CT_TWO_SIDED );

	common->Printf( "volumes: " );	// FIXME: not in back end!

	int count = 0;
	for ( viewLight_t * vLight = backEnd.viewDef->viewLights; vLight != NULL; vLight = vLight->next ) {
		count++;

		// depth buffered planes
		if ( r_showLights.GetInteger() >= 2 ) {
			GL_State( GLS_DEPTHFUNC_ALWAYS | GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA | GLS_DEPTHMASK );
			GL_Color( 0.0f, 0.0f, 1.0f, 0.25f );
			idRenderMatrix invProjectMVPMatrix;
			idRenderMatrix::Multiply( backEnd.viewDef->worldSpace.mvp, vLight->inverseBaseLightProject, invProjectMVPMatrix );
			RB_SetMVP( invProjectMVPMatrix );
			RB_DrawElementsWithCounters( &backEnd.zeroOneCubeSurface );
		}

		// non-hidden lines
		if ( r_showLights.GetInteger() >= 3 ) {
			GL_State( GLS_DEPTHFUNC_ALWAYS | GLS_POLYMODE_LINE | GLS_DEPTHMASK  );
			GL_Color( 1.0f, 1.0f, 1.0f );
			idRenderMatrix invProjectMVPMatrix;
			idRenderMatrix::Multiply( backEnd.viewDef->worldSpace.mvp, vLight->inverseBaseLightProject, invProjectMVPMatrix );
			RB_SetMVP( invProjectMVPMatrix );
			RB_DrawElementsWithCounters( &backEnd.zeroOneCubeSurface );
		}

		common->Printf( "%i ", vLight->lightDef->index );
	}

	common->Printf( " = %i total\n", count );

	// set back the default projection matrix
	qglMatrixMode( GL_PROJECTION );
	qglLoadMatrixf( backEnd.viewDef->projectionMatrix );
	qglMatrixMode( GL_MODELVIEW );
	qglLoadIdentity();
}
*/
static void RB_ShowLights() {
	if (!r_showLights.GetInteger()) {
		return;
	}
	common->CstBrtRequestEnable();
	if (!common->CstBrtAreEnabled()) {
		return;
	}

	int width = r_debugLineWidth.GetInteger();
	if (width < 1) {
		width = 1;
	} else if (width > 10) {
		width = 10;
	}
	qglLineWidth(width);

	GL_State(GLS_DEFAULT);
	GL_Cull(CT_TWO_SIDED);

	renderProgManager.BindShader_Color();

	if (r_showLights.GetInteger() == 1) {
		common->Printf("volumes: ");	// FIXME: not in back end!
	}

	int count = 0;
	for (viewLight_t * vLight = backEnd.viewDef->viewLights; vLight != NULL; vLight = vLight->next) {
		count++;

		// depth buffered planes
		if (r_showLights.GetInteger() >= 2) {
			GL_State(GLS_DEPTHFUNC_ALWAYS | GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA | GLS_DEPTHMASK);
			GL_Color(0.0f, 0.0f, 1.0f, 0.25f);
			idRenderMatrix invProjectMVPMatrix;
			idRenderMatrix::Multiply(backEnd.viewDef->worldSpace.mvp, vLight->inverseBaseLightProject, invProjectMVPMatrix);
			RB_Cst_Simple_MVP_Setup(invProjectMVPMatrix);
			RB_DrawElementsWithCounters(&backEnd.zeroOneCubeSurface);
		}

		// non-hidden lines
		if (r_showLights.GetInteger() >= 3) {
			GL_State(GLS_DEPTHFUNC_ALWAYS | GLS_POLYMODE_LINE | GLS_DEPTHMASK);
			GL_Color(1.0f, 1.0f, 1.0f);
			idRenderMatrix invProjectMVPMatrix;
			idRenderMatrix::Multiply(backEnd.viewDef->worldSpace.mvp, vLight->inverseBaseLightProject, invProjectMVPMatrix);
			RB_Cst_Simple_MVP_Setup(invProjectMVPMatrix);
			RB_DrawElementsWithCounters(&backEnd.zeroOneCubeSurface);
		}

		if (r_showLights.GetInteger() == 1) {
			common->Printf("%i ", vLight->lightDef->index);
		}
	}

	if (r_showLights.GetInteger() == 1) {
		common->Printf(" = %i total\n", count);
	}

	qglLineWidth(1);
	GL_Cull(CT_FRONT_SIDED);
	GL_State(GLS_DEFAULT);
}
//#modified-fva; END

/*
=====================
RB_ShowPortals

Debugging tool, won't work correctly with SMP or when mirrors are present
=====================
*/
//#modified-fva; BEGIN
/*
static void RB_ShowPortals() {
	if ( !r_showPortals.GetBool() ) {
		return;
	}

	// all portals are expressed in world coordinates
	RB_SimpleWorldSetup();

	globalImages->BindNull();
	renderProgManager.BindShader_Color();
	GL_State( GLS_DEPTHFUNC_ALWAYS );

	((idRenderWorldLocal *)backEnd.viewDef->renderWorld)->ShowPortals();
}
*/
static void RB_ShowPortals() {
	// some of the code below was moved from idRenderWorldLocal::ShowPortals (with changes)

	if (!r_showPortals.GetBool()) {
		return;
	}
	common->CstBrtRequestEnable();
	if (!common->CstBrtAreEnabled()) {
		return;
	}

	idRenderWorldLocal * renderWorld = backEnd.viewDef->renderWorld;
	if (!renderWorld) {
		return;
	}

	int width = r_debugLineWidth.GetInteger();
	if (width < 1) {
		width = 1;
	} else if (width > 10) {
		width = 10;
	}
	qglLineWidth(width);

	GL_State(GLS_DEPTHFUNC_ALWAYS);

	// all portals are expressed in world coordinates
	RB_SimpleWorldSetup();

	renderProgManager.BindShader_CstDebugPosColor();
	renderProgManager.CommitUniforms();

	GLuint vBuffer = 0;
	qglGenBuffersARB(1, &vBuffer);
	qglBindBufferARB(GL_ARRAY_BUFFER, vBuffer);
	backEnd.glState.currentVertexBuffer = 0; // mark as not set

	// flood out through portals, setting area viewCount
	for (int i = 0; i < renderWorld->numPortalAreas; ++i) {
		portalArea_t * area = &renderWorld->portalAreas[i];
		if (area->viewCount != tr.viewCount) {
			continue;
		}
		for (portal_t * p = area->portals; p; p = p->next) {
			idWinding * w = p->w;
			if (!w) {
				continue;
			}

			const int numBuffVerts = w->GetNumPoints();
			if (numBuffVerts < 3) {
				continue;
			}
			qglBufferDataARB(GL_ARRAY_BUFFER, numBuffVerts * sizeof(CstVertPosColor), NULL, GL_STREAM_DRAW);
			CstVertPosColor * vertsPosColor = (CstVertPosColor *)qglMapBufferARB(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
			if (!vertsPosColor) {
				qglBindBufferARB(GL_ARRAY_BUFFER, 0);
				qglDeleteBuffersARB(1, &vBuffer);
				renderProgManager.Unbind();
				qglLineWidth(1);
				GL_State(GLS_DEFAULT);
				return;
			}

			idVec4 color;
			if (renderWorld->portalAreas[p->intoArea].viewCount != tr.viewCount) {
				// red = can't see
				color.Set(1.0f, 0.0f, 0.0f, 1.0f);
			} else {
				// green = see through
				color.Set(0.0f, 1.0f, 0.0f, 1.0f);
			}

			for (int j = 0; j < w->GetNumPoints(); ++j) {
				CstVertPosColor v;
				v.position = (*w)[j].ToVec3();
				v.color = color;
				vertsPosColor[j] = v;
			}

			qglUnmapBufferARB(GL_ARRAY_BUFFER);
			RB_Cst_SetupVertPosColorLayout();

			qglDrawArrays(GL_LINE_LOOP, 0, numBuffVerts);
		}
	}

	qglBindBufferARB(GL_ARRAY_BUFFER, 0);
	qglDeleteBuffersARB(1, &vBuffer);
	renderProgManager.Unbind();
	qglLineWidth(1);
	GL_State(GLS_DEFAULT);
}
//#modified-fva; END

/*
================
RB_ClearDebugText
================
*/
void RB_ClearDebugText( int time ) {
	int			i;
	int			num;
	debugText_t	*text;

	rb_debugTextTime = time;

	if ( !time ) {
		// free up our strings
		text = rb_debugText;
		for ( i = 0; i < MAX_DEBUG_TEXT; i++, text++ ) {
			text->text.Clear();
		}
		rb_numDebugText = 0;
		return;
	}

	// copy any text that still needs to be drawn
	num	= 0;
	text = rb_debugText;
	for ( i = 0; i < rb_numDebugText; i++, text++ ) {
		//#modified-fva; BEGIN
		//if ( text->lifeTime > time ) {
		if (text->lifeTime < 0 || text->lifeTime > time) {
		//#modified-fva; END
			if ( num != i ) {
				rb_debugText[ num ] = *text;
			}
			num++;
		}
	}
	rb_numDebugText = num;
}

/*
================
RB_AddDebugText
================
*/
void RB_AddDebugText( const char *text, const idVec3 &origin, float scale, const idVec4 &color, const idMat3 &viewAxis, const int align, const int lifetime, const bool depthTest ) {
	//#modified-fva; BEGIN
	common->CstBrtRequestEnable();
	if (!common->CstBrtAreEnabled()) {
		return;
	}
	//#modified-fva; END
	debugText_t *debugText;

	if ( rb_numDebugText < MAX_DEBUG_TEXT ) {
		debugText = &rb_debugText[ rb_numDebugText++ ];
		debugText->text			= text;
		debugText->origin		= origin;
		debugText->scale		= scale;
		debugText->color		= color;
		debugText->viewAxis		= viewAxis;
		debugText->align		= align;
		//#modified-fva; BEGIN
		//debugText->lifeTime		= rb_debugTextTime + lifetime;
		if (lifetime < 0) {
			debugText->lifeTime = -1;
		} else {
			debugText->lifeTime = rb_debugTextTime + lifetime;
		}
		//#modified-fva; END
		debugText->depthTest	= depthTest;
	}
}

/*
================
RB_DrawTextLength

  returns the length of the given text
================
*/
float RB_DrawTextLength( const char *text, float scale, int len ) {
	int i, num, index, charIndex;
	float spacing, textLen = 0.0f;

	if ( text && *text ) {
		if ( !len ) {
			len = strlen(text);
		}
		for ( i = 0; i < len; i++ ) {
			charIndex = text[i] - 32;
			//#modified-fva; BEGIN
			//if ( charIndex < 0 || charIndex > NUM_SIMPLEX_CHARS ) {
			if (charIndex < 0 || charIndex >= NUM_SIMPLEX_CHARS) {
			//#modified-fva; END
				continue;
			}
			num = simplex[charIndex][0] * 2;
			spacing = simplex[charIndex][1];
			index = 2;

			while( index - 2 < num ) {
				if ( simplex[charIndex][index] < 0) {
					index++;
					continue;
				}
				index += 2;
				if ( simplex[charIndex][index] < 0) {
					index++;
					continue;
				}
			}
			textLen += spacing * scale;
		}
	}
	return textLen;
}

/*
================
RB_DrawText

  oriented on the viewaxis
  align can be 0-left, 1-center (default), 2-right
================
*/
//#modified-fva; BEGIN
/*
static void RB_DrawText( const char *text, const idVec3 &origin, float scale, const idVec4 &color, const idMat3 &viewAxis, const int align ) {
	renderProgManager.BindShader_Color();



	int i, j, len, num, index, charIndex, line;
	float textLen = 1.0f, spacing = 1.0f;
	idVec3 org, p1, p2;

	if ( text && *text ) {
		qglBegin( GL_LINES );
		qglColor3fv( color.ToFloatPtr() );

		if ( text[0] == '\n' ) {
			line = 1;
		} else {
			line = 0;
		}

		len = strlen( text );
		for ( i = 0; i < len; i++ ) {

			if ( i == 0 || text[i] == '\n' ) {
				org = origin - viewAxis[2] * ( line * 36.0f * scale );
				if ( align != 0 ) {
					for ( j = 1; i+j <= len; j++ ) {
						if ( i+j == len || text[i+j] == '\n' ) {
							textLen = RB_DrawTextLength( text+i, scale, j );
							break;
						}
					}
					if ( align == 2 ) {
						// right
						org += viewAxis[1] * textLen;
					} else {
						// center
						org += viewAxis[1] * ( textLen * 0.5f );
					}
				}
				line++;
			}

			charIndex = text[i] - 32;
			if ( charIndex < 0 || charIndex > NUM_SIMPLEX_CHARS ) {
				continue;
			}
			num = simplex[charIndex][0] * 2;
			spacing = simplex[charIndex][1];
			index = 2;

			while( index - 2 < num ) {
				if ( simplex[charIndex][index] < 0) {
					index++;
					continue;
				}
				p1 = org + scale * simplex[charIndex][index] * -viewAxis[1] + scale * simplex[charIndex][index+1] * viewAxis[2];
				index += 2;
				if ( simplex[charIndex][index] < 0) {
					index++;
					continue;
				}
				p2 = org + scale * simplex[charIndex][index] * -viewAxis[1] + scale * simplex[charIndex][index+1] * viewAxis[2];

				qglVertex3fv( p1.ToFloatPtr() );
				qglVertex3fv( p2.ToFloatPtr() );
			}
			org -= viewAxis[1] * ( spacing * scale );
		}

		qglEnd();
	}
}
*/
//#modified-fva; END

/*
================
RB_ShowDebugText
================
*/
//#modified-fva; BEGIN
/*
void RB_ShowDebugText() {
	int			i;
	int			width;
	debugText_t	*text;

	if ( !rb_numDebugText ) {
		return;
	}

	// all lines are expressed in world coordinates
	RB_SimpleWorldSetup();

	globalImages->BindNull();

	width = r_debugLineWidth.GetInteger();
	if ( width < 1 ) {
		width = 1;
	} else if ( width > 10 ) {
		width = 10;
	}

	// draw lines
	qglLineWidth( width );


	if ( !r_debugLineDepthTest.GetBool() ) {
		GL_State( GLS_POLYMODE_LINE | GLS_DEPTHFUNC_ALWAYS );
	} else {
		GL_State( GLS_POLYMODE_LINE );
	}

	text = rb_debugText;
	for ( i = 0; i < rb_numDebugText; i++, text++ ) {
		if ( !text->depthTest ) {
			RB_DrawText( text->text, text->origin, text->scale, text->color, text->viewAxis, text->align );
		}
	}

	if ( !r_debugLineDepthTest.GetBool() ) {
		GL_State( GLS_POLYMODE_LINE );
	}

	text = rb_debugText;
	for ( i = 0; i < rb_numDebugText; i++, text++ ) {
		if ( text->depthTest ) {
			RB_DrawText( text->text, text->origin, text->scale, text->color, text->viewAxis, text->align );
		}
	}

	qglLineWidth( 1 );
}
*/
static int RB_Cst_DebugText_GetNumVerts(const idStr &str) {
	static int simplexNumVertsTable[NUM_SIMPLEX_CHARS];
	static bool tableValid = false;

	if (!tableValid) {
		for (int charIndex = 0; charIndex < NUM_SIMPLEX_CHARS; ++charIndex) {
			int num = simplex[charIndex][0] * 2;
			int index = 2;
			int numVerts = 0;
			while (index - 2 < num) {
				if (simplex[charIndex][index] < 0) {
					index++;
					continue;
				}
				index += 2;
				if (simplex[charIndex][index] < 0) {
					index++;
					continue;
				}
				numVerts += 2;
			}
			simplexNumVertsTable[charIndex] = numVerts;
		}
		tableValid = true;
	}

	int numVerts = 0;
	for (int i = 0; i < str.Length(); ++i) {
		if (str[i] == '\n') {
			continue;
		}
		int charIndex = str[i] - 32;
		if (charIndex < 0 || charIndex >= NUM_SIMPLEX_CHARS) {
			continue;
		}
		numVerts += simplexNumVertsTable[charIndex];
	}
	return numVerts;
}

static int RB_Cst_DebugText_AddVertsToBuffer(CstVertPosColor *vertsBuffer, int vertsBufferOffset, idStr &str, const idVec3 &origin, float scale, const idVec4 &color, const idMat3 &viewAxis, const int align) {
	if (!vertsBuffer) {
		return 0;
	}
	int numVertsAdded = 0;

	int i, j, len, num, index, charIndex, line;
	float textLen = 1.0f, spacing = 1.0f;
	idVec3 org, p1, p2;

	if (str[0] == '\n') {
		line = 1;
	} else {
		line = 0;
	}

	len = str.Length();
	const char * text = str.c_str();
	for (i = 0; i < len; i++) {
		if (i == 0 || str[i] == '\n') {
			org = origin - viewAxis[2] * (line * 36.0f * scale);
			if (align != 0) {
				for (j = 1; i + j <= len; j++) {
					if (i + j == len || text[i + j] == '\n') {
						textLen = RB_DrawTextLength(text + i, scale, j);
						break;
					}
				}
				if (align == 2) {
					// right
					org += viewAxis[1] * textLen;
				} else {
					// center
					org += viewAxis[1] * (textLen * 0.5f);
				}
			}
			line++;
		}

		charIndex = text[i] - 32;
		if (charIndex < 0 || charIndex >= NUM_SIMPLEX_CHARS) {
			continue;
		}
		num = simplex[charIndex][0] * 2;
		spacing = simplex[charIndex][1];
		index = 2;

		while (index - 2 < num) {
			if (simplex[charIndex][index] < 0) {
				index++;
				continue;
			}
			p1 = org + scale * simplex[charIndex][index] * -viewAxis[1] + scale * simplex[charIndex][index + 1] * viewAxis[2];
			index += 2;
			if (simplex[charIndex][index] < 0) {
				index++;
				continue;
			}
			p2 = org + scale * simplex[charIndex][index] * -viewAxis[1] + scale * simplex[charIndex][index + 1] * viewAxis[2];

			vertsBuffer[vertsBufferOffset].position = p1;
			vertsBuffer[vertsBufferOffset++].color = color;
			vertsBuffer[vertsBufferOffset].position = p2;
			vertsBuffer[vertsBufferOffset++].color = color;
			numVertsAdded += 2;
		}
		org -= viewAxis[1] * (spacing * scale);
	}
	return numVertsAdded;
}

void RB_ShowDebugText() {
	if (!common->CstBrtAreEnabled()) {
		return;
	}
	if (!rb_numDebugText) {
		return;
	}

	for (int i = 0; i < rb_numDebugText; ++i) {
		if (rb_debugText[i].lifeTime < 0) {
			// draw this string now and clear it on the next game frame
			rb_debugText[i].lifeTime = 0;
		}
	}

	// all lines are expressed in world coordinates
	RB_SimpleWorldSetup();

	int width = r_debugLineWidth.GetInteger();
	if (width < 1) {
		width = 1;
	} else if (width > 10) {
		width = 10;
	}
	qglLineWidth(width);

	const int maxBufferSize = 1 * 1024 * 1024;
	const int maxBufferVerts = maxBufferSize / sizeof(CstVertPosColor);

	GLuint vBuffer = 0;
	qglGenBuffersARB(1, &vBuffer);
	qglBindBufferARB(GL_ARRAY_BUFFER, vBuffer);
	backEnd.glState.currentVertexBuffer = 0; // mark as not set

	renderProgManager.BindShader_CstDebugPosColor();
	renderProgManager.CommitUniforms();

	for (int pass = 0; pass < 2; ++pass) {
		int numBufferVerts = 0;
		int firstStringIndex = 0;
		bool drawNow = false;
		bool depthTestCondition = false;

		if (pass == 0) {
			// strings with depthTest == false
			if (!r_debugLineDepthTest.GetBool()) {
				GL_State(GLS_DEPTHFUNC_ALWAYS);
			} else {
				GL_State(GLS_DEFAULT);
			}
		} else {
			// strings with depthTest == true
			if (!r_debugLineDepthTest.GetBool()) {
				GL_State(GLS_DEFAULT);
			}
			depthTestCondition = true;
		}

		// draw
		for (int i = 0; i < rb_numDebugText; ++i) {
			if (rb_debugText[i].depthTest == depthTestCondition) {
				int numVertsText = RB_Cst_DebugText_GetNumVerts(rb_debugText[i].text);
				if (numVertsText > maxBufferVerts) {
					// string is too large; remove it
					rb_debugText[i].text = "";
					numVertsText = 0;
				}
				if (numBufferVerts + numVertsText > maxBufferVerts) {
					--i; // so the current string goes on the next buffer
					drawNow = true;
				} else {
					numBufferVerts += numVertsText;
				}
				if (numBufferVerts > 0 && (drawNow || i == rb_numDebugText - 1)) {
					qglBufferDataARB(GL_ARRAY_BUFFER, numBufferVerts * sizeof(CstVertPosColor), NULL, GL_STREAM_DRAW);
					CstVertPosColor * verts = (CstVertPosColor *)qglMapBufferARB(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
					if (!verts) {
						qglBindBufferARB(GL_ARRAY_BUFFER, 0);
						qglDeleteBuffersARB(1, &vBuffer);
						renderProgManager.Unbind();
						qglLineWidth(1);
						GL_State(GLS_DEFAULT);
						return;
					}
					int vertsBufferOffet = 0;
					for (int j = firstStringIndex; j <= i; ++j) {
						debugText_t &text = rb_debugText[j];
						if (text.depthTest == depthTestCondition) {
							vertsBufferOffet += RB_Cst_DebugText_AddVertsToBuffer(verts, vertsBufferOffet, text.text, text.origin, text.scale, text.color, text.viewAxis, text.align);
						}
					}
					qglUnmapBufferARB(GL_ARRAY_BUFFER);
					RB_Cst_SetupVertPosColorLayout();

					qglDrawArrays(GL_LINES, 0, numBufferVerts);

					numBufferVerts = 0;
					firstStringIndex = i + 1;
					drawNow = false;
				}
			}
		}
	}

	qglBindBufferARB(GL_ARRAY_BUFFER, 0);
	qglDeleteBuffersARB(1, &vBuffer);
	renderProgManager.Unbind();
	qglLineWidth(1);
	GL_State(GLS_DEFAULT);
}
//#modified-fva; END

/*
================
RB_ClearDebugLines
================
*/
void RB_ClearDebugLines( int time ) {
	int			i;
	int			num;
	debugLine_t	*line;

	rb_debugLineTime = time;

	if ( !time ) {
		rb_numDebugLines = 0;
		return;
	}

	// copy any lines that still need to be drawn
	num	= 0;
	line = rb_debugLines;
	for ( i = 0; i < rb_numDebugLines; i++, line++ ) {
		//#modified-fva; BEGIN
		//if ( line->lifeTime > time ) {
		if (line->lifeTime < 0 || line->lifeTime > time) {
		//#modified-fva; END
			if ( num != i ) {
				rb_debugLines[ num ] = *line;
			}
			num++;
		}
	}
	rb_numDebugLines = num;
}

/*
================
RB_AddDebugLine
================
*/
void RB_AddDebugLine( const idVec4 &color, const idVec3 &start, const idVec3 &end, const int lifeTime, const bool depthTest ) {
	//#modified-fva; BEGIN
	common->CstBrtRequestEnable();
	if (!common->CstBrtAreEnabled()) {
		return;
	}
	//#modified-fva; END
	debugLine_t *line;

	if ( rb_numDebugLines < MAX_DEBUG_LINES ) {
		line = &rb_debugLines[ rb_numDebugLines++ ];
		line->rgb		= color;
		line->start		= start;
		line->end		= end;
		line->depthTest = depthTest;
		//#modified-fva; BEGIN
		//line->lifeTime	= rb_debugLineTime + lifeTime;
		if (lifeTime < 0) {
			line->lifeTime = -1;
		} else {
			line->lifeTime = rb_debugLineTime + lifeTime;
		}
		//#modified-fva; END
	}
}

/*
================
RB_ShowDebugLines
================
*/
//#modified-fva; BEGIN
/*
void RB_ShowDebugLines() {
	int			i;
	int			width;
	debugLine_t	*line;

	if ( !rb_numDebugLines ) {
		return;
	}

	// all lines are expressed in world coordinates
	RB_SimpleWorldSetup();

	globalImages->BindNull();

	width = r_debugLineWidth.GetInteger();
	if ( width < 1 ) {
		width = 1;
	} else if ( width > 10 ) {
		width = 10;
	}

	// draw lines
	qglLineWidth( width );

	if ( !r_debugLineDepthTest.GetBool() ) {
		GL_State( GLS_POLYMODE_LINE | GLS_DEPTHFUNC_ALWAYS );
	} else {
		GL_State( GLS_POLYMODE_LINE );
	}

	qglBegin( GL_LINES );

	line = rb_debugLines;
	for ( i = 0; i < rb_numDebugLines; i++, line++ ) {
		if ( !line->depthTest ) {
			qglColor3fv( line->rgb.ToFloatPtr() );
			qglVertex3fv( line->start.ToFloatPtr() );
			qglVertex3fv( line->end.ToFloatPtr() );
		}
	}
	qglEnd();

	if ( !r_debugLineDepthTest.GetBool() ) {
		GL_State( GLS_POLYMODE_LINE );
	}

	qglBegin( GL_LINES );

	line = rb_debugLines;
	for ( i = 0; i < rb_numDebugLines; i++, line++ ) {
		if ( line->depthTest ) {
			qglColor4fv( line->rgb.ToFloatPtr() );
			qglVertex3fv( line->start.ToFloatPtr() );
			qglVertex3fv( line->end.ToFloatPtr() );
		}
	}

	qglEnd();

	qglLineWidth( 1 );
	GL_State( GLS_DEFAULT );
}
*/
void RB_ShowDebugLines() {
	if (!common->CstBrtAreEnabled()) {
		return;
	}
	if (!rb_numDebugLines) {
		return;
	}

	for (int i = 0; i < rb_numDebugLines; ++i) {
		if (rb_debugLines[i].lifeTime < 0) {
			// draw this line now and clear it on the next game frame
			rb_debugLines[i].lifeTime = 0;
		}
	}

	// all lines are expressed in world coordinates
	RB_SimpleWorldSetup();

	int width = r_debugLineWidth.GetInteger();
	if (width < 1) {
		width = 1;
	} else if (width > 10) {
		width = 10;
	}
	qglLineWidth(width);

	const int maxBufferSize = 1 * 1024 * 1024;
	const int maxBufferVerts = maxBufferSize / sizeof(CstVertPosColor);

	GLuint vBuffer = 0;
	qglGenBuffersARB(1, &vBuffer);
	qglBindBufferARB(GL_ARRAY_BUFFER, vBuffer);
	backEnd.glState.currentVertexBuffer = 0; // mark as not set

	renderProgManager.BindShader_CstDebugPosColor();
	renderProgManager.CommitUniforms();

	for (int pass = 0; pass < 2; ++pass) {
		int numBufferVerts = 0;
		int firstLine = 0;
		bool drawNow = false;
		bool depthTestCondition = false;

		if (pass == 0) {
			// lines with depthTest == false
			if (!r_debugLineDepthTest.GetBool()) {
				GL_State(GLS_DEPTHFUNC_ALWAYS);
			} else {
				GL_State(GLS_DEFAULT);
			}
		} else {
			// lines with depthTest == true
			if (!r_debugLineDepthTest.GetBool()) {
				GL_State(GLS_DEFAULT);
			}
			depthTestCondition = true;
		}

		// draw
		for (int i = 0; i < rb_numDebugLines; ++i) {
			if (rb_debugLines[i].depthTest == depthTestCondition) {
				if (numBufferVerts + 2 > maxBufferVerts) {
					--i; // so the current line goes on the next buffer
					drawNow = true;
				} else {
					numBufferVerts += 2;
				}
				if (numBufferVerts > 0 && (drawNow || i == rb_numDebugLines - 1)) {
					qglBufferDataARB(GL_ARRAY_BUFFER, numBufferVerts * sizeof(CstVertPosColor), NULL, GL_STREAM_DRAW);
					CstVertPosColor * verts = (CstVertPosColor *)qglMapBufferARB(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
					if (!verts) {
						qglBindBufferARB(GL_ARRAY_BUFFER, 0);
						qglDeleteBuffersARB(1, &vBuffer);
						renderProgManager.Unbind();
						qglLineWidth(1);
						GL_State(GLS_DEFAULT);
						return;
					}
					int vertsBufferOffet = 0;
					for (int j = firstLine; j <= i; ++j) {
						debugLine_t	&line = rb_debugLines[j];
						if (line.depthTest == depthTestCondition) {
							CstVertPosColor v1, v2;
							v1.position = line.start;
							v2.position = line.end;
							v1.color = v2.color = line.rgb;
							verts[vertsBufferOffet++] = v1;
							verts[vertsBufferOffet++] = v2;
						}
					}
					qglUnmapBufferARB(GL_ARRAY_BUFFER);
					RB_Cst_SetupVertPosColorLayout();

					qglDrawArrays(GL_LINES, 0, numBufferVerts);

					numBufferVerts = 0;
					firstLine = i + 1;
					drawNow = false;
				}
			}
		}
	}

	qglBindBufferARB(GL_ARRAY_BUFFER, 0);
	qglDeleteBuffersARB(1, &vBuffer);
	renderProgManager.Unbind();
	qglLineWidth(1);
	GL_State(GLS_DEFAULT);
}
//#modified-fva; END

/*
================
RB_ClearDebugPolygons
================
*/
void RB_ClearDebugPolygons( int time ) {
	int				i;
	int				num;
	debugPolygon_t	*poly;

	rb_debugPolygonTime = time;

	if ( !time ) {
		rb_numDebugPolygons = 0;
		return;
	}

	// copy any polygons that still need to be drawn
	num	= 0;

	poly = rb_debugPolygons;
	for ( i = 0; i < rb_numDebugPolygons; i++, poly++ ) {
		//#modified-fva; BEGIN
		//if ( poly->lifeTime > time ) {
		if (poly->lifeTime < 0 || poly->lifeTime > time) {
		//#modified-fva; END
			if ( num != i ) {
				rb_debugPolygons[ num ] = *poly;
			}
			num++;
		}
	}
	rb_numDebugPolygons = num;
}

/*
================
RB_AddDebugPolygon
================
*/
void RB_AddDebugPolygon( const idVec4 &color, const idWinding &winding, const int lifeTime, const bool depthTest ) {
	//#modified-fva; BEGIN
	common->CstBrtRequestEnable();
	if (!common->CstBrtAreEnabled()) {
		return;
	}
	//#modified-fva; END
	debugPolygon_t *poly;

	if ( rb_numDebugPolygons < MAX_DEBUG_POLYGONS ) {
		poly = &rb_debugPolygons[ rb_numDebugPolygons++ ];
		poly->rgb		= color;
		poly->winding	= winding;
		poly->depthTest = depthTest;
		//#modified-fva; BEGIN
		//poly->lifeTime	= rb_debugPolygonTime + lifeTime;
		if (lifeTime < 0) {
			poly->lifeTime = -1;
		} else {
			poly->lifeTime = rb_debugPolygonTime + lifeTime;
		}
		//#modified-fva; END
	}
}

/*
================
RB_ShowDebugPolygons
================
*/
//#modified-fva; BEGIN
/*
void RB_ShowDebugPolygons() {
	int				i, j;
	debugPolygon_t	*poly;

	if ( !rb_numDebugPolygons ) {
		return;
	}

	// all lines are expressed in world coordinates
	RB_SimpleWorldSetup();

	globalImages->BindNull();

	qglDisable( GL_TEXTURE_2D );

	if ( r_debugPolygonFilled.GetBool() ) {
		GL_State( GLS_POLYGON_OFFSET | GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA | GLS_DEPTHMASK );
		GL_PolygonOffset( -1, -2 );
	} else {
		GL_State( GLS_POLYGON_OFFSET | GLS_POLYMODE_LINE );
		GL_PolygonOffset( -1, -2 );
	}

	poly = rb_debugPolygons;
	for ( i = 0; i < rb_numDebugPolygons; i++, poly++ ) {
//		if ( !poly->depthTest ) {

			qglColor4fv( poly->rgb.ToFloatPtr() );

			qglBegin( GL_POLYGON );

			for ( j = 0; j < poly->winding.GetNumPoints(); j++) {
				qglVertex3fv( poly->winding[j].ToFloatPtr() );
			}

			qglEnd();
//		}
	}

	GL_State( GLS_DEFAULT );

	if ( r_debugPolygonFilled.GetBool() ) {
		qglDisable( GL_POLYGON_OFFSET_FILL );
	} else {
		qglDisable( GL_POLYGON_OFFSET_LINE );
	}

	GL_State( GLS_DEFAULT );
}
*/
void RB_ShowDebugPolygons() {
	if (!common->CstBrtAreEnabled()) {
		return;
	}
	if (!rb_numDebugPolygons) {
		return;
	}

	for (int i = 0; i < rb_numDebugPolygons; ++i) {
		if (rb_debugPolygons[i].lifeTime < 0) {
			// draw this polygon now and clear it on the next game frame
			rb_debugPolygons[i].lifeTime = 0;
		}
	}

	RB_SimpleWorldSetup();

	const bool drawFilled = r_debugPolygonFilled.GetBool();
	if (!drawFilled) {
		int width = r_debugLineWidth.GetInteger();
		if (width < 1) {
			width = 1;
		} else if (width > 10) {
			width = 10;
		}
		qglLineWidth(width);
	}

	if (drawFilled) {
		GL_State(GLS_POLYGON_OFFSET | GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA | GLS_DEPTHMASK);
		GL_PolygonOffset(-1, -2);
	} else {
		if (!r_debugLineDepthTest.GetBool()) {
			GL_State(GLS_DEPTHFUNC_ALWAYS);
		} else {
			GL_State(GLS_DEFAULT);
		}
	}

	int maxBufferSize = 1 * 1024 * 1024;
	int maxBufferVerts = maxBufferSize / sizeof(CstVertPosColor);

	GLuint vBuffer = 0;
	qglGenBuffersARB(1, &vBuffer);
	qglBindBufferARB(GL_ARRAY_BUFFER, vBuffer);
	backEnd.glState.currentVertexBuffer = 0; // mark as not set

	renderProgManager.BindShader_CstDebugPosColor();
	renderProgManager.CommitUniforms();

	int numBufferVerts = 0;
	int firstPolygon = 0;
	for (int i = 0; i < rb_numDebugPolygons; ++i) {
		const int numPoints = rb_debugPolygons[i].winding.GetNumPoints();
		int numVertsPoly = 0;
		if (drawFilled) {
			numVertsPoly = (numPoints - 2) * 3; // num triangle verts
		} else {
			numVertsPoly = numPoints * 2; // num line verts
		}
		bool drawNow = false;
		if (numPoints < 3 || numVertsPoly > maxBufferVerts) {
			// the polygon has too few points or is too large to fit the buffer; remove it
			rb_debugPolygons[i].winding.Clear();
			numVertsPoly = 0;
		}
		if (numBufferVerts + numVertsPoly > maxBufferVerts) {
			--i; // so the current polygon goes on the next buffer
			drawNow = true;
		} else {
			numBufferVerts += numVertsPoly;
		}
		if (numBufferVerts > 0 && (drawNow || i == rb_numDebugPolygons - 1)) {
			qglBufferDataARB(GL_ARRAY_BUFFER, numBufferVerts * sizeof(CstVertPosColor), NULL, GL_STREAM_DRAW);
			CstVertPosColor * verts = (CstVertPosColor *)qglMapBufferARB(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
			if (!verts) {
				qglBindBufferARB(GL_ARRAY_BUFFER, 0);
				qglDeleteBuffersARB(1, &vBuffer);
				renderProgManager.Unbind();
				if (!drawFilled) {
					qglLineWidth(1);
				}
				GL_State(GLS_DEFAULT);
				return;
			}
			int vertsBufferOffet = 0;
			if (drawFilled) {
				// add windings as triangle fans
				for (int j = firstPolygon; j <= i; ++j) {
					debugPolygon_t &poly = rb_debugPolygons[j];
					for (int k = 1; k < poly.winding.GetNumPoints() - 1; ++k) {
						CstVertPosColor v1, v2, v3;
						v1.position = poly.winding[0].ToVec3();
						v2.position = poly.winding[k].ToVec3();
						v3.position = poly.winding[k + 1].ToVec3();
						v1.color = v2.color = v3.color = poly.rgb;
						verts[vertsBufferOffet++] = v1;
						verts[vertsBufferOffet++] = v2;
						verts[vertsBufferOffet++] = v3;
					}
				}
			} else {
				// add windings as lines
				for (int j = firstPolygon; j <= i; ++j) {
					debugPolygon_t &poly = rb_debugPolygons[j];
					int k = 0;
					for (; k < poly.winding.GetNumPoints(); ++k) {
						CstVertPosColor v1, v2;
						v1.position = poly.winding[k].ToVec3();
						if (k == poly.winding.GetNumPoints() - 1) {
							v2.position = poly.winding[0].ToVec3();
						} else {
							v2.position = poly.winding[k + 1].ToVec3();
						}
						v1.color = v2.color = poly.rgb;
						verts[vertsBufferOffet++] = v1;
						verts[vertsBufferOffet++] = v2;
					}
				}
			}
			qglUnmapBufferARB(GL_ARRAY_BUFFER);
			RB_Cst_SetupVertPosColorLayout();

			if (drawFilled) {
				qglDrawArrays(GL_TRIANGLES, 0, numBufferVerts);
			} else {
				qglDrawArrays(GL_LINES, 0, numBufferVerts);
			}

			numBufferVerts = 0;
			firstPolygon = i + 1;
			drawNow = false;
		}
	}

	qglBindBufferARB(GL_ARRAY_BUFFER, 0);
	qglDeleteBuffersARB(1, &vBuffer);
	renderProgManager.Unbind();
	if (!drawFilled) {
		qglLineWidth(1);
	}
	GL_State(GLS_DEFAULT);
}
//#modified-fva; END

/*
================
RB_ShowCenterOfProjection
================
*/
void RB_ShowCenterOfProjection() {
	if ( !r_showCenterOfProjection.GetBool() ) {
		return;
	}
	//#modified-fva; BEGIN
	common->CstBrtRequestEnable();
	if (!common->CstBrtAreEnabled()) {
		return;
	}
	//#modified-fva; END

	const int w = backEnd.viewDef->scissor.GetWidth();
	const int h = backEnd.viewDef->scissor.GetHeight();
	qglClearColor( 1, 0, 0, 1 );
	for ( float f = 0.0f ; f <= 1.0f ; f += 0.125f ) {
		qglScissor( w * f - 1 , 0, 3, h );
		qglClear( GL_COLOR_BUFFER_BIT );
		qglScissor( 0, h * f - 1 , w, 3 );
		qglClear( GL_COLOR_BUFFER_BIT );
	}
	qglClearColor( 0, 1, 0, 1 );
	float f = 0.5f;
	qglScissor( w * f - 1 , 0, 3, h );
	qglClear( GL_COLOR_BUFFER_BIT );
	qglScissor( 0, h * f - 1 , w, 3 );
	qglClear( GL_COLOR_BUFFER_BIT );

	//#modified-fva; BEGIN
	//qglScissor( 0, 0, w, h );
	GL_Scissor(backEnd.viewDef->viewport.x1 + backEnd.viewDef->scissor.x1,
		backEnd.viewDef->viewport.y1 + backEnd.viewDef->scissor.y1,
		backEnd.viewDef->scissor.x2 + 1 - backEnd.viewDef->scissor.x1,
		backEnd.viewDef->scissor.y2 + 1 - backEnd.viewDef->scissor.y1);
	backEnd.currentScissor = backEnd.viewDef->scissor;
	//#modified-fva; END
}

/*
================
RB_ShowLines

Draw exact pixel lines to check pixel center sampling
================
*/
//#modified-fva; BEGIN
/*
void RB_ShowLines() {
	if ( !r_showLines.GetBool() ) {
		return;
	}

	glEnable( GL_SCISSOR_TEST );
	if ( backEnd.viewDef->renderView.viewEyeBuffer == 0 ) {
		glClearColor( 1, 0, 0, 1 );
	} else if ( backEnd.viewDef->renderView.viewEyeBuffer == 1 ) {
		glClearColor( 0, 1, 0, 1 );
	} else {
		glClearColor( 0, 0, 1, 1 );
	}

	const int start = ( r_showLines.GetInteger() > 2 );	// 1,3 = horizontal, 2,4 = vertical
	if ( r_showLines.GetInteger() == 1 || r_showLines.GetInteger() == 3 ) {
		for ( int i = start ; i < tr.GetHeight() ; i+=2 ) {
			glScissor( 0, i, tr.GetWidth(), 1 );
			glClear( GL_COLOR_BUFFER_BIT );
		}
	} else {
		for ( int i = start ; i < tr.GetWidth() ; i+=2 ) {
			glScissor( i, 0, 1, tr.GetHeight() );
			glClear( GL_COLOR_BUFFER_BIT );
		}
	}
}
*/
void RB_ShowLines() {
	if (!r_showLines.GetBool()) {
		return;
	}
	common->CstBrtRequestEnable();
	if (!common->CstBrtAreEnabled()) {
		return;
	}

	int option = r_showLines.GetInteger();
	bool allowGuis = true;
	if (option < 0) {
		option = -option;
		allowGuis = false;
	}

	if (!allowGuis && backEnd.viewDef->is2Dgui) {
		return;
	}

	if (backEnd.viewDef->renderView.viewEyeBuffer == 0) {
		qglClearColor(1, 0, 0, 1);
	} else if (backEnd.viewDef->renderView.viewEyeBuffer == 1) {
		qglClearColor(0, 1, 0, 1);
	} else {
		qglClearColor(0, 0, 1, 1);
	}

	const int start = (option > 2);	// 1,3 = horizontal, 2,4 = vertical
	if (option == 1 || option == 3) {
		for (int i = start; i < tr.GetHeight(); i += 2) {
			qglScissor(0, i, tr.GetWidth(), 1);
			qglClear(GL_COLOR_BUFFER_BIT);
		}
	} else {
		for (int i = start; i < tr.GetWidth(); i += 2) {
			qglScissor(i, 0, 1, tr.GetHeight());
			qglClear(GL_COLOR_BUFFER_BIT);
		}
	}

	GL_Scissor(backEnd.viewDef->viewport.x1 + backEnd.viewDef->scissor.x1,
		backEnd.viewDef->viewport.y1 + backEnd.viewDef->scissor.y1,
		backEnd.viewDef->scissor.x2 + 1 - backEnd.viewDef->scissor.x1,
		backEnd.viewDef->scissor.y2 + 1 - backEnd.viewDef->scissor.y1);
	backEnd.currentScissor = backEnd.viewDef->scissor;
}
//#modified-fva; END


/*
================
RB_TestGamma
================
*/
#define	G_WIDTH		512
#define	G_HEIGHT	512
#define	BAR_HEIGHT	64

//#modified-fva; BEGIN
/*
void RB_TestGamma() {
	byte	image[G_HEIGHT][G_WIDTH][4];
	int		i, j;
	int		c, comp;
	int		v, dither;
	int		mask, y;

	if ( r_testGamma.GetInteger() <= 0 ) {
		return;
	}

	v = r_testGamma.GetInteger();
	if ( v <= 1 || v >= 196 ) {
		v = 128;
	}

	memset( image, 0, sizeof( image ) );

	for ( mask = 0; mask < 8; mask++ ) {
		y = mask * BAR_HEIGHT;
		for ( c = 0; c < 4; c++ ) {
			v = c * 64 + 32;
			// solid color
			for ( i = 0; i < BAR_HEIGHT/2; i++ ) {
				for ( j = 0; j < G_WIDTH/4; j++ ) {
					for ( comp = 0; comp < 3; comp++ ) {
						if ( mask & ( 1 << comp ) ) {
							image[y+i][c*G_WIDTH/4+j][comp] = v;
						}
					}
				}
				// dithered color
				for ( j = 0; j < G_WIDTH/4; j++ ) {
					if ( ( i ^ j ) & 1 ) {
						dither = c * 64;
					} else {
						dither = c * 64 + 63;
					}
					for ( comp = 0; comp < 3; comp++ ) {
						if ( mask & ( 1 << comp ) ) {
							image[y+BAR_HEIGHT/2+i][c*G_WIDTH/4+j][comp] = dither;
						}
					}
				}
			}
		}
	}

	// draw geometrically increasing steps in the bottom row
	y = 0 * BAR_HEIGHT;
	float	scale = 1;
	for ( c = 0; c < 4; c++ ) {
		v = (int)(64 * scale);
		if ( v < 0 ) {
			v = 0;
		} else if ( v > 255 ) {
			v = 255;
		}
		scale = scale * 1.5;
		for ( i = 0; i < BAR_HEIGHT; i++ ) {
			for ( j = 0; j < G_WIDTH/4; j++ ) {
				image[y+i][c*G_WIDTH/4+j][0] = v;
				image[y+i][c*G_WIDTH/4+j][1] = v;
				image[y+i][c*G_WIDTH/4+j][2] = v;
			}
		}
	}

	qglLoadIdentity();

	qglMatrixMode( GL_PROJECTION );
	GL_State( GLS_DEPTHFUNC_ALWAYS );
	GL_Color( 1, 1, 1 );
	qglPushMatrix();
	qglLoadIdentity();
	qglDisable( GL_TEXTURE_2D );
    qglOrtho( 0, 1, 0, 1, -1, 1 );
	qglRasterPos2f( 0.01f, 0.01f );
	qglDrawPixels( G_WIDTH, G_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, image );
	qglPopMatrix();
	qglEnable( GL_TEXTURE_2D );
	qglMatrixMode( GL_MODELVIEW );
}
*/
void RB_TestGamma() {
	byte	image[G_HEIGHT][G_WIDTH][4];
	int		i, j;
	int		c, comp;
	int		v, dither;
	int		mask, y;

	if (r_testGamma.GetInteger() <= 0) {
		return;
	}
	common->CstBrtRequestEnable();
	if (!common->CstBrtAreEnabled()) {
		return;
	}

	v = r_testGamma.GetInteger();
	if (v <= 1 || v >= 196) {
		v = 128;
	}

	memset(image, 0, sizeof(image));

	for (mask = 0; mask < 8; mask++) {
		y = mask * BAR_HEIGHT;
		for (c = 0; c < 4; c++) {
			v = c * 64 + 32;
			// solid color
			for (i = 0; i < BAR_HEIGHT / 2; i++) {
				for (j = 0; j < G_WIDTH / 4; j++) {
					for (comp = 0; comp < 3; comp++) {
						if (mask & (1 << comp)) {
							image[y + i][c * G_WIDTH / 4 + j][comp] = v;
						}
					}
				}
				// dithered color
				for (j = 0; j < G_WIDTH / 4; j++) {
					if ((i ^ j) & 1) {
						dither = c * 64;
					} else {
						dither = c * 64 + 63;
					}
					for (comp = 0; comp < 3; comp++) {
						if (mask & (1 << comp)) {
							image[y + BAR_HEIGHT / 2 + i][c * G_WIDTH / 4 + j][comp] = dither;
						}
					}
				}
			}
		}
	}

	// draw geometrically increasing steps in the bottom row
	y = 0 * BAR_HEIGHT;
	float	scale = 1;
	for (c = 0; c < 4; c++) {
		v = (int)(64 * scale);
		if (v < 0) {
			v = 0;
		} else if (v > 255) {
			v = 255;
		}
		scale = scale * 1.5;
		for (i = 0; i < BAR_HEIGHT; i++) {
			for (j = 0; j < G_WIDTH / 4; j++) {
				image[y + i][c * G_WIDTH / 4 + j][0] = v;
				image[y + i][c * G_WIDTH / 4 + j][1] = v;
				image[y + i][c * G_WIDTH / 4 + j][2] = v;
			}
		}
	}

	GLuint texId;
	qglGenTextures(1, &texId);
	qglBindMultiTextureEXT(GL_TEXTURE0_ARB, GL_TEXTURE_2D, texId);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, G_WIDTH, G_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	GL_State(GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO | GLS_DEPTHMASK | GLS_DEPTHFUNC_ALWAYS);
	GL_Cull(CT_TWO_SIDED);

	float texS[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
	float texT[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
	renderProgManager.SetRenderParm(RENDERPARM_TEXTUREMATRIX_S, texS);
	renderProgManager.SetRenderParm(RENDERPARM_TEXTUREMATRIX_T, texT);

	float texGenEnabled[4] = { 0, 0, 0, 0 };
	renderProgManager.SetRenderParm(RENDERPARM_TEXGEN_0_ENABLED, texGenEnabled);

	GL_Color(1.0f, 1.0f, 1.0f, 1.0f);

	float screenWidth = renderSystem->GetWidth();
	float screenHeight = renderSystem->GetHeight();
	float invScreenWidth = 1.0f / screenWidth;
	float invScreenHeight = 1.0f / screenHeight;
	float scaleX = G_WIDTH * invScreenWidth;
	float scaleY = G_HEIGHT * invScreenHeight;

	float offsetBaseY = 0.02f;
	float offsetBaseX = (offsetBaseY * screenHeight) * invScreenWidth;
	float offsetX = offsetBaseX - (screenWidth - G_WIDTH) * invScreenWidth;
	float offsetY = offsetBaseY - (screenHeight - G_HEIGHT) * invScreenHeight;

	idRenderMatrix mvp(
		scaleX, 0.0f, 0.0f, offsetX,
		0.0f, scaleY, 0.0f, offsetY,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	RB_Cst_Simple_MVP_Setup(mvp);
	renderProgManager.BindShader_Texture();

	RB_DrawElementsWithCounters(&backEnd.unitSquareSurface);

	qglBindMultiTextureEXT(GL_TEXTURE0_ARB, GL_TEXTURE_2D, 0);
	qglDeleteTextures(1, &texId);
	RB_Cst_ClearBackendTextureBindingCaches();
	renderProgManager.Unbind();
	GL_State(GLS_DEFAULT);
	GL_Cull(CT_FRONT_SIDED);
}
//#modified-fva; END


/*
==================
RB_TestGammaBias
==================
*/
//#modified-fva; BEGIN
/*
static void RB_TestGammaBias() {
	byte	image[G_HEIGHT][G_WIDTH][4];

	if ( r_testGammaBias.GetInteger() <= 0 ) {
		return;
	}

	int y = 0;
	for ( int bias = -40; bias < 40; bias+=10, y += BAR_HEIGHT ) {
		float	scale = 1;
		for ( int c = 0; c < 4; c++ ) {
			int v = (int)(64 * scale + bias);
			scale = scale * 1.5;
			if ( v < 0 ) {
				v = 0;
			} else if ( v > 255 ) {
				v = 255;
			}
			for ( int i = 0; i < BAR_HEIGHT; i++ ) {
				for ( int j = 0; j < G_WIDTH/4; j++ ) {
					image[y+i][c*G_WIDTH/4+j][0] = v;
					image[y+i][c*G_WIDTH/4+j][1] = v;
					image[y+i][c*G_WIDTH/4+j][2] = v;
				}
			}
		}
	}

	qglLoadIdentity();
	qglMatrixMode( GL_PROJECTION );
	GL_State( GLS_DEPTHFUNC_ALWAYS );
	GL_Color( 1, 1, 1 );
	qglPushMatrix();
	qglLoadIdentity();
	qglDisable( GL_TEXTURE_2D );
    qglOrtho( 0, 1, 0, 1, -1, 1 );
	qglRasterPos2f( 0.01f, 0.01f );
	qglDrawPixels( G_WIDTH, G_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, image );
	qglPopMatrix();
	qglEnable( GL_TEXTURE_2D );
	qglMatrixMode( GL_MODELVIEW );
}
*/
static void RB_TestGammaBias() {
	byte	image[G_HEIGHT][G_WIDTH][4];

	if (r_testGammaBias.GetInteger() <= 0) {
		return;
	}
	common->CstBrtRequestEnable();
	if (!common->CstBrtAreEnabled()) {
		return;
	}

	int y = 0;
	for (int bias = -40; bias < 40; bias += 10, y += BAR_HEIGHT) {
		float	scale = 1;
		for (int c = 0; c < 4; c++) {
			int v = (int)(64 * scale + bias);
			scale = scale * 1.5;
			if (v < 0) {
				v = 0;
			} else if (v > 255) {
				v = 255;
			}
			for (int i = 0; i < BAR_HEIGHT; i++) {
				for (int j = 0; j < G_WIDTH / 4; j++) {
					image[y + i][c * G_WIDTH / 4 + j][0] = v;
					image[y + i][c * G_WIDTH / 4 + j][1] = v;
					image[y + i][c * G_WIDTH / 4 + j][2] = v;
				}
			}
		}
	}

	GLuint texId;
	qglGenTextures(1, &texId);
	qglBindMultiTextureEXT(GL_TEXTURE0_ARB, GL_TEXTURE_2D, texId);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, G_WIDTH, G_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	GL_State(GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO | GLS_DEPTHMASK | GLS_DEPTHFUNC_ALWAYS);
	GL_Cull(CT_TWO_SIDED);

	float texS[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
	float texT[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
	renderProgManager.SetRenderParm(RENDERPARM_TEXTUREMATRIX_S, texS);
	renderProgManager.SetRenderParm(RENDERPARM_TEXTUREMATRIX_T, texT);

	float texGenEnabled[4] = { 0, 0, 0, 0 };
	renderProgManager.SetRenderParm(RENDERPARM_TEXGEN_0_ENABLED, texGenEnabled);

	GL_Color(1.0f, 1.0f, 1.0f, 1.0f);

	float screenWidth = renderSystem->GetWidth();
	float screenHeight = renderSystem->GetHeight();
	float invScreenWidth = 1.0f / screenWidth;
	float invScreenHeight = 1.0f / screenHeight;
	float scaleX = G_WIDTH * invScreenWidth;
	float scaleY = G_HEIGHT * invScreenHeight;

	float offsetBaseY = 0.02f;
	float offsetBaseX = (offsetBaseY * screenHeight) * invScreenWidth;
	float offsetX = offsetBaseX - (screenWidth - G_WIDTH) * invScreenWidth;
	float offsetY = offsetBaseY - (screenHeight - G_HEIGHT) * invScreenHeight;

	idRenderMatrix mvp(
		scaleX, 0.0f, 0.0f, offsetX,
		0.0f, scaleY, 0.0f, offsetY,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	RB_Cst_Simple_MVP_Setup(mvp);
	renderProgManager.BindShader_Texture();

	RB_DrawElementsWithCounters(&backEnd.unitSquareSurface);

	qglBindMultiTextureEXT(GL_TEXTURE0_ARB, GL_TEXTURE_2D, 0);
	qglDeleteTextures(1, &texId);
	RB_Cst_ClearBackendTextureBindingCaches();
	renderProgManager.Unbind();
	GL_State(GLS_DEFAULT);
	GL_Cull(CT_FRONT_SIDED);
}
//#modified-fva; END

/*
================
RB_TestImage

Display a single image over most of the screen
================
*/
//#modified-fva; BEGIN
/*
void RB_TestImage() {
	idImage	*image = NULL;
	idImage *imageCr = NULL;
	idImage *imageCb = NULL;
	int		max;
	float	w, h;

	image = tr.testImage;
	if ( !image ) {
		return;
	}

	if ( tr.testVideo ) {
		cinData_t	cin;

		cin = tr.testVideo->ImageForTime( backEnd.viewDef->renderView.time[1] - tr.testVideoStartTime );
		if ( cin.imageY != NULL ) {
			image = cin.imageY;
			imageCr = cin.imageCr;
			imageCb = cin.imageCb;
		} else {
			tr.testImage = NULL;
			return;
		}
		w = 0.25;
		h = 0.25;
	} else {
		max = image->GetUploadWidth() > image->GetUploadHeight() ? image->GetUploadWidth() : image->GetUploadHeight();

		w = 0.25 * image->GetUploadWidth() / max;
		h = 0.25 * image->GetUploadHeight() / max;

		w *= (float)renderSystem->GetHeight() / renderSystem->GetWidth();
	}

	// Set State
	GL_State( GLS_DEPTHFUNC_ALWAYS | GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO );

	// Set Parms
	float texS[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
	float texT[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
	renderProgManager.SetRenderParm( RENDERPARM_TEXTUREMATRIX_S, texS );
	renderProgManager.SetRenderParm( RENDERPARM_TEXTUREMATRIX_T, texT );

	float texGenEnabled[4] = { 0, 0, 0, 0 };
	renderProgManager.SetRenderParm( RENDERPARM_TEXGEN_0_ENABLED, texGenEnabled );

	// not really necessary but just for clarity
	const float screenWidth = 1.0f;
	const float screenHeight = 1.0f;
	const float halfScreenWidth = screenWidth * 0.5f;
	const float halfScreenHeight = screenHeight * 0.5f;

	float scale[16] = { 0 };
	scale[0] = w; // scale
	scale[5] = h; // scale
	scale[12] = halfScreenWidth - ( halfScreenWidth * w ); // translate
	scale[13] = halfScreenHeight - ( halfScreenHeight * h ); // translate
	scale[10] = 1.0f;
	scale[15] = 1.0f;

	float ortho[16] = { 0 };
	ortho[0] = 2.0f / screenWidth;
	ortho[5] = -2.0f / screenHeight;
	ortho[10] = -2.0f;
	ortho[12] = -1.0f;
	ortho[13] = 1.0f;
	ortho[14] = -1.0f;
	ortho[15] = 1.0f;

	float finalOrtho[16];
	R_MatrixMultiply( scale, ortho, finalOrtho );

	float projMatrixTranspose[16];
	R_MatrixTranspose( finalOrtho, projMatrixTranspose );
	renderProgManager.SetRenderParms( RENDERPARM_MVPMATRIX_X, projMatrixTranspose, 4 );
	qglMatrixMode( GL_PROJECTION );
	qglLoadMatrixf( finalOrtho );
	qglMatrixMode( GL_MODELVIEW );
	qglLoadIdentity();

	// Set Color
	GL_Color( 1, 1, 1, 1 );

	// Bind the Texture
	if ( ( imageCr != NULL ) && ( imageCb != NULL ) ) {
		GL_SelectTexture( 0 );
		image->Bind();
		GL_SelectTexture( 1 );
		imageCr->Bind();
		GL_SelectTexture( 2 );
		imageCb->Bind();
		renderProgManager.BindShader_Bink();
	} else {
		GL_SelectTexture( 0 );
		image->Bind();
		// Set Shader
		renderProgManager.BindShader_Texture();
	}

	// Draw!
	RB_DrawElementsWithCounters( &backEnd.testImageSurface );
}
*/
void RB_TestImage() {
	idImage	*image = NULL;
	idImage *imageCr = NULL;
	idImage *imageCb = NULL;

	image = tr.testImage;
	if (!image) {
		return;
	}

	if (tr.testVideo) {
		cinData_t cin = tr.testVideo->ImageForTime(backEnd.viewDef->renderView.time[1] - tr.testVideoStartTime);
		if (cin.imageY != NULL) {
			image = cin.imageY;
			imageCr = cin.imageCr;
			imageCb = cin.imageCb;
		} else {
			tr.testImage = NULL;
			return;
		}
	}

	common->CstBrtRequestEnable();
	if (!common->CstBrtAreEnabled()) {
		return;
	}

	// Set State
	GL_State(GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO | GLS_DEPTHMASK | GLS_DEPTHFUNC_ALWAYS);
	GL_Cull(CT_TWO_SIDED);

	// Set Parms
	float texS[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
	float texT[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
	renderProgManager.SetRenderParm(RENDERPARM_TEXTUREMATRIX_S, texS);
	renderProgManager.SetRenderParm(RENDERPARM_TEXTUREMATRIX_T, texT);

	float texGenEnabled[4] = { 0, 0, 0, 0 };
	renderProgManager.SetRenderParm(RENDERPARM_TEXGEN_0_ENABLED, texGenEnabled);

	float screenWidth = renderSystem->GetWidth();
	float screenHeight = renderSystem->GetHeight();
	float invScreenWidth = 1.0f / screenWidth;

	float imageWidth = (float)image->GetUploadWidth();
	float imageHeight = (float)image->GetUploadHeight();
	float invImageHeight = 1.0f / imageHeight;

	float scaleY = 0.25;
	float scaleX = ((scaleY * screenHeight) * imageWidth * invImageHeight) * invScreenWidth;
	if (scaleX > 1.0f) {
		scaleX = 1.0f; // make the image fit the screen if it would be cropped; this changes proportions, though
	}
	scaleY = -scaleY;

	idRenderMatrix mvp(
		scaleX, 0.0f, 0.0f, 0.0f,
		0.0f, scaleY, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	RB_Cst_Simple_MVP_Setup(mvp);

	// Set Color
	GL_Color(1.0f, 1.0f, 1.0f, 1.0f);

	// Bind the Texture
	if ((imageCr != NULL) && (imageCb != NULL)) {
		GL_SelectTexture(0);
		image->Bind();
		GL_SelectTexture(1);
		imageCr->Bind();
		GL_SelectTexture(2);
		imageCb->Bind();
		renderProgManager.BindShader_Bink();
	} else {
		GL_SelectTexture(0);
		image->Bind();
		// Set Shader
		renderProgManager.BindShader_Texture();
	}

	// Draw!
	RB_DrawElementsWithCounters(&backEnd.unitSquareSurface);

	renderProgManager.Unbind();
	GL_State(GLS_DEFAULT);
	GL_Cull(CT_FRONT_SIDED);
}
//#modified-fva; END

/*
=================
RB_DrawExpandedTriangles
=================
*/
//#modified-fva; BEGIN
/*
void RB_DrawExpandedTriangles( const srfTriangles_t *tri, const float radius, const idVec3 &vieworg ) {
	int i, j, k;
	idVec3 dir[6], normal, point;

	for ( i = 0; i < tri->numIndexes; i += 3 ) {

		idVec3 p[3] = { tri->verts[ tri->indexes[ i + 0 ] ].xyz, tri->verts[ tri->indexes[ i + 1 ] ].xyz, tri->verts[ tri->indexes[ i + 2 ] ].xyz };

		dir[0] = p[0] - p[1];
		dir[1] = p[1] - p[2];
		dir[2] = p[2] - p[0];

		normal = dir[0].Cross( dir[1] );

		if ( normal * p[0] < normal * vieworg ) {
			continue;
		}

		dir[0] = normal.Cross( dir[0] );
		dir[1] = normal.Cross( dir[1] );
		dir[2] = normal.Cross( dir[2] );

		dir[0].Normalize();
		dir[1].Normalize();
		dir[2].Normalize();

		qglBegin( GL_LINE_LOOP );

		for ( j = 0; j < 3; j++ ) {
			k = ( j + 1 ) % 3;

			dir[4] = ( dir[j] + dir[k] ) * 0.5f;
			dir[4].Normalize();

			dir[3] = ( dir[j] + dir[4] ) * 0.5f;
			dir[3].Normalize();

			dir[5] = ( dir[4] + dir[k] ) * 0.5f;
			dir[5].Normalize();

			point = p[k] + dir[j] * radius;
			qglVertex3f( point[0], point[1], point[2] );

			point = p[k] + dir[3] * radius;
			qglVertex3f( point[0], point[1], point[2] );

			point = p[k] + dir[4] * radius;
			qglVertex3f( point[0], point[1], point[2] );

			point = p[k] + dir[5] * radius;
			qglVertex3f( point[0], point[1], point[2] );

			point = p[k] + dir[k] * radius;
			qglVertex3f( point[0], point[1], point[2] );
		}

		qglEnd();
	}
}
*/
//#modified-fva; END

/*
================
RB_ShowTrace

Debug visualization

FIXME: not thread safe!
================
*/
//#modified-fva; BEGIN
/*
void RB_ShowTrace( drawSurf_t **drawSurfs, int numDrawSurfs ) {
	int						i;
	const srfTriangles_t	*tri;
	const drawSurf_t		*surf;
	idVec3					start, end;
	idVec3					localStart, localEnd;
	localTrace_t			hit;
	float					radius;

	if ( r_showTrace.GetInteger() == 0 ) {
		return;
	}

	if ( r_showTrace.GetInteger() == 2 ) {
		radius = 5.0f;
	} else {
		radius = 0.0f;
	}

	// determine the points of the trace
	start = backEnd.viewDef->renderView.vieworg;
	end = start + 4000 * backEnd.viewDef->renderView.viewaxis[0];

	// check and draw the surfaces
	globalImages->whiteImage->Bind();

	// find how many are ambient
	for ( i = 0; i < numDrawSurfs; i++ ) {
		surf = drawSurfs[i];
		tri = surf->frontEndGeo;

		if ( tri == NULL || tri->verts == NULL ) {
			continue;
		}

		// transform the points into local space
		R_GlobalPointToLocal( surf->space->modelMatrix, start, localStart );
		R_GlobalPointToLocal( surf->space->modelMatrix, end, localEnd );

		// check the bounding box
		if ( !tri->bounds.Expand( radius ).LineIntersection( localStart, localEnd ) ) {
			continue;
		}

		qglLoadMatrixf( surf->space->modelViewMatrix );

		// highlight the surface
		GL_State( GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );

		GL_Color( 1, 0, 0, 0.25 );
		RB_DrawElementsWithCounters( surf );

		// draw the bounding box
		GL_State( GLS_DEPTHFUNC_ALWAYS );

		GL_Color( 1, 1, 1, 1 );
		RB_DrawBounds( tri->bounds );

		if ( radius != 0.0f ) {
			// draw the expanded triangles
			GL_Color( 0.5f, 0.5f, 1.0f, 1.0f );
			RB_DrawExpandedTriangles( tri, radius, localStart );
		}

		// check the exact surfaces
		hit = R_LocalTrace( localStart, localEnd, radius, tri );
		if ( hit.fraction < 1.0 ) {
			GL_Color( 1, 1, 1, 1 );
			RB_DrawBounds( idBounds( hit.point ).Expand( 1 ) );
		}
	}
}
*/
void RB_ShowTrace(drawSurf_t **drawSurfs, int numDrawSurfs) {
	if (r_showTrace.GetInteger() == 0) {
		return;
	}
	common->CstBrtRequestEnable();
	if (!common->CstBrtAreEnabled()) {
		return;
	}

	float radius = 0.0f;
	if (r_showTrace.GetInteger() == 2) {
		radius = 5.0f;
	}

	int width = r_debugLineWidth.GetInteger();
	if (width < 1) {
		width = 1;
	} else if (width > 10) {
		width = 10;
	}
	qglLineWidth(width);

	GLuint vBuffer = 0;
	qglGenBuffersARB(1, &vBuffer);

	// determine the points of the trace
	idVec3 start = backEnd.viewDef->renderView.vieworg;
	idVec3 end = start + 4000 * backEnd.viewDef->renderView.viewaxis[0];

	// check and draw the surfaces
	for (int i = 0; i < numDrawSurfs; ++i) {
		const drawSurf_t * surf = drawSurfs[i];

		const srfTriangles_t * tri = surf->frontEndGeo;
		if (!tri || !tri->verts) {
			continue;
		}
		if (tri->numVerts == 0 || tri->numIndexes == 0) {
			continue;
		}

		// transform the points into local space
		idVec3 localStart, localEnd;
		R_GlobalPointToLocal(surf->space->modelMatrix, start, localStart);
		R_GlobalPointToLocal(surf->space->modelMatrix, end, localEnd);

		// check the bounding box
		if (!tri->bounds.Expand(radius).LineIntersection(localStart, localEnd)) {
			continue;
		}

		// highlight the surface
		GL_State(GLS_DEPTHFUNC_EQUAL | GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);

		GL_Color(1.0f, 0.0f, 0.0f, 0.25f);
		RB_SimpleSurfaceSetup(surf);
		if (surf->jointCache) {
			renderProgManager.BindShader_CstDebugColorSkinned();
		} else {
			renderProgManager.BindShader_Color();
		}
		RB_DrawElementsWithCounters(surf);

		//------------
		const int maxBuffVerts = 24 + (radius != 0.0f ? tri->numIndexes * 5 : 0) + 24; // worst case: [bounding box: 24] + [expanded triangles: (tri->numIndexes / 3) * 3 * 5] + [bounding box: 24]

		qglBindBufferARB(GL_ARRAY_BUFFER, vBuffer);
		backEnd.glState.currentVertexBuffer = 0; // mark as not set

		qglBufferDataARB(GL_ARRAY_BUFFER, maxBuffVerts * sizeof(CstVertPosColor), NULL, GL_STREAM_DRAW);
		CstVertPosColor * vertsPosColor = (CstVertPosColor *)qglMapBufferARB(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		if (!vertsPosColor) {
			qglBindBufferARB(GL_ARRAY_BUFFER, 0);
			qglDeleteBuffersARB(1, &vBuffer);
			renderProgManager.Unbind();
			qglLineWidth(1);
			GL_State(GLS_DEFAULT);
			return;
		}

		// add the bounding box to the buffer
		int vertsBufferOffset = 0;
		idVec4 color;

		color.Set(1.0f, 1.0f, 1.0f, 1.0f);
		vertsBufferOffset = RB_Cst_AddBoundsToBuffer(tri->bounds, vertsPosColor, vertsBufferOffset, color);

		const int boundingBoxOffset = 0;
		const int boundingBoxCount = vertsBufferOffset;

		// add the expanded triangles to the buffer
		if (radius != 0.0f) {
			color.Set(0.5f, 0.5f, 1.0f, 1.0f);
			vertsBufferOffset = RB_Cst_AddExpandedTrianglesToBuffer(tri, radius, localStart, vertsPosColor, vertsBufferOffset, color);
		}
		const int expandedTrianglesOffset = boundingBoxOffset + boundingBoxCount;
		const int expandedTrianglesCount = vertsBufferOffset - expandedTrianglesOffset;

		// check the exact surfaces
		localTrace_t hit = R_LocalTrace(localStart, localEnd, radius, tri);
		if (hit.fraction < 1.0) {
			color.Set(1.0f, 1.0f, 1.0f, 1.0f);
			vertsBufferOffset = RB_Cst_AddBoundsToBuffer(idBounds(hit.point).Expand(1), vertsPosColor, vertsBufferOffset, color);
		}
		const int hitBoxOffset = expandedTrianglesOffset + expandedTrianglesCount;
		const int hitBoxCount = vertsBufferOffset - hitBoxOffset;

		// draw
		qglUnmapBufferARB(GL_ARRAY_BUFFER);
		RB_Cst_SetupVertPosColorLayout();

		RB_Cst_SetupViewDefScissor();
		renderProgManager.BindShader_CstDebugPosColor();
		renderProgManager.CommitUniforms();

		GL_State(GLS_DEPTHFUNC_ALWAYS);

		qglDrawArrays(GL_LINES, boundingBoxOffset, boundingBoxCount);
		if (expandedTrianglesCount > 0) {
			qglDrawArrays(GL_LINE_LOOP, expandedTrianglesOffset, expandedTrianglesCount);
		}
		if (hitBoxCount > 0) {
			qglDrawArrays(GL_LINES, hitBoxOffset, hitBoxCount);
		}
	}

	qglBindBufferARB(GL_ARRAY_BUFFER, 0);
	backEnd.glState.currentVertexBuffer = 0; // mark as not set

	qglDeleteBuffersARB(1, &vBuffer);
	renderProgManager.Unbind();
	qglLineWidth(1);
	GL_State(GLS_DEFAULT);
}
//#modified-fva; END

/*
=================
RB_RenderDebugTools
=================
*/
void RB_RenderDebugTools( drawSurf_t **drawSurfs, int numDrawSurfs ) {
	// don't do much if this was a 2D rendering
	if ( !backEnd.viewDef->viewEntitys ) {
		RB_TestImage();
		RB_ShowLines();
		return;
	}

	renderLog.OpenMainBlock( MRB_DRAW_DEBUG_TOOLS );
	RENDERLOG_PRINTF( "---------- RB_RenderDebugTools ----------\n" );

	GL_State( GLS_DEFAULT );

	GL_Scissor( backEnd.viewDef->viewport.x1 + backEnd.viewDef->scissor.x1,
				backEnd.viewDef->viewport.y1 + backEnd.viewDef->scissor.y1,
				backEnd.viewDef->scissor.x2 + 1 - backEnd.viewDef->scissor.x1,
				backEnd.viewDef->scissor.y2 + 1 - backEnd.viewDef->scissor.y1 );
	backEnd.currentScissor = backEnd.viewDef->scissor;

	RB_ShowLightCount();
	RB_ShowTexturePolarity( drawSurfs, numDrawSurfs );
	RB_ShowTangentSpace( drawSurfs, numDrawSurfs );
	RB_ShowVertexColor( drawSurfs, numDrawSurfs );
	RB_ShowTris( drawSurfs, numDrawSurfs );
	RB_ShowUnsmoothedTangents( drawSurfs, numDrawSurfs );
	RB_ShowSurfaceInfo( drawSurfs, numDrawSurfs );
	RB_ShowEdges( drawSurfs, numDrawSurfs );
	RB_ShowNormals( drawSurfs, numDrawSurfs );
	RB_ShowViewEntitys( backEnd.viewDef->viewEntitys );
	RB_ShowLights();
	RB_ShowTextureVectors( drawSurfs, numDrawSurfs );
	RB_ShowDominantTris( drawSurfs, numDrawSurfs );
	if ( r_testGamma.GetInteger() > 0 ) {	// test here so stack check isn't so damn slow on debug builds
		RB_TestGamma();
	}
	if ( r_testGammaBias.GetInteger() > 0 ) {
		RB_TestGammaBias();
	}
	RB_TestImage();
	RB_ShowPortals();
	RB_ShowSilhouette();
	RB_ShowDepthBuffer();
	RB_ShowIntensity();
	RB_ShowCenterOfProjection();
	RB_ShowLines();
	RB_ShowDebugLines();
	RB_ShowDebugText();
	RB_ShowDebugPolygons();
	RB_ShowTrace( drawSurfs, numDrawSurfs );
	//#modified-fva; BEGIN
	RB_ShowOverdraw(); // moved from RB_DrawView
	//#modified-fva; END

	renderLog.CloseMainBlock();
}

/*
=================
RB_ShutdownDebugTools
=================
*/
void RB_ShutdownDebugTools() {
	for ( int i = 0; i < MAX_DEBUG_POLYGONS; i++ ) {
		rb_debugPolygons[i].winding.Clear();
	}
}
