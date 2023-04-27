// Bryn Mawr College, alinen, 2020
//
/**
 * Lerping Cameras: 
 * https://superhedral.com/2021/10/30/lerping-cameras-in-unity/
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

	void render(Renderer& renderer, float planeLocationY) {
		renderer.scale(vec3(this->widthRatio * this->yScale, this->yScale, 1));
		renderer.translate(vec3(-0.5, -0.5, 0));
		renderer.quad();
	}
};

struct Grass: public Billboard {};

struct Tree: public Billboard {};


class Viewer : public Window {
  public:
    Viewer() : Window() {
    }

		float randBound(float lowerBound, float upperBound) {
    	return rand() / float(RAND_MAX) * (upperBound - lowerBound) + lowerBound;
  	}

		void initPlayerFlashlight() {
			Image img;
			img.load("../textures/flashlight/flashlight.jpg", true);
			renderer.loadTexture("flashlightTex", img, 0);

			vec3 pos= vec3(-0.1, -0.1, 0.15);
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
				if (cell != vec2(-1)) {
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
			this->gridTrees= vector<vector<vec2>>(numXCells, std::vector<vec2>(numZCells, vec2(-1)));

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
					vec2 newPoint= generateRandomPointAround(p, treeCellSize+1);

					if (inRectangle(newPoint) && !inNeighborhood(gridTrees, newPoint, treeCellSize+1)) {
						treeCellProcessor.push_back(newPoint);
						samplePoints.push_back(newPoint);

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
				tree.pos= vec3(point.x,
					tree.yTranslate, point.y);

				int texIndex= rand() % 2;
				tree.texture= treeTextures[texIndex];
				tree.widthRatio= treeRatios[texIndex];

				treeParticles[i]= tree;
			}
		}


		void drawRenderingItems()
		{
			vec3 cameraPos = renderer.cameraPosition();

			// we sort by descending order, so we render farther ones first
			std::sort(renderingItems.begin(), renderingItems.end(), [&](RenderingItem* b1, RenderingItem* b2) 
			{
				float dSqr1 = length2(b1->pos - cameraPos);
				float dSqr2 = length2(b2->pos - cameraPos);
				
				return (dSqr1 > dSqr2);
			});


			renderer.beginShader("spotlight");
				for (auto* item : renderingItems) {
					if (item->isVisible) {
						initSpotlightShader(item->texture, vec2(1), item->useAlpha);
						renderer.push();
							renderer.translate(item->pos);
							renderer.rotate(item->calculateHeading(player.getPos()), item->headingAxis);
							item->render(renderer, planeLocation.y);
						renderer.pop();
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
			billboards.reserve(treeParticles.size() + numGrass);

			billboards.insert(billboards.end(), std::begin(grassParticles), std::end(grassParticles));
			billboards.insert(billboards.end(), std::begin(treeParticles), std::end(treeParticles));

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

			for (int i= 0; i < billboards.size(); i++) {
				renderingItems.push_back(&billboards[i]);
			}
			renderingItems.push_back(&slenderman);
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

    void mouseDown(int button, int mods) {
    }

    void mouseUp(int button, int mods) {
    }

    void scroll(float dx, float dy) {
    }

    // handles WASD release
    void keyUp(int key, int mods) {
			if (key == GLFW_KEY_LEFT_SHIFT) {
				player.setVelocity(player.getVelocity() * 0.5f);
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
				player.setVelocity(player.getVelocity() * 2);
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
			}
    }

    // updates the eyePos when the user presses WASD
    void updatePlayerPosition() {
			vec3 targetPos= player.getPos();


      if (WASD_KEY_HELD[W_KEY]) {
        //player.moveForward(dt());
				float velocity= player.getVelocity();
				vec3 zAxis= player.getZAxis();

				targetPos+= velocity * zAxis;
      }

      if (WASD_KEY_HELD[A_KEY]) {
        //player.moveLeft(dt());

				float velocity= player.getVelocity();
				vec3 xAxis= player.getXAxis();

				targetPos+= velocity * xAxis;
      }

      if (WASD_KEY_HELD[S_KEY]) {
        //player.moveBackward(dt());
				
				float velocity= player.getVelocity();
				vec3 zAxis= player.getZAxis();

				targetPos-= velocity * zAxis;
      }

      if (WASD_KEY_HELD[D_KEY]) {
        //player.moveRight(dt());
				float velocity= player.getVelocity();
				vec3 xAxis= player.getXAxis();

				targetPos-= velocity * xAxis;
      }

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

    void initSpotlightShader(const std::string& texture, vec2 uvScale, bool useAlpha) {
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
    }

    void draw() {
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


      updatePlayerPosition();

			lateUpdate();

      player.setCameraAspect(((float) width()) / height());
      renderer.perspective(player.getCameraFOV(), player.getCameraAspect(), 
        player.getCameraNear(), player.getCameraFar());
      
      renderer.lookAt(player.getPos(), player.getLookPos(), player.getCameraUp());


      // draw plane
			
			renderer.beginShader("spotlight");
				initSpotlightShader("dead_grass", vec2(planeScale.x, planeScale.z), false);
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
					initSpotlightShader(child.getTexture(), vec2(1), false);
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

    }

		void lateUpdate() {
			vec3 curPlayerPos= LERP(player.getPos(), player.getTargetPosition(), dt());
			player.setPos(curPlayerPos);
			player.setTargetPosition(curPlayerPos);

		}

		vec3 LERP(const vec3& a, const vec3& b, float t) {
			return a * (1 - t) + b * t;
		}

  protected:
    Player player;
    Object slenderman;

    vec4 lightPosition;
    vec3 lightIntensityAmbient;
    vec3 lightIntensityDiffuse;
    vec3 lightIntensitySpecular;

		vec3 secondDiffuseIntensity= vec3(0);
		vec3 secondSpecularIntensity= vec3(0);

    bool WASD_KEY_HELD[4]= {false, false, false, false};
    float stepSize= 0.1;


		// plane information
		vec3 planeScale= vec3(100.0f, 0.1f, 100.0f);
		vec3 planeLocation= vec3(0.0f, -0.5f, 0.0f);
		float xDim;
		float zDim;
		
		// grass information
		const int numGrass= 500;
		Grass grassParticles[500];

		// tree information
		vector<Tree> treeParticles;

		// billboard information
		vector<Billboard> billboards;

		vector<RenderingItem*> renderingItems;

		// model information
		std::map<string, PLYMesh> models;

		enum GameStatus {WIN, LOSE, ONGOING};
		GameStatus gameStatus= ONGOING;

		vector<vector<vec2>> gridTrees;
		int numXCells;
		int numZCells;
		float treeCellSize= 2.0f;
		int numPointsAround= 20;
};

int main(int argc, char** argv)
{
  Viewer viewer;
  viewer.run();
  return 0;
}
