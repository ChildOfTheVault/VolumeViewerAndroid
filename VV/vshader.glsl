#version 300 es

#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform mat4 mvp_matrix;
uniform float passIt;

attribute vec4 a_position;
attribute vec4 a_texcoord;

out vec4 v_texcoord;


//varying vec3 lightDir,normal;

//varying vec3 vViewPosition;
out float passIt2;

//varying vec4 v_Color;

//! [0]
void main()
{
    passIt2 = passIt;
    //vViewPosition = a_position.xyz;

    // Calculate vertex position in screen space
    gl_Position = mvp_matrix * a_position;

    // Pass texture coordinate to fragment shader
    // Value will be automatically interpolated to fragments inside polygon faces
    v_texcoord = a_texcoord;
}
//! [0]
