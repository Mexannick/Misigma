#pragma once
#include "WickedEngine.h"

class MisigmaApp;

namespace net { struct RawPacket; }

class GameplayState : public wi::RenderPath3D
{
public:
    void Init(MisigmaApp* app);

    void Load()           override;
    void Update(float dt) override;
    void ResizeLayout()   override;
    void Compose(wi::graphics::CommandList cmd) const override;

private:
    MisigmaApp* owner = nullptr;

    float camYaw   = 0.0f;
    float camPitch = 0.0f;
    XMFLOAT3 camPos = XMFLOAT3(0.0f, 1.75f, -8.0f);

    float moveSpeed  = 3.5f;
    float lookSpeed  = 0.002f;

    bool mouseCaptured = false;
    bool paused        = false;

    wi::gui::Button btnResume;
    wi::gui::Button btnQuit;

    float skyTime = 0.0f;

    wi::graphics::Shader marsGradeCS;
    bool  postFxLoaded = false;

    static constexpr XMFLOAT3 kMonolithPos = XMFLOAT3(0.0f, 0.0f, 400.0f);
    static constexpr float kMaxDistance = 650.0f;
    static constexpr float kApproachDistance = 450.0f;
    float teleportBlackout = 0.0f;

    wi::ecs::Entity terrainEntity = wi::ecs::INVALID_ENTITY;

    static constexpr int kSilhouetteCount = 5;

    struct Silhouette
    {
        wi::ecs::Entity bodyEntity  = wi::ecs::INVALID_ENTITY;
        wi::ecs::Entity headEntity  = wi::ecs::INVALID_ENTITY;

        XMFLOAT3 pos    = XMFLOAT3(0, 0, 0);
        float    yaw    = 0.0f;

        float visTimer  = 0.0f;
        float hideTimer = 0.0f;

        bool    lookingAtPlayer = false;
        float   lookTimer = 0.0f;
    };

    Silhouette silhouettes[kSilhouetteCount];

    struct PeerPlayer
    {
        wi::ecs::Entity entity = wi::ecs::INVALID_ENTITY;
        uint8_t slotId = 0xFF;
        std::string name;
        XMFLOAT3 position = XMFLOAT3(0, 0, 0);
    };
    std::unordered_map<uint8_t, PeerPlayer> peers;

    void CaptureMouseCursor(bool capture);
    void SetPaused(bool p);
    void ApplyCamera();

    XMFLOAT3 GetForward() const;
    XMFLOAT3 GetRight()   const;

    float GetTerrainHeight(float x, float z);

    void SetupGround();
    void SpawnMonolith();
    void InitSilhouettes();

    void SetupPostprocess();
    void UpdatePostprocess();

    void UpdateSkyPulse(float dt);
    void CheckTeleportBoundary(float dt);
    void UpdateSilhouettes(float dt);

    void SetSilhouetteVisible(Silhouette& s, bool visible);
    void RepositionSilhouette(Silhouette& s);

    void ProcessNetworkPackets();
    void HandleNetworkPacket(const net::RawPacket& pkt);
};
