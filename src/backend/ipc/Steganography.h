#pragma once

#include <cstdint>
#include <vector>

namespace AetherVisor {
    namespace IPC {

        // Use pragma pack to ensure the structs have no padding, just like
        // real file format headers would on disk.
        #pragma pack(push, 1)

        // A simplified BMP File Header structure.
        struct FakeBMPFileHeader {
            uint16_t fileType = 0x4D42; // 'BM' for Bitmap
            uint32_t fileSize;          // Total size of the fake file
            uint16_t reserved1 = 0;
            uint16_t reserved2 = 0;
            uint32_t anprOffset;        // Offset to where our actual data begins
        };

        // A simplified BMP Info Header structure.
        struct FakeBMPInfoHeader {
            uint32_t headerSize = 40;
            int32_t  width;             // Can be used to store payload size
            int32_t  height;            // Can be used to store message type
            uint16_t planes = 1;
            uint16_t bitCount = 24;     // 24-bit color is common
            uint32_t compression = 0;
            uint32_t imageSize;         // Size of the raw pixel data
            int32_t  xPixelsPerM = 0;
            int32_t  yPixelsPerM = 0;
            uint32_t colorsUsed = 0;
            uint32_t colorsImportant = 0;
        };

        #pragma pack(pop)

        /**
         * @class StegoPacket
         * @brief A container for hiding our real IPC message inside a fake BMP structure.
         *
         * The real message (type and payload) will be encoded into the fields of
         * these headers and the pixelData buffer.
         */
        class StegoPacket {
        public:
            FakeBMPFileHeader fileHeader;
            FakeBMPInfoHeader infoHeader;
            std::vector<uint8_t> pixelData;

            // Serializes the entire packet into a byte vector for sending over a pipe.
            std::vector<uint8_t> Serialize() const;

            // Deserializes a byte vector back into a StegoPacket.
            static StegoPacket Deserialize(const std::vector<uint8_t>& buffer);
        };

    } // namespace IPC
} // namespace AetherVisor
