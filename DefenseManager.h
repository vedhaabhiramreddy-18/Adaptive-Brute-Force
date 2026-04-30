#pragma once

#include "Account.h"
#include "AttackLog.h"

#include <vector>
#include <map>
#include <queue>

// ─────────────────────────────────────────────────────────────────────────────
// DefenseOption – a candidate (account + defense) pair evaluated by the
//                 greedy selector.
// ─────────────────────────────────────────────────────────────────────────────
struct DefenseOption {
    std::string username;
    DefenseType defense;
    double      riskBefore;      // effective risk before applying defense
    double      riskReduction;   // riskBefore * effectFactor
    double      cost;            // resource cost of this defense
    double      efficiency;      // riskReduction / cost  ← greedy key

    // Max-heap comparator (highest efficiency first)
    bool operator<(const DefenseOption& o) const {
        return efficiency < o.efficiency;   // reversed for max-heap
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// DefenseCatalog – static data for each defense type
// ─────────────────────────────────────────────────────────────────────────────
struct DefenseCatalog {
    DefenseType type;
    std::string name;
    double      cost;            // resource units consumed
    double      effectFactor;    // fraction of risk eliminated (0–1)
    std::string description;
};

// ─────────────────────────────────────────────────────────────────────────────
// DefenseManager – allocates defenses using a greedy algorithm
//
// Greedy strategy:
//   At every allocation step compute, for every unprotected account and
//   every available defense, the efficiency = riskReduction / cost.
//   Pick the (account, defense) pair with the HIGHEST efficiency, apply it,
//   deduct the cost from the available budget, and repeat until:
//     (a) the budget is exhausted, or
//     (b) all accounts are protected, or
//     (c) remaining unprotected accounts have risk < threshold.
//
// Justification: greedy-by-efficiency is optimal for the fractional
// knapsack variant; it also gives a good approximation for the 0-1 case
// because defenses are mutually exclusive per account.
//
// Time complexity per round: O(A * D * log(A*D))  where A = #accounts,
//                                                        D = #defense types
// Total across all rounds: O(A^2 * D * log(A*D))
// ─────────────────────────────────────────────────────────────────────────────
class DefenseManager {
public:
    explicit DefenseManager(double budget);

    // ── Main entry point ─────────────────────────────────────────────────────
    // Run the greedy allocation loop over the provided accounts.
    // All events are recorded in `log`.
    void allocate(std::vector<Account>& accounts, AttackLog& log);

    // ── Reporting ────────────────────────────────────────────────────────────
    void printAllocationReport() const;
    void printRemainingRisk(const std::vector<Account>& accounts) const;

    double remainingBudget()   const { return budget_; }
    double totalRiskReduced()  const { return totalRiskReduced_; }
    int    decisionsCount()    const { return static_cast<int>(decisions_.size()); }

    // ── Catalog (static) ─────────────────────────────────────────────────────
    static const std::vector<DefenseCatalog>& catalog();
    static const DefenseCatalog& catalogEntry(DefenseType d);

private:
    double budget_;
    double totalRiskReduced_ = 0.0;
    double initialBudget_;

    struct Decision {
        std::string username;
        DefenseType defense;
        double      riskBefore;
        double      riskAfter;
        double      efficiency;
        double      costSpent;
    };
    std::vector<Decision> decisions_;

    // Build the priority queue of options for one round
    std::priority_queue<DefenseOption>
    buildOptions(const std::vector<Account>& accounts) const;

    // Minimum risk threshold below which we skip protection
    static constexpr double kRiskThreshold = 5.0;
};
