#version 400

uniform sampler2D Image;
uniform sampler2D Bump;
uniform sampler2D Roughness;
uniform sampler2D AO; // ambient oocclusion
uniform sampler2D Metallic;

in vec2 uv;
out vec4 FragColor;

void main()
{
  vec4 c = texture(Image, uv);
  FragColor = c;
}