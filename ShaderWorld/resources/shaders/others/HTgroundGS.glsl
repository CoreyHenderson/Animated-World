// This is a 'passthrough' or empty Geomtery Shader.
// All it does is take the input from the previous stage
// (vertex shader) and pass it to the next stage (fragment
// shader). No modifcation to the geometry (added/deleted)
// is done in this example.

#version 330 core

// Tell the shader what format the input data is in. With
// this example, it is expected the vertex data represents
// triangles.
layout (triangles) in;

// The following line speficy what the output will be, in terms
// of the style and the number of vertices. In this case, since
// we are getting triangles in (3 verticies) -- we pass out up to
// 3 verticies (well, in this case, exactly 3) 
layout (triangle_strip, max_vertices=3) out;

// The data structure/format of the additional data passed on 
// form the previous stage (vertex shader).
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

	float height;               // Height value
    float terrx;                // x-coord
    float terry;                // y-coord
} VertexIn[];

// The data structure/format of the data to be passed on 
// to the next stage (fragment shader).
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

	float height;               // Height value
    float terrx;                // x-coord
    float terry;                // y-coord
} VertexOut;

// The actual shader code
void main()
{
    int i;


    // 'gl_in' is the default data passed to this stage
    // from the previous stage. 
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
