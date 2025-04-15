#include "testFramework.hpp"
#include "libs/audioHelper.hpp"

// Test suite for AudioBuffer class
void register_audio_buffer_tests(TestFramework& framework) {
    // Test constructor initialization
    framework.add_test("AudioBuffer Constructor Initialization", []() {
        Audio::AudioBuffer buffer(1);
        
        TEST_ASSERT_EQUAL(buffer.audioBuffer.id, 1);
        TEST_ASSERT_EQUAL(buffer.audioBuffer.data_L, nullptr);
        TEST_ASSERT_EQUAL(buffer.audioBuffer.data_R, nullptr);
        TEST_ASSERT_EQUAL(buffer.audioBuffer.length, 0);
        TEST_ASSERT_EQUAL(buffer.audioBuffer.sampleRate, 0);
        TEST_ASSERT_EQUAL(buffer.audioBuffer.idx, 0);
        TEST_ASSERT_EQUAL(buffer.audioBuffer.isLoaded, false);
    });
    
    // Test makeBuffer
    framework.add_test("AudioBuffer makeBuffer", []() {
        Audio::AudioBuffer buffer(2);
        
        const uint32_t length = 1000;
        const uint32_t sampleRate = 44100;
        
        Audio::Buffer result = buffer.makeBuffer(length, sampleRate);
        
        TEST_ASSERT_EQUAL(result.length, length);
        TEST_ASSERT_EQUAL(result.sampleRate, sampleRate);
        TEST_ASSERT_NOT_EQUAL(result.data_L, nullptr);
        TEST_ASSERT_NOT_EQUAL(result.data_R, nullptr);
        TEST_ASSERT_EQUAL(result.isLoaded, true);
        
        // Check a few samples are initialized to zero
        TEST_ASSERT_EQUAL(result.data_L[0], 0);
        TEST_ASSERT_EQUAL(result.data_L[length/2], 0);
        TEST_ASSERT_EQUAL(result.data_L[length-1], 0);
        
        TEST_ASSERT_EQUAL(result.data_R[0], 0);
        TEST_ASSERT_EQUAL(result.data_R[length/2], 0);
        TEST_ASSERT_EQUAL(result.data_R[length-1], 0);
    });
    
    // Test resampleBuffer
    framework.add_test("AudioBuffer resampleBuffer", []() {
        Audio::AudioBuffer buffer(3);
        
        // First create a buffer
        const uint32_t originalLength = 1000;
        const uint32_t originalSampleRate = 44100;
        buffer.makeBuffer(originalLength, originalSampleRate);
        
        // Fill with sample data
        for (uint32_t i = 0; i < originalLength; i++) {
            buffer.audioBuffer.data_L[i] = i * 100;
            buffer.audioBuffer.data_R[i] = i * 200;
        }
        
        // Now resample to half the rate
        const uint32_t newSampleRate = 22050;
        buffer.resampleBuffer(newSampleRate);
        
        // New buffer should be half as large with half the sample rate
        TEST_ASSERT_EQUAL(buffer.audioBuffer.sampleRate, newSampleRate);
        TEST_ASSERT_EQUAL(buffer.audioBuffer.length, 500);
    });
    
    // Test multiple makeBuffer calls (memory cleanup)
    framework.add_test("AudioBuffer Multiple makeBuffer Calls", []() {
        Audio::AudioBuffer buffer(4);
        
        // Create first buffer
        buffer.makeBuffer(1000, 44100);
        
        // Create second buffer (should clean up the first)
        Audio::Buffer result = buffer.makeBuffer(500, 22050);
        
        TEST_ASSERT_EQUAL(result.length, 500);
        TEST_ASSERT_EQUAL(result.sampleRate, 22050);
        TEST_ASSERT_NOT_EQUAL(result.data_L, nullptr);
        TEST_ASSERT_NOT_EQUAL(result.data_R, nullptr);
    });
    
    // Test resampleBuffer with same rate (should be no-op)
    framework.add_test("AudioBuffer resampleBuffer Same Rate", []() {
        Audio::AudioBuffer buffer(5);
        
        const uint32_t length = 1000;
        const uint32_t sampleRate = 44100;
        
        buffer.makeBuffer(length, sampleRate);
        
        // Fill with sample data
        for (uint32_t i = 0; i < length; i++) {
            buffer.audioBuffer.data_L[i] = i * 100;
            buffer.audioBuffer.data_R[i] = i * 200;
        }
        
        // Should be a no-op
        buffer.resampleBuffer(sampleRate);
        
        TEST_ASSERT_EQUAL(buffer.audioBuffer.length, length);
        TEST_ASSERT_EQUAL(buffer.audioBuffer.sampleRate, sampleRate);
        
        // Data should remain unchanged
        TEST_ASSERT_EQUAL(buffer.audioBuffer.data_L[100], 100 * 100);
        TEST_ASSERT_EQUAL(buffer.audioBuffer.data_R[100], 100 * 200);
    });
    
    // Test exception handling for resampleBuffer with empty buffer
    framework.add_test("AudioBuffer resampleBuffer Empty Buffer Exception", []() {
        Audio::AudioBuffer buffer(6);
        
        bool exception_thrown = false;
        try {
            buffer.resampleBuffer(22050);
        } catch (const std::runtime_error& e) {
            exception_thrown = true;
        }
        
        TEST_ASSERT(exception_thrown);
    });
} 