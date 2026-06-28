#include "states/GameplayState.h"
#include "core/Application.h"
#include "net/NetManager.h"
#include "net/Packets.h"
#include "wiTerrain.h"

#include <random>
#include <cmath>
#include <algorithm>

using namespace wi::ecs;
using namespace wi::scene;

void GameplayState::Init(MisigmaApp* app)
{
    owner = app;
}

float GameplayState::GetTerrainHeight(float x, float z)
{
    float height = 0.0f;
    XMFLOAT2 world_pos(x, z);
    Scene& scene = GetScene();
    if (scene.terrains.GetCount() > 0)
    {
        wi::terrain::Terrain& terrain = scene.terrains[0];
        for (auto& modifier : terrain.modifiers)
        {
            modifier->Apply(world_pos, height);
        }
        height = terrain.bottomLevel + (terrain.topLevel - terrain.bottomLevel) * height;
    }
    return height;
}

void GameplayState::Load()
{
    setSSREnabled(false);
    setBloomEnabled(true);
    setFXAAEnabled(true);
    setReflectionsEnabled(false);
    setMotionBlurEnabled(false);
    wi::renderer::SetTemporalAAEnabled(true);
    wi::renderer::SetOcclusionCullingEnabled(true);

    wi::renderer::ClearWorld(GetScene());

    WeatherComponent& weather = GetScene().weather;
    weather.ambient       = XMFLOAT3(0.18f, 0.04f, 0.02f);
    weather.sunColor      = XMFLOAT3(0.85f, 0.18f, 0.04f);
    weather.sunDirection  = XMFLOAT3(0.3f, 0.55f, 0.25f);
    weather.fogStart      = 30.0f;
    weather.fogDensity    = 0.010f;
    weather.skyExposure   = 0.35f;

    weather.zenith        = XMFLOAT3(0.04f, 0.01f, 0.01f);
    weather.horizon       = XMFLOAT3(0.28f, 0.06f, 0.02f);

    SetupGround();
    SpawnMonolith();

    {
        Scene& scene = GetScene();
        Entity sunEntity = scene.Entity_CreateLight("SunLight");
        LightComponent* light = scene.lights.GetComponent(sunEntity);
        if (light)
        {
            light->SetType(LightComponent::LightType::DIRECTIONAL);
            light->intensity   = 1.8f;
            light->SetCastShadow(true);
            light->color = XMFLOAT3(0.9f, 0.25f, 0.08f);
        }
        TransformComponent* t = scene.transforms.GetComponent(sunEntity);
        if (t)
        {
            t->RotateRollPitchYaw(XMFLOAT3(0.6f, 0.0f, 0.2f));
            t->SetDirty(true);
        }
    }

    InitSilhouettes();

    camYaw   = 0.0f;
    camPitch = 0.0f;
    camPos   = XMFLOAT3(0.0f, GetTerrainHeight(0.0f, -8.0f) + 1.75f, -8.0f);
    paused   = false;
    mouseCaptured = false;
    skyTime  = 0.0f;
    teleportBlackout = 0.0f;
    peers.clear();

    wi::gui::GUI& gui = GetGUI();

    btnResume.Create("BtnResume");
    btnResume.SetText("ПРОДОЛЖИТЬ");
    btnResume.SetSize(XMFLOAT2(240, 48));
    btnResume.SetColor(wi::Color(55, 5,  5, 210),  wi::gui::IDLE);
    btnResume.SetColor(wi::Color(100, 12,12, 240),  wi::gui::FOCUS);
    btnResume.SetColor(wi::Color(30,  2,  2, 255),  wi::gui::ACTIVE);
    btnResume.font.params.size = 20;
    btnResume.font.params.h_align = wi::font::WIFALIGN_CENTER;
    btnResume.font.params.v_align = wi::font::WIFALIGN_CENTER;
    btnResume.font.params.color = wi::Color(200, 80, 80, 230);
    btnResume.SetVisible(false);
    btnResume.OnClick([this](wi::gui::EventArgs)
    {
        SetPaused(false);
    });
    gui.AddWidget(&btnResume);

    btnQuit.Create("BtnQuitToMenu");
    btnQuit.SetText("ВЫЙТИ В МЕНЮ");
    btnQuit.SetSize(XMFLOAT2(240, 48));
    btnQuit.SetColor(wi::Color(35, 4,  4, 170),  wi::gui::IDLE);
    btnQuit.SetColor(wi::Color(70, 8,  8, 200),  wi::gui::FOCUS);
    btnQuit.SetColor(wi::Color(18, 2,  2, 255),  wi::gui::ACTIVE);
    btnQuit.font.params.size = 20;
    btnQuit.font.params.h_align = wi::font::WIFALIGN_CENTER;
    btnQuit.font.params.v_align = wi::font::WIFALIGN_CENTER;
    btnQuit.font.params.color = wi::Color(160, 50, 50, 200);
    btnQuit.SetVisible(false);
    btnQuit.OnClick([this](wi::gui::EventArgs)
    {
        CaptureMouseCursor(false);
        wi::renderer::ClearWorld(GetScene());
        peers.clear();
        net::NetManager::Get().Disconnect("Left match");
        if (owner) owner->GoToMainMenu(0.6f);
    });
    gui.AddWidget(&btnQuit);

    wi::RenderPath3D::Load();
    CaptureMouseCursor(true);
}

void GameplayState::SetupGround()
{
    Scene& scene = GetScene();
    terrainEntity = wi::ecs::CreateEntity();
    scene.names.Create(terrainEntity) = "terrain";

    wi::terrain::Terrain& terrain = scene.terrains.Create(terrainEntity);
    terrain.terrainEntity = terrainEntity;
    terrain.scene = &scene;

    terrain.chunk_scale = 2.0f;
    terrain.bottomLevel = -15.0f;
    terrain.topLevel = 35.0f;
    terrain.seed = 4242;

    terrain.SetCenterToCamEnabled(true);
    terrain.SetRemovalEnabled(true);
    terrain.SetGrassEnabled(false);
    terrain.SetPhysicsEnabled(false);

    terrain.materialEntities.clear();
    terrain.materialEntities.resize(wi::terrain::MATERIAL_COUNT);
    for (int i = 0; i < wi::terrain::MATERIAL_COUNT; ++i)
    {
        terrain.materialEntities[i] = wi::ecs::CreateEntity();
        MaterialComponent& mat = scene.materials.Create(terrain.materialEntities[i]);
        scene.names.Create(terrain.materialEntities[i]) = "TerrainMat_" + std::to_string(i);
        scene.Component_Attach(terrain.materialEntities[i], terrainEntity);

        switch (i)
        {
        case wi::terrain::MATERIAL_BASE:
            mat.baseColor = XMFLOAT4(0.32f, 0.10f, 0.05f, 1.0f);
            mat.textures[MaterialComponent::BASECOLORMAP].name = "Content/terrain/darkrock.jpg";
            mat.textures[MaterialComponent::NORMALMAP].name = "Content/terrain/darkrock_nor.jpg";
            mat.roughness = 0.95f;
            mat.metalness = 0.0f;
            break;
        case wi::terrain::MATERIAL_SLOPE:
            mat.baseColor = XMFLOAT4(0.18f, 0.06f, 0.03f, 1.0f);
            mat.textures[MaterialComponent::BASECOLORMAP].name = "Content/terrain/rock.jpg";
            mat.textures[MaterialComponent::NORMALMAP].name = "Content/terrain/rock_nor.jpg";
            mat.roughness = 0.9f;
            mat.metalness = 0.0f;
            break;
        case wi::terrain::MATERIAL_LOW_ALTITUDE:
            mat.baseColor = XMFLOAT4(0.12f, 0.04f, 0.02f, 1.0f);
            mat.textures[MaterialComponent::BASECOLORMAP].name = "Content/terrain/ground2.jpg";
            mat.textures[MaterialComponent::NORMALMAP].name = "Content/terrain/ground2_nor.jpg";
            mat.roughness = 1.0f;
            mat.metalness = 0.0f;
            break;
        case wi::terrain::MATERIAL_HIGH_ALTITUDE:
            mat.baseColor = XMFLOAT4(0.24f, 0.08f, 0.04f, 1.0f);
            mat.textures[MaterialComponent::BASECOLORMAP].name = "Content/terrain/slope.jpg";
            mat.textures[MaterialComponent::NORMALMAP].name = "Content/terrain/slope_nor.jpg";
            mat.roughness = 1.0f;
            mat.metalness = 0.0f;
            break;
        }
        mat.SetTextureStreamingDisabled();
    }

    auto perlin = wi::allocator::make_shared<wi::terrain::PerlinModifier>();
    perlin->SetScale(600.0f);
    perlin->weight = 0.6f;
    perlin->octaves = 4;
    perlin->blend = wi::terrain::Modifier::BlendMode::Normal;
    terrain.modifiers.push_back(perlin);

    auto voronoi = wi::allocator::make_shared<wi::terrain::VoronoiModifier>();
    voronoi->SetScale(150.0f);
    voronoi->weight = 0.4f;
    voronoi->blend = wi::terrain::Modifier::BlendMode::Additive;
    voronoi->falloff = 2.0f;
    voronoi->fade = 1.5f;
    terrain.modifiers.push_back(voronoi);

    auto bumps = wi::allocator::make_shared<wi::terrain::PerlinModifier>();
    bumps->SetScale(15.0f);
    bumps->weight = 0.15f;
    bumps->octaves = 2;
    bumps->blend = wi::terrain::Modifier::BlendMode::Additive;
    terrain.modifiers.push_back(bumps);

    terrain.Generation_Restart();
}

void GameplayState::SpawnMonolith()
{
    Scene& scene = GetScene();
    float baseHeight = GetTerrainHeight(kMonolithPos.x, kMonolithPos.z);

    {
        Entity e = scene.Entity_CreateCube("Monolith");
        TransformComponent* t = scene.transforms.GetComponent(e);
        if (t)
        {
            t->translation_local = XMFLOAT3(kMonolithPos.x, baseHeight + 4.5f, kMonolithPos.z);
            t->scale_local       = XMFLOAT3(1.0f, 4.5f, 0.8f);
            t->SetDirty(true);
        }
        MaterialComponent* mat = scene.materials.GetComponent(e);
        if (mat)
        {
            mat->baseColor    = XMFLOAT4(0.01f, 0.01f, 0.01f, 1.0f);
            mat->roughness    = 0.05f;
            mat->metalness    = 0.95f;
            mat->emissiveColor = XMFLOAT4(0.4f, 0.02f, 0.0f, 0.6f);
        }
    }

    {
        Entity e = scene.Entity_CreateCube("MonolithBase");
        TransformComponent* t = scene.transforms.GetComponent(e);
        if (t)
        {
            t->translation_local = XMFLOAT3(kMonolithPos.x, baseHeight + 0.08f, kMonolithPos.z);
            t->scale_local       = XMFLOAT3(3.0f, 0.08f, 2.2f);
            t->SetDirty(true);
        }
        MaterialComponent* mat = scene.materials.GetComponent(e);
        if (mat)
        {
            mat->baseColor = XMFLOAT4(0.04f, 0.02f, 0.01f, 1.0f);
            mat->roughness = 0.7f;
            mat->metalness = 0.3f;
        }
    }

    struct RubbleDef { float x, z, sx, sy, sz, ry; };
    static const RubbleDef rubble[] = {
        {  4.5f,  2.0f, 1.8f, 0.35f, 0.6f,  0.3f },
        { -3.8f,  3.5f, 1.2f, 0.25f, 0.5f, -0.6f },
        {  2.0f, -4.2f, 2.1f, 0.4f,  0.7f,  1.1f },
        { -5.0f, -2.0f, 0.9f, 0.3f,  0.4f,  0.2f },
        {  6.0f, -1.5f, 0.8f, 0.2f,  0.9f, -0.8f },
    };

    for (const auto& r : rubble)
    {
        Entity e = scene.Entity_CreateCube("Rubble");
        TransformComponent* t = scene.transforms.GetComponent(e);
        if (t)
        {
            float rx = kMonolithPos.x + r.x;
            float rz = kMonolithPos.z + r.z;
            float ry = GetTerrainHeight(rx, rz);
            t->translation_local = XMFLOAT3(rx, ry + r.sy * 0.5f, rz);
            t->scale_local       = XMFLOAT3(r.sx, r.sy, r.sz);
            t->RotateRollPitchYaw(XMFLOAT3(0, r.ry, 0));
            t->SetDirty(true);
        }
        MaterialComponent* mat = scene.materials.GetComponent(e);
        if (mat)
        {
            mat->baseColor = XMFLOAT4(0.12f, 0.06f, 0.03f, 1.0f);
            mat->roughness = 0.95f;
            mat->metalness = 0.0f;
        }
    }

    {
        Entity e = scene.Entity_CreateLight("MonolithLight",
            XMFLOAT3(kMonolithPos.x, baseHeight + 0.5f, kMonolithPos.z),
            XMFLOAT3(1.0f, 0.05f, 0.0f),
            1.5f,
            8.0f);
        (void)e;
    }
}

void GameplayState::InitSilhouettes()
{
    std::mt19937 rng(1337);
    std::uniform_real_distribution<float> angleD(0.0f, 6.28318f);
    std::uniform_real_distribution<float> hideD(8.0f, 20.0f);

    Scene& scene = GetScene();

    for (int i = 0; i < kSilhouetteCount; ++i)
    {
        Silhouette& s = silhouettes[i];

        s.bodyEntity = scene.Entity_CreateCube("SilBody");
        s.headEntity = scene.Entity_CreateSphere("SilHead", 0.35f);

        auto applyBlack = [&](Entity e)
        {
            MaterialComponent* mat = scene.materials.GetComponent(e);
            if (mat)
            {
                mat->baseColor    = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
                mat->roughness    = 0.0f;
                mat->metalness    = 1.0f;
                mat->emissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
            }
        };
        applyBlack(s.bodyEntity);
        applyBlack(s.headEntity);

        s.hideTimer = hideD(rng);
        s.visTimer  = 0.0f;
        s.pos       = XMFLOAT3(9999.f, -99.f, 9999.f);
        s.lookingAtPlayer = false;
        s.lookTimer = 0.0f;
        SetSilhouetteVisible(s, false);
    }
}

void GameplayState::SetSilhouetteVisible(Silhouette& s, bool visible)
{
    Scene& scene = GetScene();

    auto setPos = [&](Entity e, XMFLOAT3 pos, XMFLOAT3 scale)
    {
        TransformComponent* t = scene.transforms.GetComponent(e);
        if (t)
        {
            t->translation_local = pos;
            t->scale_local = scale;
            t->SetDirty(true);
        }
    };

    if (!visible)
    {
        setPos(s.bodyEntity, XMFLOAT3(9999.f, -100.f, 9999.f), XMFLOAT3(0.5f, 1.0f, 0.3f));
        setPos(s.headEntity, XMFLOAT3(9999.f, -100.f, 9999.f), XMFLOAT3(1.0f, 1.0f, 1.0f));
    }
    else
    {
        setPos(s.bodyEntity,
            XMFLOAT3(s.pos.x, s.pos.y + 1.0f, s.pos.z),
            XMFLOAT3(0.5f, 1.0f, 0.3f));
        setPos(s.headEntity,
            XMFLOAT3(s.pos.x, s.pos.y + 2.4f, s.pos.z),
            XMFLOAT3(1.0f, 1.0f, 1.0f));
    }
}

void GameplayState::RepositionSilhouette(Silhouette& s)
{
    static std::mt19937 rng(4242);
    std::uniform_real_distribution<float> angleD(0.0f, 6.28318f);
    std::uniform_real_distribution<float> radiusD(28.0f, 48.0f);
    std::uniform_real_distribution<float> visD(2.5f, 6.0f);
    std::uniform_real_distribution<float> hideD(10.0f, 25.0f);
    std::uniform_real_distribution<float> lookChanceD(0.0f, 1.0f);

    float angle  = angleD(rng);
    float radius = radiusD(rng);
    s.pos.x = camPos.x + std::cos(angle) * radius;
    s.pos.z = camPos.z + std::sin(angle) * radius;
    s.pos.y = GetTerrainHeight(s.pos.x, s.pos.z);
    
    s.yaw   = angle + 3.14159f;

    s.visTimer  = visD(rng);
    s.hideTimer = hideD(rng);

    s.lookingAtPlayer = (lookChanceD(rng) < 0.35f);
    s.lookTimer = s.lookingAtPlayer ? (1.5f + lookChanceD(rng) * 2.0f) : 0.0f;
}

void GameplayState::UpdateSilhouettes(float dt)
{
    Scene& scene = GetScene();

    for (int i = 0; i < kSilhouetteCount; ++i)
    {
        Silhouette& s = silhouettes[i];

        if (s.visTimer > 0.0f)
        {
            s.visTimer -= dt;

            if (s.lookingAtPlayer && s.lookTimer > 0.0f)
            {
                float dx = camPos.x - s.pos.x;
                float dz = camPos.z - s.pos.z;
                s.yaw = std::atan2(dx, dz);
                s.lookTimer -= dt;
            }

            float walkSpeed = 0.5f;
            s.pos.x += std::sin(s.yaw) * walkSpeed * dt;
            s.pos.z += std::cos(s.yaw) * walkSpeed * dt;
            s.pos.y = GetTerrainHeight(s.pos.x, s.pos.z);

            TransformComponent* tb = scene.transforms.GetComponent(s.bodyEntity);
            TransformComponent* th = scene.transforms.GetComponent(s.headEntity);
            if (tb)
            {
                tb->translation_local = XMFLOAT3(s.pos.x, s.pos.y + 1.0f, s.pos.z);
                tb->scale_local = XMFLOAT3(0.5f, 1.0f, 0.3f);
                tb->RotateRollPitchYaw(XMFLOAT3(0, s.yaw, 0));
                tb->SetDirty(true);
            }
            if (th)
            {
                th->translation_local = XMFLOAT3(s.pos.x, s.pos.y + 2.4f, s.pos.z);
                th->SetDirty(true);
            }

            if (s.visTimer <= 0.0f)
            {
                SetSilhouetteVisible(s, false);
            }
        }
        else
        {
            s.hideTimer -= dt;
            if (s.hideTimer <= 0.0f)
            {
                RepositionSilhouette(s);
                SetSilhouetteVisible(s, true);
            }
        }
    }
}

void GameplayState::UpdateSkyPulse(float dt)
{
    skyTime += dt;

    float pulse = 0.5f + 0.3f * std::sin(skyTime * 0.55f)
                       + 0.2f * std::sin(skyTime * 1.1f + 1.2f);
    pulse = std::max(0.0f, std::min(1.0f, pulse));

    WeatherComponent& weather = GetScene().weather;
    weather.horizon = XMFLOAT3(
        0.22f + 0.14f * pulse,
        0.04f + 0.02f * pulse,
        0.01f);
    weather.zenith = XMFLOAT3(
        0.03f + 0.03f * pulse,
        0.005f,
        0.005f);
}

void GameplayState::CheckTeleportBoundary(float dt)
{
    float dx = camPos.x - kMonolithPos.x;
    float dz = camPos.z - kMonolithPos.z;
    float d = std::sqrt(dx * dx + dz * dz);

    if (d > kMaxDistance)
    {
        float scale = kApproachDistance / d;
        camPos.x = kMonolithPos.x - dx * scale;
        camPos.z = kMonolithPos.z - dz * scale;

        camPitch += (camPitch > 0.0f ? -0.015f : 0.015f);
    }
}

void GameplayState::CaptureMouseCursor(bool capture)
{
    mouseCaptured = capture;
    wi::input::HidePointer(capture);
    if (capture)
    {
        float cx = GetLogicalWidth()  * 0.5f;
        float cy = GetLogicalHeight() * 0.5f;
        wi::input::SetPointer(XMFLOAT4(cx, cy, 0, 0));
    }
}

void GameplayState::SetPaused(bool p)
{
    paused = p;
    btnResume.SetVisible(p);
    btnQuit.SetVisible(p);
    CaptureMouseCursor(!p);
}

XMFLOAT3 GameplayState::GetForward() const
{
    float cy = std::cos(camYaw),   sy = std::sin(camYaw);
    float cp = std::cos(camPitch), sp = std::sin(camPitch);
    return XMFLOAT3(sy * cp, sp, cy * cp);
}

XMFLOAT3 GameplayState::GetRight() const
{
    float cy = std::cos(camYaw), sy = std::sin(camYaw);
    return XMFLOAT3(cy, 0.0f, -sy);
}

void GameplayState::ApplyCamera()
{
    XMFLOAT3 fwd = GetForward();
    camera->Eye = camPos;
    camera->At  = fwd;
    camera->Up  = XMFLOAT3(0.0f, 1.0f, 0.0f);
    camera->UpdateCamera();
}

void GameplayState::Update(float dt)
{
    wi::RenderPath3D::Update(dt);

    if (wi::input::Press(wi::input::KEYBOARD_BUTTON_ESCAPE))
        SetPaused(!paused);

    if (wi::input::Press(wi::input::KEYBOARD_BUTTON_F1) && !paused)
        CaptureMouseCursor(!mouseCaptured);

    if (paused)
    {
        ApplyCamera();
        return;
    }

    UpdateSkyPulse(dt);

    ProcessNetworkPackets();

    if (mouseCaptured)
    {
        const auto& mouse = wi::input::GetMouseState();
        float dx = mouse.delta_position.x;
        float dy = mouse.delta_position.y;

        camYaw   += dx * lookSpeed;
        camPitch -= dy * lookSpeed;
        camPitch = std::max(-1.48f, std::min(1.48f, camPitch));

        float cx = GetLogicalWidth()  * 0.5f;
        float cy = GetLogicalHeight() * 0.5f;
        wi::input::SetPointer(XMFLOAT4(cx, cy, 0, 0));
    }

    if (teleportBlackout <= 0.0f)
    {
        float speed = moveSpeed * dt;
        if (wi::input::Down(wi::input::KEYBOARD_BUTTON_LSHIFT) ||
            wi::input::Down(wi::input::KEYBOARD_BUTTON_RSHIFT))
            speed *= 1.6f;

        XMFLOAT3 fwd   = GetForward();
        XMFLOAT3 right = GetRight();

        if (wi::input::Down((wi::input::BUTTON)'W'))
        { camPos.x += fwd.x * speed; camPos.z += fwd.z * speed; }
        if (wi::input::Down((wi::input::BUTTON)'S'))
        { camPos.x -= fwd.x * speed; camPos.z -= fwd.z * speed; }
        if (wi::input::Down((wi::input::BUTTON)'A'))
        { camPos.x -= right.x * speed; camPos.z -= right.z * speed; }
        if (wi::input::Down((wi::input::BUTTON)'D'))
        { camPos.x += right.x * speed; camPos.z += right.z * speed; }
    }

    CheckTeleportBoundary(dt);

    camPos.y = GetTerrainHeight(camPos.x, camPos.z) + 1.75f;

    UpdateSilhouettes(dt);

    ApplyCamera();
}

void GameplayState::ResizeLayout()
{
    wi::RenderPath3D::ResizeLayout();

    float W = GetLogicalWidth();
    float H = GetLogicalHeight();
    float cx = W * 0.5f;

    float btnW = 240.0f, btnH = 48.0f, gap = 12.0f;
    float startY = H * 0.46f;

    btnResume.SetPos(XMFLOAT2(cx - btnW * 0.5f, startY));
    btnQuit.SetPos  (XMFLOAT2(cx - btnW * 0.5f, startY + btnH + gap));
}

void GameplayState::Compose(wi::graphics::CommandList cmd) const
{
    float W = GetLogicalWidth();
    float H = GetLogicalHeight();

    const float vEdge = H * 0.20f;
    const float hEdge = W * 0.12f;
    {
        wi::image::Params v; v.pos = XMFLOAT3(0,0,0); v.siz = XMFLOAT2(W, vEdge);
        v.color = XMFLOAT4(0.08f, 0.0f, 0.0f, 0.7f);
        wi::image::Draw(nullptr, v, cmd);
    }
    {
        wi::image::Params v; v.pos = XMFLOAT3(0, H-vEdge, 0); v.siz = XMFLOAT2(W, vEdge);
        v.color = XMFLOAT4(0.08f, 0.0f, 0.0f, 0.7f);
        wi::image::Draw(nullptr, v, cmd);
    }
    {
        wi::image::Params v; v.pos = XMFLOAT3(0,0,0); v.siz = XMFLOAT2(hEdge, H);
        v.color = XMFLOAT4(0.05f, 0.0f, 0.0f, 0.55f);
        wi::image::Draw(nullptr, v, cmd);
    }
    {
        wi::image::Params v; v.pos = XMFLOAT3(W-hEdge,0,0); v.siz = XMFLOAT2(hEdge, H);
        v.color = XMFLOAT4(0.05f, 0.0f, 0.0f, 0.55f);
        wi::image::Draw(nullptr, v, cmd);
    }

    if (teleportBlackout > 0.0f)
    {
        float alpha = std::min(1.0f, teleportBlackout / 0.15f);
        wi::image::Params bo;
        bo.enableFullScreen();
        bo.color = XMFLOAT4(0.0f, 0.0f, 0.0f, alpha);
        wi::image::Draw(nullptr, bo, cmd);
    }

    if (!paused && mouseCaptured)
    {
        float cx = W * 0.5f;
        float cy = H * 0.5f;

        wi::font::Params fp;
        fp.posX    = cx;
        fp.posY    = cy;
        fp.size    = 14;
        fp.h_align = wi::font::WIFALIGN_CENTER;
        fp.v_align = wi::font::WIFALIGN_CENTER;
        fp.color   = wi::Color(160, 60, 60, 140);
        wi::font::Draw("+", fp, cmd);
    }

    if (paused)
    {
        wi::image::Params dim;
        dim.enableFullScreen();
        dim.color = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.55f);
        wi::image::Draw(nullptr, dim, cmd);

        wi::font::Params fp;
        fp.posX    = W * 0.5f;
        fp.posY    = H * 0.35f;
        fp.size    = 36;
        fp.h_align = wi::font::WIFALIGN_CENTER;
        fp.v_align = wi::font::WIFALIGN_CENTER;
        fp.color   = wi::Color(200, 40, 40, 220);
        fp.shadowColor     = wi::Color(60, 0, 0, 160);
        fp.shadow_offset_x = 2;
        fp.shadow_offset_y = 2;
        wi::font::Draw("ПАУЗА", fp, cmd);
    }

    if (!paused)
    {
        wi::font::Params hint;
        hint.posX    = W * 0.5f;
        hint.posY    = H - 20.0f;
        hint.size    = 11;
        hint.h_align = wi::font::WIFALIGN_CENTER;
        hint.v_align = wi::font::WIFALIGN_BOTTOM;
        hint.color   = wi::Color(100, 30, 30, 120);
        wi::font::Draw("ESC — пауза   |   F1 — курсор", hint, cmd);
    }

    wi::RenderPath3D::Compose(cmd);
}

void GameplayState::ProcessNetworkPackets()
{
    auto& nm = net::NetManager::Get();
    if (!nm.IsConnected()) return;

    nm.PollInbound([this](const net::RawPacket& pkt)
    {
        HandleNetworkPacket(pkt);
    });
}

void GameplayState::HandleNetworkPacket(const net::RawPacket& pkt)
{
    (void)pkt;
}
