class Viewer {
public:
  Viewer() {}

  virtual void Startup() override;
  virtual void Update(float deltaT) override;
  virtual void RenderScene() override;
  virtual void Cleanup() override;

private:
};