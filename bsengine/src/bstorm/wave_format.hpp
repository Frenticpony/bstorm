#pragma once

namespace bstorm
{
class WaveFormat
{
public:
    size_t totalBytes = 0;
    size_t sampleRate = 0;
    size_t bitsPerSample = 0;
    size_t channelCount = 0;
    size_t formatTag = 0;
    size_t byteRate = 0;
    size_t blockAlign = 0;
};
}
