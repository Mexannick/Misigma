#include "states/LobbyState.h"
#include "core/Application.h"

static constexpr float ROW_H = 46.0f;


void LobbyState::Init(MisigmaApp* app)
{
    owner = app;
}


void LobbyState::AppendLog(const std::string& msg)
{
    if (!statusLog.empty()) statusLog += "\n";
    statusLog += "> " + msg;
    
    size_t newlines = 0;
    for (char c : statusLog) if (c == '\n') ++newlines;
    while (newlines > 9)
    {
        size_t pos = statusLog.find('\n');
        if (pos == std::string::npos) break;
        statusLog = statusLog.substr(pos + 1);
        --newlines;
    }
}


void LobbyState::HandlePacket(const net::RawPacket& pkt)
{
    if (pkt.data.size() < sizeof(net::PacketHeader)) return;
    const auto* hdr = reinterpret_cast<const net::PacketHeader*>(pkt.data.data());

    switch (hdr->type)
    {
    case net::PacketType::ConnectAck:
    {
        const auto* ack = reinterpret_cast<const net::PacketConnectAck*>(pkt.data.data());
        AppendLog("Connected! Slot #" + std::to_string(ack->slotId));
        btnStartGame.SetEnabled(true);
        break;
    }
    case net::PacketType::Disconnect:
    {
        const auto* dc = reinterpret_cast<const net::PacketDisconnect*>(pkt.data.data());
        AppendLog("Disconnected: " + std::string(dc->reason));
        btnStartGame.SetEnabled(false);
        break;
    }
    case net::PacketType::Ping:
    {
        
        net::PacketPong pong;
        const auto* ping = reinterpret_cast<const net::PacketPing*>(pkt.data.data());
        pong.timestamp = ping->timestamp;
        net::NetManager::Get().Send(pkt.sender, &pong, sizeof(pong));
        break;
    }
    case net::PacketType::ChatMessage:
    {
        const auto* chat = reinterpret_cast<const net::PacketChatMessage*>(pkt.data.data());
        AppendLog("[chat] " + std::string(chat->text));
        break;
    }
    default:
        break;
    }
}


void LobbyState::Load()
{
    wi::RenderPath2D::Load();

    wi::gui::GUI& gui = GetGUI();

    
    btnBack.Create("LobbyBtnBack");
    btnBack.SetText("< BACK");
    btnBack.SetSize(XMFLOAT2(160, 44));
    btnBack.SetColor(wi::Color(45, 30, 80, 200),  wi::gui::IDLE);
    btnBack.SetColor(wi::Color(80, 50, 140, 240),  wi::gui::FOCUS);
    btnBack.SetColor(wi::Color(25, 10, 50, 255),   wi::gui::ACTIVE);
    btnBack.font.params.size = 18;
    btnBack.font.params.h_align = wi::font::WIFALIGN_CENTER;
    btnBack.font.params.v_align = wi::font::WIFALIGN_CENTER;
    btnBack.OnClick([this](wi::gui::EventArgs)
    {
        net::NetManager::Get().Disconnect("Left lobby");
        if (owner) owner->GoToMainMenu(0.4f);
    });
    gui.AddWidget(&btnBack);

    
    txtPlayerName.Create("TxtPlayerName");
    txtPlayerName.SetText("Player");
    txtPlayerName.SetSize(XMFLOAT2(220, 32));
    txtPlayerName.SetDescription("Name: ");
    gui.AddWidget(&txtPlayerName);

    
    txtPort.Create("TxtPort");
    txtPort.SetText("7777");
    txtPort.SetSize(XMFLOAT2(120, 32));
    txtPort.SetDescription("Port: ");
    gui.AddWidget(&txtPort);

    btnHost.Create("BtnHost");
    btnHost.SetText("HOST");
    btnHost.SetSize(XMFLOAT2(140, 44));
    btnHost.SetColor(wi::Color(30, 90, 50, 220),  wi::gui::IDLE);
    btnHost.SetColor(wi::Color(50, 140, 80, 255),  wi::gui::FOCUS);
    btnHost.SetColor(wi::Color(15, 60, 30, 255),   wi::gui::ACTIVE);
    btnHost.font.params.size = 18;
    btnHost.font.params.h_align = wi::font::WIFALIGN_CENTER;
    btnHost.font.params.v_align = wi::font::WIFALIGN_CENTER;
    btnHost.OnClick([this](wi::gui::EventArgs)
    {
        uint16_t port = static_cast<uint16_t>(std::stoi(txtPort.GetCurrentInputValue()));
        if (net::NetManager::Get().Host(port))
            AppendLog("Hosting on port " + std::to_string(port));
        else
            AppendLog("Failed to start host!");
        btnStartGame.SetEnabled(true);
    });
    gui.AddWidget(&btnHost);

    
    txtIP.Create("TxtIP");
    txtIP.SetText("127.0.0.1");
    txtIP.SetSize(XMFLOAT2(180, 32));
    txtIP.SetDescription("IP: ");
    gui.AddWidget(&txtIP);

    txtJoinPort.Create("TxtJoinPort");
    txtJoinPort.SetText("7777");
    txtJoinPort.SetSize(XMFLOAT2(120, 32));
    txtJoinPort.SetDescription("Port: ");
    gui.AddWidget(&txtJoinPort);

    btnJoin.Create("BtnJoin");
    btnJoin.SetText("JOIN");
    btnJoin.SetSize(XMFLOAT2(140, 44));
    btnJoin.SetColor(wi::Color(30, 50, 120, 220),  wi::gui::IDLE);
    btnJoin.SetColor(wi::Color(50, 80, 200, 255),   wi::gui::FOCUS);
    btnJoin.SetColor(wi::Color(15, 30, 80, 255),    wi::gui::ACTIVE);
    btnJoin.font.params.size = 18;
    btnJoin.font.params.h_align = wi::font::WIFALIGN_CENTER;
    btnJoin.font.params.v_align = wi::font::WIFALIGN_CENTER;
    btnJoin.OnClick([this](wi::gui::EventArgs)
    {
        std::string ipStr = txtIP.GetCurrentInputValue();
        uint16_t port = static_cast<uint16_t>(std::stoi(txtJoinPort.GetCurrentInputValue()));

        
        wi::network::Connection target;
        target.port = port;
        int parts[4] = {};
        if (sscanf_s(ipStr.c_str(), "%d.%d.%d.%d",
                     &parts[0], &parts[1], &parts[2], &parts[3]) == 4)
        {
            for (int i = 0; i < 4; ++i)
                target.ipaddress[i] = static_cast<uint8_t>(parts[i]);
        }

        std::string name = txtPlayerName.GetCurrentInputValue();
        if (name.empty()) name = "Player";

        AppendLog("Connecting to " + ipStr + ":" + std::to_string(port) + " as " + name);
        net::NetManager::Get().Connect(target, name);
    });
    gui.AddWidget(&btnJoin);

    
    btnStartGame.Create("BtnStartGame");
    btnStartGame.SetText("START GAME");
    btnStartGame.SetSize(XMFLOAT2(220, 52));
    btnStartGame.SetColor(wi::Color(90, 30, 200, 220),  wi::gui::IDLE);
    btnStartGame.SetColor(wi::Color(120, 50, 240, 255),  wi::gui::FOCUS);
    btnStartGame.SetColor(wi::Color(60, 10, 160, 255),   wi::gui::ACTIVE);
    btnStartGame.font.params.size = 20;
    btnStartGame.font.params.h_align = wi::font::WIFALIGN_CENTER;
    btnStartGame.font.params.v_align = wi::font::WIFALIGN_CENTER;
    btnStartGame.SetEnabled(false);
    btnStartGame.OnClick([this](wi::gui::EventArgs)
    {
        if (owner) owner->StartGame(0.5f);
    });
    gui.AddWidget(&btnStartGame);

    
    btnDisconnect.Create("BtnDisconnect");
    btnDisconnect.SetText("DISCONNECT");
    btnDisconnect.SetSize(XMFLOAT2(180, 40));
    btnDisconnect.SetColor(wi::Color(100, 20, 20, 200),  wi::gui::IDLE);
    btnDisconnect.SetColor(wi::Color(160, 40, 40, 240),   wi::gui::FOCUS);
    btnDisconnect.SetColor(wi::Color(70, 10, 10, 255),    wi::gui::ACTIVE);
    btnDisconnect.font.params.size = 16;
    btnDisconnect.font.params.h_align = wi::font::WIFALIGN_CENTER;
    btnDisconnect.font.params.v_align = wi::font::WIFALIGN_CENTER;
    btnDisconnect.OnClick([this](wi::gui::EventArgs)
    {
        net::NetManager::Get().Disconnect("User disconnected");
        btnStartGame.SetEnabled(false);
        AppendLog("Disconnected.");
    });
    gui.AddWidget(&btnDisconnect);

    statusLog = "Ready.";
}


void LobbyState::Update(float dt)
{
    wi::RenderPath2D::Update(dt);
    time += dt;

    if (wi::input::Press(wi::input::KEYBOARD_BUTTON_ESCAPE))
    {
        net::NetManager::Get().Disconnect("Left lobby");
        if (owner) owner->GoToMainMenu(0.4f);
    }

    
    net::NetManager::Get().PollInbound([this](const net::RawPacket& pkt)
    {
        HandlePacket(pkt);
    });
}


void LobbyState::ResizeLayout()
{
    wi::RenderPath2D::ResizeLayout();

    float W  = GetLogicalWidth();
    float H  = GetLogicalHeight();
    float cx = W * 0.5f;

    btnBack.SetPos(XMFLOAT2(28.0f, 28.0f));

    float leftX  = cx - 320.0f;
    float rightX = cx + 20.0f;
    float startY = H * 0.20f;

    
    txtPlayerName.SetPos(XMFLOAT2(cx - 110.0f, startY));

    
    float hy = startY + ROW_H + 16.0f;
    txtPort.SetPos(XMFLOAT2(leftX, hy));       hy += ROW_H;
    btnHost.SetPos(XMFLOAT2(leftX, hy));

    
    float jy = startY + ROW_H + 16.0f;
    txtIP.SetPos(XMFLOAT2(rightX, jy));        jy += ROW_H;
    txtJoinPort.SetPos(XMFLOAT2(rightX, jy));  jy += ROW_H;
    btnJoin.SetPos(XMFLOAT2(rightX, jy));

    
    float bottomY = H * 0.72f;
    btnStartGame.SetPos(XMFLOAT2(cx - 110.0f, bottomY));
    btnDisconnect.SetPos(XMFLOAT2(cx - 90.0f, bottomY + 60.0f));
}


void LobbyState::Compose(wi::graphics::CommandList cmd) const
{
    
    {
        wi::image::Params bg;
        bg.enableFullScreen();
        bg.color = XMFLOAT4(0.03f, 0.02f, 0.08f, 1.0f);
        wi::image::Draw(nullptr, bg, cmd);
    }

    float W = GetLogicalWidth();
    float H = GetLogicalHeight();
    float cx = W * 0.5f;

    
    {
        wi::image::Params glow;
        float gw = W * 0.7f, gh = H * 0.8f;
        glow.pos   = XMFLOAT3((W - gw) * 0.5f, H * 0.08f, 0.0f);
        glow.siz   = XMFLOAT2(gw, gh);
        glow.color = XMFLOAT4(0.10f, 0.04f, 0.35f, 0.12f);
        wi::image::Draw(nullptr, glow, cmd);
    }

    
    {
        wi::font::Params fp;
        fp.posX    = cx;
        fp.posY    = H * 0.08f;
        fp.size    = 38;
        fp.h_align = wi::font::WIFALIGN_CENTER;
        fp.v_align = wi::font::WIFALIGN_CENTER;
        fp.color   = wi::Color(200, 180, 255, 240);
        fp.shadowColor    = wi::Color(80, 20, 180, 160);
        fp.shadow_offset_x = 3.0f;
        fp.shadow_offset_y = 3.0f;
        wi::font::Draw("MULTIPLAYER", fp, cmd);
    }

    
    auto drawHeader = [&](const char* label, float x, float y)
    {
        wi::font::Params h;
        h.posX    = x;
        h.posY    = y;
        h.size    = 14;
        h.h_align = wi::font::WIFALIGN_LEFT;
        h.v_align = wi::font::WIFALIGN_CENTER;
        h.color   = wi::Color(140, 120, 200, 220);
        wi::font::Draw(label, h, cmd);
    };
    float startY = H * 0.20f + ROW_H + 4.0f;
    drawHeader("HOST A SERVER",  cx - 320.0f, startY);
    drawHeader("JOIN A SERVER",  cx + 20.0f,  startY);

    
    {
        wi::image::Params div;
        div.pos   = XMFLOAT3(cx - 1.0f, H * 0.18f, 0.0f);
        div.siz   = XMFLOAT2(2.0f, H * 0.55f);
        div.color = XMFLOAT4(0.3f, 0.1f, 0.6f, 0.4f);
        wi::image::Draw(nullptr, div, cmd);
    }

    
    {
        wi::font::Params lp;
        lp.posX    = cx - 220.0f;
        lp.posY    = H * 0.60f;
        lp.size    = 13;
        lp.h_align = wi::font::WIFALIGN_LEFT;
        lp.v_align = wi::font::WIFALIGN_TOP;
        lp.color   = wi::Color(150, 180, 150, 200);
        lp.h_wrap  = 440.0f;
        wi::font::Draw(statusLog, lp, cmd);
    }

    
    if (net::NetManager::Get().IsConnected() || net::NetManager::Get().IsHosting())
    {
        std::string pingStr = "Ping: " + std::to_string(net::NetManager::Get().GetPing()) + " ms";
        wi::font::Params pp;
        pp.posX    = W - 12.0f;
        pp.posY    = H - 30.0f;
        pp.size    = 13;
        pp.h_align = wi::font::WIFALIGN_RIGHT;
        pp.v_align = wi::font::WIFALIGN_CENTER;
        pp.color   = wi::Color(150, 220, 150, 200);
        wi::font::Draw(pingStr, pp, cmd);
    }

    wi::RenderPath2D::Compose(cmd);
}

