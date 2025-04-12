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
    
    struct Identifier
    {
        uint8_t category;
        uint8_t subcategory; 
        uint16_t index; 
    };

    struct Resource 
    {
        Identifier id;
        uint32_t offset;
        uint32_t length;
        uint32_t frames;

        ResourceState state;
        uint32_t loopStart;
        uint32_t loopEnd;
        uint16_t loopCount;

        uint32_t sampleRate;
    };

    class Manager
    {
        public:
            Manager();
            ~Manager();
            
            bool initialize(const std::string& bankPath);
            void shutdown();
            
            std::vector<Resource> getSound(const Identifier& id);
            int8_t loadSoundData(const Identifier& id, AudioBuffer* buffer);
            
        private:
            std::vector<uint16_t> bankData;
            std::unordered_map<uint32_t, Resource> resourceBank;
            
            bool parseHeader();
            uint32_t encodeIdentifier(const Identifier& id) const;
    };
}