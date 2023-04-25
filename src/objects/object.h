
/**
 * This program acts as the superclass for the objects
 * 
 * 
*/

#ifndef object_H
#define object_H

#include "agl/aglm.h"
#include "plymesh.h"

using namespace agl;
using namespace glm;

struct Object {
  public:
		Object() : pos(vec3(0)), dir(vec3(0)), texture(""), rot(0.0), scale(vec3(1)) {};

    Object(vec3 pos, vec3 dir, vec3 scale, float rot, PLYMesh mesh, std::string texture) : 
      pos(pos), dir(dir), rot(rot), scale(scale), mesh(mesh), texture(texture) {};

		void appendChild(Object obj) { children.push_back(obj); }

		vec3 getMinBounds() { return mesh.minBounds(); };
		vec3 getMaxBounds() { return mesh.maxBounds(); };

		vec3 getMidPoint() { return (mesh.maxBounds() - mesh.minBounds()) * 0.5f; };

    vec3 pos= vec3(0, 0, 0);
    vec3 dir= vec3(0, 0, 1);
		vec3 scale= vec3(1);
		float rot= 0.0;

    PLYMesh mesh;
    std::string texture;

		std::vector<Object> children;
};


#endif