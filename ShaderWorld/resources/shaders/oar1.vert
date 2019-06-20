#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec2 VertexST;
layout (location = 2) in vec3 VertexNormal;

uniform vec4 LightPosition;
uniform mat4 WorldMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 ShadowMatrix;

out VertexData
{
    vec3 normal;
    vec4 colour;
    vec2 st;
    vec4 EyeSpaceLightPosition;
    vec4 EyeSpacePosition;
    vec4 EyeSpaceObjectPosition;
    vec4 ShadowCoord; 
} VertexOut; 

// Main function - Vertex shader
void main()
{
    vec4 Position = vec4 (VertexPosition, 1);
	
	Position.y += -Position.x / 1.7;

    // Calculate the normal matrix and apply to normal vector... 
    mat4 normalMatrix = transpose(inverse(ViewMatrix * WorldMatrix));

    vec3 Normal = VertexNormal;

    VertexOut.normal = (normalMatrix * vec4(Normal, 0)).xyz;
    VertexOut.colour = vec4(1.0,0.0,1.0,1.0);    // Testing - hard code colour to BLUE;
    VertexOut.st = VertexST;                    // Texture mapping coordinate as passed in

    // Populate  other important quantities, also in eye coordinates. The position of the light. 
    VertexOut.EyeSpaceLightPosition =  ViewMatrix * WorldMatrix * LightPosition;
    VertexOut.EyeSpacePosition = ViewMatrix * WorldMatrix * Position;
    VertexOut.EyeSpaceObjectPosition = ViewMatrix * WorldMatrix * vec4 (0, 0, 0, 1);

    VertexOut.ShadowCoord = ProjectionMatrix * ShadowMatrix * WorldMatrix * Position;
    VertexOut.ShadowCoord = VertexOut.ShadowCoord / VertexOut.ShadowCoord.w;
    VertexOut.ShadowCoord = (VertexOut.ShadowCoord + vec4 (1.,1.,1.,0.)) *
                               vec4 (1./2., 1./2., 1./2., 1);

    // Populate the requried output of the Vertex Shader
    gl_Position = ProjectionMatrix * ViewMatrix * WorldMatrix * Position;
}
