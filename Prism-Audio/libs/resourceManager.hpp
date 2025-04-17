#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>

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
            
            bool parsePAB();
            const Resource& getSound(const uint32_t id);
            std::string mainHeader;
            std::vector<int16_t> soundData;
        private:
            std::string managerBankPath;
            std::unordered_map<uint32_t, Resource> resourceBank;
            uint8_t version;
            uint32_t entryCount, headerSize, totalSize;
            
            bool parseHeader();
            uint32_t encodeIdentifier(const Identifier& id) const;
    };
}