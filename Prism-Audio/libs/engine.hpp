#include "libs/audioHelper.hpp"
#include "libs/channel.hpp"
#include "chainFX.hpp"

namespace Audio
{
    class Engine 
    {
        public:
            Engine();
            ~Engine();
            
            bool initialize(uint32_t sampleRate = 44100, uint16_t bufferSize = 1024);
            void shutdown();

            std::shared_ptr<AudioBuffer> master;

    };
}