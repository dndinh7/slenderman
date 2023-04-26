
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
using namespace std;

struct Object {
  public:
		Object() : pos(vec3(0)), texture(""), scale(vec3(1)) {};

    Object(PLYMesh mesh, std::string texture, vec3 pos= vec3(0, 0, 0), 
			vec3 scale= vec3(1), quat rot= quat(vec3(0, 0, 0))) : 
      pos(pos), rot(rot), scale(scale), mesh(mesh), texture(texture) 
			{
				this->minBounds= mesh.minBounds() * scale;
				this->maxBounds= mesh.maxBounds() * scale;

				this->dimensions= this->maxBounds - this->minBounds;
				finalRot= rot;
			};

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
		void appendChild(Object obj) { children.push_back(obj); }

		PLYMesh& getMesh() { return this->mesh; };
		std::string& getTexture() { return this->texture; };

		vec3 getMinBounds() { return this->minBounds; };
		vec3 getMaxBounds() { return this->maxBounds; };

		vec3 getMidPoint() { return (this->maxBounds + this->minBounds) * 0.5f; };

		vec3 getDimensions() { return this->dimensions;	};

		quat getRot() { return this->finalRot; };
		void setHeading(float heading) { 
			this->heading= heading;

			// for some reason z is now up :(
			finalRot= rot * quat(vec3(0, heading, 0));
		}

		// easy access and change
    vec3 pos= vec3(0, 0, 0);
		vec3 scale= vec3(1);
		float heading= 0.0f;

		bool visible= false;


	private:
		vec3 minBounds;
		vec3 maxBounds;
		PLYMesh mesh;
		std::string texture;
		vec3 dimensions= vec3(0);
		// initial rot to get mesh upright
		quat rot= quat(vec3(0, 0, 0));

		quat finalRot= quat(vec3(0, 0, 0));
		
		vec3 xAxis= vec3(1,0,0);
		vec3 yAxis= vec3(0,1,0);
		vec3 zAxis= vec3(0,0,1);

		std::vector<Object> children;
};


#endif