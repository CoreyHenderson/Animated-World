// Give the (minimum) version of the shading language
// that this shader requires.
#version 330  

// Speficy what precision the GPU should try and use
precision highp float;

// The vertex shader (or later stage shader, i.e,
// Geometry shader) is expected to provide
// these values for the fragments. These will now
// be for each fragment in a triangle, rather
// than per vertex (thanks to rasterization and
// interpolation). 

// Data is provided to the fragment shader in the following 
// format 
in FragmentData
{
    vec3 normal;
    vec4 colour;
    vec2 st;
    vec4 EyeSpaceLightPosition;     // Used for lighting
    vec4 EyeSpacePosition;          // Used for lighting
    vec4 EyeSpaceObjectPosition;
    vec4 WorldSpaceObjectPosition;
    vec4 ShadowCoord;               // Shodow Corrdinates

	float height;               // Height value
    float terrx;                // x-coord
    float terry;                // y-coord
} frag;

// GLSL textureless classic 2D noise "cnoise",
// with an RSL-style periodic variant "pnoise".
// Author: Stefan Gustavson (stefan.gustavson@liu.se)
// Version: 2011-08-22
//
// Many thanks to Ian McEwan of Ashima Arts for the
// ideas for permutation and gradient selection.
//
// Copyright (c) 2011 Stefan Gustavson. All rights reserved.
// Distributed under the MIT license. See LICENSE file.
// https://github.com/ashima/webgl-noise
//

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

// Classic Perlin noise
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

#define SPLINELENGTH 7

vec4 spline (float value, vec4 cp [SPLINELENGTH]) 
{
    float i = value * (SPLINELENGTH - 3);
    int offset = int (floor (i));
    float t = fract (i);
    vec4 result = 0.5 * ((2.0 * cp[offset + 1]) + 
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


// The output of the fragment shader is provided through
// this data element. This 'fragment' is then blended with
// any other 'fragment colours' from other primitives being
// rendered to determine the *final* pixel colour
out vec4 out_fragmentColour;

// The dimensions (resolution) of the drawing buffer
uniform vec2 u_resolution;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;

// The base lighting fragment shader code.
void main()
{
  // Used to build the fragment colour
  vec4 colour = vec4 (0.0, 0.0, 0.0, 1.0);

  // The gl_FragCoords is in the range of the number of pixels
  // in the viewing window. Resize these such that they are     
  vec2 st = gl_FragCoord.xy/u_resolution.xy;
  vec2 pos = (vec2(0.5)-st)*2;

  // Set up the texture coordinates
  float s = frag.st.x;
  float t = 1 - frag.st.y;             // Fix inverted texture (vertical axis)  

  float freq = 10.0;
  float i = (1 + sin ((-0.7+1.01f)*0.1f*frag.height +  0.3*cnoise (vec2 (freq * frag.terrx, freq * frag.terry)))) / 2;

  // Initialise the base material colour
  vec4 MaterialColour = vec4 (0.7, 0.3, 0.1, 1.0);
  vec4 AmbientColour = vec4 (0.6, 0.6, 0.6, 1.0);

  // Set up the vectors.
  vec3 N = normalize (frag.normal);
  vec3 L = normalize (vec3 (frag.EyeSpaceLightPosition - frag.EyeSpacePosition));
  vec3 R = normalize (reflect (L, N));
  vec3 V = normalize (vec3 (frag.EyeSpaceObjectPosition) - vec3 (0, 0, 0) );

  // Lighting Parameters
  float Ka = 0.7;   // a-slider : Range [0..0.7]
  float Kd = 0.7;   // b-slider : Range [0..0.7]
  float Ks = 0.7;    // c-slider : Range [0..0.3]
  float n = 10.0;
  vec4 LightColour = vec4 (0.8, 0.8, 0.8, 1.0);
  vec4 HighlightColour = vec4 (0.5, 0.9, 0.5, 1.0);

  // ambient
  colour = colour + Ka * MaterialColour;
  // diffuse
  colour = colour + MaterialColour * (Kd * max (0.0, dot (N, L))) * LightColour;
  // Specular
  colour = colour + (Ks * pow (max (0.0, dot (R, V)), n)) * HighlightColour;

  // Assign an RGBA (red/green/blue/alpha) value to the output data element
  // Values for each component (R/G/B/A) range from [0.0 to 1.0]
  out_fragmentColour = colour;
}