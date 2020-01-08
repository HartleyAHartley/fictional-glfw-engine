#include <glad/glad.h>
#include "meshsystem.h"
#include "glfwsystem.h"
#include "madd.h"

void MeshSystem::Init(){
  glfw = dynamic_cast<GlfwSystem*>(Madd::GetInstance().GetSystem("GlfwSystem"));
}

MeshSystem::~MeshSystem(){
  for(MeshComponent* m : mesh){
    destruct(*m);
    delete m;
  }
}

bool MeshSystem::Register(Component* component){
  component->cID = Madd::GetInstance().GetNewComponentID();
  MeshComponent* m = dynamic_cast<MeshComponent *>(component);
  initialize(*m);
  m->update = false;
  mesh.push_back(m);
  return true;
}

bool MeshSystem::Unregister(Component* component){
  for (auto i = begin(mesh); i != end(mesh); i++) {
    if ((*i)->cID == component->cID) {
      destruct(**i);
      mesh.erase(i);
      return true;
    }
  }
  return false;
}

void MeshSystem::Update(){
  for(MeshComponent* m : mesh){
    if(m->update){
      destruct(*m);
      initialize(*m);
    }
  }
}

void MeshSystem::destruct(MeshComponent& m) {
  if(GlfwSystem::GetCurrentWindow() != NULL){
    if(glfw != nullptr){
      glfw->DeleteComponentVAO(&m);
    }
    glDeleteBuffers(1, &VBO[m.cID][0]);
    glDeleteBuffers(1, &VBO[m.cID][1]);
    VBO.erase(m.cID);
  }
}

void MeshSystem::initialize(MeshComponent& m) {

  //Vertex Data  
  glGenBuffers(2, VBO[m.cID]);
  glBindBuffer(GL_ARRAY_BUFFER, VBO[m.cID][0]);
  glBufferData(GL_ARRAY_BUFFER, m.verts.size() * sizeof(glm::vec3),
              m.verts.data(), GL_STATIC_DRAW);

  //Texture Data
  glBindBuffer(GL_ARRAY_BUFFER, VBO[m.cID][1]);
  glBufferData(GL_ARRAY_BUFFER, m.texcoords.size() * sizeof(glm::vec2),
              m.texcoords.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

uint MeshSystem::CreateVAO(Component* component){
  uint VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO); 

  //Vertex Data
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, VBO[component->cID][0]);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);


  //Texture Data
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, VBO[component->cID][1]);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *)0);

  glBindVertexArray(0);
  return VAO;
}

void MeshSystem::Draw(MeshComponent& m){
  using namespace std::placeholders; 
  glBindVertexArray(glfw->GetCurrentContextVAO(static_cast<Component*>(&m),
    std::bind(&MeshSystem::CreateVAO, this, _1)));
  glDrawArrays(GL_TRIANGLES, 0, m.verts.size());
  glBindVertexArray(0);
}
