#include <shellapi.h>

#include "cdx.h"
#include "game.h"
#include "clarg.h"
#include "systime.h"
#include "input.h"

#pragma comment(lib, "runtimeobject.lib") 

namespace Core {
  using namespace Graphics;

  void InitializeGame(Game& game) {
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    CommandLineArgs::Initialize(argc, argv);

    Graphics::Initialize();
    SystemTime::Initialize();
    Input::Initialize();

    game.Startup();
  }

  void TerminateGame(Game& game) {
  
  }

  void UpdateGame(Game& game) {

  }
}