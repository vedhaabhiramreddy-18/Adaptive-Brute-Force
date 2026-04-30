#pragma once

#include "Account.h"
#include "AttackLog.h"
#include "DefenseManager.h"

#include <vector>
#include <string>

// ─────────────────────────────────────────────────────────────────────────────
// SimulationConfig – tuneable parameters passed to the Simulator
// ─────────────────────────────────────────────────────────────────────────────
struct SimulationConfig {
    int    numAccounts      = 8;
    int    simulationTicks  = 5;    // rounds of greedy allocation
    double initialBudget    = 30.0;
    int    seed             = 42;   // RNG seed for reproducibility
};

// ─────────────────────────────────────────────────────────────────────────────
// Simulator – orchestrates the whole simulation
//
//  1. Creates accounts with random attack intensities
//  2. Generates synthetic login attempts for each account
//  3. Runs the greedy DefenseManager for `simulationTicks` rounds
//  4. Prints a full report at the end
// ─────────────────────────────────────────────────────────────────────────────
class Simulator {
public:
    explicit Simulator(SimulationConfig cfg = {});

    void run();

private:
    SimulationConfig  cfg_;
    std::vector<Account> accounts_;
    AttackLog         log_;

    void buildAccounts();
    void printBanner()       const;
    void printInitialState() const;
    void printFinalReport()  const;

    // Pretty-print helpers
    static void printSeparator(char c = '=', int width = 72);
    static void printHeader(const std::string& title);
};
