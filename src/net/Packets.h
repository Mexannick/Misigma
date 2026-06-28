#pragma once
#include <array>
#include <cstdint>
#include <cstring>













namespace net
{


static constexpr size_t   MAX_PACKET_SIZE = 1400; 
static constexpr uint32_t PROTOCOL_MAGIC  = 0x4D534947u; 


enum class PacketType : uint8_t
{
    
    Connect     = 0x01,  
    ConnectAck  = 0x02,  
    Disconnect  = 0x03,  
    Ping        = 0x04,  
    Pong        = 0x05,  

    
    PlayerState = 0x10,  
    WorldState  = 0x11,  
    ChatMessage = 0x20,  
    Build       = 0x30,  
    DestroyEntity = 0x31, 
};


#pragma pack(push, 1)
struct PacketHeader
{
    uint32_t   magic    = PROTOCOL_MAGIC;
    PacketType type     = PacketType::Ping;
    uint8_t    sequence = 0;   
    uint16_t   bodySize = 0;   

    explicit PacketHeader(PacketType t) : magic(PROTOCOL_MAGIC), type(t), sequence(0), bodySize(0) {}
    PacketHeader() = default;
};
static_assert(sizeof(PacketHeader) == 8, "PacketHeader size mismatch");


struct PacketConnect
{
    PacketHeader header;
    char         playerName[32] = {};   
    uint8_t      reserved[4]    = {};

    PacketConnect() { header = PacketHeader(PacketType::Connect); }
};


struct PacketConnectAck
{
    PacketHeader header;
    uint8_t      slotId     = 0;    
    uint32_t     sessionKey = 0;    
    uint8_t      reserved[3] = {};

    PacketConnectAck() { header = PacketHeader(PacketType::ConnectAck); }
};


struct PacketDisconnect
{
    PacketHeader header;
    char         reason[64] = {};

    PacketDisconnect() { header = PacketHeader(PacketType::Disconnect); }
};


struct PacketPing
{
    PacketHeader header;
    uint64_t     timestamp = 0; 

    PacketPing() { header = PacketHeader(PacketType::Ping); }
};

struct PacketPong
{
    PacketHeader header;
    uint64_t     timestamp = 0; 

    PacketPong() { header = PacketHeader(PacketType::Pong); }
};


struct PacketPlayerState
{
    PacketHeader header;
    uint8_t  slotId    = 0;
    float    posX      = 0.0f;
    float    posY      = 0.0f;
    float    posZ      = 0.0f;
    float    yaw       = 0.0f;  
    float    pitch     = 0.0f; 
    uint16_t inputMask = 0;    

    PacketPlayerState() { header = PacketHeader(PacketType::PlayerState); }
};


struct PlayerSlot
{
    uint8_t slotId = 0xFF;     
    float   posX   = 0.0f;
    float   posY   = 0.0f;
    float   posZ   = 0.0f;
    float   yaw    = 0.0f;
};

struct PacketWorldState
{
    PacketHeader header;
    uint32_t     tick      = 0; 
    uint8_t      slotCount = 0;
    PlayerSlot   slots[32] = {};

    PacketWorldState() { header = PacketHeader(PacketType::WorldState); }
};


struct PacketChatMessage
{
    PacketHeader header;
    uint8_t      slotId  = 0;
    char         text[256] = {};

    PacketChatMessage() { header = PacketHeader(PacketType::ChatMessage); }
};


struct PacketBuild
{
    PacketHeader header;
    uint8_t      structureType; 
    float        posX;
    float        posY;
    float        posZ;
    float        rotX;
    float        rotY;
    float        rotZ;
    float        rotW;
    uint32_t     netID;         

    PacketBuild() { header = PacketHeader(PacketType::Build); }
};


struct PacketDestroyEntity
{
    PacketHeader header;
    uint32_t     netID;         

    PacketDestroyEntity() { header = PacketHeader(PacketType::DestroyEntity); }
};

#pragma pack(pop)

} 
