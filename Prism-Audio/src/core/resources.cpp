#include "resource_manager.hpp"
#include <fstream>
#include <stdexcept>

namespace Audio 
{
    Manager::Manager() = default;
    
    Manager::~Manager() = default;

    bool Manager::initialize(const std::string& bankPath)
    {
        std::ifstream file(bankPath, std::ios::binary);
        if (!file) 
        {
            return false;
        }

        uint32_t headerLength;
        file.read((char*)(&headerLength), sizeof(headerLength));

        std::vector<char> headerBuffer(headerLength);
        file.read(headerBuffer.data(), headerLength);
        
        std::string headerJson(headerBuffer.begin(), headerBuffer.end());
        
        // TODO: Parse JSON headers
        

        std::vector<char> rawData((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());
                                 bankData.resize(rawData.size() / 2);
        std::copy(
            reinterpret_cast<uint16_t*>(rawData.data()),
            reinterpret_cast<uint16_t*>(rawData.data() + rawData.size()),
            bankData.begin()
        );
    }

    void Manager::shutdown()
    {
        resourceBank.clear();
        bankData.clear();
    }

    bool Manager::parseHeader()
    {
        // TODO: Implement JSON parsing and resource bank population
        
        return !resourceBank.empty();
    }

    uint32_t Manager::encodeIdentifier(const Identifier& id) const
    {
        return (static_cast<uint32_t>(id.category) << 24) |
               (static_cast<uint32_t>(id.subcategory) << 16) |
               id.index;
    }

    std::vector<Resource> Manager::getSound(const Identifier& id)
    {
        std::vector<Resource> matchingResources;
        
        uint32_t key = encodeIdentifier(id);
        auto range = resourceBank.equal_range(key);
        
        for (auto it = range.first; it != range.second; ++it) 
        {
            matchingResources.push_back(it->second);
        }
        
        return matchingResources;
    }

    int8_t Manager::loadSoundData(const Identifier& id, AudioBuffer* buffer)
    {
        auto resources = getSound(id);
        if (resources.empty()) {
            return -1;
        }

        const Resource& resource = resources.front();

        if (resource.state != ResourceState::READY) {
            return -2; 
        }
        buffer->makeBuffer(resource.frames, resource.sampleRate);

        const uint16_t* sourceData = bankData.data() + resource.offset;
        
        for (uint32_t i = 0; i < resource.frames; ++i) {
            buffer->audioBuffer.data_L[i] = sourceData[i * 2] << 16;
            buffer->audioBuffer.data_R[i] = sourceData[i * 2 + 1] << 16;
        }

        return 0;
    }
}