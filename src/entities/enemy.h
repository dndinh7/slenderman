#ifndef enemy_H
#define enemy_H

#include "agl/aglm.h"
#include "entity.h"
#include "plymesh.h"

using namespace glm;
using namespace agl;

namespace assets {
  class Enemy: public Entity {
    public:
      Enemy(): Entity(vec3(0), vec3(0,0,1)) {};

      Enemy(vec3 pos, vec3 dir, PLYMesh mesh, std::string texture) : 
        Entity(pos, dir), mesh(mesh), texture(texture) {};

    private:
      PLYMesh mesh;
      std::string texture;
  };
}


#endif