#include "net/NetManager.h"
#include <chrono>

namespace net
{


NetManager& NetManager::Get()
{
    static NetManager instance;
    return instance;
}


bool NetManager::Host(uint16_t port)
{
    if (!wi::network::CreateSocket(&socket))
    {
        wi::backlog::post("[NetManager] CreateSocket failed", wi::backlog::LogLevel::Error);
        return false;
    }
    if (!wi::network::ListenPort(&socket, port))
    {
        wi::backlog::post("[NetManager] ListenPort failed", wi::backlog::LogLevel::Error);
        return false;
    }
    hosting = true;
    StartReceiveLoop();
    wi::backlog::post("[NetManager] Hosting on port " + std::to_string(port));
    return true;
}


bool NetManager::Connect(wi::network::Connection target, const std::string& playerName)
{
    if (!wi::network::CreateSocket(&socket))
    {
        wi::backlog::post("[NetManager] CreateSocket failed", wi::backlog::LogLevel::Error);
        return false;
    }
    
    wi::network::ListenPort(&socket, 0);

    serverConnection = target;

    
    PacketConnect pkt {};
    size_t nameLen = std::min(playerName.size(), sizeof(pkt.playerName) - 1);
    std::memcpy(pkt.playerName, playerName.c_str(), nameLen);
    pkt.header.bodySize = static_cast<uint16_t>(sizeof(pkt) - sizeof(PacketHeader));

    if (!Send(target, &pkt, sizeof(pkt)))
    {
        wi::backlog::post("[NetManager] Failed to send Connect packet", wi::backlog::LogLevel::Error);
        return false;
    }

    connected = true;
    StartReceiveLoop();
    wi::backlog::post("[NetManager] Connecting to server as '" + playerName + "'");
    return true;
}


void NetManager::Disconnect(const std::string& reason)
{
    if (!socket.IsValid()) return;

    PacketDisconnect pkt {};
    size_t len = std::min(reason.size(), sizeof(pkt.reason) - 1);
    std::memcpy(pkt.reason, reason.c_str(), len);
    pkt.header.bodySize = static_cast<uint16_t>(sizeof(pkt) - sizeof(PacketHeader));

    if (connected && !hosting)
        Send(serverConnection, &pkt, sizeof(pkt));
    else if (hosting)
        Broadcast(pkt);

    StopReceiveLoop();
    socket = {}; 
    connected = false;
    hosting   = false;

    std::lock_guard<std::mutex> lock(peersMutex);
    peers.clear();

    wi::backlog::post("[NetManager] Disconnected");
}


bool NetManager::Send(const wi::network::Connection& to, const void* data, size_t size)
{
    if (!socket.IsValid()) return false;
    return wi::network::Send(&socket, &to, data, size);
}


void NetManager::PollInbound(const std::function<void(const RawPacket&)>& handler)
{
    std::queue<RawPacket> local;
    {
        std::lock_guard<std::mutex> lock(inboundMutex);
        std::swap(local, inboundQueue);
    }
    while (!local.empty())
    {
        handler(local.front());
        local.pop();
    }
}


void NetManager::EnqueuePacket(const wi::network::Connection& from,
                               const void* data, size_t size)
{
    RawPacket pkt;
    pkt.sender = from;
    pkt.data.assign(
        reinterpret_cast<const uint8_t*>(data),
        reinterpret_cast<const uint8_t*>(data) + size);
    {
        std::lock_guard<std::mutex> lock(inboundMutex);
        inboundQueue.push(std::move(pkt));
    }
}


void NetManager::StartReceiveLoop()
{
    if (running) return;
    running = true;

    
    
    wi::jobsystem::Execute(wi::jobsystem::context{}, [this](wi::jobsystem::JobArgs)
    {
        static constexpr size_t RECV_BUF_SIZE = net::MAX_PACKET_SIZE;
        uint8_t buf[RECV_BUF_SIZE];

        while (running)
        {
            if (!socket.IsValid()) break;

            
            if (!wi::network::CanReceive(&socket, 1))
            {
                std::this_thread::sleep_for(std::chrono::microseconds(500));
                continue;
            }

            wi::network::Connection sender;
            if (wi::network::Receive(&socket, &sender, buf, RECV_BUF_SIZE))
            {
                
                if (RECV_BUF_SIZE >= sizeof(PacketHeader))
                {
                    const auto* hdr = reinterpret_cast<const PacketHeader*>(buf);
                    if (hdr->magic == PROTOCOL_MAGIC)
                    {
                        size_t total = sizeof(PacketHeader) + hdr->bodySize;
                        if (total <= RECV_BUF_SIZE)
                            EnqueuePacket(sender, buf, total);
                    }
                }
            }
        }
    });
}


void NetManager::StopReceiveLoop()
{
    running = false;
    
}

} 
