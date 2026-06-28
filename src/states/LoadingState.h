#pragma once
#include "WickedEngine.h"







class LoadingState : public wi::LoadingScreen
{
public:
    
    void Init(wi::Application* app);

    
    void Load()         override;
    void ResizeLayout() override;
    void Compose(wi::graphics::CommandList cmd) const override;

private:
    
    float barX = 0, barY = 0, barW = 0, barH = 0;
};
