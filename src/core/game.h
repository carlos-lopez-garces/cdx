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

#define CREATE_GAME( app_class ) \
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPWSTR /*lpCmdLine*/, _In_ int nCmdShow) \
{ \
    return GameCore::RunApplication( app_class(), L#app_class, hInstance, nCmdShow ); \
}