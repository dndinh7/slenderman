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

// stuff for shader toy
uniform bool useGlitch;
uniform vec2 iResolution;
uniform float iTime;




// fog info
struct FogInfo {
  float maxDist; // distance where camera can only see fog
  float minDist; // distance from eye, so that there is no fog
  vec3 color; // color of fog
};
uniform FogInfo Fog;
uniform bool useFog;


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

https://www.shadertoy.com/view/XtK3W3
vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec2 mod289(vec2 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 permute(vec3 x) {
  return mod289(((x*34.0)+1.0)*x);
}

float snoise(vec2 v)
  {
  const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                      0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                     -0.577350269189626,  // -1.0 + 2.0 * C.x
                      0.024390243902439); // 1.0 / 41.0
// First corner
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);

// Other corners
  vec2 i1;
  //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
  //i1.y = 1.0 - i1.x;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  // x0 = x0 - 0.0 + 0.0 * C.xx ;
  // x1 = x0 - i1 + 1.0 * C.xx ;
  // x2 = x0 - 1.0 + 2.0 * C.xx ;
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;

// Permutations
  i = mod289(i); // Avoid truncation effects in permutation
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
		+ i.x + vec3(0.0, i1.x, 1.0 ));

  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;

// Gradients: 41 points uniformly over a line, mapped onto a diamond.
// The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;

// Normalise gradients implicitly by scaling m
// Approximation of: m *= inversesqrt( a0*a0 + h*h );
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

// Compute final noise value at P
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}

float rand(vec2 co)
{
   return fract(sin(dot(co.xy,vec2(12.9898,78.233))) * 43758.5453);
}


void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	vec2 uv = fragCoord.xy / iResolution.xy;    
	float time = iTime * 2.0;
	
	// Create large, incidental noise waves
	float noise = max(0.0, snoise(vec2(time, uv.y * 0.3)) - 0.3) * (1.0 / 0.7);
	
	// Offset by smaller, constant noise waves
	noise = noise + (snoise(vec2(time*10.0, uv.y * 2.4)) - 0.5) * 0.15;
	
	// Apply the noise as x displacement for every line
	float xpos = uv.x - noise * noise * 0.25;
	fragColor = texture(diffuseTexture, vec2(xpos, uv.y));
	
	// Mix in some random interference for lines
	fragColor.rgb = mix(fragColor.rgb, vec3(rand(vec2(uv.y * time))), noise * 0.3).rgb;
	
	// Apply a line pattern every 4 pixels
	if (floor(mod(fragCoord.y * 0.25, 2.0)) == 0.0)
	{
		fragColor.rgb *= 1.0 - (0.15 * noise);
	}
	
	// Shift green/blue channels (using the red channel)
	fragColor.g = mix(fragColor.r, texture(diffuseTexture, vec2(xpos + noise * 0.05, uv.y)).g, 0.25);
	fragColor.b = mix(fragColor.r, texture(diffuseTexture, vec2(xpos - noise * 0.05, uv.y)).b, 0.25);
}


void main()
{
  
	vec4 phongColor= phongSpot();

	float alpha= phongColor.w;

	vec3 color= phongColor.xyz;

	if (useFog) {
		// fog from camera
		float dist= abs(p_eye.z);
		float fogFactor;
		// linear fog factor
		fogFactor= (Fog.maxDist - dist) / (Fog.maxDist - Fog.minDist);
		fogFactor= max(min(fogFactor, 1.0f), 0.0f);

		color= mix(Fog.color, phongColor.xyz, fogFactor);
	}

	if (useGlitch) {
		vec4 fragColor= vec4(1, 1, 1, 1);
		mainImage(fragColor, gl_FragCoord.xy);

		color= fragColor.xyz;
	}
	

  FragColor = vec4(color, alpha);
}


