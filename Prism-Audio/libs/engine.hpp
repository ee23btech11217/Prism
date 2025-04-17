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
            std::shared_ptr<AudioBuffer> master;
            
    };
}