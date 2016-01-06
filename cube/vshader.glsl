#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform mat4 mvp_matrix;
uniform float passIt;

attribute vec4 a_position;
attribute vec2 a_texcoord;

varying vec2 v_texcoord;


varying vec3 lightDir,normal;

varying vec3 N;
varying vec3 v;
varying vec3 vViewPosition;
varying float passIt2;

varying vec4 v_Color;

//! [0]
void main()
{
      //pass varyings to fragment shader
    //vec3 u_LightPos = (2.0,2.0,2.0);
    passIt2 = passIt;
    vViewPosition = a_position.xyz;
    //passIt2.x = passIt.x;
    /*
      //vUv = uv;
    vec3 a_Color = (0.0, 0.0, 1.0);
    //vec3 a_Normal = normalize(gl_NormalMatrix * gl_Normal);
    vec3 a_Normal = (0.0, 0.0, 1.0);
    vec3 modelViewVertex = vec3(mvp_matrix * a_position);
    vec3 modelViewNormal = vec3(mvp_matrix * vec4(a_Normal, 0.0));
    float distance = length(u_LightPos - modelViewVertex);
    vec3 lightVector = normalize(u_LightPos - modelViewVertex);
    float diffuse = max(dot(modelViewNormal, lightVector), 0.1);
    diffuse = diffuse * (1.0 / (1.0 + (0.25 * distance * distance)));*/

    //v_Color = a_Color * diffuse;
    //vec3 a_Color = (0.0, 0.0, 1.0);
    //v_Color = (a_Color, 1.0);

      //v = vec3(gl_ModelViewMatrix * gl_Vertex);
      //N = normalize(gl_NormalMatrix * gl_Normal);

      //determine final 3D position
      //gl_Position = projection * viewModelPosition;

    // Calculate vertex position in screen space
    gl_Position = mvp_matrix * a_position;

    // Pass texture coordinate to fragment shader
    // Value will be automatically interpolated to fragments inside polygon faces
    v_texcoord = a_texcoord;
}
//! [0]
