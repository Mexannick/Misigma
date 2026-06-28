#include "states/OptionsState.h"
#include "states/GameplayState.h"
#include "core/Application.h"

#include <cmath>


void OptionsState::Init(MisigmaApp* app, GameplayState* gp)
{
    owner    = app;
    gameplay = gp;
}


void OptionsState::Load()
{
    wi::RenderPath2D::Load();

    wi::gui::GUI& gui = GetGUI();

    
    btnBack.Create("BtnBack");
    btnBack.SetText("< НАЗАД");
    btnBack.SetSize(XMFLOAT2(160, 44));
    btnBack.SetColor(wi::Color(45, 6,  6, 200),  wi::gui::IDLE);
    btnBack.SetColor(wi::Color(90, 12, 12, 240), wi::gui::FOCUS);
    btnBack.SetColor(wi::Color(24, 2,  2, 255),  wi::gui::ACTIVE);
    btnBack.font.params.size = 18;
    btnBack.font.params.h_align = wi::font::WIFALIGN_CENTER;
    btnBack.font.params.v_align = wi::font::WIFALIGN_CENTER;
    btnBack.font.params.color = wi::Color(200, 80, 80, 230);
    btnBack.OnClick([this](wi::gui::EventArgs)
    {
        if (owner) owner->GoToMainMenu(0.4f);
    });
    gui.AddWidget(&btnBack);

    
    chkBloom.Create("ChkBloom");
    chkBloom.SetText("Bloom");
    chkBloom.SetSize(XMFLOAT2(26, 26));
    chkBloom.SetCheck(gameplay ? gameplay->getBloomEnabled() : true);
    chkBloom.OnClick([this](wi::gui::EventArgs args)
    {
        if (gameplay) gameplay->setBloomEnabled(args.bValue);
    });
    gui.AddWidget(&chkBloom);

    chkSSR.Create("ChkSSR");
    chkSSR.SetText("Screen Space Reflections");
    chkSSR.SetSize(XMFLOAT2(26, 26));
    chkSSR.SetCheck(gameplay ? gameplay->getSSREnabled() : true);
    chkSSR.OnClick([this](wi::gui::EventArgs args)
    {
        if (gameplay) gameplay->setSSREnabled(args.bValue);
    });
    gui.AddWidget(&chkSSR);

    chkTAA.Create("ChkTAA");
    chkTAA.SetText("Temporal Anti-Aliasing");
    chkTAA.SetSize(XMFLOAT2(26, 26));
    chkTAA.SetCheck(true);
    chkTAA.OnClick([](wi::gui::EventArgs args)
    {
        wi::renderer::SetTemporalAAEnabled(args.bValue);
    });
    gui.AddWidget(&chkTAA);

    chkMotionBlur.Create("ChkMotionBlur");
    chkMotionBlur.SetText("Motion Blur");
    chkMotionBlur.SetSize(XMFLOAT2(26, 26));
    chkMotionBlur.SetCheck(gameplay ? gameplay->getMotionBlurEnabled() : false);
    chkMotionBlur.OnClick([this](wi::gui::EventArgs args)
    {
        if (gameplay) gameplay->setMotionBlurEnabled(args.bValue);
    });
    gui.AddWidget(&chkMotionBlur);

    
    cmbResolution.Create("CmbResolution");
    cmbResolution.SetText("Resolution: ");
    cmbResolution.SetSize(XMFLOAT2(200, 28));
    cmbResolution.AddItem("1280 x 720",  0);
    cmbResolution.AddItem("1920 x 1080", 1);
    cmbResolution.AddItem("2560 x 1440", 2);
    cmbResolution.AddItem("3840 x 2160", 3);
    cmbResolution.SetMaxVisibleItemCount(4);
    cmbResolution.SetSelectedWithoutCallback(0);
    gui.AddWidget(&cmbResolution);

    
    sldMasterVolume.Create(0.0f, 1.0f, 1.0f, 100, "SldMaster");
    sldMasterVolume.SetText("Master Volume");
    sldMasterVolume.SetSize(XMFLOAT2(260, 26));
    sldMasterVolume.OnSlide([](wi::gui::EventArgs args)
    {
        wi::audio::SetVolume(args.fValue);
    });
    gui.AddWidget(&sldMasterVolume);

    sldMusicVolume.Create(0.0f, 1.0f, 0.8f, 100, "SldMusic");
    sldMusicVolume.SetText("Music Volume");
    sldMusicVolume.SetSize(XMFLOAT2(260, 26));
    gui.AddWidget(&sldMusicVolume);
}


void OptionsState::Update(float dt)
{
    wi::RenderPath2D::Update(dt);
    time += dt;

    
    if (wi::input::Press(wi::input::KEYBOARD_BUTTON_ESCAPE))
        if (owner) owner->GoToMainMenu(0.4f);
}


void OptionsState::ResizeLayout()
{
    wi::RenderPath2D::ResizeLayout();

    float W  = GetLogicalWidth();
    float H  = GetLogicalHeight();
    float cx = W * 0.5f;

    
    btnBack.SetPos(XMFLOAT2(28.0f, 28.0f));

    
    float panelX  = cx - 180.0f;
    float rowH    = 48.0f;
    float startY  = H * 0.22f;
    float row     = startY;

    
    chkBloom.SetPos(XMFLOAT2(panelX, row));                      row += rowH;
    chkSSR.SetPos(XMFLOAT2(panelX, row));                        row += rowH;
    chkTAA.SetPos(XMFLOAT2(panelX, row));                        row += rowH;
    chkMotionBlur.SetPos(XMFLOAT2(panelX, row));                 row += rowH + 12.0f;
    cmbResolution.SetPos(XMFLOAT2(panelX, row));                 row += rowH + 8.0f;

    
    sldMasterVolume.SetPos(XMFLOAT2(panelX, row));               row += rowH;
    sldMusicVolume.SetPos(XMFLOAT2(panelX, row));
}


void OptionsState::Compose(wi::graphics::CommandList cmd) const
{
    float W = GetLogicalWidth();
    float H = GetLogicalHeight();

    {
        float pulse = 0.5f + 0.5f * std::sin(time * 0.5f);
        wi::image::Params bg;
        bg.enableFullScreen();
        bg.color = XMFLOAT4(0.022f + 0.016f * pulse, 0.004f, 0.006f, 1.0f);
        wi::image::Draw(nullptr, bg, cmd);
    }

    {
        float pulse = 0.5f + 0.5f * std::sin(time * 0.7f);
        wi::image::Params glow;
        float gw = W * 0.55f, gh = H * 0.78f;
        glow.pos   = XMFLOAT3((W - gw) * 0.5f, H * 0.08f, 0.0f);
        glow.siz   = XMFLOAT2(gw, gh);
        glow.color = XMFLOAT4(0.30f + 0.08f * pulse, 0.02f, 0.01f, 0.10f);
        wi::image::Draw(nullptr, glow, cmd);
    }

    {
        float pw = 440.0f, ph = H * 0.52f;
        wi::image::Params panel;
        panel.pos   = XMFLOAT3((W - pw) * 0.5f, H * 0.18f, 0.0f);
        panel.siz   = XMFLOAT2(pw, ph);
        panel.color = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.32f);
        wi::image::Draw(nullptr, panel, cmd);
    }

    {
        wi::font::Params fp;
        fp.posX    = W * 0.5f;
        fp.posY    = H * 0.10f;
        fp.size    = 40;
        fp.h_align = wi::font::WIFALIGN_CENTER;
        fp.v_align = wi::font::WIFALIGN_CENTER;
        fp.color   = wi::Color(210, 40, 30, 240);
        fp.shadowColor    = wi::Color(70, 0, 0, 170);
        fp.shadow_offset_x = 3.0f;
        fp.shadow_offset_y = 3.0f;
        wi::font::Draw("НАСТРОЙКИ", fp, cmd);
    }

    auto drawSection = [&](const char* label, float y)
    {
        wi::font::Params sp;
        sp.posX    = W * 0.5f - 180.0f;
        sp.posY    = y;
        sp.size    = 13;
        sp.h_align = wi::font::WIFALIGN_LEFT;
        sp.v_align = wi::font::WIFALIGN_CENTER;
        sp.color   = wi::Color(170, 70, 60, 210);
        wi::font::Draw(label, sp, cmd);
    };

    drawSection("ГРАФИКА", H * 0.17f);
    drawSection("ЗВУК",    H * 0.17f + 4 * 48.0f + 20.0f + 48.0f + 8.0f - 14.0f);

    {
        float vh = H * 0.14f;
        wi::image::Params v;
        v.pos = XMFLOAT3(0, 0, 0); v.siz = XMFLOAT2(W, vh);
        v.color = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.6f);
        wi::image::Draw(nullptr, v, cmd);
        v.pos = XMFLOAT3(0, H - vh, 0);
        wi::image::Draw(nullptr, v, cmd);
    }

    {
        wi::font::Params ver;
        ver.posX    = W - 12.0f;
        ver.posY    = H - 12.0f;
        ver.size    = 12;
        ver.h_align = wi::font::WIFALIGN_RIGHT;
        ver.v_align = wi::font::WIFALIGN_BOTTOM;
        ver.color   = wi::Color(90, 20, 20, 140);
        wi::font::Draw("v0.1.0-dev", ver, cmd);
    }

    wi::RenderPath2D::Compose(cmd);
}

