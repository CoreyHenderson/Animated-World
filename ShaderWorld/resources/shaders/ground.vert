#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec2 VertexST;
layout (location = 2) in vec3 VertexNormal;

uniform vec4 LightPosition;
uniform mat4 WorldMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 ShadowMatrix;

uniform sampler2D texture1;

uniform float pauto;

out VertexData
{
    vec3 normal;
    vec4 colour;
    vec2 st;
    vec4 EyeSpaceLightPosition;     // Used in later steps
    vec4 EyeSpacePosition;          // Used in later steps
    vec4 EyeSpaceObjectPosition;
    vec4 WorldSpaceObjectPosition;
    vec4 ShadowCoord;               // Used in later steps
	
	// The terrain data to pass onto fragment shader
    float height;          // Height value
    float terrx;           // x-coord
    float terry;           // y-coord (z-axis in world space)
} VertexOut; 

/** MATHS FUNCTIONS **/
vec4 mod289(vec4 x)
{
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x)
{
  return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

vec2 fade(vec2 t) {
  return t*t*t*(t*(t*6.0-15.0)+10.0);
}

/** PERLIN NOISE FUNCTION **/
float cnoise(vec2 P)
{
  vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
  vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
  Pi = mod289(Pi); // To avoid truncation effects in permutation
  vec4 ix = Pi.xzxz;
  vec4 iy = Pi.yyww;
  vec4 fx = Pf.xzxz;
  vec4 fy = Pf.yyww;

  vec4 i = permute(permute(ix) + iy);

  vec4 gx = fract(i * (1.0 / 41.0)) * 2.0 - 1.0 ;
  vec4 gy = abs(gx) - 0.5 ;
  vec4 tx = floor(gx + 0.5);
  gx = gx - tx;

  vec2 g00 = vec2(gx.x,gy.x);
  vec2 g10 = vec2(gx.y,gy.y);
  vec2 g01 = vec2(gx.z,gy.z);
  vec2 g11 = vec2(gx.w,gy.w);

  vec4 norm = taylorInvSqrt(vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11)));
  g00 *= norm.x;
  g01 *= norm.y;
  g10 *= norm.z;
  g11 *= norm.w;

  float n00 = dot(g00, vec2(fx.x, fy.x));
  float n10 = dot(g10, vec2(fx.y, fy.y));
  float n01 = dot(g01, vec2(fx.z, fy.z));
  float n11 = dot(g11, vec2(fx.w, fy.w));

  vec2 fade_xy = fade(Pf.xy);
  vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
  float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
  return 2.3 * n_xy;
}

/** TERRAIN FUNCTION **/
float terrain (float x, float y)
{
	// Set terrain height variables for the procedural waves
	float height = 0.0;
	int levels = 6;
	float frequency = 0.4;
    float amplitude = 0.007;
    float frequencyFactor = pow (2, 0.55/2.0);
	float amplitudeFactor = pow (2, 1.10/2.0);
	
	// Check if near the shores of the island to gradually reduce the amplitude of the waves.
	// (closer it is to the island, the smaller the waves are).
	// Pythagoras has been used to calculate 'radianDistance' and 'distance' variables
	if ((VertexST.x > 0.3 && VertexST.x < 0.7) && (VertexST.y > 0.30 && VertexST.y < 0.70))
	{
		float middle = 0.5;
		float radianDistance = sqrt(0.08); // radianDistance is the distance: From the centre -> edge of the boundary.
		float distance = sqrt( pow(middle - VertexST.x, 2) + pow(middle - VertexST.y, 2) ) / radianDistance;
		
		// Change amplitude of the waves from 0.007 -> 0.002 the closer the S and T coordinates are to the centre of the island
		amplitude = 0.002 + (0.005 * distance);
	}
	
	// Obtain RGB from texture map in range [0, 1]
	vec3 textureRGB = texture(texture1, VertexST).xyz;
	
	if (!(textureRGB.x > 0.001))
	{
		for (int i = 0; i < levels; i++)
		{
			height += amplitude * cnoise (vec2 (frequency * x, 4 * frequency * y));
			
			frequency = frequency * frequencyFactor;
			amplitude = amplitude * amplitudeFactor;
		}
	}
	else
		height += textureRGB.x / 2.0;

    return height;
}

/** NORMALIZE TERRAIN FUNCTION (For lighting) **/
vec3 terrainNormal (float x, float y)
{
    float delta = 0.1;
    vec3 A = vec3 (delta, terrain (x + delta, y) - terrain (x, y), 0);
    vec3 B = vec3 (0, terrain (x, y + delta) - terrain (x, y), delta);
    return normalize (cross (B,A));
}

// Main function - Vertex Shader
void main()
{
    vec4 Position = vec4 (VertexPosition, 1);
	
	VertexOut.terrx = VertexPosition.x;
    VertexOut.terry = VertexPosition.z;
	
	VertexOut.terrx += pauto / 1500;
	VertexOut.terry -= pauto / 1500;
	
	// Procedurally generate terrain based on the loaded texture
	VertexOut.height = terrain( VertexOut.terrx, VertexOut.terry );
	Position.y = VertexOut.height;

    // Calculate the normal matrix and apply to normal vector... 
    mat4 normalMatrix = transpose(inverse(ViewMatrix * WorldMatrix));
    vec3 Normal = terrainNormal(VertexOut.terrx, VertexOut.terry);

    VertexOut.normal = (normalMatrix * vec4(Normal, 0)).xyz;
    VertexOut.colour = vec4(1.0,0.0,1.0,1.0);
    VertexOut.st = VertexST;                    // Texture mapping coordinate as passed in

    // Populate  other important quantities, also in eye
    // coordinates. The position of the light. 
    VertexOut.EyeSpaceLightPosition =  ViewMatrix * WorldMatrix * LightPosition;
    VertexOut.EyeSpacePosition = ViewMatrix * WorldMatrix * Position;
    VertexOut.EyeSpaceObjectPosition = ViewMatrix * WorldMatrix * vec4 (0, 0, 0, 1);
    VertexOut.WorldSpaceObjectPosition = WorldMatrix * Position;

    VertexOut.ShadowCoord = ProjectionMatrix * ShadowMatrix * WorldMatrix * Position;
    VertexOut.ShadowCoord = VertexOut.ShadowCoord / VertexOut.ShadowCoord.w;
    VertexOut.ShadowCoord = (VertexOut.ShadowCoord + vec4 (1.,1.,1.,0.)) *
                               vec4 (1./2., 1./2., 1./2., 1);

    // Populate the requried output of the Vertex Shader
    gl_Position = ProjectionMatrix * ViewMatrix * WorldMatrix * Position;
}
