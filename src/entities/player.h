#ifndef player_H
#define player_H

#include "agl/aglm.h"
#include "entity.h"
#include "objects/object.h"

using namespace glm;

struct CameraInfo {
  float FOV;
  float aspect;
  float near;
  float far;

  float azimuth;
  float elevation;

	float targetAzimuth;
	float targetElevation;

  float radius;

  vec3 xAxis;
  vec3 yAxis;
  vec3 zAxis;

  vec3 up;
};

// class is the player object, also holds information about the camera
namespace assets {
  class Player: public Entity {
    public:
      Player(): Entity(vec3(0), vec3(0,0,1)) {};

      Player(vec3 pos, vec3 dir) :
        Entity(pos, dir), targetPosition(pos) {};

      Player(vec3 pos, vec3 dir, CameraInfo camera) : 
        Entity(pos, dir), camera(camera), targetPosition(pos) {};

			vec3 getTargetPosition() { return this->targetPosition; };
			void setTargetPosition(vec3 tp) { this->targetPosition= tp; };
      
      void moveLeft(float dt) { this->pos= this->pos + velocity * dt * xAxis; };
      void moveRight(float dt) { this->pos= this->pos - velocity * dt * xAxis; };

      void moveForward(float dt) { this->pos= this->pos + velocity * dt * zAxis; };
      void moveBackward(float dt) { this->pos= this->pos - velocity * dt * zAxis; };

      void setHealth(int health) { this->health= health; };
      int getHealth() { return this->health; };

      float getVelocity() { return this->velocity; };
      void  setVelocity(float velocity) { this->velocity= velocity; };

      float getCameraFOV() { return this->camera.FOV; };
      float getCameraAspect() { return this->camera.aspect; };
      float getCameraNear() { return this->camera.near; };
      float getCameraFar() { return this->camera.far; };
      vec3  getCameraUp() { return this->camera.up; };
      float getCameraAzimuth() { return this->camera.azimuth; };
      float getCameraElevation() { return this->camera.elevation; };
      float getLookRadius() { return this->camera.radius; };
      vec3  getCameraXAxis() { return this->camera.xAxis; };
      vec3  getCameraYAxis() { return this->camera.yAxis; };
      vec3  getCameraZAxis() { return this->camera.zAxis; };

      void setCameraFOV(float FOV) { this->camera.FOV= FOV; };
      void setCameraAspect(float aspect) { this->camera.aspect= aspect; };
      void setCameraNear(float near) { this->camera.near= near; };
      void setCameraFar(float far) { this->camera.far= far; };
      void setCameraUp(vec3 up) { this->camera.up= up; };
      void setCameraAzimuth(float azimuth) { this->camera.azimuth= azimuth; };
      void setCameraElevation(float elevation) { this->camera.elevation= elevation; };
      void setLookRadius(float radius) { this->camera.radius= radius; };
      void setCameraXAxis(vec3 xAxis) { this->camera.xAxis= xAxis; };
      void setCameraYAxis(vec3 yAxis) { this->camera.yAxis= yAxis; };
      void setCameraZAxis(vec3 zAxis) { this->camera.zAxis= zAxis; };

			void appendChild(Object obj) { children.push_back(obj); }
			std::vector<Object>& getChildren() { return this->children; };

			Object& getFlashlight() { return this->children[0]; };

			float getCameraTargetAzimuth() { return this->camera.targetAzimuth; };
			void  setCameraTargetAzimuth(float azimuth) { this->camera.targetAzimuth= azimuth; };

			float getCameraTargetElevation() { return this->camera.targetElevation; };
			void  setCameraTargetElevation(float elevation) { this->camera.targetElevation= elevation; };
			
			int getPagesCollected() { return this->pagesCollected; };
			void incrementPagesCollected() { this->pagesCollected++; };
    private:
			vec3 targetPosition= vec3(0);
			

      int health= 100; // player should always start with 100 health
      float velocity= 1.0f;
      
      CameraInfo camera;

      // flashlight should be the first element
      std::vector<Object> children;

			int pagesCollected= 0;


  };
}


#endif