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
    vec4 ShadowCoord;
} frag;

// Output fragment colour
out vec4 out_fragmentColour;

// The dimensions (resolution) of the drawing buffer
uniform vec2 u_resolution = vec2(800.0, 800.0);

// Main function - Fragment shader
void main()
{
	vec4 fragColour = vec4 (0.0, 0.0, 0.0, 1.0);

	// The gl_FragCoords is in the range of the number of pixels
	// in the viewing window. Resize these such that they are     
	vec2 st = gl_FragCoord.xy/u_resolution.xy;
	vec2 pos = (vec2(0.5)-st)*2;

	// Set up the texture coordinates
	float s = frag.st.x;
	float t = 1 - frag.st.y;             // Fix inverted texture (vertical axis)  

	// Initialise the base material colour
	vec4 MaterialColour = vec4 (0.7, 0.7, 0.7, 1.0);
	vec4 AmbientColour = vec4 (0.8, 0.8, 0.8, 1.0);

	// Set up the vectors.
	vec3 N = normalize (frag.normal);
	vec3 L = normalize (vec3 (frag.EyeSpaceLightPosition - frag.EyeSpacePosition));
	vec3 R = normalize (reflect (L, N));
	vec3 V = normalize (vec3 (frag.EyeSpaceObjectPosition) - vec3 (0, 0, 0) );

	// Lighting Parameters
	float Ka = 0.7;
	float Kd = 0.7;
	float Ks = 0.7;
	float n = 10.0;
	vec4 LightColour = vec4 (1.0, 1.0, 1.0, 1.0);
	vec4 HighlightColour = vec4 (1.0, 1.0, 1.0, 1.0);

	// Ambient
	fragColour += Ka * MaterialColour;
	// Diffuse
	fragColour += MaterialColour * (Kd * max (0.0, dot (N, L))) * LightColour;
	// Specular
	fragColour += (Ks * pow (max (0.0, dot (R, V)), n)) * HighlightColour;

	out_fragmentColour = fragColour;
}