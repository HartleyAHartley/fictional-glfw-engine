#include "mouseeventsystem.h"
#include "madd.h"

void MouseEventSystem::Init(){
}

void MouseEventSystem::Deinit(){
  subscribers.clear();
}


void MouseEventSystem::CursorCallBack(WindowComponent *window, double xpos, double ypos){
    for(auto const c : GetInstance().subscribers) {
      c->callback(c->c, window, xpos, ypos);
    }
}

MouseEventSystem& MouseEventSystem::GetInstance() {
    static MouseEventSystem instance;
    return instance;
}

bool MouseEventSystem::Register(Component* component){
  component->cID = Madd::GetInstance().GetNewComponentID();
  subscribers.push_back(dynamic_cast<MouseEventComponent*>(component));
  return true;
}

bool MouseEventSystem::Unregister(Component* component){
  for(auto i = begin(subscribers); i != end(subscribers); i++){
    if((*i)->cID == component->cID){
      subscribers.erase(i);
      return true;
    }
  }
  return false;
}

void MouseEventSystem::Update(){
}


