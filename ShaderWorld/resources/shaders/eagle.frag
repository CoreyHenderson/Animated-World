#version 330  

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

uniform sampler2D texture1;

// Main function - Fragment shader
void main()
{
  /*************************************************************************************/
	
	vec4 fragColour = vec4 (0.0, 0.0, 0.0, 1.0);

	// Set up the texture coordinates
	float s = frag.st.x;
	float t = 1 - frag.st.y;             // Fix inverted texture (vertical axis)  
	
  /*************************************************************************************/
  
	// Get eagle texture
	float amplitude = 0.7;
	vec4 texture = amplitude * vec4 ( texture2D( texture1, vec2(s, t)));

  /*************************************************************************************/

	// Initialise the base lighting colours
	vec4 MaterialColour = texture;
	vec4 LightColour = vec4 (0.2, 0.2, 0.2, 1.0);
	vec4 HighlightColour = vec4 (0.6, 0.6, 0.6, 1.0);

	// Set up the lighting vectors
	vec3 N = normalize (frag.normal);
	vec3 L = normalize (vec3 (frag.EyeSpaceLightPosition - frag.EyeSpacePosition));
	vec3 R = normalize (reflect (L, N));
	vec3 V = normalize (vec3 (frag.EyeSpaceObjectPosition) - vec3 (0, 0, 0) );

	// Lighting Parameters
	float Ka = 0.7;
	float Kd = 0.7;
	float Ks = 0.0;
	float n = 10.0;

	// Ambient
	fragColour += Ka * MaterialColour;
	// Diffuse
	fragColour += MaterialColour * (Kd * max (0.0, dot (N, L))) * LightColour;
	// Specular
	fragColour += (Ks * pow (max (0.0, dot (R, V)), n)) * HighlightColour;
	
  /*************************************************************************************/

	out_fragmentColour = fragColour;
}