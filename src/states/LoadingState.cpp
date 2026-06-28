#include "states/LoadingState.h"
#include "core/Application.h"

#include <chrono>
#include <thread>


void LoadingState::Init(wi::Application* app)
{
    
    
    addLoadingFunction([](wi::jobsystem::JobArgs)
    {
        
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1200));
    });

    
    onFinished([app]()
    {
        auto* misigma = static_cast<MisigmaApp*>(app);
        misigma->GoToMainMenu(0.6f);
    });
}


void LoadingState::Load()
{
    wi::RenderPath2D::Load();
}


void LoadingState::ResizeLayout()
{
    wi::RenderPath2D::ResizeLayout();

    float W = GetLogicalWidth();
    float H = GetLogicalHeight();

    barH = 6.0f;
    barW = W * 0.5f;
    barX = (W - barW) * 0.5f;
    barY = H * 0.72f;
}


void LoadingState::Compose(wi::graphics::CommandList cmd) const
{
    
    {
        wi::image::Params bg;
        bg.enableFullScreen();
        bg.color = XMFLOAT4(0.02f, 0.005f, 0.006f, 1.0f);
        wi::image::Draw(nullptr, bg, cmd);
    }

    {
        wi::font::Params fp;
        fp.posX            = GetLogicalWidth()  * 0.5f;
        fp.posY            = GetLogicalHeight() * 0.38f;
        fp.size            = 52;
        fp.h_align         = wi::font::WIFALIGN_CENTER;
        fp.v_align         = wi::font::WIFALIGN_CENTER;
        fp.color           = wi::Color(200, 40, 30, 240);
        fp.shadowColor     = wi::Color(70, 0, 0, 180);
        fp.shadow_offset_x = 3.0f;
        fp.shadow_offset_y = 3.0f;
        wi::font::Draw("MISIGMA", fp, cmd);
    }

    {
        wi::font::Params sub;
        sub.posX    = GetLogicalWidth()  * 0.5f;
        sub.posY    = barY - 22.0f;
        sub.size    = 16;
        sub.h_align = wi::font::WIFALIGN_CENTER;
        sub.v_align = wi::font::WIFALIGN_CENTER;
        sub.color   = wi::Color(150, 60, 50, 200);
        wi::font::Draw("Loading...", sub, cmd);
    }

    {
        wi::image::Params track;
        track.pos   = XMFLOAT3(barX, barY, 0.0f);
        track.siz   = XMFLOAT2(barW, barH);
        track.color = XMFLOAT4(0.14f, 0.04f, 0.03f, 1.0f);
        wi::image::Draw(nullptr, track, cmd);
    }

    float progress = static_cast<float>(getProgress()) / 100.0f;
    if (progress > 0.0f)
    {
        wi::image::Params fill;
        fill.pos   = XMFLOAT3(barX, barY, 0.0f);
        fill.siz   = XMFLOAT2(barW * progress, barH);
        fill.color = XMFLOAT4(0.7f, 0.12f, 0.06f, 1.0f);
        wi::image::Draw(nullptr, fill, cmd);
    }

    
    wi::RenderPath2D::Compose(cmd);
}
