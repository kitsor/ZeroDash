#include "ConfigParser.h"
#include "InfoManager.h"
#include <iostream>
#include <cstring>

#if __GNUC__
#if __x86_64__ || __aarch64__
#define PLATFORM_ENVIRONMENT "x64"
#else
#define PLATFORM_ENVIRONMENT "x32"
#endif
#endif

using namespace std;

int parseArgs(int argc, char* argv[], Config &config) {
    string confFilePath = "default.conf";

    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            if (std::strcmp(argv[i], "-h") == 0 || std::strcmp(argv[i], "--help") == 0) {
                std::cout << "Usage: ZeroDash [options]\n";
                std::cout << "Options:\n";
                std::cout << "  -h, --help              Show help message\n";
                std::cout << "  -c, --config <filename> Use config file (default: default.conf)\n";
                return 0;
            }
            else if (std::strcmp(argv[i], "-c") == 0 || std::strcmp(argv[i], "--config") == 0) {
                if (i + 1 < argc) {
                    confFilePath = argv[i + 1];
                    std::cout << "Using config: '" << confFilePath << "'" << std::endl;
                    ++i;
                }
                else {
                    std::cerr << "Error: Missing config file after -c or --config" << std::endl;
                    return 1;
                }
            }
            else {
                std::cerr << "Error: Invalid option: " << argv[i] << std::endl;
                return 1;
            }
        }
    }
    else {
        std::cout << "No arguments provided. Will use default config " << confFilePath << ".\n";
    }

    config = ConfigParser::Parse(confFilePath);
    return -1;
}


int main(int argc, char* argv[]) {
#ifdef PLATFORM_ENVIRONMENT
    std::cout << "ZeroDash " << PLATFORM_ENVIRONMENT << std::endl;
#else
    std::cout << "ZeroDash" << std::endl;
#endif

    try {
        Config config;

        int result = parseArgs(argc, argv, config);
        if (result >= 0) {
            return result;
        }

        InfoManager manager(config);
        manager.Run();
    }
    catch (std::exception &ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        return 1;
    }    

    return 0;
}