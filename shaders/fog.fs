#version 400

struct LightSource {
  vec4 pos; // position of light in eye coordinates
  vec3 intensity; // light intensity
};

uniform LightSource Light;

struct FogInfo {
  float maxDist; // distance where camera can only see fog
  float minDist; // distance from eye, so that there is no fog
  vec3 color; // color of fog
};

uniform FogInfo Fog;

struct MaterialProp {
  vec3 Ka; // reflect ambience
  vec3 Kd; // reflect diffusion
  vec3 Ks; // reflect specular
  float alpha; // specular exponent factor
};

uniform MaterialProp Material;

in vec3 n_eye;
in vec4 p_eye;

// texture information
uniform sampler2D diffuseTexture;
uniform bool HasUV; // tells if there are uv coordinates
in vec2 uv; // texture coordinates
const float uvScale= 3.0f; // scales the coordinates

out vec4 FragColor;

vec3 phong() {
  // vector to the light source
  vec3 s;
  vec3 n= normalize(n_eye);

  if (Light.pos.w == 0.0f) // directional light source
    s= normalize(vec3(Light.pos));
  else // positional light source
    s= normalize(vec3(Light.pos - p_eye));

  vec3 v= normalize(vec3(-p_eye)); // vector to camera

  vec3 ambient= Light.intensity * Material.Ka; // ambient light

  float sDotn= max(dot(s, n), 0.0f);
  vec3 diffuse=  Light.intensity * Material.Kd * sDotn; // diffuse color

  vec3 r= 2 * (sDotn) * n - s; // reflected light 
  vec3 specular= vec3(0.0f);

  // this condition checks so that we only calculate
  // specular when the angle between light and normal
  // is acute 
  if (sDotn > 0.0f)
    specular= Light.intensity * Material.Ks * pow(max(dot(r, v), 0), Material.alpha);

  vec3 color;
  if (HasUV) {
    color= (ambient + diffuse) * texture(diffuseTexture, uv * uvScale).xyz + specular;
  } else {
    color= ambient + diffuse + specular;
  }

  return color;
}

void main()
{
  float dist= abs(p_eye.z);
  float fogFactor;
  // linear fog factor
  fogFactor= (Fog.maxDist - dist) / (Fog.maxDist - Fog.minDist);

  fogFactor= max(min(fogFactor, 1.0f), 0.0f);
  vec3 color= phong();
  color= mix(Fog.color, color, fogFactor);

  FragColor = vec4(color, 1.0);
}