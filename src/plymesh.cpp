//--------------------------------------------------
// Author: David Dinh
// Date: March 2. 2023
// Description: Loads PLY files in ASCII format
//--------------------------------------------------

#include "plymesh.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;
using namespace glm;

namespace agl {

  PLYMesh::PLYMesh(const std::string& filename) {
    load(filename);
  }

  PLYMesh::PLYMesh() {
  }

  void PLYMesh::init() {
    assert(_positions.size() != 0);
    if (this->_texCoords.size() == 0) {// that means there are no texCoords
      initBuffers(&_faces, &_positions, &_normals);
    } else {
      initBuffers(&_faces, &_positions, &_normals, &_texCoords);
    }
  }

  PLYMesh::~PLYMesh() {
  }


  // Will return true if a warning occurs... where the condition
  // wordIdx == correctIdx and token != correctString
  // This will also print the warningMsg if the condition is true
  bool warning(int wordIdx, int correctIdx, const string& token, 
  const string& correctString, const string& warningMsg) {
    if (wordIdx == correctIdx && token != correctString) {
      std::cout << warningMsg << std::endl;
      return true;
    }
    return false;
  }

  void PLYMesh::clear() {
    this->_positions.clear();
    this->_normals.clear();
    this->_faces.clear();
		this->_texCoords.clear();
  }

  bool PLYMesh::load(const std::string& filename) {
    if (_positions.size() != 0) {
      std::cout << "WARNING: Cannot load different files with the same PLY mesh\n";
      return false;
    }

    string line;
    string token;
    ifstream file(filename);

    
    enum Section{PLY, FORMAT, VERTEX_NUM, VERTEX_PROP, FACE_NUM, FACE_PROP, 
      END_HEADER, VERTICES, FACES, END};

    // determines which part of the file we're at
    Section curSection= PLY;


    int wordIdx= 0; // indexes words on each line
    int numVertices= 0;
    int numFaces= 0;
    int numVertexComponents= 0; // counts the number of components a vertex line has


    if (file.is_open()) {
      while (getline(file, line)) {
        // tokenize the line
        stringstream streamLine(line);
        if (getline(streamLine, token, ' ')) {

          // if line starts with comment, we continue onto the next
          if (token == "comment") continue;

          if (curSection == PLY) { // check if the line is ply
            if (warning(wordIdx, 0, token, "ply", "WARNING: not a ply file"))
              return false;
            curSection= FORMAT;
          } else if (curSection == FORMAT) {
            // we skip this formatting line
            curSection= VERTEX_NUM;
          } else if (curSection == VERTEX_NUM && token == "element") { // # of vertices
            while (getline(streamLine, token, ' ')) {
              wordIdx++;
              if (warning(wordIdx, 1, token, "vertex", "WARNING: invalid vertex number")) {
                return false;
              } else if (wordIdx == 2) { // the actual number
                try {
                  numVertices= std::stoi(token);
                } catch (std::invalid_argument const& ex) {
                  std::cout << "WARNING: cannot get a vertex for the face" << std::endl;
                  return false;
                } catch (std::out_of_range const& ex) {
                  std::cout << "WARNING: cannot get a vertex for the face" << std::endl;
                  return false;
                }
                curSection= VERTEX_PROP;
                break;
              }
            }
          } else if (curSection == VERTEX_PROP && token == "property") { // verifying vertex / normal properties

            while (getline(streamLine, token, ' ')) {
              wordIdx++;
              if (warning(wordIdx, 1, token, "float",  "WARNING: invalid vertex type"))
                return false;
              else if (wordIdx == 2) {
                numVertexComponents++;
              }
            }

          } else if (curSection == VERTEX_PROP && token == "element") { // this is essentially the FACE_NUM section now 
            curSection= FACE_NUM;

            while (getline(streamLine, token, ' ')) {
              wordIdx++;
              if (warning(wordIdx, 1, token, "face", "WARNING: invalid face line")) 
                return false;

              if (wordIdx == 2) { // the actual number
                try {
                  numFaces= std::stoi(token);
                } catch (std::invalid_argument const& ex) {
                  std::cout << "WARNING: cannot get a vertex for the face" << std::endl;
                  return false;
                } catch (std::out_of_range const& ex) {
                  std::cout << "WARNING: cannot get a vertex for the face" << std::endl;
                  return false;
                }
                curSection= FACE_PROP;
                break;
              }
            }

          } else if (curSection == FACE_PROP && token == "property") {
            // skip this section, because face vertices should be unsigned int anyway
            curSection= END_HEADER;
          } else if (curSection == END_HEADER) { // ends header
            if (warning(wordIdx, 0, token, "end_header", "WARNING: no header end"))
              return false;
            curSection= VERTICES;
          } else if (curSection == VERTICES && numVertices != 0) { // vertices
            if (--numVertices == 0) { // if none left, we go onto faces after processing
              curSection= FACES;
            }

            // manually handling the first vertex component 
            try {
              this->_positions.push_back(std::stof(token));
            } catch (std::invalid_argument const& ex) {
              std::cout << "WARNING: vertex component is not a number" << std::endl;
              return false;
            } catch (std::out_of_range const& ex) {
              std::cout << "WARNING: vertex component is not a number" << std::endl;
              return false;
            }

            while (getline(streamLine, token, ' ')) {
              wordIdx++;
              try {
                float num= std::stof(token);

                // this will x, y, z
                if (wordIdx < 3) {
                  this->_positions.push_back(num);
                } else if (wordIdx >= 3 && wordIdx < 6) { // nx, ny, nz
                  this->_normals.push_back(num);
                } else if (wordIdx >= 6 && wordIdx < 8) { // s and t, but not sure where to put them
                  this->_texCoords.push_back(num);
                }

              } catch (std::invalid_argument const& ex) {
                std::cout << "WARNING: vertex component is not a number" << std::endl;
                return false;
              } catch (std::out_of_range const& ex) {
                std::cout << "WARNING: vertex component is not a number" << std::endl;
                return false;
              }
            }
            

          } else if (curSection == FACES && numFaces != 0) {
            if (--numFaces == 0) {
              curSection= END;
            }
            // assures that the polygon starts as a triangle
            if (warning(wordIdx, 0, token, "3", "WARNING: this face does not contain 3 vertices"))
              return false;
            while (getline(streamLine, token, ' ')) { // get the vertices for the face
              wordIdx++;
              int vertex;
              try {
                vertex= std::stoi(token);
              } catch (std::invalid_argument const& ex) {
                std::cout << "WARNING: cannot get a vertex for the face" << std::endl;
                return false;
              } catch (std::out_of_range const& ex) {
                std::cout << "WARNING: cannot get a vertex for the face" << std::endl;
                return false;
              }
              this->_faces.push_back(vertex);
            }
            
          } else {
            cout << "WARNING: something went wrong with loading PLY file" << std::endl;
            return false;
          }

          wordIdx++;
        }

        wordIdx= 0; // reset the wordIndex
      }
      file.close();
    }

    return true;
  }

  glm::vec3 PLYMesh::minBounds() const {
    GLfloat minX= FLT_MAX;
    GLfloat minY= FLT_MAX;
    GLfloat minZ= FLT_MAX;
    int n= this->_positions.size();
    vector<GLfloat> verts= this->positions();
    for (int i= 0; i < n; i+= 3) { 
      if (verts[i] < minX) {
        minX= verts[i];
      }

      if (verts[i+1] < minY) {
        minY= verts[i+1];
      }

      if (verts[i+2] < minZ) {
        minZ= verts[i+2];
      }
    }
    return glm::vec3(minX, minY, minZ);
  }

  glm::vec3 PLYMesh::maxBounds() const {
    GLfloat maxX= FLT_MIN;
    GLfloat maxY= FLT_MIN;
    GLfloat maxZ= FLT_MIN;
    int n= this->_positions.size();
    vector<GLfloat> verts= this->positions();
    for (int i= 0; i < n; i+= 3) { 
      if (verts[i] > maxX) {
        maxX= verts[i];
      }

      if (verts[i+1] > maxY) {
        maxY= verts[i+1];
      }

      if (verts[i+2] > maxZ) {
        maxZ= verts[i+2];
      }
    }
    return glm::vec3(maxX, maxY, maxZ);
  }

  int PLYMesh::numVertices() const {
    return _positions.size() / 3;
  }

  int PLYMesh::numTriangles() const {
    return _faces.size() / 3;
  }

  const std::vector<GLfloat>& PLYMesh::positions() const {
    return _positions;
  }

  const std::vector<GLfloat>& PLYMesh::normals() const {
    return _normals;
  }

  const std::vector<GLfloat>& PLYMesh::texCoords() const {
    return _texCoords;
  }

  const std::vector<GLuint>& PLYMesh::indices() const {
    return _faces;
  }
}
