// Bryn Mawr College, alinen, 2020
//

#include <cmath>
#include <string>
#include <vector>
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

struct Billboard {
	vec3 pos;
	std::string texture;
	float yScale;
	float yTranslate;
	float widthRatio;
};

struct Grass: public Billboard {
};

struct Tree: public Billboard {

};


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
				cout << s << endl;
			}
		}
		
		void initTrees() {
			Image img;
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

			for (int i= 0; i < numTrees; i++) {
				Tree tree= treeParticles[i];
				tree.yScale= randBound(0.8, 1.20);
				tree.yTranslate= -0.5 + 0.5 * tree.yScale;
				tree.pos= vec3(randBound(-planeScale.x * 0.40, planeScale.x * 0.40),
					tree.yTranslate, randBound(-planeScale.z * 0.40, planeScale.z * 0.40));

				int texIndex= rand() % 2;
				tree.texture= treeTextures[texIndex];
				tree.widthRatio= treeRatios[texIndex];

				treeParticles[i]= tree;
			}
		}

		void initGrass() {
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
		}

		// used as comparator for sorting billboards
		bool comparePosition(Billboard b1, Billboard b2) {
			vec3 cameraPos= renderer.cameraPosition();
			
			float dSqr1 = length2(b1.pos - cameraPos);
			float dSqr2 = length2(b2.pos - cameraPos);
			
			return (dSqr1 < dSqr2);
		}

		void drawBillboards()
		{
			vec3 cameraPos = renderer.cameraPosition();

			// we sort by descending order, so we render farther ones first
			std::sort(billboards.begin(), billboards.end(), [&](Billboard b1, Billboard b2) 
			{
				float dSqr1 = length2(b1.pos - cameraPos);
				float dSqr2 = length2(b2.pos - cameraPos);
				
				return (dSqr1 > dSqr2);
			});


			renderer.beginShader("spotlight");
				for (int i= 0; i < billboards.size(); i++) {
					Billboard billboard= billboards[i];
					initSpotlightShader(billboard.texture, vec2(1));
					renderer.push();
						vec3 n= normalize(player.getPos() - billboard.pos);
						float thetaY= atan2(n.x, n.z);
						renderer.translate(billboard.pos);
						renderer.rotate(thetaY, vec3(0, 1, 0));
						renderer.scale(vec3(billboard.widthRatio * billboard.yScale, billboard.yScale, 1));
						renderer.translate(vec3(-0.5, -0.5, 0));
						renderer.quad();
					renderer.pop();
				}
      renderer.endShader();
		}

    void setup() {
      setWindowSize(1000, 1000);
				 
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

			initGrass();
			//initTrees();

			//billboards.reserve(numTrees + numGrass);
			billboards.reserve(numGrass);
			billboards.insert(billboards.end(), std::begin(grassParticles), std::end(grassParticles));
			//billboards.insert(billboards.end(), std::begin(treeParticles), std::end(treeParticles));

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

			Image img;
			img.load("../textures/slenderman/slender_clothing_base_color.jpg", true);
			renderer.loadTexture("slenderman_base", img, 0);
      slenderman= Object(models["slenderman"], "slenderman_base", vec3(1), vec3(1)); 

			initModels();
			initPlayerFlashlight();


    }

    void mouseMotion(int x, int y, int dx, int dy) {
      // we're subtracting because it's opposite to the eyePos
      if (mouseIsDown(GLFW_MOUSE_BUTTON_RIGHT)) {
        player.setCameraAzimuth(player.getCameraAzimuth() - ((float)dx) * 2.0f * dt());
        float elevation= player.getCameraElevation() - ((float)dy) * 2.0f * dt();

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
      if (WASD_KEY_HELD[W_KEY]) {
        player.moveForward(dt());
      }

      if (WASD_KEY_HELD[A_KEY]) {
        player.moveLeft(dt());
      }

      if (WASD_KEY_HELD[S_KEY]) {
        player.moveBackward(dt());
      }

      if (WASD_KEY_HELD[D_KEY]) {
        player.moveRight(dt());
      }
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

    void initSpotlightShader(const std::string& texture, vec2 uvScale) {
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

      updatePlayerPosition();

      player.setCameraAspect(((float) width()) / height());
      renderer.perspective(player.getCameraFOV(), player.getCameraAspect(), 
        player.getCameraNear(), player.getCameraFar());
      
      renderer.lookAt(player.getPos(), player.getLookPos(), player.getCameraUp());



      // draw plane
      renderer.beginShader("spotlight");
        initSpotlightShader("dead_grass", vec2(10.0));
        renderer.push();
          renderer.translate(planeLocation);
          renderer.scale(planeScale);
          renderer.cube();
        renderer.pop();
      renderer.endShader();

			drawBillboards();


			renderer.beginShader("simple-texture");
				for (Object &child: player.getChildren()) {
					renderer.texture("Image", child.getTexture());
					renderer.push();
						renderer.translate(player.getPos());
						renderer.push();
							renderer.translate(child.pos);
							renderer.scale(child.scale);
							renderer.translate((-child.getMidPoint()));
							renderer.mesh(child.getMesh());
						renderer.pop();
					renderer.pop();
				}
			renderer.endShader();
		

			renderer.beginShader("spotlight");
				initSpotlightShader(slenderman.getTexture(), vec2(1));
				renderer.push();
					renderer.translate(slenderman.pos);

					renderer.mesh(slenderman.getMesh());
				renderer.pop();

			renderer.endShader();

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
		vec3 planeScale= vec3(10.0f, 0.1f, 10.0f);
		vec3 planeLocation= vec3(0.0f, -0.5f, 0.0f);
		
		// grass information
		float numGrass= 100;
		Grass grassParticles[100];

		// tree information
		float numTrees= 100;
		Tree treeParticles[100];

		// billboard information
		vector<Billboard> billboards;

		// model information
		std::map<string, PLYMesh> models;

		enum GameStatus {WIN, LOSE, ONGOING};
		GameStatus gameStatus= ONGOING;
};

int main(int argc, char** argv)
{
  Viewer viewer;
  viewer.run();
  return 0;
}
