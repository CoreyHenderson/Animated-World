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
uniform vec2 u_resolution;

uniform float pauto;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;

// Main function - Fragment Shader
void main()
{
	// Set up the texture coordinates
	float s = frag.st.x;
	float t = 1 - frag.st.y;             // Fix inverted texture (vertical axis)  

	float phase = pauto / 30000;
  
	// TEXTURE MAP
	out_fragmentColour = vec4 ( texture2D( texture1, vec2(s + phase, t)));
}