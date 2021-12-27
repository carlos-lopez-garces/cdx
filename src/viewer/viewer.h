#include "core/game.h"

using namespace Core;

class Viewer : public Game {
public:
  Viewer() {}

  virtual void Startup() override;
  virtual void Cleanup() override;
  virtual void Update(float deltaT) override;
  virtual void RenderScene() override;

private:
};