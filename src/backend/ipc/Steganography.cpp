#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "Steganography.h"
#include <cstring> // For memcpy

namespace AetherVisor {
    namespace IPC {

        std::vector<uint8_t> StegoPacket::Serialize() const {
            // Calculate total size
            size_t totalSize = sizeof(FakeBMPFileHeader) + sizeof(FakeBMPInfoHeader) + pixelData.size();
            std::vector<uint8_t> buffer(totalSize);

            // Copy header, info header, and pixel data into the buffer
            uint8_t* ptr = buffer.data();
            memcpy(ptr, &fileHeader, sizeof(FakeBMPFileHeader));
            ptr += sizeof(FakeBMPFileHeader);
            memcpy(ptr, &infoHeader, sizeof(FakeBMPInfoHeader));
            ptr += sizeof(FakeBMPInfoHeader);
            if (!pixelData.empty()) {
                memcpy(ptr, pixelData.data(), pixelData.size());
            }

            return buffer;
        }

        StegoPacket StegoPacket::Deserialize(const std::vector<uint8_t>& buffer) {
            StegoPacket packet;
            if (buffer.size() < sizeof(FakeBMPFileHeader) + sizeof(FakeBMPInfoHeader)) {
                // Buffer is too small to be a valid packet, return empty packet
                return packet;
            }

            const uint8_t* ptr = buffer.data();
            memcpy(&packet.fileHeader, ptr, sizeof(FakeBMPFileHeader));
            ptr += sizeof(FakeBMPFileHeader);
            memcpy(&packet.infoHeader, ptr, sizeof(FakeBMPInfoHeader));
            ptr += sizeof(FakeBMPInfoHeader);

            // Calculate size of pixel data and copy it
            size_t pixelDataSize = buffer.size() - (sizeof(FakeBMPFileHeader) + sizeof(FakeBMPInfoHeader));
            if (pixelDataSize > 0) {
                packet.pixelData.resize(pixelDataSize);
                memcpy(packet.pixelData.data(), ptr, pixelDataSize);
            }

            return packet;
        }

    } // namespace IPC
} // namespace AetherVisor
