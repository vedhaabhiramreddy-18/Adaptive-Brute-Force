//  ═══════════════════════════════════════════════════════════════════════════
//  Adaptive Brute-Force Attack Mitigation Simulator
//  ─────────────────────────────────────────────────
//  Entry point.  Parse optional CLI flags and launch the simulation.
//
//  Usage:
//    ./abfams                          run with defaults
//    ./abfams --accounts 10            simulate 10 accounts
//    ./abfams --budget   50            use 50 resource units
//    ./abfams --seed     123           set RNG seed
//    ./abfams --help                   show usage
//  ═══════════════════════════════════════════════════════════════════════════

#include "Simulator.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <stdexcept>

static void printUsage(const char* prog) {
    std::cout << "Usage: " << prog << " [OPTIONS]\n\n"
              << "  --accounts  N   Number of simulated accounts  (default: 8, max: 12)\n"
              << "  --budget    B   Defense resource budget        (default: 30.0)\n"
              << "  --seed      S   RNG seed for reproducibility   (default: 42)\n"
              << "  --help          Show this help message\n\n";
}

int main(int argc, char* argv[]) {
    SimulationConfig cfg;

    // ── Simple CLI parser ────────────────────────────────────────────────────
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);

        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return EXIT_SUCCESS;
        }

        auto requireNext = [&](const std::string& flag) -> std::string {
            if (i + 1 >= argc) {
                std::cerr << "Error: " << flag << " requires a value.\n";
                std::exit(EXIT_FAILURE);
            }
            return std::string(argv[++i]);
        };

        try {
            if (arg == "--accounts") {
                cfg.numAccounts = std::stoi(requireNext(arg));
                cfg.numAccounts = std::max(1, std::min(cfg.numAccounts, 12));
            } else if (arg == "--budget") {
                cfg.initialBudget = std::stod(requireNext(arg));
            } else if (arg == "--seed") {
                cfg.seed = std::stoi(requireNext(arg));
            } else {
                std::cerr << "Unknown option: " << arg << "\n";
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }
        } catch (const std::invalid_argument&) {
            std::cerr << "Invalid value for " << arg << "\n";
            return EXIT_FAILURE;
        }
    }

    // ── Run ──────────────────────────────────────────────────────────────────
    Simulator sim(cfg);
    sim.run();

    return EXIT_SUCCESS;
}
