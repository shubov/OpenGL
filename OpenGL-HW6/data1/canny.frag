 uniform sampler2D colorTexture, depthTexture;
 uniform float xPos;

 float thresh = 1.0;

 in Vertex
 {
 	vec2 texcoord;
 } Vert;

 layout(location = FRAG_OUTPUT0) out vec4 colorO;

 const float texWidth  = 1.0 / 800.0;	///< Web cam width size 
 const float texHeight = 1.0 / 600.0;	///< Web cam height size
 const float threshold = 1.0005;			///< Threshold value

 const vec2 unshift = vec2(1.0 / 256.0, 1.0); ///< Value used to unpack 16 bit float data

 const float atan0   = 0.414213;  ///< Support value for atan
 const float atan45  = 2.414213;  ///< Support value for atan
 const float atan90  = -2.414213; ///< Support value for atan
 const float atan135 = -0.414213; ///< Support value for atan

 /// Fast atan for canny usage.
 vec2 atanForCanny(float x) {
     if (x < atan0 && x > atan135) {
         return vec2(1.0, 0.0);
     }
     if (x < atan90 && x > atan45) {
         return vec2(0.0, 1.0);
     }
     if (x > atan135 && x < atan90) {
         return vec2(-1.0, 1.0);
     }
     return vec2(1.0, 1.0);
 }

 /**
  * Function that performs canny edge detection.
  * @param coords Texture coordinates to analyize
  */
 vec4 cannyEdge(in vec2 texcoord) {
   vec4 color = texture(colorTexture, texcoord);
   color.z = dot(color.zw, unshift);

   // Thresholding 
   if (color.z > threshold) {
     // Restore gradient directions.
     color.x -= 0.5;
     color.y -= 0.5;

     vec2 offset = atanForCanny(color.y / color.x);
     offset.x *= texWidth;
     offset.y *= texHeight;

     vec4 forward  = texture(colorTexture, texcoord + offset);
     vec4 backward = texture(colorTexture, texcoord - offset);
     // Uncompress mag data
     forward.z  = dot(forward.zw, unshift);
     backward.z = dot(backward.zw, unshift);
    
     // Check maximum.
     if (forward.z >= color.z || 
         backward.z >= color.z) {
       return vec4(0.0, 0.0, 0.0, 1.0);
     } else {
       color.x += 0.5; color.y += 0.5;
       //return vec4(1.0, color.x, color.y, 1.0);
 	  return vec4(1.0, 1.0, 1.0, 1.0);
     }
   }
   return vec4(0.0, 0.0, 0.0, 1.0);
 }

 /// Shader entry point
 void main() {
 	colorO = cannyEdge(Vert.texcoord);
 } 