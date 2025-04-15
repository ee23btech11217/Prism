#include "resourceManager.hpp"

namespace Audio 
{
    Manager::Manager() = default;
    
    Manager::~Manager() = default;
    
    bool Manager::initialize(const std::string& bankPath)
    {
        return parsePAB(bankPath);
    }
    
    void Manager::shutdown()
    {
        resourceBank.clear();
        soundData.clear();
    }
    
    uint32_t Manager::encodeIdentifier(const Identifier& id) const
    {
        return (static_cast<uint32_t>(id.category) << 24) |
               (static_cast<uint32_t>(id.subcategory) << 16) |
               id.index;
    }
    
    const Resource& Manager::getSound(const uint32_t id)
    {
        static Resource NULL_RS;
        NULL_RS.frames = 0;
        NULL_RS.id = Identifier{UINT8_MAX, UINT8_MAX, UINT16_MAX};
        NULL_RS.length = 0;
        NULL_RS.loopCount = 0;
        NULL_RS.loopEnd = 0;
        NULL_RS.loopStart = 0;
        NULL_RS.offset = 0;
        NULL_RS.sampleRate = 0;
        NULL_RS.state = ResourceState::FAILED;
        auto it = resourceBank.find(id);
        return it != resourceBank.end() ? it->second : NULL_RS;
    }

    bool Manager::parsePAB(const std::string& filename)
    {
        std::ifstream bankFile(filename, std::ios::binary);
        if (!bankFile.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return false;
        }

        char magicBuf[10] = {0};
        bankFile.read(magicBuf, 9);
        std::string magic = std::string(magicBuf);
        if (magic != "PRSMAUBNK") {
            std::cerr << "Invalid magic identifier: " << magic << std::endl;
            return false;
        }
        bankFile.read(reinterpret_cast<char*>(&version), 1);
        bankFile.read(reinterpret_cast<char*>(&entryCount), 4);
        bankFile.read(reinterpret_cast<char*>(&headerSize), 4);
        bankFile.read(reinterpret_cast<char*>(&totalSize), 4);

        size_t currentPos = bankFile.tellg();
        size_t padding = (4 - (currentPos % 4)) % 4;
        bankFile.seekg(currentPos + padding);

        for (uint32_t i = 0; i < entryCount; ++i) {
            Resource resource;
            
            bankFile.read(reinterpret_cast<char*>(&resource.id.category), 1);
            bankFile.read(reinterpret_cast<char*>(&resource.id.subcategory), 1);
            bankFile.read(reinterpret_cast<char*>(&resource.id.index), 2);
            
            bankFile.read(reinterpret_cast<char*>(&resource.offset), 4);
            bankFile.read(reinterpret_cast<char*>(&resource.length), 4);
            bankFile.read(reinterpret_cast<char*>(&resource.frames), 4);
            bankFile.read(reinterpret_cast<char*>(&resource.sampleRate), 4);
            bankFile.read(reinterpret_cast<char*>(&resource.loopStart), 4);
            bankFile.read(reinterpret_cast<char*>(&resource.loopEnd), 4);
            bankFile.read(reinterpret_cast<char*>(&resource.loopCount), 2);
            
            resource.state = ResourceState::UNLOADED;
            
            uint32_t key = encodeIdentifier(resource.id);
            resourceBank.insert({key, resource});
        }

        soundData.resize(totalSize / sizeof(int16_t));
        bankFile.read(reinterpret_cast<char*>(soundData.data()), totalSize - headerSize);

        std::cout << "Parsed PAB file successfully: " << filename << std::endl;
        std::cout << "Version: " << static_cast<int>(version) << ", Entry Count: " << entryCount
                  << ", Header Size: " << headerSize << ", Total Size: " << totalSize << std::endl;

        return true;
    }
    
}