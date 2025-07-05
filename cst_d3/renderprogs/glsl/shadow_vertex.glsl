#version 150
#define PC

float saturate( float v ) { return clamp( v, 0.0, 1.0 ); }
vec2 saturate( vec2 v ) { return clamp( v, 0.0, 1.0 ); }
vec3 saturate( vec3 v ) { return clamp( v, 0.0, 1.0 ); }
vec4 saturate( vec4 v ) { return clamp( v, 0.0, 1.0 ); }
vec4 tex2Dlod( sampler2D sampler, vec4 texcoord ) { return textureLod( sampler, texcoord.xy, texcoord.w ); }


uniform vec4 _va_[5];

float dot4 (vec4 a , vec4 b ) {return dot ( a , b ) ; }
float dot4 (vec2 a , vec4 b ) {return dot ( vec4 ( a , 0 , 1 ) , b ) ; }

in vec4 in_Position;


void main() {
	vec4 vPos = in_Position - _va_[0 /* rpLocalLightOrigin */] ;
	vPos = ( vPos. wwww * _va_[0 /* rpLocalLightOrigin */] ) + vPos ;
	gl_Position . x = dot4 ( vPos , _va_[1 /* rpMVPmatrixX */] ) ;
	gl_Position . y = dot4 ( vPos , _va_[2 /* rpMVPmatrixY */] ) ;
	gl_Position . z = dot4 ( vPos , _va_[3 /* rpMVPmatrixZ */] ) ;
	gl_Position . w = dot4 ( vPos , _va_[4 /* rpMVPmatrixW */] ) ;
}
