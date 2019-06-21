#version 330  

// Speficy what precision the GPU should try and use
precision highp float;

// Input fragment data
in FragmentData
{
    vec3 normal;
    vec4 colour;
    vec2 st;
    vec4 EyeSpaceLightPosition;
    vec4 EyeSpacePosition;
    vec4 EyeSpaceObjectPosition;
    vec4 WorldSpaceObjectPosition;
    vec4 ShadowCoord;
	
	// The terrain data passed in from previous shader
    float height;               // Height value
    float terrx;                // x-coord
    float terry;                // y-coord
} frag;

/** SPLINE FUNCTION **/
vec3 spline (float value, vec3 cp [9]) 
{
    float i = value * (9 - 3);
    int offset = int (floor (i));
    float t = fract (i);
    vec3 result = 0.5 * ((2.0 * cp[offset + 1]) + 
                        (-1.0 * cp[offset + 0] + 
                            cp[offset + 2]) * t + 
                        (2.0 * cp[offset + 0] - 
                            5.0 * cp[offset + 1] + 
                            4.0 * cp[offset + 2] - 
                        cp[offset + 3]) * t * t + 
                        (-1.0 * cp[offset + 0] + 
                            3.0 * cp[offset + 1] -
                            3.0 * cp[offset + 2] + 
                    cp[offset + 3]) * t * t * t);
    return result;
}

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

// Output fragment colour
out vec4 out_fragmentColour;

// The dimensions (resolution) of the drawing buffer
uniform vec2 u_resolution;

uniform sampler2D texture1;

// Main function - Fragment shader
void main()
{
  /*************************************************************************************/
  
	vec3 fragColour = vec3 (0.0, 0.0, 0.0);
	vec3 MaterialColour = vec3(0.0, 0.0, 0.0);

	// Set up the texture coordinates
	float s = frag.st.x;
	float t = 1 - frag.st.y;             // Fix inverted texture (vertical axis)

  /*************************************************************************************/
  
	// Set up surface colours 
	vec3 darkblue =  vec3 (0.0000,0.3700,0.8500);
	vec3 darkgreen = vec3 (0.0000,0.5176,0.2078);
	vec3 green = 	 vec3 (0.2000,0.8000,0.0000);
	vec3 yellow = 	 vec3 (0.9569,0.9412,0.4431);
	vec3 orange = 	 vec3 (0.9569,0.7412,0.2706);
	vec3 brown = 	 vec3 (0.6000,0.3922,0.1686);
	vec3 white = 	 vec3 (1.0000,1.0000,1.0000);

	// Create Mountain colour range
	vec3 range [] = vec3 [] ( darkblue, darkblue, darkgreen, green, yellow, orange, brown, white, white );
	
	// Create grass range
	vec3 grassRange [] = vec3 [] ( darkgreen, darkgreen, green, darkgreen, green, darkgreen, green, darkgreen, darkgreen);
	// Create sand range
	vec3 sandRange [] = vec3 [] (yellow, yellow, yellow, orange, yellow, orange, yellow, yellow, yellow);
	
  /*************************************************************************************/
	
	// Noise calculation for mountain material colour
	float noise = 1.0;
	float noiseHeight = 0.75;
	noise = noiseHeight + sin (frag.height / 0.6 + 0.9 * cnoise (vec2 (s, t)));
	
	// Get the mountain heightmap RGB colours
	vec3 textureRGB = texture(texture1, frag.st).xyz;
	
	if (!(textureRGB.x > 0.01))
	{
		MaterialColour = darkblue;			// Set blue ocean colour
	}
	else
	{
		// Set material colour for the mountain based on noise and the colour range
		MaterialColour = spline (noise, range);
		
		// Set sand procedural shading on the sandy (yellow) region
		if (noise > 0.40 && noise < 0.70)
		{
			// Sand Colour
			float sandNoise = (1.5 + pnoise (vec2 (0.5 * sin (2.0 * 3.14159 * s) + 3.14159, 4.0 * t + 1.414), vec2 (10, 10))) / 2;
			vec3 sandColour = spline(sandNoise, sandRange);
			
			// Sand streams
			float sandFrequency = 130;
			float sandStretch = 2.0;
			float sandVisibility = 0.40;
			
			// Calculate sand visibility to go from 0.40 - 2.0 based on the noise going from 0.60 - 0.70
			// so that the sand fades out as it gets towards the brown mountain slope region. Vice verse for fading out
			// when hitting the grassland region.
			if (noise > 0.60)
				sandVisibility += (noise - 0.60) * 16;
			if (noise < 0.50)
				sandVisibility += (noise - 0.50) * -16;
			
			float sand = (cnoise (vec2 (sandFrequency * s / sandStretch, sandFrequency * t * sandStretch))) / sandVisibility;
			
			if (sand > 0.40)
				MaterialColour = mix(MaterialColour, sandColour, sand - 0.44);
			
			sandFrequency = 8000;
			sand = (cnoise (vec2 (sandFrequency * s, sandFrequency * t))) / sandVisibility;
			
			if (sand > 0.40)
				MaterialColour = mix(MaterialColour, sandColour, sand - 0.44);
		}
		
		// Set grass procedural shading on the grassland (green) region
		if (noise > 0.25 && noise < 0.55)
		{	
			// Grass Colour
			float grassNoise = (1.2 + pnoise (vec2 (0.5 * sin (2.0 * 3.14159 * s) + 3.14159, 2.0 * t + 1.414), vec2 (10, 4))) / 2;
			vec3 grassColour = spline(grassNoise, grassRange);		// Set colour using spline amd grassNoise
			
			// Grass Marks
			float grassFrequency = 4000;
			float grassStretch = 1.5;
			float grassVisibility = 0.40;
			
			// Calculate grass visibility to go from 0.40 - 2.0 based on the noise going from 0.40 - 0.55
			// so that the grass fades out as it gets towards the yellow sandy region.
			if (noise > 0.40)
				grassVisibility += (noise - 0.40) * ( (2.0-0.40) / (0.55-0.40) );
			
			float grassMarks = (cnoise (vec2 (grassFrequency * s / grassStretch, grassFrequency * t * grassStretch ))) / grassVisibility;
			
			// Set vertical marks
			if (grassMarks > 0.7)
				MaterialColour = mix(MaterialColour, grassColour, grassMarks - 0.77);
			
			grassMarks = (cnoise (vec2 (grassFrequency * s * grassStretch, grassFrequency * t / grassStretch ))) / grassVisibility;
			
			// Set horizontal marks
			if (grassMarks > 0.7)
				MaterialColour = mix(MaterialColour, grassColour, grassMarks - 0.77);
		}
		
		// Set dark green swamp reeds that fades into the dark blue ocean colour
		if (frag.height < 0.04)
			MaterialColour = spline(clamp(frag.height * 4, 0.0, 0.35), range);
	}
	
  /*************************************************************************************/

	// Set up the vectors.
	vec3 N = normalize (frag.normal);
	vec3 L = normalize (vec3 (frag.EyeSpaceLightPosition - frag.EyeSpacePosition));
	vec3 R = normalize (reflect (L, N));
	vec3 V = normalize (vec3 (frag.EyeSpaceObjectPosition) - vec3 (0, 0, 0) );

	// Lighting Parameters
	float Ka = 0.7;		// Ambient Coeffient
	float Kd = 0.3;		// Diffuse Coeffient
	float Ks = 0.4;		// Specular Coeffient
	float n = 6.0;
	vec3 LightColour = vec3 (0.4, 0.4, 0.4);
	vec3 HighlightColour = vec3 (0.1, 0.5, 0.5);

	// Ambient
	fragColour += Ka * MaterialColour;
	// Diffuse
	fragColour += MaterialColour * (Kd * max (0.0, dot (N, L))) * LightColour;
	// Specular
	fragColour += (Ks * pow (max (0.0, dot (R, V)), n)) * HighlightColour;

  /*************************************************************************************/

	out_fragmentColour = vec4(fragColour, 1.0);
}