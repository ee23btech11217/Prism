#include "libs/audioHelper.hpp"
#include <stdexcept>

namespace Audio
{
    AudioBuffer::AudioBuffer(uint32_t bufID)
    {
        audioBuffer.id = bufID;
        audioBuffer.data_L = nullptr;
        audioBuffer.data_R = nullptr;
        audioBuffer.length = 0;
        audioBuffer.sampleRate = 0;
        audioBuffer.idx = 0;
        audioBuffer.isLoaded = false;
    }

    AudioBuffer::~AudioBuffer()
    {
        // Only try to free memory if it was allocated
        if (audioBuffer.data_L)
        {
            delete[] audioBuffer.data_L;
            audioBuffer.data_L = nullptr;
        }
        
        if (audioBuffer.data_R)
        {
            delete[] audioBuffer.data_R;
            audioBuffer.data_R = nullptr;
        }
    }

    Buffer AudioBuffer::makeBuffer(uint32_t length, uint32_t sampleRate)
    {
        if (audioBuffer.data_L)
        {
            delete[] audioBuffer.data_L;
        }
        
        if (audioBuffer.data_R)
        {
            delete[] audioBuffer.data_R;
        }
        
        audioBuffer.length = length;
        audioBuffer.sampleRate = sampleRate;
        audioBuffer.data_L = new int32_t[length];
        audioBuffer.data_R = new int32_t[length];
        audioBuffer.isLoaded = true;
        
        for (uint32_t i = 0; i < length; i++)
        {
            audioBuffer.data_L[i] = 0;
            audioBuffer.data_R[i] = 0;
        }
        
        return audioBuffer;
    }

    void AudioBuffer::loadBuffer(const std::string& filePath)
    {
        // Have to implement later
        // For now, we throw an exception so errors aren't silent
        throw std::runtime_error("AudioBuffer::loadBuffer not implemented for file: " + filePath);
    }

    void AudioBuffer::resampleBuffer(uint32_t newSampleRate)
    {
        if (!audioBuffer.data_L || !audioBuffer.data_R || audioBuffer.length == 0)
        {
            throw std::runtime_error("Error: Buffer is empty or not loaded.");
        }
    
        if (audioBuffer.sampleRate == newSampleRate)
        {
            return;
        }
        
        // Resampling logic
        // This is a simple linear interpolation resampling
        double ratio = static_cast<double>(audioBuffer.sampleRate) / static_cast<double>(newSampleRate);
        uint32_t newLength = static_cast<uint32_t>(audioBuffer.length / ratio);
        
        // Allocate new buffers
        int32_t* newData_L = new int32_t[newLength];
        int32_t* newData_R = new int32_t[newLength];
        
        // Resample using linear interpolation
        for (uint32_t i = 0; i < newLength; ++i)
        {
            double pos = i * ratio;
    
            uint32_t idx1 = static_cast<uint32_t>(pos);
            uint32_t idx2 = idx1 + 1;
            
            if (idx2 >= audioBuffer.length)
            {
                idx2 = audioBuffer.length - 1;
            }
            
            double frac = pos - idx1;
            
            newData_L[i] = static_cast<int32_t>((1.0 - frac) * audioBuffer.data_L[idx1] + frac * audioBuffer.data_L[idx2]);
            newData_R[i] = static_cast<int32_t>((1.0 - frac) * audioBuffer.data_R[idx1] + frac * audioBuffer.data_R[idx2]);
        }
        
        // Replace old buffers with new ones
        delete[] audioBuffer.data_L;
        delete[] audioBuffer.data_R;
        
        audioBuffer.data_L = newData_L;
        audioBuffer.data_R = newData_R;
        audioBuffer.length = newLength;
        audioBuffer.sampleRate = newSampleRate;
    }
}