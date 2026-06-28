#pragma once
#include "WickedEngine.h"

class MisigmaApp;
class GameplayState;







class OptionsState : public wi::RenderPath2D
{
public:
    void Init(MisigmaApp* app, GameplayState* gameplay);

    void Load()         override;
    void Update(float dt) override;
    void ResizeLayout() override;
    void Compose(wi::graphics::CommandList cmd) const override;

private:
    MisigmaApp*   owner    = nullptr;
    GameplayState* gameplay = nullptr;
    float time = 0.0f;

    
    wi::gui::Button btnBack;

    
    wi::gui::CheckBox chkBloom;
    wi::gui::CheckBox chkSSR;
    wi::gui::CheckBox chkMotionBlur;
    wi::gui::CheckBox chkTAA;
    wi::gui::ComboBox cmbResolution;

    
    wi::gui::Slider  sldMasterVolume;
    wi::gui::Slider  sldMusicVolume;

    
    static wi::gui::Button MakeButton(const char* name, const char* label,
                                      wi::Color idle, wi::Color focus, wi::Color active);
    static wi::gui::CheckBox MakeCheck(const char* name, const char* label, bool checked);
};
