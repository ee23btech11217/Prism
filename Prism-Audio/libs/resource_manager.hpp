#pragma once

#include "libs/audioHelper.hpp"
#include "libs/channel.hpp"
#include <unordered_map>
#include <vector>
#include <string>

namespace Audio 
{
    enum class ResourceState 
    { 
        UNLOADED, 
        LOADING, 
        READY, 
        FAILED 
    };

    struct BankHeader
    {
        char magic[8];
        uint32_t count;
        uint32_t headerSize;
        uint32_t totalSize; 
    };

    struct Identifier
    {
        uint8_t category;
        uint8_t subcategory; 
        uint16_t index; 
    };

    struct Entry {
        Identifier id;
        uint32_t offset;
        uint32_t length;
        uint32_t frames;
        uint16_t channels;
        uint32_t sampleRate;
    };
    
    struct Resource
    {
        uint32_t id;
        uint32_t bankIndex;
        ResourceState state;
        uint32_t refCount;
        uint32_t loopStart;
        uint32_t loopEnd;
        
        uint32_t sampleRate;
        uint32_t channels;
        uint32_t duration;
        Identifier id;
    };
    
    class Manager
    {
    public:
        Manager();
        ~Manager();
        
        bool initialize(const std::string& bankPath);
        void shutdown();
        
        uint32_t getSound(const std::string& soundName);
        uint32_t getSound(uint32_t bankIndex);
        void releaseSound(uint32_t resourceId);
        
        Resource* getResource(uint32_t resourceId);
        ResourceState getResourceState(uint32_t resourceId) const;
        
        bool assignSound(uint32_t resourceId, Channel& channel);
        bool assignSound(const std::string& soundName, Channel& channel);
        
        bool setLoopPoints(uint32_t resourceId, uint32_t loopStart, uint32_t loopEnd);
        
        std::vector<std::string> listAllSounds() const;
        uint32_t getSoundIdByName(const std::string& soundName) const;
        std::string getSoundNameById(uint32_t resourceId) const;
        
    private:
        uint8_t* bankData;
        uint32_t bankSize;
        std::vector<Entry> soundEntries;
        uint32_t dataOffset;

        std::unordered_map<uint32_t, Resource> resources;
        std::unordered_map<std::string, uint32_t> nameToId;
        std::unordered_map<uint32_t, uint32_t> bankIndexToId;
        
        uint32_t nextResourceId;
        
        bool createBufferFromBankData(uint32_t bankIndex, AudioBuffer* buffer);
    };
}