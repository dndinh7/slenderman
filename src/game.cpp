// Bryn Mawr College, alinen, 2020
//

/**
 * Lerping Cameras: 
 * https://superhedral.com/2021/10/30/lerping-cameras-in-unity/
 * http://devmag.org.za/2009/05/03/poisson-disk-sampling/
 * https://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula
 * https://www.shadertoy.com/view/XtK3W3
 * 
*/


#include <cmath>
#include <string>
#include <vector>
#include <deque>
#include <algorithm>
#include <map>
#include "agl/window.h"
#include "plymesh.h"
#include "osutils.h"
#include "entities/player.h"
#include "objects/object.h"
#include <set>
#include "fmod_errors.h"
#include <cstdlib>
#include <iostream>
#include "fmod.hpp"

using namespace std;
using namespace glm;
using namespace agl;
using namespace assets;

#define M_PI 3.14159265358979323846

enum KEY {
  W_KEY, A_KEY, S_KEY, D_KEY
};



struct Billboard : public RenderingItem {
	float yScale;
	float yTranslate;
	float widthRatio;

	vec3 headingAxis= vec3(0, 1, 0);

	float calculateHeading(vec3 playerPos) {
		vec3 n= normalize(playerPos - this->pos);
		float thetaY= atan2(n.x, n.z);
		return thetaY;
	}

	void render(Renderer& renderer, float planeLocationY, vec3 playerPos) {
		if (isVisible) {
			renderer.push();
				renderer.translate(this->pos);
				renderer.rotate(this->calculateHeading(playerPos), headingAxis);
				renderer.scale(vec3(this->widthRatio * this->yScale, this->yScale, 1));
				renderer.translate(vec3(-0.5, -0.5, 0));
				renderer.quad();
			renderer.pop();
		}
	}
};

struct Grass: public Billboard {};

struct Tree: public Billboard {};

struct Page : public RenderingItem {
	float yScale;
	float widthRatio;

	vec3 headingAxis= vec3(0, 1, 0);
	void render(Renderer& renderer, float planeLocationY, vec3 playerPos) {
		if (isVisible) {
			renderer.push();
				renderer.translate(parent->pos);
				renderer.rotate(parent->calculateHeading(playerPos), headingAxis);

				renderer.translate(this->pos);
				renderer.scale(vec3(this->widthRatio * this->yScale * 0.75, this->yScale, 1));
				renderer.translate(vec3(-0.5, -0.5, 0));
				renderer.quad();
			renderer.pop();
		}
	}

	vec3 getWorldPos(vec3 playerPos) {
		vec3 toPlayer= normalize(playerPos - parent->getWorldPos(playerPos));

		return toPlayer * 0.1f + parent->getWorldPos(playerPos);
	}

	bool isPlayerClose(vec3 playerPos) {
		vec3 toPlayer= playerPos - parent->getWorldPos(playerPos);

		if (length(toPlayer) <= playerCloseRadius) {
			return true;
		}

		return false;
	}

	float playerCloseRadius= 1.0f;

	Tree* parent;
};

class Viewer : public Window {
  public:
    Viewer() : Window() {
    }

	~Viewer() {
		if (crickets != NULL) {
			result = crickets->release();
			ERRCHECK(result);
		}

		if (flashlightButton != NULL) {
			result = flashlightButton->release();
			ERRCHECK(result);
		}

		if (glitching != NULL) {
			result = glitching->release();
			ERRCHECK(result);
		}

		result = system->release();
		ERRCHECK(result);
	}

		float randBound(float lowerBound, float upperBound) {
    	return rand() / float(RAND_MAX) * (upperBound - lowerBound) + lowerBound;
  	}

		void initPages() {
			Image img;
			// names are 1-8
			for (int i= 1; i <= 8; i++) {
				string filename= std::to_string(i) + ".png";
				img.load("../textures/pages/" + filename, true);
				renderer.loadTexture(filename, img, 0);
				Page page;


				page.yScale= 0.3f;
				page.widthRatio= ((float) img.width() / img.height());

				page.pos= vec3(0, -0.50, 0.1f); // local to tree, so we want it to be in front
				page.texture= filename;
				page.usesHeading= false;


				pages.push_back(page);
			}
		}

		void initPlayerFlashlight() {
			Image img;
			img.load("../textures/flashlight/flashlight.jpg", true);
			renderer.loadTexture("flashlightTex", img, 0);

			vec3 pos= vec3(-0.11, -0.11, 0.15);
			vec3 scale= vec3(0.15);

			Object flashlight= Object(models["flashlight-uv"], "flashlightTex", pos, scale);

			player.appendChild(flashlight);
		}

		void initModels() {
			std::vector<string> modelStrings= GetFilenamesInDir("../models", "ply");
			for (int i= 0; i < modelStrings.size(); i++) {
				string s= modelStrings[i];
				// does not get the extension for the key value
				models[s.substr(0, s.size()-4)]= PLYMesh("../models/" + s);
			}
		}
		
		vec2 pointToGrid(vec2 point) {
			int x= (int)(point.x / treeCellSize);
			int z= (int)(point.y / treeCellSize);

			return vec2(x, z);
		}

		Tree createTree(vec2 point, float widthRatio, string tex) {
			Tree tree;
			tree.yScale= randBound(1.5, 2);
			tree.yTranslate= -0.5 + 0.5 * tree.yScale;
			tree.pos= vec3(point.x, tree.yTranslate, point.y);

			tree.texture= tex;
			tree.widthRatio= widthRatio;

			return tree;
		}

		vec2 generateRandomPointAround(vec2 point, float minDist) {
			float r1= randBound(0.0, 1.0);
			float r2= randBound(0.0f, 1.0f);

			// random radius
			float radius= minDist * (r1 + 1);

			float angle= 2 * M_PI * r2;

			float x= point.x + radius * cos(angle);
			float z= point.y + radius * sin(angle);

			return vec2(x, z);

		}

		float dist(vec2 p1, vec2 p2) {
			return sqrt(pow((p1.x - p2.x), 2) + pow((p1.y - p2.y), 2));
		}

		vector<vec2> squareAroundPoint(const vector<vector<vec2>>& grid, vec2 gridPointIdx, int square) {
			vector<vec2> gridPointIndices;

			for (int i= -square/2; i <= square/2; i++) {
				for (int j= -square/2; j <= square/2; j++) {
					int xIdx= gridPointIdx.x - i;
					int zIdx= gridPointIdx.y - j;
					if (xIdx < 0 || xIdx >= numXCells) continue; // out of bounds
					if (zIdx < 0 || zIdx >= numZCells) continue;

					gridPointIndices.push_back(grid[xIdx][zIdx]);
				}
			}

			return gridPointIndices;
		}

		// check that the point does not come to close to any other point in the neighborhood
		bool inNeighborhood(const vector<vector<vec2>>& grid, vec2 point, float minDist) {
			vec2 gridPointIdx= pointToGrid(point);

			vector<vec2> cellsAroundPoint= squareAroundPoint(grid, gridPointIdx, 4);

			for (auto cell : cellsAroundPoint) {

				// only negative points are from initializiation
				// so if they are positive, then it is in the neighborhood
				if (cell.x >= 0.0f && cell.y >= 0.0f) {
					if (dist(cell, point) < minDist) {
						return true;
					}
				}
			}
			return false;
		}

		bool inRectangle(vec2 point) {
			return point.x > 0 && point.x < xDim && point.y > 0 && point.y < zDim;
		}

		void initBillboards() {
			Image img;
			float grassRatios[4];
			img.load("../textures/grass_billboards/n_grass_diff_0_18.png", true);
			renderer.loadTexture("grass0", img, 0);
			grassRatios[0]= float(img.width()) / img.height();

			img.load("../textures/grass_billboards/n_grass_diff_0_19.png", true);
			renderer.loadTexture("grass1", img, 0);
			grassRatios[1]= float(img.width()) / img.height();
			
			img.load("../textures/grass_billboards/n_grass_diff_0_52.png", true);
			renderer.loadTexture("grass2", img, 0);
			grassRatios[2]= float(img.width()) / img.height();
			
			img.load("../textures/grass_billboards/n_grass_diff_0_53.png", true);
			renderer.loadTexture("grass3", img, 0);
			grassRatios[3]= float(img.width()) / img.height();

			renderer.blendMode(agl::BLEND);

			std::string grassTextures[4];
			grassTextures[0]= "grass0";
			grassTextures[1]= "grass1";
			grassTextures[2]= "grass2";
			grassTextures[3]= "grass3";
		
			for (int i= 0; i < numGrass; i++) {
				Grass grass= grassParticles[i];
				grass.yScale= randBound(0.10, 0.20);
				grass.yTranslate= -0.5 + 0.5 * grass.yScale;
				grass.pos= vec3(randBound(-planeScale.x * 0.40, planeScale.x * 0.40),
					grass.yTranslate, randBound(-planeScale.z * 0.40, planeScale.z * 0.40));
				int texIndex= rand() % 4;
				grass.texture= grassTextures[texIndex];
				grass.widthRatio= grassRatios[texIndex];

				grassParticles[i]= grass;
			}


			float treeRatios[2];
			img.load("../textures/tree_billboards/fir.png", true);
			renderer.loadTexture("fir", img, 0);
			treeRatios[0]= float(img.width()) / img.height();

			img.load("../textures/tree_billboards/pine.png", true);
			renderer.loadTexture("pine", img, 0);
			treeRatios[1]= float(img.width()) / img.height();

			std::string treeTextures[2];
			treeTextures[0]= "fir";
			treeTextures[1]= "pine";

			numXCells= ceil(xDim/treeCellSize);
			numZCells= ceil(zDim/treeCellSize);
			vector<vector<vec2>> gridTrees= 
				vector<vector<vec2>>(numXCells, std::vector<vec2>(numZCells, vec2(-1)));

			deque<vec2> treeCellProcessor= deque<vec2>();
			vector<vec2> samplePoints= vector<vec2>();

			vec2 firstPoint= vec2(randBound(0, xDim-1), randBound(0, zDim-1));

			treeCellProcessor.push_back(firstPoint);
			samplePoints.push_back(firstPoint);

			vec2 gridIdx= pointToGrid(firstPoint);
			gridTrees[gridIdx.x][gridIdx.y]= firstPoint;

			while (!treeCellProcessor.empty()) {
				vec2 p= treeCellProcessor.front();
				treeCellProcessor.pop_front();

				for (int i= 0; i < numPointsAround; i++) {
					float minDist= treeCellSize;
					vec2 newPoint= generateRandomPointAround(p, minDist);

					if (inRectangle(newPoint) && !inNeighborhood(gridTrees, newPoint, minDist)) {
						treeCellProcessor.push_back(newPoint);

						vec2 newPointIdx= pointToGrid(newPoint);
						samplePoints.push_back(newPoint);
						gridTrees[newPointIdx.x][newPointIdx.y]= newPoint;
					}
				}
			}

			treeParticles.reserve(samplePoints.size());
			for (int i= 0; i < samplePoints.size(); i++) {
				vec2 point= samplePoints[i];
				Tree tree;
				tree.yScale= randBound(1.5, 2);
				tree.yTranslate= -0.5 + 0.5 * tree.yScale;
				tree.pos= vec3(point.x - xDim * 0.5,
					tree.yTranslate, point.y - zDim * 0.5);

				int texIndex= rand() % 2;
				tree.texture= treeTextures[texIndex];
				tree.widthRatio= treeRatios[texIndex];

				treeParticles.push_back(tree);
			}
		}


		void drawRenderingItems()
		{
			vec3 cameraPos = renderer.cameraPosition();

			// we sort by descending order, so we render farther ones first
			std::sort(renderingItems.begin(), renderingItems.end(), [&](RenderingItem* b1, RenderingItem* b2) 
			{
				float dSqr1 = length2(b1->getWorldPos(cameraPos) - cameraPos);
				float dSqr2 = length2(b2->getWorldPos(cameraPos) - cameraPos);
				
				return (dSqr1 > dSqr2);
			});


			renderer.beginShader("spotlight");
				for (auto* item : renderingItems) {
					if (item->isVisible) {
						initSpotlightShader(item->texture, vec2(1), item->useAlpha, item->useFog);
						item->render(renderer, planeLocation.y, player.getPos());
					}	
				}
      renderer.endShader();
		}

    void setup() {
      setWindowSize(1000, 1000);

			xDim= planeScale.x;
			zDim= planeScale.z;
				 
      renderer.loadShader("simple-texture",
        "../shaders/simple-texture.vs",
        "../shaders/simple-texture.fs");

      renderer.loadShader("spotlight",
        "../shaders/spotlight.vs",
        "../shaders/spotlight.fs");

      renderer.loadShader("fog",
        "../shaders/fog.vs",
        "../shaders/fog.fs");

      renderer.loadShader("only-color",
        "../shaders/only-color.vs",
        "../shaders/only-color.fs");

      this->lightPosition= vec4(0.0f, 5.0f, 0.0f, 1.0f); 

      this->lightIntensityAmbient= vec3(0.825f);
      this->lightIntensityDiffuse= vec3(0.825f);
      this->lightIntensitySpecular= vec3(0.5f);

      renderer.loadTexture("dead_grass", "../textures/dead_grass.png", 0);

			initBillboards();
			initPages();

      // init camera
      CameraInfo camera;

      camera.FOV= glm::radians(60.0f);
      camera.aspect= ((float) width()) / height();
      camera.near= 0.1f;
      camera.far= 50.0f;
      camera.up= vec3(0,1,0);
      camera.azimuth= 0.0f;
      camera.elevation= 0.0f;
      camera.radius= 1.0f;


      // init player and slenderman
      player= Player(vec3(0), vec3(0,0,1), camera);


			initModels();
			initPlayerFlashlight();

			Image img;
			img.load("../textures/slenderman.PNG", true);
			renderer.loadTexture("slenderman_base", img, 0);
      slenderman= Object(models["slenderman"], "slenderman_base", vec3(0, 0, 0), 
				vec3(0.283), quat(vec3(0, 0, 0)));
			
			slenderman.isVisible= false;
			slenderman.useFog= false;

			slenderman.pos= vec3(0, -0.5 + slenderman.getDimensions().y / 2, 0);

			for (int i= 0; i < treeParticles.size(); i++) {
				renderingItems.push_back(&treeParticles[i]);
			}

			for (int i= 0; i < numGrass; i++) {
				renderingItems.push_back(&grassParticles[i]);
			}

			renderingItems.push_back(&slenderman);

			// ensure that a tree does not have multiple pages
			set<int> treeIndicesUsed;
			for (int i= 0; i < pages.size(); i++) {
				int randTreeIdx= rand() % treeParticles.size();
				while (treeIndicesUsed.count(randTreeIdx) == 1) {
					randTreeIdx= rand() % treeParticles.size();
				}
				treeIndicesUsed.insert(randTreeIdx);
				pages[i].parent= &treeParticles[randTreeIdx];
				renderingItems.push_back(&pages[i]);
			}


			// SOUNDS ------------------------------------
			initSounds();

    }

	void ERRCHECK(FMOD_RESULT result) {
		if (result != FMOD_OK)
		{
			printf("FMOD error! (%d) %s\n",
				result, FMOD_ErrorString(result));
			exit(-1);
		}
	}

	void initSounds() {
		result = FMOD::System_Create(&system);
		ERRCHECK(result);

		result = system->init(100, FMOD_INIT_NORMAL, 0);
		ERRCHECK(result);

		// background noise
		result = system->createStream(
			"../sounds/crickets.wav",
			FMOD_DEFAULT, 0, &crickets);
		ERRCHECK(result);

		result = crickets->setMode(FMOD_LOOP_NORMAL);
		ERRCHECK(result);

		// set volume while paused
		result = system->playSound(crickets, 0, true, &backgroundChannel);
		ERRCHECK(result);

		result = backgroundChannel->setVolume(0.15f);
		ERRCHECK(result);

		result = backgroundChannel->setPaused(false);
		ERRCHECK(result);

		// init foreground

		result = system->createStream(
			"../sounds/flashlight.wav",
			FMOD_DEFAULT, 0, &flashlightButton);
		ERRCHECK(result);

		result = system->createStream(
			"../sounds/slender_static.mp3",
			FMOD_DEFAULT, 0, &glitching);
		ERRCHECK(result);


	}

    void mouseMotion(int x, int y, int dx, int dy) {
      // we're subtracting because it's opposite to the eyePos
      if (mouseIsDown(GLFW_MOUSE_BUTTON_RIGHT)) {
        player.setCameraAzimuth(player.getCameraAzimuth() - dx * 0.01f);
        float elevation= player.getCameraElevation() - dy * 0.01f;

        // clamp between (-pi/2, pi/2), don't want the bounds since it might lead to weird rotation
        elevation= std::max(elevation, (float) (-M_PI/2) + 0.00001f);
        elevation= std::min(elevation, (float) (M_PI/2)  - 0.00001f);

        player.setCameraElevation(elevation);
      }
    }


    void spawnSlender() {
			if (player.getPagesCollected() > 0) {
				if (slendermanVisibleTime < 0) {
					//slendermanVisibleTime= randBound(9, 19);
					slendermanVisibleTime= randBound(3.23, 7.8);
				}

				if (slendermanSpawnTime < 0) {
					//slendermanSpawnTime= randBound(25.0f, 48.0f);
					slendermanSpawnTime= randBound(3.5, 12.5);
				}

				// cout << "spawn: " << slendermanSpawnTime << endl;
				// cout << "spawnTime: " << timeSinceLastSpawn << endl;
				// cout << "vis: " << slendermanVisibleTime << endl;
				// cout << "visTime: " << timeSinceVisibility << endl;


				// make him appear 
				if (timeSinceLastSpawn >= slendermanSpawnTime && !slenderman.isVisible) {
					// want the position to be behind the player
					vec3 v= normalize(player.getZAxis());
					vec3 k= vec3(0, 1, 0);

					// want slenderman to spawn behind the player
					float randAngle= glm::radians(randBound(90, 270));
					float randRadius= randBound(2.2, 7.8);

					vec3 vRot= v*cos(randAngle) +
						cross(k, v)*sin(randAngle) +
						k*(dot(k, v)) * (1-cos(randAngle));

					vec3 pPos= player.getPos();

					vec3 newSlenderPos= pPos + vRot * randRadius;
					newSlenderPos.y= slenderman.pos.y; // y should stay static
					slenderman.pos= newSlenderPos;

					slenderman.isVisible= true;
					slendermanSpawnTime= -1.0f;
					timeSinceVisibility= 0.0f;
				}


				// make him disappear when the player is not looking at him
				if (timeSinceVisibility >= slendermanVisibleTime && slenderman.isVisible) {
					
					vec3 toSlender= slenderman.pos - player.getPos();
					// get rid of the y axis, since his midpoint is higher anyway
					toSlender.y= 0;
					vec3 playerForward= player.getZAxis();
					float slenderDotPlayer= dot(toSlender, playerForward);
					
					if (slenderDotPlayer < 0) {	
						
						slenderman.isVisible= false;
						slendermanVisibleTime= -1.0f;
						timeSinceLastSpawn= 0.0f;
					}
				}

				if (!slenderman.isVisible) {
					timeSinceLastSpawn+= dt();	
				} else {
					timeSinceVisibility+= dt();
				}
				

			}
		}

		void checkPlayerLookingAtSlender() {
			if (slenderman.isVisible) {
				vec3 toSlender= slenderman.pos - player.getPos();
				// get rid of the y axis, since his midpoint is higher anyway
				toSlender.y= 0;

				vec3 playerForward= player.getZAxis();

				float slenderDotPlayer= dot(toSlender, playerForward);

				// we are looking at some sort of slender
				if (slenderDotPlayer > 0) {

					float ratio= dot(toSlender, playerForward) / 
						(length(toSlender) * length(playerForward));
					
					float angle= acos(ratio);

					// fov of 50 to get hurt
					if (angle < hurtAngle) {
						if (angle < glitchAngle) slenderman.useGlitch= true; // want to glitch it here
						else slenderman.useGlitch= false;

						// currently getting damaged
						timeSinceDamage= 0.0f;

						// should hurt the most when the angle is 0 :)
						float hurtRatio= (hurtAngle - angle) / hurtAngle;

						// will take directDmg per second if they stare directly at
						// slender
						player.decreaseHealth(hurtRatio * directDmg * dt());


						// sound to indicate he's here

						if (timeSinceJumpScareSound > jumpScareSoundDuration) {
							result = system->playSound(glitching, 0, true, &glitchChannel);
							ERRCHECK(result);

							result = glitchChannel->setVolume(0.25f);
							ERRCHECK(result);

							result = glitchChannel->setPaused(false);
							ERRCHECK(result);

							timeSinceJumpScareSound = 0.0f;
						}


					} else {
						slenderman.useGlitch= false;
						if (timeSinceDamage >= timeToRecover) { 
							player.increaseHealth(HPS * dt());
						} else {
							timeSinceDamage+= dt();
						}

					}
				} else {
					slenderman.useGlitch= false;
					if (timeSinceDamage >= timeToRecover) { 
						player.increaseHealth(HPS * dt());
					} else {
						timeSinceDamage+= dt();
					}
				}
			}
		}

		void checkPageProximity() {
			for (auto& page: pages) {
				// collect a page
				if (page.isPlayerClose(player.getPos()) && keyIsDown(GLFW_KEY_E) && page.isVisible) {
					page.isVisible= false;
					player.incrementPagesCollected();
					cout << player.getPagesCollected() << endl;
				}
			}
		}

		void isWin() {
			if (player.getPagesCollected() == 8) {
				gameStatus= WIN;
			}
		}

		void isLose() {
			if (player.getHealth() <= 0) {
				gameStatus= LOSE;
			}
		}

    // handles WASD release
    void keyUp(int key, int mods) {
			if (key == GLFW_KEY_LEFT_SHIFT) {
				player.setVelocity(player.getVelocity() / 1.5f);
			}

      if (key == GLFW_KEY_W) {
        WASD_KEY_HELD[W_KEY]= false;
      }

      if (key == GLFW_KEY_A) {
        WASD_KEY_HELD[A_KEY]= false;
      }

      if (key == GLFW_KEY_S) {
        WASD_KEY_HELD[S_KEY]= false;
      }

      if (key == GLFW_KEY_D) {
        WASD_KEY_HELD[D_KEY]= false;
      }
    }

    // handles WASD press/hold
    void keyDown(int key, int mods) {
	if (key == GLFW_KEY_LEFT_SHIFT) {
		player.setVelocity(player.getVelocity() * 1.5f);
	}

      if (key == GLFW_KEY_W) {
        WASD_KEY_HELD[W_KEY]= true;
      }

      if (key == GLFW_KEY_A) {
        WASD_KEY_HELD[A_KEY]= true;
      }

      if (key == GLFW_KEY_S) {
        WASD_KEY_HELD[S_KEY]= true;
      }

      if (key == GLFW_KEY_D) {
        WASD_KEY_HELD[D_KEY]= true;
      }


		if (key == GLFW_KEY_F) {
			vec3 tempDiffuse= this->lightIntensityDiffuse;
			vec3 tempSpecular= this->lightIntensitySpecular;
			this->lightIntensityDiffuse= this->secondDiffuseIntensity;
			this->lightIntensitySpecular= this->secondSpecularIntensity;

			this->secondDiffuseIntensity= tempDiffuse;
			this->secondSpecularIntensity= tempSpecular;

			result = system->playSound(flashlightButton, 0, true, &userChannel);
			ERRCHECK(result);

			result = userChannel->setVolume(0.3f);
			ERRCHECK(result);

			result = userChannel->setPaused(false);
			ERRCHECK(result);


		}
    }

    // updates the eyePos when the user presses WASD
    void updatePlayerPosition() {
			vec3 targetPos= player.getTargetPosition();


      if (WASD_KEY_HELD[W_KEY]) {
        //player.moveForward(dt());
				float velocity= player.getVelocity();
				vec3 zAxis= player.getZAxis();

				targetPos+= velocity * dt() * zAxis;
      }

      if (WASD_KEY_HELD[A_KEY]) {
        //player.moveLeft(dt());

				float velocity= player.getVelocity();
				vec3 xAxis= player.getXAxis();

				targetPos+= velocity * dt() * xAxis;
      }

      if (WASD_KEY_HELD[S_KEY]) {
        //player.moveBackward(dt());
				
				float velocity= player.getVelocity();
				vec3 zAxis= player.getZAxis();

				targetPos-= velocity * dt() * zAxis;
      }

      if (WASD_KEY_HELD[D_KEY]) {
        //player.moveRight(dt());
				float velocity= player.getVelocity();
				vec3 xAxis= player.getXAxis();

				targetPos-= velocity * dt() * xAxis;

      }


		targetPos.x= clamp(targetPos.x, -xDim / 2, xDim / 2);
		targetPos.z= clamp(targetPos.z, -zDim / 2, zDim / 2);

		player.setTargetPosition(targetPos);
    }

    void updateLookPos() {
      float azimuth= player.getCameraAzimuth();
      float elevation= player.getCameraElevation();

      vec3 eyePos= player.getPos();
      vec3 lookPos= player.getLookPos();
      float lookRadius= player.getLookRadius();

      lookPos.x= lookRadius * sin(azimuth) * cos(elevation) + eyePos.x;
      lookPos.y= lookRadius * sin(elevation) + eyePos.y;
      lookPos.z= lookRadius * cos(azimuth) * cos(elevation) + eyePos.z;

      player.setLookPos(lookPos);

      player.setXAxis(vec3(sin(azimuth + M_PI/2), 0, cos(azimuth + M_PI/2)));
      player.setZAxis(vec3(sin(azimuth), 0, cos(azimuth)));
    }

    void initSpotlightShader(const std::string& texture, vec2 uvScale, bool useAlpha, bool useFog) {
      vec3 Ka= vec3(0.1f);
      vec3 Kd= vec3(0.775f, 0.0f, 0.0f);
      vec3 Ks= vec3(0.1f, 0.1f, 0.1f);

      float shininess= 128.0f * 0.10f;

      vec4 lightPos_eye= renderer.viewMatrix() * vec4(player.getPos(), 1.0f);
      vec3 lightDir= renderer.viewMatrix() * vec4(normalize(player.getLookPos() - 
        player.getPos()), 0.0f);

      float lightExp= 1.0f;
      float innerCutOff= cos(radians(7.5f));

      float outerCutOff= cos(radians(17.5f));

      renderer.setUniform("Spot.pos", lightPos_eye);
      renderer.setUniform("Spot.intensityAmbient", lightIntensityAmbient);
      renderer.setUniform("Spot.intensityDiffuse", lightIntensityDiffuse);
      renderer.setUniform("Spot.intensitySpecular", lightIntensitySpecular);
      renderer.setUniform("Spot.dir", lightDir);
      renderer.setUniform("Spot.exp", lightExp);
      renderer.setUniform("Spot.innerCutOff", innerCutOff);
      renderer.setUniform("Spot.outerCutOff", outerCutOff);
      
      renderer.setUniform("Material.Ka", Ka);
      renderer.setUniform("Material.Kd", Kd);
      renderer.setUniform("Material.Ks", Ks);
      renderer.setUniform("Material.alpha", shininess);
      renderer.setUniform("uvScale", uvScale);
			renderer.setUniform("useAlpha", useAlpha);
			
			renderer.texture("diffuseTexture", texture);

			// fog info
      renderer.setUniform("Fog.maxDist", 5.0f);
      renderer.setUniform("Fog.minDist", 1.75f);
      // this is gray fog
      //vec3 c= vec3(0xab/255.0f, 0xae/255.0f, 0xb0/255.0f);
      // but I like the black fog better
      vec3 c= vec3(0.1f);
      renderer.setUniform("Fog.color", c);
			renderer.setUniform("useFog", useFog);

			// glitches
			renderer.setUniform("iResolution", vec2(width(), height()));
			renderer.setUniform("iTime", elapsedTime());
			renderer.setUniform("useGlitch", slenderman.useGlitch);
    }

		void randomLosingGlitches() {
			if (randTimeLoseGlitch < 0.0f) {
				randTimeLoseGlitch= randBound(1.5, 3.5);
			}

			if (randTimeGlitching < 0.0f) {
				randTimeGlitching= randBound(0.1, 0.5);
			}

			// cout << "randTime: " << randTimeLoseGlitch << endl;
			// cout << "randTimeSince: " << timeSinceLoseGlitch << endl;
			// cout << "timeGlitch: " << randTimeGlitching << endl;
			// cout << "timeGlitchDuring: " << timeGlitching << endl;

			if (timeGlitching >= randTimeGlitching && slenderman.useGlitch) {
				randTimeGlitching= -1.0f;
				timeSinceLoseGlitch= 0.0f;
				slenderman.useGlitch= false;
			}

			if (timeSinceLoseGlitch >= randTimeLoseGlitch && !slenderman.useGlitch) {
				randTimeLoseGlitch= -1.0f;
				timeGlitching= 0.0f;
				slenderman.useGlitch= true;
			}

			if (slenderman.useGlitch) {
				timeGlitching+= dt();
			} else {
				timeSinceLoseGlitch+= dt();
			}
		}

    void draw() {
			if (gameStatus == ONGOING) {
				// update player position
				updateLookPos();

				mat4 VM= renderer.viewMatrix();
				vec3 xAxis= vec3(VM[0][0], VM[1][0], VM[2][0]);
				vec3 yAxis= vec3(VM[0][1], VM[1][1], VM[2][1]);
				vec3 zAxis= vec3(VM[0][2], VM[1][2], VM[2][2]);

				player.setCameraXAxis(xAxis);
				player.setCameraYAxis(yAxis);
				player.setCameraZAxis(zAxis);

				quat orientation= quat(vec3(-player.getCameraElevation(), player.getCameraAzimuth(), 0));
				orientation= normalize(orientation);

				checkPageProximity();

				checkPlayerLookingAtSlender();

				isWin();

				isLose();

				spawnSlender();

				updatePlayerPosition();

				lateUpdate();

				timeSinceJumpScareSound += dt();

				player.setCameraAspect(((float) width()) / height());
				renderer.perspective(player.getCameraFOV(), player.getCameraAspect(), 
					player.getCameraNear(), player.getCameraFar());
				
				renderer.lookAt(player.getPos(), player.getLookPos(), player.getCameraUp());


				// draw plane
				
				renderer.beginShader("spotlight");
					initSpotlightShader("dead_grass", vec2(planeScale.x, planeScale.z), false, true);
					renderer.push();
						renderer.translate(planeLocation);
						renderer.scale(planeScale);
						renderer.cube();
					renderer.pop();
				renderer.endShader();


				drawRenderingItems();

				
				renderer.beginShader("spotlight");
					renderer.push();
					renderer.translate(player.getPos());
					renderer.rotate(orientation);
					for (Object &child: player.getChildren()) {
						initSpotlightShader(child.getTexture(), vec2(1), false, true);
							renderer.push();
								renderer.translate(child.pos);
								renderer.scale(child.scale);
								renderer.translate((-child.getMidPoint()));
								renderer.mesh(child.getMesh());
							renderer.pop();
						renderer.pop();
					}
				renderer.endShader();
				

				/*	
				renderer.beginShader("spotlight");
					initSpotlightShader(slenderman.getTexture(), vec2(1));
					renderer.push();
						renderer.translate(slenderman.pos);
						renderer.rotate(slenderman.calculateHeading(player.getPos()), slenderman.headingAxis);
						slenderman.render(renderer, planeLocation.y);
					renderer.pop();
				renderer.endShader();
				*/
			} else if (gameStatus == WIN) {
				renderer.fontColor(glm::vec4(0.95, 1.0, 0, 0.8));
				std::string message = "YOU WIN! :]";
				renderer.fontSize(128);
				float x = 500 - renderer.textWidth(message) * 0.5f;
				float y = 500;
				renderer.text(message, x, y);
			} else {
				player.setCameraAspect(((float) width()) / height());
				renderer.perspective(player.getCameraFOV(), player.getCameraAspect(), 
					player.getCameraNear(), player.getCameraFar());

				renderer.lookAt(player.getPos(), player.getLookPos(), player.getCameraUp());
				slenderman.pos= player.getLookPos();

				/*
					renderer.fontColor(glm::vec4(0.95, 0.0, 0, 0.8));
					renderer.fontSize(128);
					std::string message = "YOU LOSE! :[";
					float x = 500 - renderer.textWidth(message) * 0.5f;
					float y = 500;
					renderer.text(message, x, y);
				*/
				
				randomLosingGlitches();
				renderer.beginShader("spotlight");
					this->lightIntensityDiffuse= vec3(0);
					this->lightIntensitySpecular= vec3(0);
					initSpotlightShader(slenderman.texture, vec2(1), false, false);
					slenderman.render(renderer, planeLocation.y, player.getPos());
				renderer.endShader();
			}

    }

		void lateUpdate() {
			vec3 curPlayerPos= LERP(player.getPos(), player.getTargetPosition(), 0.25);
			player.setPos(curPlayerPos);


		}

		vec3 LERP(const vec3& a, const vec3& b, float t) {
			return a * (1 - t) + b * t;
		}

  protected:
    Player player;
    Object slenderman;

		// time since the audio was played, I really don't know how to stop playing
		// the sound, so I time it since the last audio file
		float timeSinceJumpScareSound = 20.0f;
		float jumpScareSoundDuration = 6.3f;

		// will randomly glitch slenderman
		float randTimeLoseGlitch= -1.0f;
		float timeSinceLoseGlitch= 0.0f;

		float randTimeGlitching= -1.0f;
		float timeGlitching= 0.0f;

		// the player will lose health 
		float hurtAngle= radians(50.0f);
		float glitchAngle= radians(10.0f);
		float directDmg= 35.0f;
		// hp per second
		float HPS= 3.0f;

		float timeSinceDamage= 0.0f;
		float timeToRecover= 2.0f;

		// in seconds
		float slendermanVisibleTime= -1.0f;
		float slendermanSpawnTime= -1.0f;
		float timeSinceLastSpawn= 0.0f;
		float timeSinceVisibility= 0.0f;

    vec4 lightPosition;
    vec3 lightIntensityAmbient;
    vec3 lightIntensityDiffuse;
    vec3 lightIntensitySpecular;

		vec3 secondDiffuseIntensity= vec3(0);
		vec3 secondSpecularIntensity= vec3(0);

    bool WASD_KEY_HELD[4]= {false, false, false, false};
    float stepSize= 0.1;


		// plane information
		vec3 planeScale= vec3(45.0f, 0.01f, 45.0f);
		vec3 planeLocation= vec3(0.0f, -0.5f, 0.0f);
		float xDim;
		float zDim;
		
		// grass information
		const int numGrass= 1;
		Grass grassParticles[1];

		// tree information
		vector<Tree> treeParticles;

		vector<RenderingItem*> renderingItems;

		// model information
		std::map<string, PLYMesh> models;

		enum GameStatus {WIN, LOSE, ONGOING};
		GameStatus gameStatus= ONGOING;

		// pages
		vector<Page> pages;

		// pages collected info
		int pagesX= 750;
		int pagesY= 100;

		
		int numXCells;
		int numZCells;
		float treeCellSize= 1.85f;
		int numPointsAround= 15;

	// sounds
		FMOD_RESULT result;
		FMOD::System* system = NULL;
		FMOD::Channel* backgroundChannel = NULL;
		FMOD::Channel* glitchChannel = NULL;
		FMOD::Channel* userChannel = NULL;
		FMOD::Sound* flashlightButton;
		FMOD::Sound* glitching;
		FMOD::Sound* crickets;
};

int main(int argc, char** argv)
{
  Viewer viewer;
  viewer.run();
  return 0;
}
