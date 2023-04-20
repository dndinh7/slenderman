
#version 400

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormals;
layout (location = 2) in vec2 vTextureCoords;

uniform mat3 NormalMatrix;
uniform mat4 ModelViewMatrix;
uniform mat4 MVP;
uniform bool HasUV;

out vec3 n_eye;
out vec4 p_eye;

out vec2 uv;

void main()
{
  // get the normal and vertex position to eye coordinates
  n_eye= normalize(NormalMatrix * vNormals);
  p_eye= ModelViewMatrix * vec4(vPos, 1.0);

  uv= vTextureCoords;
  
  uv= vTextureCoords;

  gl_Position = MVP * vec4(vPos, 1.0);
}

