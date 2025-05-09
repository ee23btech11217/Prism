#include "libs/audioHelper.hpp"
#include "libs/chainFX.hpp"
#include "libs/channel.hpp"
#include "libs/modifiers.hpp"

namespace Audio
{   
    class Engine
    {
        public:
            struct Command
            {
                enum class Type {};
                Type type;
            };
            Engine() = default;
            ~Engine() = default;

        private:
            std::vector<std::shared_ptr<Channel>> channels;
            std::queue<Engine::Command> globalCommandQueue;
            bool hasCommands = false;
    };
}
