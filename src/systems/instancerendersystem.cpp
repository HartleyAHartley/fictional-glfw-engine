#include "instancerendersystem.h"
#include "madd.h"
#include "camerasystem.h"
#include <glad/glad.h>
#include "shadersystem.h"
#include "meshsystem.h"
#include "texturesystem.h"
#include "glfwsystem.h"
#include "rendersystem.h"

void InstanceRenderSystem::Init() {
  shadersys = dynamic_cast<ShaderSystem*>(Madd::GetInstance().GetSystem("ShaderSystem"));
  meshsys = dynamic_cast<MeshSystem*>(Madd::GetInstance().GetSystem("MeshSystem"));
  texturesys = dynamic_cast<TextureSystem*>(Madd::GetInstance().GetSystem("TextureSystem"));
  rendersys = dynamic_cast<RenderSystem*>(Madd::GetInstance().GetSystem("RenderSystem"));
  glfwsys= dynamic_cast<GlfwSystem*>(Madd::GetInstance().GetSystem("GlfwSystem"));
  instanceSync = 0;
}

InstanceRenderSystem::~InstanceRenderSystem(){
    for(auto &[meshcID, inst] : instanceData){
      for(auto & cID : inst.cIDs){
        destruct(objects[cID]);
        delete objects[cID];
      }
    }
}

bool InstanceRenderSystem::Register(Component* component){
  RenderedComponent* rc = dynamic_cast<RenderedComponent*>(component);
  rc->cID = Madd::GetInstance().GetNewComponentID();
  objects[rc->cID] = dynamic_cast<RenderedComponent*>(component);
  if(!instanceData.contains(rc->mesh->cID)){
    instanceData[rc->mesh->cID] = instanceDatum{};
    glGenBuffers(2, instanceData[rc->mesh->cID].VBO);
    instanceData[rc->mesh->cID].shader = rc->shader;
    instanceData[rc->mesh->cID].mesh = rc->mesh;
  }
  shadersys->Enable(*(rc->shader));
  shadersys->SetTextureEnabled(*(rc->shader),false);
  instanceData[rc->mesh->cID].models.push_back(rc->model);
  instanceData[rc->mesh->cID].shades.push_back(rc->shade);
  instanceData[rc->mesh->cID].cIDs.push_back(component->cID);
  return true;
}

uint InstanceRenderSystem::CreateVAO(Component* component){
  MeshComponent* mesh = dynamic_cast<MeshComponent*>(component);
  uint VAO = meshsys->CreateVAO(mesh);
  glBindVertexArray(VAO);

  /*
    opengl only allows for 4 floats per vertex attrib so mat4 has 16 floats so
    it gets spread across 4 attribs. The last attrib has a wilder stride to
    account for the id at the end of the structs.
  */

  size_t vec4size = sizeof(glm::vec4);
  //Model Data
  glBindBuffer(GL_ARRAY_BUFFER, instanceData[mesh->cID].VBO[0]);

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, vec4size*4, static_cast<void*>(0));

  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, vec4size*4, reinterpret_cast<void*>(vec4size));

  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, vec4size*4, reinterpret_cast<void*>(vec4size*2));

  glEnableVertexAttribArray(5);
  glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, vec4size*4, reinterpret_cast<void*>(vec4size*3));

  glVertexAttribDivisor(2,1);
  glVertexAttribDivisor(3,1);
  glVertexAttribDivisor(4,1);
  glVertexAttribDivisor(5,1);

  //Shade Data
  glEnableVertexAttribArray(6);
  glBindBuffer(GL_ARRAY_BUFFER, instanceData[mesh->cID].VBO[1]);
  glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, vec4size, static_cast<void*>(0));

  glVertexAttribDivisor(6,1);

  glBindVertexArray(0);
  return VAO;
}

bool InstanceRenderSystem::Unregister(Component* component){
  RenderedComponent* rc = dynamic_cast<RenderedComponent*>(component);
  objects.erase(rc->cID);
  for(size_t i = 0; i < instanceData[rc->mesh->cID].models.size(); i++){
    if(instanceData[rc->mesh->cID].cIDs[i] == component->cID){
      instanceData[rc->mesh->cID].models.erase(begin(instanceData[rc->mesh->cID].models)+i);
      instanceData[rc->mesh->cID].shades.erase(begin(instanceData[rc->mesh->cID].shades)+i);
      instanceData[rc->mesh->cID].cIDs.erase(begin(instanceData[rc->mesh->cID].cIDs)+i);
      destruct(rc);
      if(instanceData[rc->mesh->cID].models.empty()){
        instanceData.erase(rc->mesh->cID);
      }
      return true;
    }
  }
  return false;
}

void InstanceRenderSystem::destruct(RenderedComponent* rc){
  if(GlfwSystem::GetCurrentWindow() != NULL){
    glDeleteBuffers(2, instanceData[rc->mesh->cID].VBO);
  }
}

void InstanceRenderSystem::Update(){
  for(WindowComponent* w : glfwsys->GetWindows()){
    instanceSync = 1;
    if(rendersys == nullptr || rendersys->instanceSync == 0){
      Start(*w);
    }
    for(CameraComponent* c : w->cameras){
      for(auto &[meshcID, inst] : instanceData){
        updateInstance(inst,*c);
      }
    }
    if(rendersys == nullptr || rendersys->instanceSync == 1){
      instanceSync = 0;
      if(rendersys != nullptr){
        rendersys->instanceSync = 0;
      }
      Finish(*w);
    }
  }
}

void InstanceRenderSystem::updateInstance(instanceDatum& inst, CameraComponent& c){
  shadersys->Enable(*inst.shader);
  shadersys->SetView(*inst.shader, &c.view);
  shadersys->SetProjection(*inst.shader, &c.projection);
  shadersys->SetTime(*inst.shader, Madd::GetInstance().GetTime());
  for(size_t i = 0; i < inst.models.size(); i++){
    inst.models[i] = objects[inst.cIDs[i]]->model;
    inst.shades[i] = objects[inst.cIDs[i]]->shade;
  }
  glBindBuffer(GL_ARRAY_BUFFER,inst.VBO[0]);
  glBufferData(GL_ARRAY_BUFFER, inst.models.size() * sizeof(glm::mat4),
    inst.models.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER,inst.VBO[1]);
  glBufferData(GL_ARRAY_BUFFER, inst.shades.size() * sizeof(glm::vec4),
    inst.shades.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  using namespace std::placeholders; 
  glBindVertexArray(glfwsys->GetCurrentContextVAO(static_cast<Component*>(inst.mesh),
    std::bind(&InstanceRenderSystem::CreateVAO, this, _1)));
    
  glDrawArraysInstanced(GL_TRIANGLES, 0, inst.mesh->verts.size(), inst.models.size());
  glBindVertexArray(0);
}

void InstanceRenderSystem::Start(WindowComponent& w){
  glfwsys->Enable(w);
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void InstanceRenderSystem::Finish(WindowComponent& w){
  glfwsys->Finish(w);
}
