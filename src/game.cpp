// Bryn Mawr College, alinen, 2020
//

#include <cmath>
#include <string>
#include <vector>
#include "agl/window.h"
#include "plymesh.h"
#include "osutils.h"
#include "entities/player.h"
#include "entities/enemy.h"
#include "objects/object.h"

using namespace std;
using namespace glm;
using namespace agl;
using namespace assets;

#define M_PI 3.14159265358979323846

enum KEY {
  W_KEY, A_KEY, S_KEY, D_KEY
};

class Viewer : public Window {
  public:
    Viewer() : Window() {
    }

    void setup() {
      setWindowSize(1000, 1000);

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
      this->lightIntensity= vec3(0.75f);

      renderer.loadTexture("dead_grass", "../textures/dead_grass.png", 0);

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

      PLYMesh enemyMesh= PLYMesh();
      slenderman= Enemy(vec3(1), vec3(0), enemyMesh, "placeholder"); 
    }

    void mouseMotion(int x, int y, int dx, int dy) {
      // we're subtracting because it's opposite to the eyePos
      if (keyIsDown(GLFW_KEY_LEFT_CONTROL)) {
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

    void initSpotlightShader() {
      vec3 Ka= vec3(0.1f);
      vec3 Kd= vec3(0.775f, 0.0f, 0.0f);
      vec3 Ks= vec3(0.9f, 0.7f, 0.7f);

      float shininess= 128.0f * 0.25f;

      vec4 lightPos_eye= renderer.viewMatrix() * vec4(player.getPos(), 1.0f);
      vec3 lightDir= renderer.viewMatrix() * vec4(normalize(player.getLookPos() - 
        player.getPos()), 0.0f);

      float lightExp= 1.0f;
      float innerCutOff= cos(radians(7.5f));

      float outerCutOff= cos(radians(17.5f));

      renderer.setUniform("Spot.pos", lightPos_eye);
      renderer.setUniform("Spot.intensity", lightIntensity);
      renderer.setUniform("Spot.dir", lightDir);
      renderer.setUniform("Spot.exp", lightExp);
      renderer.setUniform("Spot.innerCutOff", innerCutOff);
      renderer.setUniform("Spot.outerCutOff", outerCutOff);
      
      renderer.setUniform("Material.Ka", Ka);
      renderer.setUniform("Material.Kd", Kd);
      renderer.setUniform("Material.Ks", Ks);
      renderer.setUniform("Material.alpha", shininess);
      renderer.setUniform("uvScale", vec2(10.0f));
    }

    void draw() {
      player.setCameraAspect(((float) width()) / height());
      renderer.perspective(player.getCameraFOV(), player.getCameraAspect(), 
        player.getCameraNear(), player.getCameraFar());
      
      renderer.lookAt(player.getPos(), player.getLookPos(), player.getCameraUp());

      updateLookPos();

      mat4 VM= renderer.viewMatrix();
      vec3 xAxis= vec3(VM[0][0], VM[1][0], VM[2][0]);
      vec3 yAxis= vec3(VM[0][1], VM[1][1], VM[2][1]);
      vec3 zAxis= vec3(VM[0][2], VM[1][2], VM[2][2]);

      player.setCameraXAxis(xAxis);
      player.setCameraYAxis(yAxis);
      player.setCameraZAxis(zAxis);

      updatePlayerPosition();   


      // draw plane
      renderer.beginShader("spotlight");
        initSpotlightShader();
        renderer.texture("diffuseTexture", "dead_grass");
        renderer.push();
          renderer.translate(vec3(0.0, -0.5, 0));
          renderer.scale(vec3(10.0f, 0.1f, 10.0f));
          renderer.cube();
        renderer.pop();
      renderer.endShader();
    }

  protected:
    Player player;
    Enemy slenderman;

    vec4 lightPosition;
    vec3 lightIntensity;

    bool WASD_KEY_HELD[4]= {false, false, false, false};
    float stepSize= 0.1;

};

int main(int argc, char** argv)
{
  Viewer viewer;
  viewer.run();
  return 0;
}
