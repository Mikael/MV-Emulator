#include <iostream>
#include <cstring>
#include <sstream> // For output buffering

#ifdef WIN32
#include <Windows.h>
#else
// Precompute color map to optimize switch-case
const unsigned int colorMap[16] = { 30, 34, 32, 36, 31, 35, 33, 37, 90, 94, 92, 96, 91, 95, 93, 97 };

unsigned int translateColorCode(unsigned int win) {
    return colorMap[win & 0xF];
}

void SetConsoleTextAttribute(unsigned int useless, unsigned int code) {
    unsigned int bg = (code >> 4) & 0xF;
    unsigned int fg = code & 0xF;

    std::cout << "\033[" << translateColorCode(bg) + 10 << ";" << translateColorCode(fg) << "m";
}
#endif

#include "../../include/Utils/Parser.h"

namespace Common {
    namespace Parser {
#ifdef WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#else
        unsigned int hConsole = 0;
#endif

        // Helper function to buffer and print data in a single operation
        void printHexData(std::uint8_t* data, std::size_t len) {
            std::ostringstream oss;
            for (std::size_t i = 0; i < len; ++i) {
                oss << std::hex << static_cast<uint8_t>(data[i]) << " ";
            }
            std::cout << oss.str() << "\n";
        }

        // Simplified printing functions for TCP and Command headers
        void printTcpHeader(Common::Protocol::TcpHeader header) {
            std::cout << "[Actual Size:" << header.getSize() << "]\n"
                << "[Bogus/Padding:" << header.getBogus() << "]\n"
                << "[SessionID:" << header.getSessionId() << "]\n"
                << "[Crypt:" << header.getCrypt() << "]\n";
        }

        void printCommandHeader(Common::Protocol::CommandHeader command) {
            std::cout << "[Mission:" << command.getMission() << "]\n"
                << "[Order:" << command.getOrder() << "]\n"
                << "[Extra:" << command.getExtra() << "]\n"
                << "[Option:" << command.getOption() << "]\n"
                << "[Padding:" << command.getBogus() << "]\n";
        }

        void parseCommandHeader(std::uint8_t* data) {
            SetConsoleTextAttribute(hConsole, 3);
            std::cout << "Command Header:\n";
            std::uint32_t actualCommand = *reinterpret_cast<std::uint32_t*>(data + 4); // Replacing memcpy
            Common::Protocol::CommandHeader command(actualCommand);
            SetConsoleTextAttribute(hConsole, 5);
            printCommandHeader(command);
            SetConsoleTextAttribute(hConsole, 7);
        }

        std::pair<std::size_t, uint32_t> parseTcpHeader(std::uint8_t* data) {
            SetConsoleTextAttribute(hConsole, 3);
            std::cout << "\nTCP Header:\n";
            std::uint32_t header = *reinterpret_cast<std::uint32_t*>(data); // Replacing memcpy
            Common::Protocol::TcpHeader parsedHeader(header);
            SetConsoleTextAttribute(hConsole, 5);
            printTcpHeader(parsedHeader);

            std::size_t sizeRetrievedFromTcpHeader = parsedHeader.getSize();
            std::uint32_t toCrypt = parsedHeader.getCrypt();

            SetConsoleTextAttribute(hConsole, 7);
            return { sizeRetrievedFromTcpHeader, toCrypt };
        }

        void parseDecryptedPacket(std::size_t len, std::uint8_t* data) {
            parseCommandHeader(data);
            SetConsoleTextAttribute(hConsole, 3);
            std::cout << "Decrypted Packet:\n";
            SetConsoleTextAttribute(hConsole, 7);
            printHexData(data, len);
        }

        void parse(std::uint8_t* data, std::size_t len, std::size_t port, const std::string& origin, const std::string& to, std::int32_t cryptKey, bool first) {
            SetConsoleTextAttribute(hConsole, 2);
            std::cout << "\n[" << origin << "->" << to << "]";
            SetConsoleTextAttribute(hConsole, 3);

            SetConsoleTextAttribute(hConsole, 5);
            std::cout << "[Size:" << len << "]\n";

            Common::Cryptography::Crypt cryptDefault(0);
            Common::Cryptography::Crypt userCrypt(cryptKey);

            cryptDefault.RC5Decrypt32(data, data, 4);
            std::uint32_t actualData = *reinterpret_cast<std::uint32_t*>(data); // Replacing memcpy
            SetConsoleTextAttribute(hConsole, 5);

            const Common::Protocol::TcpHeader header(actualData);
            printTcpHeader(header);

            const int toCrypt = header.getCrypt();
            const std::size_t actualSize = header.getSize();
            cryptDefault.RC5Encrypt32(data, data, 4);

            SetConsoleTextAttribute(hConsole, 7);
            printHexData(data, actualSize);

            if (first) {
                cryptDefault.RC5Decrypt32(data, data, 4);
                std::cout << "Decrypted Packet:\n";
                printHexData(data, actualSize);
                userCrypt.RC5Encrypt32(data, data, 4);
                std::uint32_t actualCommand = *reinterpret_cast<std::uint32_t*>(data + 4); // Replacing memcpy
                printCommandHeader(Common::Protocol::CommandHeader(actualCommand));
                return;
            }

            // Switch block for cryptographic handling
            switch (toCrypt) {
            case 0: // No crypt
                parseDecryptedPacket(actualSize, data);
                break;
            case 1:
                cryptDefault.RC5Decrypt64(data + 4, data + 4, static_cast<int>(actualSize - 4));
                parseDecryptedPacket(actualSize, data);
                cryptDefault.RC5Encrypt64(data + 4, data + 4, static_cast<int>(actualSize - 4));
                break;
            case 2:
                userCrypt.RC5Decrypt64(data + 4, data + 4, static_cast<int>(actualSize - 4));
                parseDecryptedPacket(actualSize, data);
                userCrypt.RC5Encrypt64(data + 4, data + 4, static_cast<int>(actualSize - 4));
                break;
            case 3:
                cryptDefault.RC6Decrypt128(data + 4, data + 4, static_cast<int>(actualSize - 4));
                parseDecryptedPacket(actualSize, data);
                cryptDefault.RC6Encrypt128(data + 4, data + 4, static_cast<int>(actualSize - 4));
                break;
            case 4:
                userCrypt.RC6Decrypt128(data + 4, data + 4, static_cast<int>(actualSize - 4));
                parseDecryptedPacket(actualSize, data);
                userCrypt.RC6Encrypt128(data + 4, data + 4, static_cast<int>(actualSize - 4));
                break;
            default:
                std::cerr << "Invalid crypt found!\n";
                break;
            }
        }

        void parse_cast(std::uint8_t* data, std::size_t len, std::size_t port, const std::string& origin, const std::string& to) {
            std::uint32_t actualData = *reinterpret_cast<std::uint32_t*>(data); // Replacing memcpy

            const Common::Protocol::TcpHeader header(actualData);
            std::uint32_t actualCommand = *reinterpret_cast<std::uint32_t*>(data + 4); // Replacing memcpy
            Common::Protocol::CommandHeader commandHeader{ actualCommand };

            if (commandHeader.getOrder() == 322 || commandHeader.getOrder() == 281) return; // Avoid printing position

            SetConsoleTextAttribute(hConsole, 2);
            std::cout << "\n[" << origin << "->" << to << "]";
            SetConsoleTextAttribute(hConsole, 3);
            std::cout << "[CastServer:" << port << "]";
            SetConsoleTextAttribute(hConsole, 5);
            std::cout << "[Size:" << len << "]\n";
            printTcpHeader(header);

            SetConsoleTextAttribute(hConsole, 7);
            printHexData(data, header.getSize());
            printCommandHeader(Common::Protocol::CommandHeader{ actualCommand });
        }
    }
}
