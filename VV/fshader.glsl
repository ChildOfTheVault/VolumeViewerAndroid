#version 300 es

#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif


uniform sampler3D atexture;

//in vec3 lightDir,normal;

//in vec3 vViewPosition;
in float passIt2;

in vec4 v_texcoord;
out vec4 FragColor;

//! [0]
void main()
{

    if (passIt2 == 0.0) {
        //gl_FragColor = vec4(vViewPosition, 1.0);
        FragColor = texture(atexture, v_texcoord.xyz);
    }
    else {
        FragColor = vec4(1.0, 0.72, 0.78, 1.0);
    }
}
//! [0]

