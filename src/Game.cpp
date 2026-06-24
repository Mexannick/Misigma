#include "Game.h"
#include <filesystem>

using namespace wi::ecs;
using namespace wi::scene;

static std::string ContentDir;

struct SceneEntry { const char* name; const char* path; float camRadius; float camHeight; };

static const SceneEntry scenes[] =
{
    { "Teapot",         "models/teapot.wiscene",                  4.0f,  2.0f },
    { "Bloom Test",     "models/bloom_test.wiscene",              8.0f,  3.0f },
    { "Physics",        "models/physics_test.wiscene",            8.0f,  4.0f },
    { "Water Surface",  "models/water_test.wiscene",             10.0f,  5.0f },
    { "Volumetric Fog", "models/volumetric_test.wiscene",         8.0f,  3.0f },
    { "Particle Smoke", "models/emitter_smoke.wiscene",           6.0f,  2.0f },
    { "Cloth Physics",  "models/cloth_test.wiscene",              8.0f,  4.0f },
    { "Hair Particles", "models/hairparticle_torus.wiscene",      4.0f,  2.0f },
    { "Shadows",        "models/shadows_test.wiscene",            8.0f,  4.0f },
    { "Emitter Fire",   "models/emitter_fire.wiscene",            5.0f,  2.0f },
};
static constexpr int SCENE_COUNT = (int)(sizeof(scenes) / sizeof(scenes[0]));

void Game::Initialize()
{
    wi::Application::Initialize();

    std::string exePath = wi::helper::GetExecutablePath();
    std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();
    ContentDir = (exeDir / "Content").string() + "/";

    renderer.Load();
    ActivatePath(&renderer);
}

void GameRenderer::Load()
{
    setSSREnabled(true);
    setBloomEnabled(true);
    setFXAAEnabled(true);
    setReflectionsEnabled(true);
    setMotionBlurEnabled(false);
    wi::renderer::SetTemporalAAEnabled(true);
    wi::renderer::SetOcclusionCullingEnabled(true);

    wi::gui::GUI& gui = GetGUI();

    titleLabel.Create("Title");
    titleLabel.SetText("Misigma Engine Demo");
    titleLabel.font.params.h_align = wi::font::WIFALIGN_CENTER;
    titleLabel.font.params.size = 28;
    titleLabel.SetSize(XMFLOAT2(400, 36));
    gui.AddWidget(&titleLabel);

    sceneSelector.Create("SceneSelector");
    sceneSelector.SetText("Scene: ");
    sceneSelector.SetSize(XMFLOAT2(260, 28));
    for (int i = 0; i < SCENE_COUNT; ++i)
        sceneSelector.AddItem(scenes[i].name, i);
    sceneSelector.SetMaxVisibleItemCount(SCENE_COUNT);
    sceneSelector.OnSelect([this](wi::gui::EventArgs args) {
        LoadScene(args.userdata);
    });
    gui.AddWidget(&sceneSelector);

    LoadScene(0);
}

void GameRenderer::LoadScene(int index)
{
    wi::renderer::ClearWorld(GetScene());
    GetScene().weather = WeatherComponent();

    orbitAngle  = 0.0f;
    orbitRadius = scenes[index].camRadius;
    orbitHeight = scenes[index].camHeight;

    wi::scene::LoadModel(GetScene(), ContentDir + scenes[index].path);

    WeatherComponent& weather = GetScene().weather;
    weather.ambient      = XMFLOAT3(0.04f, 0.04f, 0.06f);
    weather.sunColor     = XMFLOAT3(1.0f, 0.95f, 0.85f);
    weather.sunDirection = XMFLOAT3(0.5f, 0.8f, 0.3f);
    weather.fogStart     = 40.0f;
    weather.fogDensity   = 0.002f;

    SetupCamera();
}

void GameRenderer::SetupCamera()
{
    TransformComponent t;
    t.Translate(XMFLOAT3(0.0f, orbitHeight, -orbitRadius));
    t.UpdateTransform();
    wi::scene::GetCamera().TransformCamera(t);
    wi::scene::GetCamera().UpdateCamera();
}

void GameRenderer::Update(float dt)
{
    wi::RenderPath3D::Update(dt);

    orbitAngle += dt * 0.25f;

    float x = std::sin(orbitAngle) * orbitRadius;
    float z = std::cos(orbitAngle) * orbitRadius;

    CameraComponent& cam = wi::scene::GetCamera();
    XMVECTOR eye    = XMVectorSet(x, orbitHeight, z, 1.0f);
    XMVECTOR target = XMVectorSet(0.0f, orbitHeight * 0.5f, 0.0f, 1.0f);
    XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMMATRIX view   = XMMatrixLookAtLH(eye, target, up);

    XMMATRIX proj = cam.GetProjection();
    XMStoreFloat4x4(&cam.View, view);
    XMStoreFloat4x4(&cam.VP, XMMatrixMultiply(view, proj));
    XMStoreFloat3(&cam.Eye, eye);
}

void GameRenderer::ResizeLayout()
{
    wi::RenderPath3D::ResizeLayout();

    float W = GetLogicalWidth();
    titleLabel.SetPos(XMFLOAT2(W * 0.5f - 200.0f, 12.0f));
    sceneSelector.SetPos(XMFLOAT2(W * 0.5f - 130.0f, 60.0f));
}
