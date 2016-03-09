#version 300 es

#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif


uniform sampler3D texture;

varying vec3 lightDir,normal;

varying vec3 vViewPosition;
varying float passIt2;

varying vec3 v_texcoord;


//! [0]
void main()
{

    if (passIt2 == 0.0) {
        //gl_FragColor = vec4(vViewPosition, 1.0);
        gl_FragColor = texture3D(texture, v_texcoord);
    }
    else {
        gl_FragColor = vec4(1.0, 0.72, 0.78, 1.0);
    }
}
//! [0]

