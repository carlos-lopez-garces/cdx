#pragma once

#include "cdx.h"

namespace Core {
  class Game {
  public:
    // Initializes application state.
    virtual void Startup(void) = 0;

    // Updates state and renders scene. Invoked once per frame.
    virtual void Update(float deltaT) = 0;

    virtual void RenderScene(void) = 0;

    // Optional UI (overlay) rendering pass.
    virtual void RenderUI(class GraphicsContext&) {};

    virtual bool IsDone(void);
    virtual void Cleanup(void) = 0;
  };

  int Run(Game& game, const wchar_t* className, HINSTANCE hInst, int nCmdShow);
}