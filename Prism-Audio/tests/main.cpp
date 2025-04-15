#include "testFramework.hpp"

// Forward declarations of test suites
void register_audio_buffer_tests(TestFramework& framework);
void register_chain_fx_tests(TestFramework& framework);
void register_channel_tests(TestFramework& framework);

int main() {
    TestFramework framework;
    
    // Register all test suites
    register_audio_buffer_tests(framework);
    register_chain_fx_tests(framework);
    register_channel_tests(framework);
    
    // Run all tests
    framework.run_all();
    
    return framework.get_exit_code();
}

