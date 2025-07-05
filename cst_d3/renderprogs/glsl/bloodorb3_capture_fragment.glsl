#version 150
#define PC

void clip( float v ) { if ( v < 0.0 ) { discard; } }
void clip( vec2 v ) { if ( any( lessThan( v, vec2( 0.0 ) ) ) ) { discard; } }
void clip( vec3 v ) { if ( any( lessThan( v, vec3( 0.0 ) ) ) ) { discard; } }
void clip( vec4 v ) { if ( any( lessThan( v, vec4( 0.0 ) ) ) ) { discard; } }

float saturate( float v ) { return clamp( v, 0.0, 1.0 ); }
vec2 saturate( vec2 v ) { return clamp( v, 0.0, 1.0 ); }
vec3 saturate( vec3 v ) { return clamp( v, 0.0, 1.0 ); }
vec4 saturate( vec4 v ) { return clamp( v, 0.0, 1.0 ); }

vec4 tex2D( sampler2D sampler, vec2 texcoord ) { return texture( sampler, texcoord.xy ); }
vec4 tex2D( sampler2DShadow sampler, vec3 texcoord ) { return vec4( texture( sampler, texcoord.xyz ) ); }

vec4 tex2D( sampler2D sampler, vec2 texcoord, vec2 dx, vec2 dy ) { return textureGrad( sampler, texcoord.xy, dx, dy ); }
vec4 tex2D( sampler2DShadow sampler, vec3 texcoord, vec2 dx, vec2 dy ) { return vec4( textureGrad( sampler, texcoord.xyz, dx, dy ) ); }

vec4 texCUBE( samplerCube sampler, vec3 texcoord ) { return texture( sampler, texcoord.xyz ); }
vec4 texCUBE( samplerCubeShadow sampler, vec4 texcoord ) { return vec4( texture( sampler, texcoord.xyzw ) ); }

vec4 tex1Dproj( sampler1D sampler, vec2 texcoord ) { return textureProj( sampler, texcoord ); }
vec4 tex2Dproj( sampler2D sampler, vec3 texcoord ) { return textureProj( sampler, texcoord ); }
vec4 tex3Dproj( sampler3D sampler, vec4 texcoord ) { return textureProj( sampler, texcoord ); }

vec4 tex1Dbias( sampler1D sampler, vec4 texcoord ) { return texture( sampler, texcoord.x, texcoord.w ); }
vec4 tex2Dbias( sampler2D sampler, vec4 texcoord ) { return texture( sampler, texcoord.xy, texcoord.w ); }
vec4 tex3Dbias( sampler3D sampler, vec4 texcoord ) { return texture( sampler, texcoord.xyz, texcoord.w ); }
vec4 texCUBEbias( samplerCube sampler, vec4 texcoord ) { return texture( sampler, texcoord.xyz, texcoord.w ); }

vec4 tex1Dlod( sampler1D sampler, vec4 texcoord ) { return textureLod( sampler, texcoord.x, texcoord.w ); }
vec4 tex2Dlod( sampler2D sampler, vec4 texcoord ) { return textureLod( sampler, texcoord.xy, texcoord.w ); }
vec4 tex3Dlod( sampler3D sampler, vec4 texcoord ) { return textureLod( sampler, texcoord.xyz, texcoord.w ); }
vec4 texCUBElod( samplerCube sampler, vec4 texcoord ) { return textureLod( sampler, texcoord.xyz, texcoord.w ); }

uniform sampler2D samp0;
uniform sampler2D samp1;
uniform sampler2D samp2;

in vec2 vofi_TexCoord0;
in vec2 vofi_TexCoord1;
in vec2 vofi_TexCoord2;
in vec2 vofi_TexCoord3;
in vec2 vofi_TexCoord4;

out vec4 cst_FragColor;

void main() {
	float colorFactor = vofi_TexCoord4 . x ;
	vec4 color0 = vec4 ( 1.0 - colorFactor , 1.0 - colorFactor , 1.0 , 1.0 ) ;
	vec4 color1 = vec4 ( 1.0 , 0.95 - colorFactor , 0.95 , 0.5 ) ;
	vec4 color2 = vec4 ( 0.015 , 0.015 , 0.015 , 0.01 ) ;
	vec4 accumSample0 = tex2D ( samp0 , vofi_TexCoord0 ) * color0 ;
	vec4 accumSample1 = tex2D ( samp0 , vofi_TexCoord1 ) * color1 ;
	vec4 accumSample2 = tex2D ( samp0 , vofi_TexCoord2 ) * color2 ;
	vec4 maskSample = tex2D ( samp2 , vofi_TexCoord3 ) ;
	vec4 tint = vec4 ( 0.8 , 0.5 , 0.5 , 1 ) ;
	vec4 currentRenderSample = tex2D ( samp1 , vofi_TexCoord3 ) * tint ;
	vec4 accumColor = mix ( accumSample0 , accumSample1 , 0.5 ) ;
	accumColor += accumSample2 ;
	accumColor = mix ( accumColor , currentRenderSample , maskSample. a ) ;
	cst_FragColor = accumColor ;
}
