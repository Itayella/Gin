/*==============================================================================

 Copyright 2020 by Roland Rabien
 For more information visit www.rabiensoftware.com

 ==============================================================================*/

namespace RIFFAtoms
{

struct RIFF
{
    uint32_t dwRIFF;
    uint32_t dwSize;
    uint32_t dwFourCC;
};

struct CHUNK
{
    uint32_t dwFourCC;
    uint32_t dwSize;
};

struct LIST
{
    uint32_t dwList;
    uint32_t dwSize;
    uint32_t dwFourCC;
};

static bool isTag (const void* a, const void* b)
{
    return std::memcmp (a, b, 4) == 0;
}

}

RIFFParser::RIFFParser (const juce::File& f, std::function<void (uint32_t, const uint8_t*, uint32_t)> callback)
    : onChunk (callback)
{
    juce::MemoryMappedFile map (f, juce::MemoryMappedFile::readOnly, false);

    parse (map.getData(), map.getSize());
}

RIFFParser::RIFFParser (const juce::MemoryBlock& m, std::function<void (uint32_t, const uint8_t*, uint32_t)> callback)
    : onChunk (callback)
{
    parse (m.getData(), m.getSize());
}

void RIFFParser::parse (const void* data, size_t size)
{
    if (size < sizeof (RIFFAtoms::RIFF))
        return;

    auto d = (const uint8_t*)data;

    auto riff = (const RIFFAtoms::RIFF*)d;
    d += sizeof (RIFFAtoms::RIFF);

    if (! RIFFAtoms::isTag (&riff->dwRIFF, "RIFF"))
        return;

    while (d < (uint8_t*)data + size)
    {
        auto chunk = (const RIFFAtoms::CHUNK*)d;
        if (RIFFAtoms::isTag (&chunk->dwFourCC, "LIST"))
            handleList (d);
        else
            handleChunk (d);
    }
}

void RIFFParser::handleList (const uint8_t *&d)
{
    auto list = (RIFFAtoms::LIST*)d;
    auto e = d + 8 + list->dwSize;
    d += sizeof (RIFFAtoms::LIST);

    while (d < e)
    {
        auto chunk = (RIFFAtoms::CHUNK*)d;
        if (RIFFAtoms::isTag (&chunk->dwFourCC, "LIST"))
            handleList (d);
        else
            handleChunk (d);
    }
}

void RIFFParser::handleChunk (const uint8_t*& d)
{
    auto chunk = (RIFFAtoms::CHUNK*)d;

    onChunk (chunk->dwFourCC, d + sizeof (RIFFAtoms::CHUNK), chunk->dwSize);

    d += sizeof (RIFFAtoms::CHUNK);
    d += chunk->dwSize;
    if (chunk->dwSize % 2 == 1)
        d += 1;
}
