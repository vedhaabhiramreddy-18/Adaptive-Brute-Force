#include "Simulator.h"

#include <algorithm>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <random>

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────
Simulator::Simulator(SimulationConfig cfg) : cfg_(cfg) {}

// ─────────────────────────────────────────────────────────────────────────────
// Run
// ─────────────────────────────────────────────────────────────────────────────
void Simulator::run() {
    printBanner();
    buildAccounts();
    printInitialState();

    printHeader("GREEDY DEFENSE ALLOCATION");

    // Print defense catalog for reference
    std::cout << "\n  Available defenses:\n";
    for (const auto& c : DefenseManager::catalog()) {
        std::cout << "    • " << std::left << std::setw(12) << c.name
                  << "  cost=" << c.cost
                  << "  effect=" << static_cast<int>(c.effectFactor * 100) << "%"
                  << "  – " << c.description << "\n";
    }

    std::cout << "\n  Budget: " << cfg_.initialBudget << " resource units\n\n";
    std::cout << "  Allocation trace:\n";

    DefenseManager mgr(cfg_.initialBudget);
    mgr.allocate(accounts_, log_);

    printFinalReport();

    printHeader("ALLOCATION SUMMARY TABLE");
    mgr.printAllocationReport();

    printHeader("REMAINING RISK");
    mgr.printRemainingRisk(accounts_);

    // Risk timeline
    printHeader("RISK REDUCTION TIMELINE");
    std::vector<std::string> names;
    for (const auto& a : accounts_) names.push_back(a.username());
    log_.printRiskTimeline(names);

    // Summary stats
    printHeader("SIMULATION SUMMARY");
    std::cout << "  Accounts simulated   : " << accounts_.size()  << "\n"
              << "  Defenses applied     : " << mgr.decisionsCount()  << "\n"
              << "  Total risk reduced   : " << std::fixed << std::setprecision(1)
              << mgr.totalRiskReduced() << "\n"
              << "  Budget remaining     : " << mgr.remainingBudget() << " / "
              << cfg_.initialBudget << " units\n";

    printSeparator();
    std::cout << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// Build accounts with varied attack intensities
// ─────────────────────────────────────────────────────────────────────────────
void Simulator::buildAccounts() {
    // Pre-defined usernames for readability
    const std::vector<std::string> names = {
        "alice",   "bob",       "carol",    "dave",
        "eve",     "frank",     "grace",    "heidi",
        "ivan",    "judy",      "mallory",  "oscar"
    };

    std::mt19937 rng(static_cast<unsigned>(cfg_.seed));

    // Attack intensities spread from low to very high
    std::uniform_int_distribution<int> intensityDist(10, 95);
    // Attempts per account: 5-30
    std::uniform_int_distribution<int> attemptsDist(5, 30);
    // Time interval between attempts (seconds): 1-60
    std::uniform_int_distribution<int> gapDist(1, 60);

    time_t baseTime = 1700000000;   // fixed epoch for reproducibility

    int n = std::min(cfg_.numAccounts, static_cast<int>(names.size()));
    for (int i = 0; i < n; ++i) {
        int intensity = intensityDist(rng);
        int attempts  = attemptsDist(rng);
        int gap       = gapDist(rng);

        Account acc(names[i], intensity);
        acc.simulateAttacks(attempts, baseTime + i * 3600, gap);
        accounts_.push_back(std::move(acc));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Display helpers
// ─────────────────────────────────────────────────────────────────────────────
void Simulator::printBanner() const {
    printSeparator('=');
    std::cout <<
        "  Adaptive Brute-Force Attack Mitigation Simulator\n"
        "  Greedy Defense Allocation Engine  |  C++17\n";
    printSeparator('=');
    std::cout << "\n";
}

void Simulator::printInitialState() const {
    printHeader("INITIAL ACCOUNT STATE (before defenses)");
    std::cout << "\n  " << std::left
              << std::setw(14) << "Account"
              << std::setw(12) << "Intensity"
              << std::setw(10) << "Attempts"
              << std::setw(10) << "Failed"
              << std::setw(12) << "Raw Risk"
              << "Eff. Risk\n";
    std::cout << "  " << std::string(66, '-') << "\n";

    for (const auto& acc : accounts_) {
        std::cout << "  " << std::left
                  << std::setw(14) << acc.username()
                  << std::setw(12) << acc.attackIntensity()
                  << std::setw(10) << acc.totalAttempts()
                  << std::setw(10) << acc.failedAttempts()
                  << std::fixed << std::setprecision(1)
                  << std::setw(12) << acc.computeRisk()
                  << acc.effectiveRisk() << "\n";
    }
    std::cout << "\n";
}

void Simulator::printFinalReport() const {
    printHeader("FINAL ACCOUNT STATE (after defenses)");
    std::cout << "\n  " << std::left
              << std::setw(14) << "Account"
              << std::setw(14) << "Defense"
              << std::setw(12) << "Raw Risk"
              << "Eff. Risk\n";
    std::cout << "  " << std::string(52, '-') << "\n";

    for (const auto& acc : accounts_) {
        std::cout << "  " << std::left
                  << std::setw(14) << acc.username()
                  << std::setw(14) << defenseTypeName(acc.currentDefense())
                  << std::fixed << std::setprecision(1)
                  << std::setw(12) << acc.computeRisk()
                  << acc.effectiveRisk() << "\n";
    }
    std::cout << "\n";
}

void Simulator::printSeparator(char c, int width) {
    std::cout << std::string(width, c) << "\n";
}

void Simulator::printHeader(const std::string& title) {
    std::cout << "\n";
    printSeparator('-');
    std::cout << "  " << title << "\n";
    printSeparator('-');
}
