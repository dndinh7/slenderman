#version 400

in vec3 n_eye;
in vec4 p_eye;

struct Spotlight {
  vec4 pos; // where the light is located
  vec3 intensityAmbient; // light intensity
  vec3 intensityDiffuse; // light intensity
  vec3 intensitySpecular; // light intensity
  vec3 dir; // direction of light
  float exp; // angular attenuation factor
  float innerCutOff; // this is a inner cut off of the spotlight
  float outerCutOff; // this is the hard cutoff of light
};

uniform Spotlight Spot;

struct MaterialInfo {
  vec3 Ka;
  vec3 Kd;
  vec3 Ks;
  float alpha;
};

uniform MaterialInfo Material;

// texture information
uniform sampler2D diffuseTexture;
uniform bool HasUV;
uniform bool useAlpha;
in vec2 uv;

uniform vec2 uvScale;

out vec4 FragColor;

vec4 phongSpot() {
  vec3 s;
  vec3 n= normalize(n_eye);


  if (Spot.pos.w == 0.0f) // directional light source
    s= normalize(vec3(Spot.pos));
  else // positional light source
    s= normalize(vec3(Spot.pos - p_eye));

  float theta= dot(-s, Spot.dir);

  // NOTE: the TERMINOLOGY: inner and outer are based on the ANGLE
  // not the current value, the bigger the angle between [0, 90],
  // the smaller the cos(angle) will be
  float epsilon= Spot.innerCutOff - Spot.outerCutOff;

  // interpolate the light intensity based on distance the theta (our current
  // angle) is between the inner and outer cutoff angles
  float intensity= max(min((theta - Spot.outerCutOff)/epsilon, 1.0f), 0.0f);
  
  // normal spotlight calculations
  float spotFactor= pow(theta, Spot.exp);
  vec3 v= normalize(vec3(-p_eye)); // direction to camera
  vec3 h = normalize(v + s); // half way vector from 


  vec3 ambient= Spot.intensityAmbient * Material.Ka;
  vec3 diffuse= spotFactor * Spot.intensityDiffuse * intensity * Material.Kd 
    * max(dot(s, n), 0.0f);
  vec3 specular= spotFactor * Spot.intensitySpecular * intensity * Material.Ks
    * pow(max(dot(h, n), 0.0f), Material.alpha);

	float alpha= 1.0f; // default 1.0 for non textured meshes
  if (HasUV) {
    vec4 texColor= texture(diffuseTexture, uv*uvScale);
    ambient= Spot.intensityAmbient * Material.Ka * texColor.xyz;
    diffuse= spotFactor * Spot.intensityDiffuse * intensity * texColor.xyz 
      * max(dot(s, n), 0.0f);

		if (useAlpha) {
			alpha= texColor.w;
		}
  }

  vec3 color;
  if (HasUV) {
    color= (ambient + diffuse) + specular;
  } else {
    color= ambient + diffuse + specular;
  }

  return vec4(color, alpha);
}

void main()
{
  
  FragColor = phongSpot();
}