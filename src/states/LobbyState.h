#pragma once
#include "WickedEngine.h"
#include "net/NetManager.h"

class MisigmaApp;







class LobbyState : public wi::RenderPath2D
{
public:
    void Init(MisigmaApp* app);

    void Load()           override;
    void Update(float dt) override;
    void ResizeLayout()   override;
    void Compose(wi::graphics::CommandList cmd) const override;

private:
    MisigmaApp* owner = nullptr;
    float time = 0.0f;

    
    wi::gui::Button btnBack;

    
    wi::gui::Button          btnHost;
    wi::gui::TextInputField  txtPort;

    
    wi::gui::Button         btnJoin;
    wi::gui::TextInputField txtIP;
    wi::gui::TextInputField txtJoinPort;

    
    wi::gui::TextInputField txtPlayerName;

    
    wi::gui::Button btnStartGame;
    wi::gui::Button btnDisconnect;

    
    mutable std::string statusLog;

    void AppendLog(const std::string& msg);
    void HandlePacket(const net::RawPacket& pkt);
};
