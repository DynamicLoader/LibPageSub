#pragma once

#include <getopt.h>
#include <string>
#include <vector>
#include <iostream>

enum ProgramMode {
    MODE_NONE,
    MODE_SELFTEST,

    MODE_ALL,
    MODE_OPT,
    MODE_FIFO,
    MODE_LRU,
    MODE_OPTCLOCK,
    MODE_CLOCK,

};

class CmdArgParser {
public:
    CmdArgParser(int argc, char* argv[])
        : argc(argc)
        , argv(argv)
    {
        parse();
    }

    void parse()
    {
        static struct option long_options[] = {
            { "help", no_argument, 0, 'h' },
            { "input", optional_argument, 0, 'i' },
            { "output", optional_argument, 0, 'o' },
            { "algo", required_argument, 0, 'a' },
            { "psize", optional_argument, 0, 'p' },
            { "vsize", optional_argument, 0, 'v' },
            { "numops", optional_argument, 0, 'n' },
            { 0, 0, 0, 0 }
        };

        int option_index = 0;
        int c;

        if (argc == 1) {
            printHelp();
            exit(0);
        }
        while ((c = getopt_long(argc, argv, "hi:o:a:p:v:n:", long_options, &option_index)) != -1) {
            switch (c) {
            case 'i':
                inputFile = optarg;
                break;
            case 'o':
                outputFile = optarg;
                break;
            case 'h':
                printHelp();
                exit(0);
                break;
            case 'a':
                if (mode != MODE_NONE) {
                    std::cerr << "Multiple algorithm specified" << std::endl;
                    exit(-1);
                }
                if (std::string(optarg) == "all") {
                    mode = MODE_ALL;
                } else if (std::string(optarg) == "opt") {
                    mode = MODE_OPT;
                } else if (std::string(optarg) == "fifo") {
                    mode = MODE_FIFO;
                } else if (std::string(optarg) == "lru") {
                    mode = MODE_LRU;
                } else if (std::string(optarg) == "optclock") {
                    mode = MODE_OPTCLOCK;
                } else if (std::string(optarg) == "clock") {
                    mode = MODE_CLOCK;
                } else if (std::string(optarg) == "selftest") {
                    mode = MODE_SELFTEST;
                } else {
                    std::cerr << "Unknown algorithm: " << optarg << std::endl;
                    exit(-1);
                }
                break;
            case 'p':
                try {
                    psize = std::stoul(optarg);
                } catch (std::exception& e) {
                    std::cerr << "Invalid psize: " << optarg << std::endl;
                    exit(-1);
                }
                break;
            case 'v':
                try {
                    vsize = std::stoul(optarg);
                } catch (std::exception& e) {
                    std::cerr << "Invalid vsize: " << optarg << std::endl;
                    exit(-1);
                }
                break;
            case 'n':
                try {
                    numops = std::stoul(optarg);
                } catch (std::exception& e) {
                    std::cerr << "Invalid numops: " << optarg << std::endl;
                    exit(-1);
                }
                break;
            default:
                std::cerr << "Unknown option: " << (char)c << std::endl;
                printHelp();
                exit(-1);
                break;
            }
        }
        if (mode != MODE_SELFTEST) {
            if (psize == 0 || vsize == 0) {
                std::cerr << "Page size and virtual memory size must be specified during normal run" << std::endl;
                exit(-1);
            }
        }
    }

    void printHelp()
    {
        std::cerr << "Usage: " << argv[0] << " [options]\n"
                  << "Options:\n"
                  << "  -h, --help          Show this help message\n"
                  << "  -i, --input FILE    Input file\n"
                  << "  -o, --output FILE   Output file\n"
                  << "  -a, --algo ALGO     Algorithm to use (all, opt, fifo, lru, optclock, clock; selftest)\n"
                  << "  -p, --psize SIZE    Physical address space size (in pages)\n"
                  << "  -v, --vsize SIZE    Virtual address space size (in pages)\n"
                  << "  -n, --numops NUM    Number of operations to simulate\n"
                  << "\nNote 1) when running selftest, psize, vsize and numops are ignored\n"
                  << "     2) when running normal mode, psize and vsize must be specified\n"
                  << "     3) if numops is specified, random data will be generated to run\n"
                  << "        otherwise, test data will be read from stdin or input file\n"
                  << "     4) custom input format: <vpn> <access_type> (space separated)\n";
    }

    std::string getInputFile() const { return inputFile; }
    std::string getOutputFile() const { return outputFile; }

    auto getMode() const { return mode; }
    size_t getPSize() const { return psize; }
    size_t getVSize() const { return vsize; }
    size_t getNumOps() const { return numops; }

private:
    int argc;
    char** argv;
    std::string inputFile;
    std::string outputFile;
    ProgramMode mode = MODE_NONE;
    size_t psize = 0;
    size_t vsize = 0;
    size_t numops = 0;
};