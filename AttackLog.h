#pragma once

#include "Account.h"
#include <string>
#include <vector>
#include <map>
#include <ctime>

// ─────────────────────────────────────────────────────────────────────────────
// LogEntry – one record in the system-wide audit trail
// ─────────────────────────────────────────────────────────────────────────────
struct LogEntry {
    time_t      timestamp;
    std::string username;
    std::string event;          // e.g. "DEFENSE_APPLIED", "RISK_COMPUTED"
    std::string detail;         // human-readable detail
    double      riskBefore;
    double      riskAfter;
};

// ─────────────────────────────────────────────────────────────────────────────
// AttackLog – system-wide ledger that tracks every attack event and
//             all defense allocations.
//
// Responsibilities
//   • Accept new log entries from the simulator
//   • Answer "what is the current total system risk?"
//   • Print per-account and full audit reports
// ─────────────────────────────────────────────────────────────────────────────
class AttackLog {
public:
    // ── Logging ──────────────────────────────────────────────────────────────
    void record(const std::string& username,
                const std::string& event,
                const std::string& detail,
                double riskBefore,
                double riskAfter);

    void recordDefense(const std::string& username,
                       DefenseType defense,
                       double riskBefore,
                       double riskAfter);

    // ── Query ─────────────────────────────────────────────────────────────────
    double totalRiskReduced() const;
    int    defenseCount()     const;

    // Entries for a specific account
    std::vector<LogEntry> entriesFor(const std::string& username) const;

    // All entries in chronological order
    const std::vector<LogEntry>& allEntries() const { return log_; }

    // ── Reports ──────────────────────────────────────────────────────────────
    void printFullLog()     const;
    void printSummaryLog()  const;     // only DEFENSE_APPLIED entries
    void printRiskTimeline(const std::vector<std::string>& usernames) const;

private:
    std::vector<LogEntry>               log_;
    std::map<std::string, int>          defenseCountPerUser_;

    static std::string formatTime(time_t t);
};
