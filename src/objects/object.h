
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

class Object {
  public:
    Object(vec3 pos, vec3 dir, PLYMesh mesh, std::string texture) : 
      pos(pos), dir(dir), mesh(mesh), texture(texture) {};

    vec3 getPos() { return this->pos; }
    vec3 getDir() { return this->dir; }
    void setPos(vec3 pos) { this->pos= pos; }
    void setDir(vec3 dir) { this->dir= dir; }

    PLYMesh getMesh() { return this->mesh; }
    std::string getTextureKey() { return this->texture; }
  protected:   
    vec3 pos;
    vec3 dir;

    PLYMesh mesh;
    std::string texture;
};


#endif