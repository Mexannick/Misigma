#pragma once
#include "WickedEngine.h"


class MisigmaApp;







class MainMenuState : public wi::RenderPath2D
{
public:
    void Init(MisigmaApp* app);

    
    void Load()           override;
    void Update(float dt) override;
    void ResizeLayout()   override;
    void Compose(wi::graphics::CommandList cmd) const override;

private:
    MisigmaApp* owner = nullptr;

    
    wi::gui::Button btnPlay;
    wi::gui::Button btnMulti;
    wi::gui::Button btnOptions;
    wi::gui::Button btnQuit;

    
    float time         = 0.0f;   
    float logoAlpha    = 0.0f;   

    
    float glitchTimer  = 0.0f;   
    float glitchActive = 0.0f;   
    float glitchOffsetX = 0.0f;
    float glitchOffsetY = 0.0f;

    
    static constexpr int kDustCount = 24;
    struct DustParticle {
        float x, y;       
        float vx, vy;     
        float size;
        float alpha;
        float phase;      
    };
    mutable DustParticle dust[kDustCount];
    mutable bool dustInited = false;
};
