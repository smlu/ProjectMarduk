#ifndef LIBIM_WAV_H
#define LIBIM_WAV_H
#include "../../../common.h"
#include "../../../io/stream.h"
#include <cstdint>

namespace libim::content::audio
{
    template<std::size_t N>
    constexpr uint32_t MakeRiffTag(const char (&tag)[N])
    {
        static_assert(N == 5, "tag len == 5");
        int32_t ntag = (tag[3] << 24) | (tag[2] << 16) | (tag[1] << 8) | (tag[0]);
        return static_cast<uint32_t>(ntag);
    }

    constexpr static uint32_t kRiffChunkId = MakeRiffTag("RIFF");
    constexpr static uint32_t kWavFormatId = MakeRiffTag("WAVE");
    constexpr static uint32_t kFmtChunkId  = MakeRiffTag("fmt ");
    constexpr static uint32_t kDataChunkId = MakeRiffTag("data");



    enum class AudioFormat : uint16_t
    {
        LPCM = 1
    };

    template<uint32_t Tag>
    struct RiffChunkHeader
    {
        uint32_t tag = Tag;
        uint32_t size;
    };

    struct WavFmt : RiffChunkHeader<kFmtChunkId>
    {
         AudioFormat audioFormat;
         uint16_t    numChannels;
         uint32_t    sampleRate;
         uint32_t    byteRate;
         uint16_t    blockAlign;
         uint16_t    bitsPerSample;
    };

    struct WavHeader : RiffChunkHeader<kRiffChunkId>
    {
        uint32_t format = kWavFormatId;
        WavFmt fmt;
    };

    struct WavDataChunk : RiffChunkHeader<kDataChunkId>
    {
        ByteArray data;
    };


    Stream& operator << (Stream& s, const WavDataChunk& chunk)
    {
        s << static_cast<const RiffChunkHeader<kDataChunkId>&>(chunk);
        s << chunk.data;
        return s;
    }
}

#endif // LIBIM_WAV_H
