/**
 * This class serves as the super class for the player and other
 * entities, but Slenderman was just considered an object so that
 * I could render it with the billboards 
*/

#ifndef entity_H
#define entity_H

#include "agl/aglm.h"

using namespace glm;

namespace assets {
  class Entity {
    public:
      vec3 getPos() { return this->pos; }
      vec3 getLookPos() { return this->lookPos; }
      void setPos(vec3 pos) { this->pos= pos; }
      void setLookPos(vec3 lookPos) { this->lookPos= lookPos; }

      void setXYZAxes(vec3 xAxis, vec3 yAxis, vec3 zAxis) {
        this->xAxis= xAxis;
        this->yAxis= yAxis;
        this->zAxis= zAxis;
      }

      vec3 getXAxis() { return this->xAxis; }
      vec3 getYAxis() { return this->yAxis; }
      vec3 getZAxis() { return this->zAxis; }

      void setXAxis(vec3 newAxis) { this->xAxis= newAxis; }
      void setYAxis(vec3 newAxis) { this->yAxis= newAxis; }
      void setZAxis(vec3 newAxis) { this->zAxis= newAxis; }


    protected:
      Entity(vec3 pos, vec3 lookPos) : 
        pos(pos), lookPos(lookPos) {};
      
      vec3 pos;
      vec3 lookPos;

      vec3 xAxis= vec3(1,0,0);
      vec3 yAxis= vec3(0,1,0);
      vec3 zAxis= vec3(0,0,1);
  };
}


#endif