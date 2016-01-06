#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform sampler2D texture;

varying vec3 lightDir,normal;

varying vec3 N;
varying vec3 v;
varying vec3 vViewPosition;
varying float passIt2;

varying vec2 v_texcoord;

//varying vec4 v_Color;

//! [0]
void main()
{
    //vec3 u_LightPos = (2.0,2.0,2.0);
    //vec3 ct,cf;
    //vec4 texel;
    //float intensity,at,af;
    //intensity = max(dot(lightDir,normalize(normal)),0.0);
    //cf = intensity * (gl_FrontMaterial.diffuse).rgb + gl_FrontMaterial.ambient.rgb;
    //af = gl_FrontMaterial.diffuse.a;

    //texel = texture2D(texture, v_texcoord);

    //ct = texel.rgb;
    //at = texel.a;
    //gl_FragColor = vec4(ct * cf, at * af);

    //gl_FragColor = texture2D(texture, v_texcoord);

    //vec3 normal = faceNormals(vViewPosition);

    //if (vViewPosition.x > 0.0) {
    if (passIt2 > 0.0) {
        gl_FragColor = vec4(vViewPosition, 1.0);
    }
    else {
        gl_FragColor = texture2D(texture, v_texcoord);
    }
    //gl_FragColor = v_Color;
    //gl_FragColor = texel;

    //gl_FragColor = vec4(diffuseColor * vec3(texture2D(texture, v_texcoord))
     //                   + specularColor, 1.0);
}
//! [0]

