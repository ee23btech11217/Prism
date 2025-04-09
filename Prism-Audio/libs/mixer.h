#include "audioHelper.hpp"
#include "channel.hpp"


namespace Audio
{
    class Engine;
    class Mixer
    {
        public:
            Mixer(Engine& engine, std::vector<Channel*>& channels);
            ~Mixer();

            void mixChannels(float* outputBuffer);
            void setMasterVolume(float volume);
    };
} 
