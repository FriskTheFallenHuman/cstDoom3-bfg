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


uniform vec4 _fa_[1];
uniform sampler2D samp0;

in vec2 vofi_TexCoord0;

out vec4 cst_FragColor;

void main() {
	vec4 outColor = vec4 ( 0.0 , 0.0 , 0.0 , 1.0 ) ;
	vec2 tCoords = vofi_TexCoord0 ;
	if ( _fa_[0 /* rpUser0 */] . x == 0.0 ) {
		vec4 depthColor = tex2D ( samp0 , tCoords ) ;
		outColor = vec4 ( depthColor. x , depthColor. y , depthColor. z , 1.0 ) ;
	} else {
		float depthBuff = tex2D ( samp0 , tCoords ). x ;
		float depthLimit = 0.999 ;
		float depthBuffLimited = clamp ( depthBuff , 0.0 , depthLimit ) ;
		float maxDepthEye = 1.0 / ( depthLimit - 1.0 ) ;
		float depthEyeFactor1 = ( maxDepthEye + 1.0 ) ;
		float depthEyeTerm1 = 1.0 / ( ( depthBuffLimited - 1.0 ) * depthEyeFactor1 ) ;
		float depthEyeTerm2 = 1.0 / depthEyeFactor1 ;
		float depthEye = depthEyeTerm1 + depthEyeTerm2 ;
		float minIntensity = 0.1 ;
		float cyan = ( depthBuff - depthLimit ) / ( 1.0 - depthLimit ) ;
		cyan = cyan * ( 1.0 - minIntensity ) + minIntensity ;
		float selectorCyan = float ( depthBuff > depthLimit ) ;
		if ( _fa_[0 /* rpUser0 */] . x == 1.0 ) {
			float minRed = 0.0 ;
			float minGreen = 0.1 ;
			float minBlue = 0.3 ;
			float maxRed = minGreen ;
			float maxGreen = minBlue ;
			float maxBlue = 1.0 ;
			float selectorRed = float ( depthEye >= minRed && depthEye < maxRed ) ;
			float selectorGreen = float ( depthEye >= minGreen && depthEye < maxGreen ) ;
			float selectorBlue = float ( depthEye >= minBlue && depthEye <= maxBlue ) ;
			selectorRed *= ( 1.0 - selectorCyan ) ;
			selectorGreen *= ( 1.0 - selectorCyan ) ;
			selectorBlue *= ( 1.0 - selectorCyan ) ;
			float red = ( depthEye - minRed ) / ( maxRed - minRed ) ;
			red = red * ( 1.0 - minIntensity ) + minIntensity ;
			red *= selectorRed ;
			float green = ( depthEye - minGreen ) / ( maxGreen - minGreen ) ;
			green = green * ( 1.0 - minIntensity ) + minIntensity ;
			green = green * selectorGreen + cyan * selectorCyan ;
			float blue = ( depthEye - minBlue ) / ( maxBlue - minBlue ) ;
			blue = blue * ( 1.0 - minIntensity ) + minIntensity ;
			blue = blue * selectorBlue + cyan * selectorCyan ;
			outColor = vec4 ( red , green , blue , 1.0 ) ;
		} else {
			float red = depthEye * ( 1.0 - selectorCyan ) ;
			float green = depthEye * ( 1.0 - selectorCyan ) + cyan * selectorCyan ;
			float blue = depthEye * ( 1.0 - selectorCyan ) + cyan * selectorCyan ;
			outColor = vec4 ( red , green , blue , 1.0 ) ;
		}
	}
	cst_FragColor = outColor ;
}
