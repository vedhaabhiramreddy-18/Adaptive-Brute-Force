#include "DefenseManager.h"

#include <algorithm>
#include <iomanip>
#include <iostream>

// ─────────────────────────────────────────────────────────────────────────────
// Static catalog
// ─────────────────────────────────────────────────────────────────────────────
const std::vector<DefenseCatalog>& DefenseManager::catalog() {
    static const std::vector<DefenseCatalog> cat = {
        { DefenseType::DELAY,
          "Delay",      2.0, 0.20,
          "Adds 3-second delay per attempt; low disruption, low cost." },
        { DefenseType::CAPTCHA,
          "CAPTCHA",    5.0, 0.50,
          "Forces human verification; halves attack throughput." },
        { DefenseType::TEMP_BLOCK,
          "Temp-Block", 8.0, 0.85,
          "Locks account for 15 min; highest protection, highest cost." },
    };
    return cat;
}

const DefenseCatalog& DefenseManager::catalogEntry(DefenseType d) {
    for (const auto& c : catalog()) if (c.type == d) return c;
    return catalog()[0];   // fallback
}

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────
DefenseManager::DefenseManager(double budget)
    : budget_(budget), initialBudget_(budget) {}

// ─────────────────────────────────────────────────────────────────────────────
// Core greedy allocation
//
//  Algorithm (Greedy by efficiency = riskReduction / cost):
//  ─────────────────────────────────────────────────────────
//  while budget > 0 and unprotected accounts remain:
//      options = { (account, defense) | account not yet protected
//                                       and defense.cost ≤ remaining budget }
//      build max-heap keyed on efficiency
//      (account*, defense*) = heap.top()
//      apply defense* to account*
//      budget -= cost(defense*)
//      record decision
//
//  Greedy justification:
//    Choosing maximum efficiency at each step minimises the opportunity cost
//    of each resource unit spent.  For the fractional knapsack this is
//    provably optimal.  For the 0-1 variant (one defense per account) it is
//    a 1-approximation when effect factors differ little in relative size,
//    which holds for our three-tier catalog.
//
//  Time complexity per round: O(A·D·log(A·D))
//  where A = number of accounts, D = 3 defense types.
//  Total for R rounds: O(R·A·D·log(A·D)).
// ─────────────────────────────────────────────────────────────────────────────
void DefenseManager::allocate(std::vector<Account>& accounts, AttackLog& log) {
    int round = 1;

    while (budget_ > 0.0) {
        auto heap = buildOptions(accounts);
        if (heap.empty()) break;

        DefenseOption best = heap.top();

        // Check we can still afford it
        if (best.cost > budget_) {
            // Try to find something cheaper
            bool found = false;
            while (!heap.empty()) {
                best = heap.top(); heap.pop();
                if (best.cost <= budget_) { found = true; break; }
            }
            if (!found) break;
        }

        // Find and update the account
        for (auto& acc : accounts) {
            if (acc.username() == best.username) {
                double riskBefore = acc.effectiveRisk();
                acc.applyDefense(best.defense);
                double riskAfter  = acc.effectiveRisk();

                log.recordDefense(acc.username(), best.defense, riskBefore, riskAfter);

                Decision d;
                d.username   = acc.username();
                d.defense    = best.defense;
                d.riskBefore = riskBefore;
                d.riskAfter  = riskAfter;
                d.efficiency = best.efficiency;
                d.costSpent  = best.cost;
                decisions_.push_back(d);

                totalRiskReduced_ += (riskBefore - riskAfter);
                budget_           -= best.cost;

                std::cout << "  [Round " << std::setw(2) << round++ << "]  "
                          << std::left << std::setw(16) << acc.username()
                          << "  defense=" << std::setw(10) << defenseTypeName(best.defense)
                          << "  risk " << std::fixed << std::setprecision(1)
                          << std::setw(7) << riskBefore << " → "
                          << std::setw(7) << riskAfter
                          << "  efficiency=" << std::setprecision(3) << best.efficiency
                          << "  cost=" << std::setprecision(1) << best.cost
                          << "  budget_left=" << budget_ << "\n";
                break;
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Build option heap for one allocation round
// ─────────────────────────────────────────────────────────────────────────────
std::priority_queue<DefenseOption>
DefenseManager::buildOptions(const std::vector<Account>& accounts) const {
    std::priority_queue<DefenseOption> heap;

    for (const auto& acc : accounts) {
        if (acc.isProtected()) continue;

        double risk = acc.effectiveRisk();
        if (risk < kRiskThreshold) continue;   // not worth protecting

        for (const auto& cat : catalog()) {
            if (cat.cost > budget_) continue;   // can't afford

            double reduction = risk * cat.effectFactor;
            if (reduction <= 0.0) continue;

            DefenseOption opt;
            opt.username      = acc.username();
            opt.defense       = cat.type;
            opt.riskBefore    = risk;
            opt.riskReduction = reduction;
            opt.cost          = cat.cost;
            opt.efficiency    = reduction / cat.cost;
            heap.push(opt);
        }
    }
    return heap;
}

// ─────────────────────────────────────────────────────────────────────────────
// Reports
// ─────────────────────────────────────────────────────────────────────────────
void DefenseManager::printAllocationReport() const {
    std::cout << "\n  Decisions taken: " << decisions_.size() << "\n";
    std::cout << "  Budget spent   : " << std::fixed << std::setprecision(1)
              << (initialBudget_ - budget_)
              << " / " << initialBudget_ << " units\n";
    std::cout << "  Total risk Δ   : " << totalRiskReduced_ << "\n\n";

    std::cout << "  " << std::left
              << std::setw(16) << "Account"
              << std::setw(12) << "Defense"
              << std::setw(10) << "Before"
              << std::setw(10) << "After"
              << std::setw(12) << "Efficiency"
              << std::setw(8)  << "Cost" << "\n";
    std::cout << "  " << std::string(66, '-') << "\n";

    for (const auto& d : decisions_) {
        std::cout << "  " << std::left
                  << std::setw(16) << d.username
                  << std::setw(12) << defenseTypeName(d.defense)
                  << std::fixed << std::setprecision(1)
                  << std::setw(10) << d.riskBefore
                  << std::setw(10) << d.riskAfter
                  << std::setprecision(3)
                  << std::setw(12) << d.efficiency
                  << std::setprecision(1)
                  << std::setw(8)  << d.costSpent << "\n";
    }
}

void DefenseManager::printRemainingRisk(const std::vector<Account>& accounts) const {
    double total = 0.0;
    std::cout << "\n  Per-account remaining effective risk:\n\n";
    std::cout << "  " << std::left << std::setw(16) << "Account"
              << std::setw(14) << "Defense"
              << std::setw(12) << "Raw Risk"
              << "Eff. Risk\n";
    std::cout << "  " << std::string(52, '-') << "\n";

    for (const auto& acc : accounts) {
        double eff = acc.effectiveRisk();
        total += eff;
        std::cout << "  " << std::left
                  << std::setw(16) << acc.username()
                  << std::setw(14) << defenseTypeName(acc.currentDefense())
                  << std::fixed << std::setprecision(1)
                  << std::setw(12) << acc.computeRisk()
                  << eff << "\n";
    }
    std::cout << "  " << std::string(52, '-') << "\n";
    std::cout << "  " << std::setw(30) << "System total effective risk:"
              << total << "\n";
}
