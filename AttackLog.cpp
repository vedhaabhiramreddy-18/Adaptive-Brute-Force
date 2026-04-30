#include "AttackLog.h"

#include <algorithm>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

// ─────────────────────────────────────────────────────────────────────────────
// Logging
// ─────────────────────────────────────────────────────────────────────────────
void AttackLog::record(const std::string& username,
                       const std::string& event,
                       const std::string& detail,
                       double riskBefore,
                       double riskAfter) {
    LogEntry e;
    e.timestamp  = std::time(nullptr);
    e.username   = username;
    e.event      = event;
    e.detail     = detail;
    e.riskBefore = riskBefore;
    e.riskAfter  = riskAfter;
    log_.push_back(e);
}

void AttackLog::recordDefense(const std::string& username,
                               DefenseType defense,
                               double riskBefore,
                               double riskAfter) {
    std::string detail = "Applied " + defenseTypeName(defense)
                       + "  [risk: " + std::to_string(static_cast<int>(riskBefore))
                       + " → " + std::to_string(static_cast<int>(riskAfter)) + "]";
    record(username, "DEFENSE_APPLIED", detail, riskBefore, riskAfter);
    ++defenseCountPerUser_[username];
}

// ─────────────────────────────────────────────────────────────────────────────
// Query
// ─────────────────────────────────────────────────────────────────────────────
double AttackLog::totalRiskReduced() const {
    double total = 0.0;
    for (const auto& e : log_) {
        if (e.event == "DEFENSE_APPLIED")
            total += (e.riskBefore - e.riskAfter);
    }
    return total;
}

int AttackLog::defenseCount() const {
    int cnt = 0;
    for (const auto& e : log_) if (e.event == "DEFENSE_APPLIED") ++cnt;
    return cnt;
}

std::vector<LogEntry> AttackLog::entriesFor(const std::string& username) const {
    std::vector<LogEntry> result;
    for (const auto& e : log_) if (e.username == username) result.push_back(e);
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// Reports
// ─────────────────────────────────────────────────────────────────────────────
void AttackLog::printFullLog() const {
    std::string hline(70, '-');
    std::cout << "\n+- Full Audit Log (" << log_.size() << " entries) "
              << std::string(35, '-') << "+\n";

    for (const auto& e : log_) {
        std::cout << "| " << std::left << std::setw(12) << formatTime(e.timestamp)
                  << " | " << std::setw(14) << e.username
                  << " | " << std::setw(18) << e.event
                  << " | " << e.detail << "\n";
    }
    std::cout << "+" << hline << "+\n";
}

void AttackLog::printSummaryLog() const {
    std::cout << "\n  Defense allocation events:\n";
    int idx = 1;
    for (const auto& e : log_) {
        if (e.event != "DEFENSE_APPLIED") continue;
        std::cout << "  [" << idx++ << "] " << std::left << std::setw(16) << e.username
                  << "  " << e.detail << "\n";
    }
    if (idx == 1) std::cout << "  (none)\n";
}

void AttackLog::printRiskTimeline(const std::vector<std::string>& usernames) const {
    // Print a simple ASCII bar for each username showing risk reduction
    std::cout << "\n  Risk Reduction per Account (▓ = reduced, ░ = remaining):\n\n";

    // Gather per-account initial and final risk from log
    std::map<std::string, double> firstRisk, lastRisk;
    for (const auto& e : log_) {
        if (e.event != "DEFENSE_APPLIED") continue;
        if (firstRisk.find(e.username) == firstRisk.end())
            firstRisk[e.username] = e.riskBefore;
        lastRisk[e.username] = e.riskAfter;
    }

    for (const auto& u : usernames) {
        double before = firstRisk.count(u) ? firstRisk.at(u) : 0.0;
        double after  = lastRisk.count(u)  ? lastRisk.at(u)  : before;
        double maxRisk = 300.0;   // scale for bar width
        int barWidth = 40;
        int reduced  = static_cast<int>((before - after) / maxRisk * barWidth);
        int remaining= static_cast<int>(after / maxRisk * barWidth);
        reduced   = std::max(0, std::min(reduced,   barWidth));
        remaining = std::max(0, std::min(remaining, barWidth - reduced));

        std::cout << "  " << std::left << std::setw(16) << u << " [";
        for (int i = 0; i < reduced;   ++i) std::cout << "▓";
        for (int i = 0; i < remaining; ++i) std::cout << "░";
        for (int i = reduced + remaining; i < barWidth; ++i) std::cout << " ";
        std::cout << "]  " << std::fixed << std::setprecision(1)
                  << before << " → " << after << "\n";
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Private
// ─────────────────────────────────────────────────────────────────────────────
std::string AttackLog::formatTime(time_t t) {
    struct tm* tm_info = std::localtime(&t);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", tm_info);
    return std::string(buf);
}
