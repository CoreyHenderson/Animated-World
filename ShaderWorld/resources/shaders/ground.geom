#version 330 core

// Define format of input data
layout (triangles) in;

// Define output layout as 3 vertices (triangle)
layout (triangle_strip, max_vertices=3) out;

// Input vertex data
in VertexData
{
    vec3 normal;
    vec4 colour;
    vec2 st;
    vec4 EyeSpaceLightPosition;
    vec4 EyeSpacePosition;
    vec4 EyeSpaceObjectPosition;
    vec4 WorldSpaceObjectPosition;
    vec4 ShadowCoord;
	
	// The terrain data to pass onto fragment shader
    float height;          // Height value
    float terrx;           // x-coord
    float terry;           // y-coord (z-axis in world space)
} VertexIn[];

// Output fragment data
out FragmentData
{
    vec3 normal;
    vec4 colour;
    vec2 st;
    vec4 EyeSpaceLightPosition;
    vec4 EyeSpacePosition;
    vec4 EyeSpaceObjectPosition;
    vec4 WorldSpaceObjectPosition;
    vec4 ShadowCoord;
	
	// The terrain data to pass onto fragment shader
    float height;          // Height value
    float terrx;           // x-coord
    float terry;           // y-coord (z-axis in world space)
} VertexOut;

// Main function - Geometry shader
void main()
{
    int i;

    // 'gl_in' is the default data passed to this stage from the previous stage. 
    for(i = 0;i < gl_in.length();i++)
    {
        // No alteration to data -- just 'passthrough' to next stage
        VertexOut.normal = VertexIn[i].normal;
        VertexOut.colour = VertexIn[i].colour;
        VertexOut.st = VertexIn[i].st;
        VertexOut.EyeSpaceLightPosition = VertexIn[i].EyeSpaceLightPosition;
        VertexOut.EyeSpacePosition = VertexIn[i].EyeSpacePosition;
        VertexOut.EyeSpaceObjectPosition = VertexIn[i].EyeSpaceObjectPosition;
        VertexOut.WorldSpaceObjectPosition = VertexIn[i].WorldSpaceObjectPosition;
        VertexOut.ShadowCoord = VertexIn[i].ShadowCoord;
		
		VertexOut.height = VertexIn[i].height;
        VertexOut.terrx = VertexIn[i].terrx;
        VertexOut.terry = VertexIn[i].terry;

        // gl_Position must be set
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();     // A primitve here is the 'triangle'
}
