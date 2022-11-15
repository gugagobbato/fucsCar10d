#version 400

out vec4 fragColor;

#ifdef GL_ES
precision mediump float;
#endif

uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iTime;                 // shader playback time (in seconds)
uniform float     iTimeDelta;            // render time (in seconds)
uniform vec4      iMouse;
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform sampler2D iChannel2;
uniform sampler2D iChannel3;
uniform int iFrame;

layout(location = 0) out vec4 color;

#define VAPORWAVE
#define speed 5.
#define wave_thing


// Funções pra desenhar o fundo, pegadas em https://www.shadertoy.com/view/tsScRK

vec4 textureMirror(sampler2D tex, vec2 c){
    return vec4(0);
}

float jTime;

float amp(vec2 p){
    return smoothstep(1.,8.,abs(p.x));
}

float pow512(float a){
    a*=a;//^2
    a*=a;//^4
    a*=a;//^8
    a*=a;//^16
    a*=a;//^32
    a*=a;//^64
    a*=a;//^128
    a*=a;//^256
    return a*a;
}

float pow1d5(float a){
    return a*sqrt(a);
}

float hash21(vec2 co){
    return fract(sin(dot(co.xy,vec2(1.9898,7.233)))*45758.5433);
}

float hash(vec2 uv){
    float a = amp(uv);
    #ifdef wave_thing
    float w = a>0.?(1.-.4*pow512(.51+.49*sin((.02*(uv.y+.5*uv.x)-jTime)*2.))):0.;
    #else
    float w=1.;
    #endif
    return (a>0.?
    a*pow1d5(hash21(uv))*w:0.)-(textureMirror(iChannel0,vec2((uv.x*29.+uv.y)*.03125,1.)).x)*.25;
}

float edgeMin(float dx,vec2 da, vec2 db,vec2 uv){
    uv.x+=5.;
    vec3 c = fract((round(vec3(uv,uv.x+uv.y)))*(vec3(0,1,2)+0.61803398875));
    float a1 = textureMirror(iChannel0,vec2(c.y,0.)).x>.6?.15:1.;
    float a2 = textureMirror(iChannel0,vec2(c.x,0.)).x>.6?.15:1.;
    float a3 = textureMirror(iChannel0,vec2(c.z,0.)).x>.6?.15:1.;

    return min(min((1.-dx)*db.y*a3,da.x*a2),da.y*a1);
}

vec2 trinoise(vec2 uv){
    const float sq = sqrt(3./2.);
    uv.x *= sq;
    uv.y -= .5*uv.x;
    vec2 d = fract(uv);
    uv -= d;

    bool c = dot(d,vec2(1))>1.;

    vec2 dd = 1.-d;
    vec2 da = c?dd:d,db = c?d:dd;

    float nn = hash(uv+float(c));
    float n2 = hash(uv+vec2(1,0));
    float n3 = hash(uv+vec2(0,1));

    float nmid = mix(n2,n3,d.y);
    float ns = mix(nn,c?n2:n3,da.y);
    float dx = da.x/db.y;
    return vec2(mix(ns,nmid,dx),edgeMin(dx,da, db,uv+d));
}

vec2 map(vec3 p){
    vec2 n = trinoise(p.xz);
    return vec2(p.y-2.*n.x,n.y);
}

vec3 grad(vec3 p){
    const vec2 e = vec2(.005,0);
    float a =map(p).x;
    return vec3(map(p+e.xyy).x-a
                ,map(p+e.yxy).x-a
                ,map(p+e.yyx).x-a)/e.x;
}

vec2 intersect(vec3 ro,vec3 rd){
    float d =0.,h=0.;
    for(int i = 0;i<300;i++){
        vec3 p = ro+d*rd;
        vec2 s = map(p);
        h = s.x;
        d+= h*.5;
        if(abs(h)<.003*d)
            return vec2(d,s.y);
        if(d>150.|| p.y>2.) break;
    }

    return vec2(-1);
}

void addsun(vec3 rd,vec3 ld,inout vec3 col){
	float sun = smoothstep(.21,.2,distance(rd,ld));

    if(sun>0.){
        float yd = (rd.y-ld.y);

        float a =sin(3.1*exp(-(yd)*14.));

        sun*=smoothstep(-.8,0.,a);

        col = mix(col,vec3(1.,.8,.4)*.75,sun);
    }
}

float starnoise(vec3 rd){
    float c = 0.;
    vec3 p = normalize(rd)*300.;
    for (float i=0.;i<4.;i++)
    {
        vec3 q = fract(p)-.5;
        vec3 id = floor(p);
        float c2 = smoothstep(.5,0.,length(q));
        c2 *= step(hash21(id.xz/id.y),.06-i*i*0.005);
        c += c2;
        p = p*.6+.5*p*mat3(3./5.,0,4./5.,0,1,0,-4./5.,0,3./5.);
    }
    c*=c;
    float g = dot(sin(rd*10.512),cos(rd.yzx*10.512));
    c*=smoothstep(-3.14,-.9,g)*.5+.5*smoothstep(-.3,1.,g);
    return c*c;
}

vec3 gsky(vec3 rd,vec3 ld,bool mask){
    float haze = exp2(-5.*(abs(rd.y)-.2*dot(rd,ld)));
    float st = mask?(starnoise(rd))*(1.-min(haze,1.)):0.;
    vec3 col=clamp(mix(vec3(.4,.1,.7)*(1.-.5*textureMirror(iChannel0,
                    vec2(.5+.05*rd.x/rd.y,0.)).x*exp2(-.1*abs(length(rd.xz)/rd.y))*max(sign(rd.y),0.)),
                    vec3(.7,.1,.4),haze)+st,0.,1.);
    if(mask)addsun(rd,ld,col);
    return col;  
}

// Funções pra desenhar o carro

float dot2(in vec2 v ) { return dot(v,v); }

float carVector( in vec2 p, in float r1, float r2, float he )
{
    vec2 k1 = vec2(r2,he);
    vec2 k2 = vec2(r2-r1,.9*he);
    p.x = abs(p.x);
    vec2 ca = vec2(p.x-min(p.x,(p.y<0.0)?r1:r2), abs(p.y)-he);
    vec2 cb = p - k1 + k2*clamp( dot(k1-p-1.0,k2)/dot2(k2), 0.0, 1.0 );//dot-1.0
    float s = (cb.x<0.0 && ca.y<0.0) ? -1.0 : 1.0;
    return s*sqrt( min(dot2(ca),dot2(cb)) );
}

float carPiecesVector( in vec2 p, in float r1, float r2, float he )
{
    vec2 k1 = vec2(r2,he);
    vec2 k2 = vec2(r2-r1,.9*he);
    p.x = abs(p.x);
    vec2 ca = vec2(p.x-min(p.x,(p.y<0.0) ? r1:r2), abs(p.y)-he);
    vec2 cb = p - k1 + k2+clamp( dot(k1,k2)/dot2(k2), 0.0, 1.0 );
    float s = (cb.x<0.0 && ca.y<0.0) ? -1.0 : 1.0;
    return s*sqrt( min(dot2(ca),dot2(cb)) );
}

void main() {
    const float AA=1.,x=0.,y=0.;
    float dt = fract(hash21(float(AA)*(gl_FragCoord.xy+vec2(x,y)))+iTime);
    vec2 uv = (2.*(gl_FragCoord.xy+vec2(x,y))-iResolution.xy)/iResolution.y;

    float carVector = carVector( uv + vec2(uv.x * 2.4, 1.0), 1.3, 0.8, 0.3);
    float backSign = carPiecesVector( uv + vec2(uv.x * 4.4, 1.0), 0.0, 0.8, 0.18);
    float backGlass = carPiecesVector( uv + vec2(uv.x * 2.4, 1.0), 0.0, 0.8, 0.45);
    float backLightLeft = carPiecesVector( uv + vec2(-1.5+uv.x * 3.2, 1.0), 0.0, 0.1, 0.15);
    float backLightRight = carPiecesVector( uv + vec2(+1.5+uv.x * 3.2, 1.0), 0.0, 0.1, 0.15);

    fragColor=vec4(0);
    jTime = mod(iTime-dt*iTimeDelta*.25,4000.);

    vec3 ro = vec3(0.,1,(-20000.+jTime*speed));

    vec3 rd = normalize(vec3(uv,4./3.));

    vec2 i = intersect(ro,rd);
    float d = i.x;

    vec3 ld = normalize(vec3(0,.125+.05*sin(.1*jTime),1));

    vec3 fog = d>0.?exp2(-d*vec3(.14,.1,.28)):vec3(0.);
    vec3 sky = gsky(rd,ld,d<0.);

    vec3 p = ro+d*rd;
    vec3 n = normalize(grad(p));

    float diff = dot(n,ld)+.1*n.y;
    vec3 col = vec3(.1,.11,.18)*diff;
    vec3 col1 = vec3(3.01,3.01,.18)*diff;
    vec3 col2 = vec3(12.11,1.11,2.18)*diff;
    vec3 bufferUCS = texture(iChannel1, (uv + vec2(0.295, 0.82)) * 3.4 - 0.5).rgb;

    col = mix(col,vec3(.8,.1,.92),smoothstep(.05,.0,i.y));

    col = mix(sky,col,step(0.3, carVector));
    col = mix(bufferUCS,col,step(0.1, backSign));
    col = mix(col1,col,step(0.1, backGlass));
    col = mix(col2,col,step(0.1, backLightLeft));
    col = mix(col2,col,step(0.1, backLightRight));
    
    col = mix(sky,col,fog);

    fragColor += vec4(clamp(col,0.,1.),d<0.?0.:.1+exp2(-d));
}