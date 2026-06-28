#include "states/MainMenuState.h"
#include "core/Application.h"

#include <cmath>
#include <cstdlib>


void MainMenuState::Init(MisigmaApp* app)
{
    owner = app;
}


void MainMenuState::Load()
{
    wi::RenderPath2D::Load();

    wi::gui::GUI& gui = GetGUI();

    
    
    
    

    
    btnPlay.Create("BtnPlay");
    btnPlay.SetText("ПРОДОЛЖИТЬ");
    btnPlay.SetSize(XMFLOAT2(240, 48));
    btnPlay.SetColor(wi::Color(55, 5,  5, 210),  wi::gui::IDLE);
    btnPlay.SetColor(wi::Color(100, 12, 12, 240), wi::gui::FOCUS);
    btnPlay.SetColor(wi::Color(30,  2,  2, 255),  wi::gui::ACTIVE);
    btnPlay.font.params.size = 20;
    btnPlay.font.params.h_align = wi::font::WIFALIGN_CENTER;
    btnPlay.font.params.v_align = wi::font::WIFALIGN_CENTER;
    btnPlay.font.params.color = wi::Color(200, 80, 80, 230);
    btnPlay.OnClick([this](wi::gui::EventArgs)
    {
        if (owner) owner->StartGame(0.6f);
    });
    gui.AddWidget(&btnPlay);

    
    btnMulti.Create("BtnMulti");
    btnMulti.SetText("МУЛЬТИПЛЕЕР");
    btnMulti.SetSize(XMFLOAT2(240, 48));
    btnMulti.SetColor(wi::Color(40, 5,  5, 190),  wi::gui::IDLE);
    btnMulti.SetColor(wi::Color(80, 10, 10, 220),  wi::gui::FOCUS);
    btnMulti.SetColor(wi::Color(20, 2,  2, 255),   wi::gui::ACTIVE);
    btnMulti.font.params.size = 20;
    btnMulti.font.params.h_align = wi::font::WIFALIGN_CENTER;
    btnMulti.font.params.v_align = wi::font::WIFALIGN_CENTER;
    btnMulti.font.params.color = wi::Color(180, 60, 60, 210);
    btnMulti.OnClick([this](wi::gui::EventArgs)
    {
        if (owner) owner->GoToLobby(0.4f);
    });
    gui.AddWidget(&btnMulti);

    
    btnOptions.Create("BtnOptions");
    btnOptions.SetText("НАСТРОЙКИ");
    btnOptions.SetSize(XMFLOAT2(240, 48));
    btnOptions.SetColor(wi::Color(35, 4,  4, 170),  wi::gui::IDLE);
    btnOptions.SetColor(wi::Color(70, 8,  8, 200),  wi::gui::FOCUS);
    btnOptions.SetColor(wi::Color(18, 2,  2, 255),  wi::gui::ACTIVE);
    btnOptions.font.params.size = 20;
    btnOptions.font.params.h_align = wi::font::WIFALIGN_CENTER;
    btnOptions.font.params.v_align = wi::font::WIFALIGN_CENTER;
    btnOptions.font.params.color = wi::Color(160, 50, 50, 200);
    btnOptions.OnClick([this](wi::gui::EventArgs)
    {
        if (owner) owner->GoToOptions(0.4f);
    });
    gui.AddWidget(&btnOptions);

    
    btnQuit.Create("BtnQuit");
    btnQuit.SetText("ВЫХОД");
    btnQuit.SetSize(XMFLOAT2(240, 48));
    btnQuit.SetColor(wi::Color(28, 3,  3, 150),  wi::gui::IDLE);
    btnQuit.SetColor(wi::Color(60, 6,  6, 180),  wi::gui::FOCUS);
    btnQuit.SetColor(wi::Color(14, 1,  1, 255),  wi::gui::ACTIVE);
    btnQuit.font.params.size = 20;
    btnQuit.font.params.h_align = wi::font::WIFALIGN_CENTER;
    btnQuit.font.params.v_align = wi::font::WIFALIGN_CENTER;
    btnQuit.font.params.color = wi::Color(140, 40, 40, 180);
    btnQuit.OnClick([](wi::gui::EventArgs)
    {
        wi::platform::Exit();
    });
    gui.AddWidget(&btnQuit);

    
    time       = 0.0f;
    logoAlpha  = 0.0f;
    glitchTimer  = 3.5f;
    glitchActive = 0.0f;
    dustInited = false;
}


void MainMenuState::Update(float dt)
{
    wi::RenderPath2D::Update(dt);

    time += dt;

    
    logoAlpha = std::min(1.0f, logoAlpha + dt * 0.7f);

    
    if (glitchActive > 0.0f)
    {
        glitchActive -= dt;
        if (glitchActive <= 0.0f)
        {
            glitchOffsetX = 0.0f;
            glitchOffsetY = 0.0f;
            
            glitchTimer = 3.0f + (std::rand() % 500) * 0.01f;
        }
    }
    else
    {
        glitchTimer -= dt;
        if (glitchTimer <= 0.0f)
        {
            glitchActive  = 0.06f + (std::rand() % 100) * 0.001f;
            glitchOffsetX = (float)(std::rand() % 21 - 10); 
            glitchOffsetY = (float)(std::rand() % 7  - 3);  
        }
    }

    
    if (!dustInited)
    {
        dustInited = true;
        for (int i = 0; i < kDustCount; ++i)
        {
            dust[i].x     = (std::rand() % 1000) * 0.001f;
            dust[i].y     = (std::rand() % 1000) * 0.001f;
            dust[i].vx    = ((std::rand() % 200) - 100) * 0.000015f; 
            dust[i].vy    = ((std::rand() % 100) - 140) * 0.000008f; 
            dust[i].size  = 1.5f + (std::rand() % 30) * 0.1f;
            dust[i].alpha = 0.04f + (std::rand() % 100) * 0.0008f;
            dust[i].phase = (std::rand() % 628) * 0.01f;
        }
    }

    for (int i = 0; i < kDustCount; ++i)
    {
        dust[i].x += dust[i].vx;
        dust[i].y += dust[i].vy;

        if (dust[i].x < 0.0f) dust[i].x += 1.0f;
        if (dust[i].x > 1.0f) dust[i].x -= 1.0f;
        if (dust[i].y < 0.0f) dust[i].y = 1.0f;
        if (dust[i].y > 1.0f) dust[i].y = 0.0f;
    }
}


void MainMenuState::ResizeLayout()
{
    wi::RenderPath2D::ResizeLayout();

    float W = GetLogicalWidth();
    float H = GetLogicalHeight();
    float cx = W * 0.5f;

    float btnW  = 240.0f;
    float btnH  = 48.0f;
    float gap   = 12.0f;
    float startY = H * 0.54f;

    btnPlay.SetPos   (XMFLOAT2(cx - btnW * 0.5f, startY));
    btnMulti.SetPos  (XMFLOAT2(cx - btnW * 0.5f, startY + (btnH + gap)));
    btnOptions.SetPos(XMFLOAT2(cx - btnW * 0.5f, startY + (btnH + gap) * 2.0f));
    btnQuit.SetPos   (XMFLOAT2(cx - btnW * 0.5f, startY + (btnH + gap) * 3.0f));
}


void MainMenuState::Compose(wi::graphics::CommandList cmd) const
{
    float W = GetLogicalWidth();
    float H = GetLogicalHeight();

    
    {
        float pulse = 0.5f + 0.5f * std::sin(time * 0.5f);
        float rVal  = 0.022f + 0.018f * pulse;
        float gVal  = 0.004f;
        float bVal  = 0.006f;

        wi::image::Params bg;
        bg.enableFullScreen();
        bg.color = XMFLOAT4(rVal, gVal, bVal, 1.0f);
        wi::image::Draw(nullptr, bg, cmd);
    }

    
    for (int i = 0; i < kDustCount; ++i)
    {
        float flicker = 0.7f + 0.3f * std::sin(time * 2.1f + dust[i].phase);
        float a = dust[i].alpha * flicker * logoAlpha;

        wi::image::Params dp;
        dp.pos  = XMFLOAT3(dust[i].x * W - dust[i].size * 0.5f,
                            dust[i].y * H - dust[i].size * 0.5f, 0.0f);
        dp.siz  = XMFLOAT2(dust[i].size, dust[i].size);
        dp.color = XMFLOAT4(0.7f, 0.15f, 0.1f, a);
        wi::image::Draw(nullptr, dp, cmd);
    }

    
    {
        float pulse = 0.5f + 0.5f * std::sin(time * 0.7f);
        float gw = W * 0.65f, gh = H * 0.40f;
        wi::image::Params glow;
        glow.pos   = XMFLOAT3((W - gw) * 0.5f, H * 0.08f, 0);
        glow.siz   = XMFLOAT2(gw, gh);
        glow.color = XMFLOAT4(0.26f + 0.08f * pulse, 0.01f, 0.01f,
                               0.09f * logoAlpha);
        wi::image::Draw(nullptr, glow, cmd);
    }

    
    {
        float pulseSz  = std::sin(time * 0.9f) * 1.5f;
        float px = W * 0.5f + glitchOffsetX;
        float py = H * 0.27f + glitchOffsetY;

        
        if (glitchActive > 0.0f)
        {
            wi::font::Params fe;
            fe.posX    = px - 5.0f;
            fe.posY    = py + 3.0f;
            fe.size    = static_cast<int32_t>(62 + pulseSz);
            fe.h_align = wi::font::WIFALIGN_CENTER;
            fe.v_align = wi::font::WIFALIGN_CENTER;
            fe.color   = wi::Color(180, 0, 0, static_cast<uint8_t>(80 * logoAlpha));
            wi::font::Draw("MISIGMA", fe, cmd);
        }

        wi::font::Params fp;
        fp.posX         = px;
        fp.posY         = py;
        fp.size         = static_cast<int32_t>(62 + pulseSz);
        fp.h_align      = wi::font::WIFALIGN_CENTER;
        fp.v_align      = wi::font::WIFALIGN_CENTER;
        fp.color        = wi::Color(
            static_cast<uint8_t>(210),
            static_cast<uint8_t>(30 + (glitchActive > 0.0f ? 60 : 0)),
            static_cast<uint8_t>(20),
            static_cast<uint8_t>(255 * logoAlpha));
        fp.shadowColor      = wi::Color(80, 0, 0, static_cast<uint8_t>(160 * logoAlpha));
        fp.shadow_offset_x  = 3;
        fp.shadow_offset_y  = 3;
        wi::font::Draw("MISIGMA", fp, cmd);
    }

    
    {
        
        float flicker = 0.6f + 0.4f * std::sin(time * 1.3f + 1.0f);
        uint8_t a = static_cast<uint8_t>(130 * logoAlpha * flicker);

        wi::font::Params sub;
        sub.posX    = W * 0.5f;
        sub.posY    = H * 0.36f;
        sub.size    = 15;
        sub.h_align = wi::font::WIFALIGN_CENTER;
        sub.v_align = wi::font::WIFALIGN_CENTER;
        sub.color   = wi::Color(150, 35, 30, a);
        sub.spacingX = 6.0f;
        wi::font::Draw("Alpa", sub, cmd);
    }

    
    
    {
        float vh = H * 0.28f;
        wi::image::Params v;
        v.pos   = XMFLOAT3(0, 0, 0);
        v.siz   = XMFLOAT2(W, vh);
        v.color = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.78f);
        wi::image::Draw(nullptr, v, cmd);
    }

    {
        float vh = H * 0.28f;
        wi::image::Params v;
        v.pos   = XMFLOAT3(0, H - vh, 0);
        v.siz   = XMFLOAT2(W, vh);
        v.color = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.78f);
        wi::image::Draw(nullptr, v, cmd);
    }

    {
        float vw = W * 0.16f;
        wi::image::Params v;
        v.pos   = XMFLOAT3(0, 0, 0);
        v.siz   = XMFLOAT2(vw, H);
        v.color = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.6f);
        wi::image::Draw(nullptr, v, cmd);
    }

    {
        float vw = W * 0.16f;
        wi::image::Params v;
        v.pos   = XMFLOAT3(W - vw, 0, 0);
        v.siz   = XMFLOAT2(vw, H);
        v.color = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.6f);
        wi::image::Draw(nullptr, v, cmd);
    }

    
    {
        wi::font::Params ver;
        ver.posX    = W - 12.0f;
        ver.posY    = H - 12.0f;
        ver.size    = 11;
        ver.h_align = wi::font::WIFALIGN_RIGHT;
        ver.v_align = wi::font::WIFALIGN_BOTTOM;
        ver.color   = wi::Color(90, 20, 20, 120);
        wi::font::Draw("v0.1.0-dev", ver, cmd);
    }

    
    wi::RenderPath2D::Compose(cmd);
}
