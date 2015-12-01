#include <ncurses.h>
#include <stdlib.h>

#include <vector>
#include <iostream>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds

#define GLM_SWIZZLE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "Framebuffer.h"
#include "FileReader.h"
#include "Pipeline.h"
#include "VAO.h"

using namespace std;

char render_chars[] = {'.','-',':',';','i','c','x','%','#'};

static const vector<glm::vec3> cube = {
  glm::vec3(-1.0f,-1.0f,-1.0f),//A
  glm::vec3(-1.0f,-1.0f, 1.0f),
  glm::vec3(-1.0f, 1.0f, 1.0f),

  glm::vec3(-1.0f,-1.0f,-1.0f),//A
  glm::vec3(-1.0f, 1.0f, 1.0f),
  glm::vec3(-1.0f, 1.0f,-1.0f),

  glm::vec3(1.0f, 1.0f,-1.0f),//B
  glm::vec3(-1.0f,-1.0f,-1.0f),
  glm::vec3(-1.0f, 1.0f,-1.0f),

  glm::vec3(1.0f, 1.0f,-1.0f),//B
  glm::vec3(1.0f,-1.0f,-1.0f),
  glm::vec3(-1.0f,-1.0f,-1.0f),

  glm::vec3(1.0f,-1.0f, 1.0f),//C
  glm::vec3(-1.0f,-1.0f,-1.0f),
  glm::vec3(1.0f,-1.0f,-1.0f),

  glm::vec3(1.0f,-1.0f, 1.0f),//C
  glm::vec3(-1.0f,-1.0f, 1.0f),
  glm::vec3(-1.0f,-1.0f,-1.0f),

  glm::vec3(-1.0f, 1.0f, 1.0f),//D
  glm::vec3(-1.0f,-1.0f, 1.0f),
  glm::vec3(1.0f,-1.0f, 1.0f),

  glm::vec3(1.0f, 1.0f, 1.0f),//D
  glm::vec3(-1.0f, 1.0f, 1.0f),
  glm::vec3(1.0f,-1.0f, 1.0f),

  glm::vec3(1.0f, 1.0f, 1.0f),//E
  glm::vec3(1.0f,-1.0f,-1.0f),
  glm::vec3(1.0f, 1.0f,-1.0f),

  glm::vec3(1.0f,-1.0f,-1.0f),//E
  glm::vec3(1.0f, 1.0f, 1.0f),
  glm::vec3(1.0f,-1.0f, 1.0f),

  glm::vec3(1.0f, 1.0f, 1.0f),//F
  glm::vec3(1.0f, 1.0f,-1.0f),
  glm::vec3(-1.0f, 1.0f,-1.0f),

  glm::vec3(1.0f, 1.0f, 1.0f),//F
  glm::vec3(-1.0f, 1.0f,-1.0f),
  glm::vec3(-1.0f, 1.0f, 1.0f)
};

std::vector<glm::vec3> cubeColors = {
  glm::vec3(0.5f,0.0f,0.5f),//A
  glm::vec3(0.5f,0.0f,0.0f),
  glm::vec3(0.5f,0.0f,0.0f),

  glm::vec3(0.5f,0.0f,0.5f),//A
  glm::vec3(0.5f,0.0f,0.0f),
  glm::vec3(0.5f,0.0f,0.5f),


  glm::vec3(1.0f,0.0f,0.0f),//B
  glm::vec3(1.0f,1.0f,0.0f),
  glm::vec3(1.0f,0.0f,0.0f),

  glm::vec3(1.0f,0.0f,0.0f),//B
  glm::vec3(1.0f,1.0f,0.0f),
  glm::vec3(1.0f,1.0f,0.0f),

  glm::vec3(0.0f,1.0f,0.0f),//C
  glm::vec3(0.0f,1.0f,0.0f),
  glm::vec3(0.0f,1.0f,0.0f),
  
  glm::vec3(0.0f,1.0f,0.0f),//C
  glm::vec3(0.0f,1.0f,0.0f),
  glm::vec3(0.0f,1.0f,0.0f),

  glm::vec3(0.2f,0.6f,1.0f),//D
  glm::vec3(0.2f,0.6f,1.0f),
  glm::vec3(0.2f,0.6f,1.0f),

  glm::vec3(0.2f,0.6f,1.0f),//D
  glm::vec3(0.2f,0.6f,1.0f),
  glm::vec3(0.2f,0.6f,1.0f),

  glm::vec3(0.0f,1.0f,1.0f),//E
  glm::vec3(0.0f,1.0f,1.0f),
  glm::vec3(0.0f,1.0f,1.0f),

  glm::vec3(0.0f,1.0f,1.0f),//E
  glm::vec3(0.0f,1.0f,1.0f),
  glm::vec3(0.0f,1.0f,1.0f),

  glm::vec3(0.7f,0.0f,0.2f),//F
  glm::vec3(0.7f,0.0f,0.2f),
  glm::vec3(0.2f,0.2f,0.0f),

  glm::vec3(0.7f,0.0f,0.2f),//F
  glm::vec3(0.2f,0.2f,0.0f),
  glm::vec3(0.2f,0.2f,0.0f)
};

glm::vec4 vshader(const GenericMap &vertexAttributes, const GenericMap &uniforms, GenericMap &fragmentAttributes)
{
  glm::vec3 v = vertexAttributes.getVec3("position");
  glm::vec4 pos = glm::scale(glm::mat4(1.0f), glm::vec3(1,0.6,1))  * uniforms.getMat4("P") * uniforms.getMat4("M") * glm::vec4(v, 1.0f);
  glm::vec3 tNormal = (uniforms.getMat4("M") * glm::vec4(vertexAttributes.getVec3("normals"), 0.0f)).xyz();

  //glm::vec4 pos(v,1.0f);
  //fragmentAttributes.set("color", vertexAttributes.getVec3("color"));
  fragmentAttributes.set("normal", tNormal);
  fragmentAttributes.set("position", glm::vec3(pos));

  return  pos;
}

glm::vec4 fshader(const GenericMap &fragmentAttributes, const GenericMap &uniforms)
{
  glm::vec3 pos = fragmentAttributes.getVec3("fragmentPos");
  glm::vec2 uv = glm::vec2(pos.x/uniforms.getInt("screenWidth"),pos.y/uniforms.getInt("screenHeight"));

  glm::vec3 lightPos(0, 1, 1);
  glm::vec3 normal = glm::normalize(fragmentAttributes.getVec3("normal"));

  float att = glm::clamp(glm::dot(normal, glm::normalize(lightPos)), 0.0f, 1.0f);

  return glm::vec4(att * uniforms.getVec3("color"), 1);
  //glm::vec4 color = glm::vec4(fragmentAttributes.getVec3("color"), 1.0f);
  //return color;

  //if (fmod(pos.x + 2.0f*sin(pos.y*0.5f), 4.0f) <= 2.0f) return color;
 // else return glm::vec4(uv,1,1);
}

int main()
{
  initscr();
  start_color();
  idcok(stdscr,true);

  Framebuffer fb(getmaxx(stdscr), getmaxy(stdscr));
  fb.clearBuffers();

  Pipeline pl;
  pl.program.fragmentShader = fshader;
  pl.program.vertexShader = vshader;

  std::vector<glm::vec3> pos, normals;
  std::vector<glm::vec2> uvs;
  bool triangles;
  FileReader::ReadOBJ("./boy.obj", pos, uvs, normals, triangles);

  VAO vao;
  vao.addVBO("position", pos);
  vao.addVBO("normals", normals);
  //vao.addVBO("position", cube);
  //vao.addVBO("color", cubeColors);

  float rotation = 0.0f;

  glm::mat4 P = glm::perspective(M_PI/3.0, double(fb.getWidth()) / fb.getHeight(), 0.5, 40.0);
  pl.program.uniforms.set("P", P);
  pl.program.uniforms.set("screenWidth", fb.getWidth());
  pl.program.uniforms.set("screenHeight", fb.getHeight());

  static float trans = 0.0f;
  while (true)
  {
    erase();
    fb.clearBuffers();

    trans += 0.05;
    rotation += 0.005f;
    glm::mat4 M(1.0f);
    M = glm::translate(M, glm::vec3(-12,-8, ((sin(trans)*0.5+0.5f)*-7)-13));
    M = glm::rotate(M, rotation*5, glm::vec3(0,1,0));
    M = glm::rotate(M,3.141592f/2.0f,glm::vec3(-1,0,0));
    //M = glm::rotate(M,rotation,glm::vec3(1,1,0.3));
    //M = glm::rotate(M,rotation*1.5f,glm::vec3(0.5,0,1));
    M = glm::scale(M,glm::vec3(2.5));

    pl.program.uniforms.set("color", glm::vec3(1,0.2,0.2));
    pl.program.uniforms.set("M", M);
    pl.drawVAO(vao, fb);

    M = glm::mat4(1.0f);
    M = glm::translate(M, glm::vec3(((sin(trans*10))*3),-8,-13));
    M = glm::rotate(M, rotation*9, glm::vec3(0,1,0));
    M = glm::rotate(M,3.141592f/2.0f,glm::vec3(-1,0,0));
    M = glm::scale(M,glm::vec3(2.5));

    pl.program.uniforms.set("color", glm::vec3(0.2,1.0,0.2));
    pl.program.uniforms.set("M", M);
    pl.drawVAO(vao, fb);

    M = glm::mat4(1.0f);
    M = glm::translate(M, glm::vec3(12,-8,-13));
    M = glm::rotate(M, rotation*14, glm::vec3(0,1,0));
    M = glm::rotate(M,3.141592f/2.0f,glm::vec3(-1,0,0));
    M = glm::scale(M,glm::vec3(2.5));

    pl.program.uniforms.set("color", glm::vec3(0.4,0.8,1));
    pl.program.uniforms.set("M", M);
    pl.drawVAO(vao, fb);

    fb.render();

    refresh();
    std::this_thread::sleep_for (std::chrono::milliseconds(10));
  }

  getch();
  endwin();
}
