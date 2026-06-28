#pragma once
#include "WickedEngine.h"
#include "net/Packets.h"

#include <atomic>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

namespace net
{




















struct RawPacket
{
    wi::network::Connection sender;
    std::vector<uint8_t>    data;
};

class NetManager
{
public:
    static NetManager& Get();

    
    
    bool Host(uint16_t port = wi::network::DEFAULT_PORT);

    
    bool Connect(wi::network::Connection target,
                 const std::string& playerName = "Player");

    
    void Disconnect(const std::string& reason = "");

    
    
    bool Send(const wi::network::Connection& to, const void* data, size_t size);

    
    template<typename T>
    bool SendPacket(const wi::network::Connection& to, const T& pkt)
    {
        return Send(to, &pkt, sizeof(T));
    }

    
    template<typename T>
    void Broadcast(const T& pkt)
    {
        std::lock_guard<std::mutex> lock(peersMutex);
        for (auto& peer : peers)
            Send(peer, &pkt, sizeof(T));
    }

    
    template<typename T>
    void SendSystem(const T& pkt)
    {
        if (hosting.load())
        {
            Broadcast(pkt);
        }
        else if (connected.load())
        {
            Send(serverConnection, &pkt, sizeof(T));
        }
    }

    const wi::network::Connection& GetServerConnection() const { return serverConnection; }

    
    
    
    void PollInbound(const std::function<void(const RawPacket&)>& handler);

    
    bool IsConnected()  const { return socket.IsValid() && connected.load(); }
    bool IsHosting()    const { return socket.IsValid() && hosting.load();   }
    uint32_t GetPing()  const { return lastPingMs.load(); }

private:
    NetManager()  = default;
    ~NetManager() = default;
    NetManager(const NetManager&) = delete;
    NetManager& operator=(const NetManager&) = delete;

    
    void StartReceiveLoop();
    void StopReceiveLoop();
    void EnqueuePacket(const wi::network::Connection& from,
                       const void* data, size_t size);

    
    wi::network::Socket        socket;
    wi::network::Connection    serverConnection; 

    std::atomic<bool>     connected { false };
    std::atomic<bool>     hosting   { false };
    std::atomic<bool>     running   { false };   
    std::atomic<uint32_t> lastPingMs{ 0 };

    
    std::mutex                          peersMutex;
    std::vector<wi::network::Connection> peers;

    
    std::mutex             inboundMutex;
    std::queue<RawPacket>  inboundQueue;

    
    uint8_t outSequence = 0;
};

} 
