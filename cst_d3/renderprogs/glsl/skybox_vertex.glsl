#version 150
#define PC

float saturate( float v ) { return clamp( v, 0.0, 1.0 ); }
vec2 saturate( vec2 v ) { return clamp( v, 0.0, 1.0 ); }
vec3 saturate( vec3 v ) { return clamp( v, 0.0, 1.0 ); }
vec4 saturate( vec4 v ) { return clamp( v, 0.0, 1.0 ); }
vec4 tex2Dlod( sampler2D sampler, vec4 texcoord ) { return textureLod( sampler, texcoord.xy, texcoord.w ); }


uniform vec4 _va_[7];

float dot4 (vec4 a , vec4 b ) {return dot ( a , b ) ; }
float dot4 (vec2 a , vec4 b ) {return dot ( vec4 ( a , 0 , 1 ) , b ) ; }
vec4 swizzleColor (vec4 c ) {return c ; }

in vec4 in_Position;
in vec2 in_TexCoord;
in vec4 in_Normal;
in vec4 in_Tangent;
in vec4 in_Color;

out vec3 vofi_TexCoord0;
out vec4 cst_Color;

void main() {
	gl_Position . x = dot4 ( in_Position , _va_[3 /* rpMVPmatrixX */] ) ;
	gl_Position . y = dot4 ( in_Position , _va_[4 /* rpMVPmatrixY */] ) ;
	gl_Position . z = dot4 ( in_Position , _va_[5 /* rpMVPmatrixZ */] ) ;
	gl_Position . w = dot4 ( in_Position , _va_[6 /* rpMVPmatrixW */] ) ;
	vofi_TexCoord0 = in_Position . xyz - _va_[0 /* rpLocalViewOrigin */] . xyz ;
	cst_Color = ( swizzleColor ( in_Color ) * _va_[1 /* rpVertexColorModulate */] ) + _va_[2 /* rpVertexColorAdd */] ;
}
