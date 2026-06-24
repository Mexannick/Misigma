#include "wiApplication.h"
#include "wiRenderPath3D.h"

class Game : public wi::Application
{
    wi::RenderPath3D renderPath;

public:
    void Initialize() override
    {
        wi::Application::Initialize();

        renderPath.Load();
        ActivatePath(&renderPath);
    }
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    Game game;
    wi::arguments::Parse(lpCmdLine);
    game.Run();
    return 0;
}
