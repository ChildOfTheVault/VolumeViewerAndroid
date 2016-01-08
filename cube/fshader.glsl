#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform sampler2D texture;

varying vec3 lightDir,normal;

varying vec3 vViewPosition;
varying float passIt2;

varying vec2 v_texcoord;


//! [0]
void main()
{

    if (passIt2 > 0.0) {
        gl_FragColor = vec4(vViewPosition, 1.0);
    }
    else {
        gl_FragColor = texture2D(texture, v_texcoord);
    }
}
//! [0]

