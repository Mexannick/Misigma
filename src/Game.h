#pragma once
#include "WickedEngine.h"

class GameRenderer : public wi::RenderPath3D
{
public:
    void Load() override;
    void Update(float dt) override;
    void ResizeLayout() override;

private:
    wi::gui::Label titleLabel;
    wi::gui::ComboBox sceneSelector;

    float orbitAngle   = 0.0f;
    float orbitRadius  = 4.5f;
    float orbitHeight  = 2.0f;

    void LoadScene(int index);
    void SetupCamera();
};

class Game : public wi::Application
{
    GameRenderer renderer;
public:
    void Initialize() override;
};
