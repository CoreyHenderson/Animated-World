#version 330  
#define PI 3.14159

// Speficy what precision the GPU should try and use
precision highp float;

// Input fragment data
in FragmentData
{
    vec3 normal;
    vec4 colour;
    vec2 st;
    vec4 EyeSpaceLightPosition;     // Used for lighting
    vec4 EyeSpacePosition;          // Used for lighting
    vec4 EyeSpaceObjectPosition;
    vec4 ShadowCoord;               // Shodow Corrdinates
} frag;

// Output fragment colour
out vec4 out_fragmentColour;

// The dimensions (resolution) of the drawing buffer
uniform vec2 u_resolution;

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

/** PERLIN NOISE FUNCTION - PERIODIC VARIANT **/
float pnoise(vec2 P, vec2 rep)
{
  vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
  vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
  Pi = mod(Pi, rep.xyxy); // To create noise with explicit period
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

/** SPLINE FUNCTION **/
vec3 spline (float value, vec3 cp [7]) 
{
  vec3 cp0, cp1, cp2, cp3;
  float t;
  if (value < 0.5)
  {
    if (value < 0.25)
      {
        cp0 =cp[0];
        cp1 =cp[1];
        cp2 =cp[2];
        cp3 =cp[3];
        t = value - 0.;
      }
    else
      {
        cp0 =cp[1];
        cp1 =cp[2];
        cp2 =cp[3];
        cp3 =cp[4];
        t = value - 0.25;
      }
  }
  else
  {
    if (value < 0.75)
      {
        cp0 =cp[2];
        cp1 =cp[3];
        cp2 =cp[4];
        cp3 =cp[5];
        t = value - 0.5;
      }
    else
      {
        cp0 =cp[3];
        cp1 =cp[4];
        cp2 =cp[5];
        cp3 =cp[6];
        t = value - 0.75;
      }
  }
  t = t * 4.;
  vec3 result = 0.5 * ((2.0 * cp1) + (-1.0 * cp0 + cp2) * t + (2.0 * cp0 - 5.0 * cp1 + 4.0 * cp2 - cp3) * t * t + (-1.0 * cp0 + 3.0 * cp1 - 3.0 * cp2 + cp3) * t * t * t);
  return result;
}

// Main function - Fragment shader
void main()
{
  /*************************************************************************************/
	
	vec3 fragColour = vec3 (0.2, 0.0, 0.2);

	// Set up the texture coordinates
	float s = frag.st.x;
	float t = 1 - frag.st.y;             // Fix inverted texture (vertical axis)  
	  
  /*************************************************************************************/

	// Set up colours for the trunk surface. 
	vec3 green 	= 		 vec3 (0.25, 0.50, 0.00);
	vec3 darkgreen = 	 vec3 (0.25, 0.30, 0.00);
	vec3 leafgreen 	= 	 vec3 (0.25, 0.45, 0.00);
	vec3 leafdarkgreen = vec3 (0.25, 0.40, 0.00);
	vec3 brown 	= 		 vec3 (0.10, 0.05, 0.00);
	vec3 lightbrown = 	 vec3 (0.50, 0.25, 0.05);
	
	// Create surface colour ranges
	vec3 trunkRange [] = vec3 [] ( brown, lightbrown, brown, lightbrown, brown, lightbrown, brown );	
	vec3 leafRange [] = vec3 [] ( darkgreen, darkgreen, green, darkgreen, green, darkgreen, darkgreen );
	vec3 leafMarks [] = vec3 [] ( leafgreen, leafdarkgreen, darkgreen, leafgreen, leafdarkgreen, leafgreen, leafgreen );
	vec3 trunkMarks [] = vec3 [] ( lightbrown, brown, lightbrown, brown, lightbrown, brown, lightbrown );	 	

	// Create noise variable for the different colour ranges
	float colourIntensity = 1.0;		// 0.9 to + 1.4
	float tfrequency = 7.0;
	
	if (s < 0.5 && t < 0.5)
		tfrequency = 3.5;
	
	float noise = (colourIntensity + pnoise (vec2 (0.5 * sin (2.0 * PI * s) + PI, tfrequency * t + 1.414), vec2 (10, 4))) / 2;
	
  /*************************************************************************************/
	
	// Trunk colour
	vec3 trunkColour = spline (noise, trunkRange);
	// Leaf area colour
	vec3 leafColour = spline (noise, leafRange);
	// Trunk mark colour
	vec3 trunkMarkColour = spline (noise, trunkMarks);
	// Leaf mark colour
	vec3 leafMarkColour = spline (noise, leafMarks);
	
	if (s < 0.5 && t < 0.5)
		fragColour = trunkColour;		// Apply trunk colour
	else
		fragColour = leafColour;		// Apply leaf colour
	  
  /*************************************************************************************/
	
	// Leaf marks //
	float markFrequency = 17.7;		//26.9 OR 17.7
	float markStretch = 0.02;		//0.015 OR 0.02
	float markVisibility = 0.42;
	float mark = (cnoise (vec2 (markFrequency * s / markStretch, markFrequency * t * markStretch ))) / markVisibility;
	
	// Select only the dark region of the leaves
	if (mark > 0.3)
		leafColour = mix (leafColour, leafMarkColour, (mark - 0.33) * 1.5);
	
	// Trunk marks
	markFrequency = 19.5;
	markStretch = 0.17;
	markVisibility = 0.75;
	mark = (cnoise (vec2 (markFrequency * s * markStretch, markFrequency * t / markStretch ))) / markVisibility;
	
	// Select only the dark region of the leaves
	if (mark > 0.3)
		trunkColour = mix (trunkColour, trunkMarkColour, (mark - 0.33) * 1.5);
	
	if (s < 0.5 && t < 0.5)
		fragColour = trunkColour;		// Apply trunk marks
	else
		fragColour = leafColour;		// Apply leaf marks
	
  /*************************************************************************************/

	// Initialise the base lighting colours
	vec3 MaterialColour =	fragColour;
	vec3 LightColour = vec3 (0.4, 0.8, 0.2);
	vec3 HighlightColour = vec3 (1.0, 1.0, 1.0);

	// Set up the lighting vectors
	vec3 N = normalize (frag.normal);
	vec3 L = normalize (vec3 (frag.EyeSpaceLightPosition - frag.EyeSpacePosition));
	vec3 R = normalize (reflect (L, N));
	vec3 V = normalize (vec3 (frag.EyeSpaceObjectPosition) - vec3 (0, 0, 0) );

	// Lighting Parameters
	float Ka = 0.1;   // Ambient Coefficient
	float Kd = 0.4;   // Diffuse Coefficient
	float Ks = 0.7;   // Specular Coefficient
	float n = 10.0;

	// Ambient
	fragColour += Ka * MaterialColour;
	// Diffuse
	fragColour += MaterialColour * (Kd * max (0.0, dot (N, L))) * LightColour;
	// Specular - only applied to the leaves
	if (!(s < 0.5) && !(t < 0.5))
		fragColour += (Ks * pow (max (0.0, dot (R, V)), n)) * HighlightColour;
	  
  /*************************************************************************************/

	out_fragmentColour = vec4(fragColour, 1.0);
}