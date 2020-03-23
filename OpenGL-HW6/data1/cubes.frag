// glslsandbox uniforms
uniform sampler2D colorTexture;
uniform float pos;
uniform float mouse[2];
vec2 resolution = vec2(800,600);
 
// shadertoy globals
float animTime;
#define R resolution.xy
 
// --------[ ShaderToy begins here ]---------- //
#define detail 10
#define steps 40
 
layout(location = FRAG_OUTPUT0) out vec4 frag_color;

vec2 rot(vec2 v, float a) 
{ float c=cos(a), s=sin(a); 
  return v*mat2(c,s,-s,c);
}
 
//random function from https://www.shadertoy.com/view/MlsXDf
float rnd(vec4 v) { return fract(4e4*sin(dot(v,vec4(13.46,41.74,-73.36,14.24))+17.34)); }
 
//0 is empty, 1 is subdivide and 2 is full
int getvoxel(vec3 p, float size) {
    if (p.x == 0.0 && p.y == 0.0) 
        return 0;
    float val = rnd(vec4(p,size));
    
    /*if (val < 0.5)      return 0;
    else if (val < 0.8)   return 1;
    else                  return 2;   */
    
    return int(val*val*3.0);
}
 
//ray-cube intersection, on the inside of the cube
vec3 voxel(vec3 ro, vec3 rd, float size)
{
    size *= 0.5;
    return -(sign(rd)*(ro-size)-size)/max(abs(rd),0.001);
}
 
void mainImage( out vec4 fragColor,  vec2 fragCoord )
{
    fragColor = vec4(0.0);
    vec2 uv = (fragCoord.xy * 2.0 - R.xy) / R.y;
    float size = 1.0;
    
    vec3 ro = vec3(0.5+sin(animTime)*0.4,0.5+cos(animTime)*0.4,animTime);
    vec3 rd = normalize(vec3(uv,1.0));
    
    //if the mouse is in the bottom left corner, don't rotate the camera
    if (length(vec2(mouse[0], mouse[1]).xy) > 0.2)
    {
    	rd.yz = rot(rd.yz, mouse[0]*3.14-3.14*0.5);
	rd.xz = rot(rd.xz, mouse[1]*3.14*2.0-3.14);
    }
    
    vec3 lro = mod(ro,size);
    vec3 fro = ro-lro;
    vec3 mask;
    bool exitoct = false;
    int recursions = 0;
    float dist = 0.0;
    int i = 0;
    float edge = 1.0;
    
    //the octree traverser loop
    //each iteration i either:
    // - check if i need to go up a level
    // - check if i need to go down a level
    // - check if i hit a cube
    // - go one step forward if cube is empty
    for (int i = 0; i < steps; i++)
    {
        int voxelstate = getvoxel(fro,size);
        
        //i go up a level
        if (exitoct)
        {
            vec3 newfro = floor(fro/size*0.5)*size*2.0;
            
            lro += fro-newfro;
            fro = newfro;
            
            recursions--;
            size *= 2.0;
            
            exitoct = (recursions > 0) && (abs(dot(mod(fro/size+0.5,2.0)-1.0+mask*sign(rd)*0.5,mask))<0.1);
        }
        //subdivide
        else if(voxelstate == 1)
        {
            //if i have rached the octree limit, break
            if(recursions>detail) break;
            
            recursions++;
            size *= 0.5;
            
            //find which of the 8 voxels i will enter
            vec3 mask2 = step(vec3(size),lro);
            fro += mask2*size;
            lro -= mask2*size;
        }
        //move forward
        else if (voxelstate == 0)
        {
            //raycast and find distance to nearest voxel surface in ray direction
            vec3 hit = voxel(lro, rd, size);
            if (hit.x < min(hit.y,hit.z))  mask = vec3(1,0,0);
            else if (hit.y < hit.z)        mask = vec3(0,1,0);
            else                           mask = vec3(0,0,1);
            
            float len = dot(hit,mask);
            
            //moving forward in ray direction, and checking if i need to go up a level
            ro += rd*len;
            lro += rd*len-mask*sign(rd)*size;
            vec3 newfro = fro+mask*sign(rd)*size;
            exitoct = (floor(newfro/size*0.5+0.25)!=floor(fro/size*0.5+0.25))&&(recursions>0);
            fro = newfro;
        }
        else break;
    }
    
    if(i < steps)
    {
    	float val = fract(dot(fro,vec3(15.23,754.345,3.454)));
        vec3 normal = mask*sign(rd);
        vec3 color = sin(val*vec3(39.896,57.3225,48.25))*0.5+0.5;
    	fragColor = vec4(color*(normal*0.25+0.75),1.0);
        
        vec3 q = abs(lro/size-0.5)*(1.0-mask);
        edge = 1.+dot(q, q);
        fragColor *= vec4(edge);
    }
}
// --------[ ShaderToy ends here ]---------- //
 
void main(void)
{
    animTime = 0.4*pos;
    vec2 rho = 1. / R;
    vec2 uv = gl_FragCoord.xy * rho;
    // antialiasing animation using colorTexture
    vec4 back = max(0.2*texture(colorTexture, uv), 
	  0.2 * texture(colorTexture, (uv+rho*vec2(1,1)))
	+ 0.2 * texture(colorTexture, (uv+rho*vec2(-1,1)))
	+ 0.2 * texture(colorTexture, (uv+rho*vec2(1,-1)))
	+ 0.2 * texture(colorTexture, (uv+rho*vec2(-1,-1))));
 
    mainImage(frag_color, gl_FragCoord.xy);
    //frag_color = max(frag_color, back);
    frag_color.a = 1.0;
}