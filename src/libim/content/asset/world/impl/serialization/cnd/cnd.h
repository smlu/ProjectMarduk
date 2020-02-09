#ifndef LIBIM_CND_H
#define LIBIM_CND_H
#include <array>
#include <cstdint>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

#include "cndstring.h"
#include "../../../georesource.h"
#include "../../../../animation/animation.h"
#include "../../../../../audio/impl/sbtrack.h"
#include "../../../../material/material.h"

#include "../../../../../../common.h"
#include "../../../../../../io/stream.h"
#include "../../../../../../math/color.h"
#include "../../../../../../math/vector2.h"
#include "../../../../../../math/vector4.h"
#include "../../../../../../utils/hashmap.h"

namespace libim::content::asset {

    //PACKED(
    struct CndHeader
    {
        uint32_t fileSize;
        CndString<1216> copyright;
        CndString<kCndMaxNameLen> filePath;
        uint32_t type;               // 0xD = container type (jones3dstatic.cnd), 0xC = game world
        uint32_t version;
        float    worldGravity;
        float    ceilingSky_Z;
        float    horizonDistance;
        Vector2f horizonSkyOffset; // x,y
        Vector2f ceilingSkyOffset; // x,y
        Vector4f LOD_Distances;
        struct {
            int32_t enabled;
            Color color;
            float startDepth;
            float endDepth;
        } fog;

        uint32_t numSounds;

        uint32_t numMaterials;
        uint32_t sizeMaterials;
        uint32_t aMaterials;          // 32-bit void*
        uint32_t apMatArray;          // 32-bit pointer to pointer

        uint32_t numVertices;
        uint32_t aVerticies;          // 32-bit pointer Vector3f*

        uint32_t unknown28;
        uint32_t unknown29;

        uint32_t numTexVertices;
        uint32_t aTexVerticies;       // 32-bit pointer Vector2f*

        uint32_t unknown32;
        uint32_t unknown33;

        uint32_t numAdjoins;
        uint32_t aAdjoins;            // 32-bit pointer

        uint32_t numSurfaces;
        uint32_t aSurfaces;           // 32-bit pointer

        uint32_t numSectors;
        uint32_t aSectors;            // 32-bit pointer

        uint32_t numAiClasses;
        uint32_t sizeAiClasses;
        uint32_t aAiClasses;          // 32-bit pointer

        uint32_t numModels;
        uint32_t sizeModels;
        uint32_t aModels;             // 32-bit pointer

        uint32_t numSprites;
        uint32_t sizeSprites;
        uint32_t aSprites;            // 32-bit pointer

        uint32_t numKeyframes;
        uint32_t sizeKeyframes;
        uint32_t aKeyframes;          // 32-bit pointer

        uint32_t numPuppets;
        uint32_t sizePuppets;
        uint32_t aPuppets;            // 32-bit pointer

        uint32_t numSoundClasses;
        uint32_t sizeSoundClasses;
        uint32_t aSoundClasses;       // 32-bit pointer

        uint32_t numCogScripts;
        uint32_t sizeCogScripts;
        uint32_t aCogScripts;         // 32-bit pointer

        uint32_t numCogs;
        uint32_t sizeCogs;
        uint32_t aCogs;               // 32-bit pointer

        uint32_t numThingTemplates;
        uint32_t sizeThingTemplates;
        uint32_t aThingTemplates;     // 32-bit pointer

        uint32_t numThings;
        uint32_t sizeThings;
        uint32_t aThings;             // 32-bit pointer

        uint32_t pvsSize;
        uint32_t aPvs;                // 32-bit pointer (struct of 16 * int)
    };//);


    struct CND final
    {
        static CndHeader LoadHeader(const InputStream& istream);

        /**
        * Parses sounds section.
        * Offset of istream has to be at the beginning of sound section.
        *
        * @param Reference to the SbTrack
        * @param Const reference to the InputStream
        * @return Returns last sound file ID nonce
        */
        static uint32_t ParseSectionSounds(audio::impl::SbTrack& track, const InputStream& istream);

        static std::size_t GetOffset_Materials(const InputStream& istream);
        static utils::HashMap<Material> ParseSection_Materials(const CndHeader& header, const InputStream& istream); // Reads materials section. Offset of istream hast to be at beginning of material section.
        static utils::HashMap<Material> ReadMaterials(const InputStream& istream);
        static void WriteSection_Materials(OutputStream& ostream, const utils::HashMap<Material>& materials);
        static bool ReplaceMaterial(const Material& mat, const std::string& filename);

        static std::size_t GetOffset_Georesource(const CndHeader& header, const InputStream& istream);
        static Georesource ParseSection_Georesource(const CndHeader& cndHeader, const InputStream& istream);
        static Georesource ReadGeoresource(const InputStream& istream);
        static void WriteSection_Georesource(OutputStream& ostream, const Georesource& geores);

        static std::size_t GetOffset_Sectors(const InputStream& istream, const CndHeader& header);
        static std::vector<Sector> ParseSection_Sectors(const InputStream& istream, const CndHeader& header);
        static std::vector<Sector> ReadSectors(const InputStream& istream);
        static void WriteSection_Sectors(OutputStream& ostream, const std::vector<Sector>& sectors);

        static std::size_t GetOffset_AiClass(const InputStream& istream, const CndHeader& header);
        static std::vector<std::string> ParseSection_AiClass(const InputStream& istream, const CndHeader& header);
        static std::vector<std::string> ReadAiClass(const InputStream& istream);
        static void WriteSection_AiClass(OutputStream& ostream, const std::vector<std::string>& aiclasses);

        static std::size_t GetOffset_Models(const InputStream& istream, const CndHeader& header);
        static std::vector<std::string> ParseSection_Models(const InputStream& istream, const CndHeader& header);
        static std::vector<std::string> ReadModels(const InputStream& istream);
        static void WriteSection_Models(OutputStream& ostream, const std::vector<std::string>& models);

        static std::size_t GetOffset_Sprites(const InputStream& istream, const CndHeader& header);
        static std::vector<std::string> ParseSection_Sprites(const InputStream& istream, const CndHeader& header);
        static std::vector<std::string> ReadSprites(const InputStream& istream);
        static void WriteSection_Sprites(OutputStream& ostream, const std::vector<std::string>& sprites);

        static std::size_t GetOffset_Keyframes(const InputStream& istream, const CndHeader& header);
        static utils::HashMap<Animation> ParseSection_Keyframes(const InputStream& istream, const CndHeader& header); // Reads keyframes section. Offset of istream hast to be at beginning of keyframe section.
        static utils::HashMap<Animation> ReadKeyframes(const InputStream& istream);
        static void WriteSection_Keyframes(OutputStream& ostream, const utils::HashMap<Animation>& animations);

        static std::size_t GetOffset_AnimClass(const InputStream& istream, const CndHeader& header);
        static std::vector<std::string> ParseSection_AnimClass(const InputStream& istream, const CndHeader& header);
        static std::vector<std::string> ReadAnimClass(const InputStream& istream);
        static void WriteSection_AnimClass(OutputStream& ostream, const std::vector<std::string>& animclasses);

        static std::size_t GetOffset_SoundClass(const InputStream& istream, const CndHeader& header);
        static std::vector<std::string> ParseSection_SoundClass(const InputStream& istream, const CndHeader& header);
        static std::vector<std::string> ReadSoundClass(const InputStream& istream);
        static void WriteSection_SoundClass(OutputStream& ostream, const std::vector<std::string>& sndclasses);

        static std::size_t GetOffset_CogScripts(const InputStream& istream, const CndHeader& header);
        static std::vector<std::string> ParseSection_CogScripts(const InputStream& istream, const CndHeader& header);
        static std::vector<std::string> ReadCogScripts(const InputStream& istream);
        static void WriteSection_CogScripts(OutputStream& ostream, const std::vector<std::string>& scripts);
    };


}
#endif // LIBIM_CND_H