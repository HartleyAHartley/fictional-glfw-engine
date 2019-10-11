#pragma once

#include "system.h"
#include "components/windowcomponent.h"
#include "components/component.h"
#include <string>
#include <vector>
#include <map>

#include "components/meshcomponent.h"

class MeshSystem;
class GLFWwindow;
class GlfwSystem : public System{
public:
  void Init();
  void Deinit();
  bool Register(Component* component);
  bool Unregister(Component* component);
  void Update();
  std::string Name() { return "GlfwSystem"; }
  std::vector<std::string> Requires() {return {};};
  std::vector<WindowComponent*>& GetWindows();

  void Enable(WindowComponent& w);
  void Finish(WindowComponent& w);
  float GetAspectRatio(WindowComponent& w);
  void LockCursor(WindowComponent& w);
  void UnlockCursor(WindowComponent& w);
  uint GetCurrentContextVAO(MeshComponent* c);
  void DeleteComponentVAO(MeshComponent* c);
  static WindowComponent* GetCurrentWindow();
private:
  static void keyCallBack(GLFWwindow *window, int key, int scancode, int action, int mods);
  static void cursorCallBack(GLFWwindow *window, double xpos, double ypos);
  static void framebufferSizeCallback(GLFWwindow *window, int width, int height);
  static void errorCallback(int, const char* err_str);
  static void windowFocusCallback(GLFWwindow* window, int focused);
  void initialize(WindowComponent& w);
  void destruct(WindowComponent& w);
  std::vector<WindowComponent*> windows;
  std::map<ComponentID, GLFWwindow*> glfwWindows;
  std::map<std::pair<ComponentID,ComponentID>,uint> VAOs;
  GLFWwindow* mainContext;
  MeshSystem* meshsystem;
};
