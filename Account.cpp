#include "Account.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <random>

// ─────────────────────────────────────────────────────────────────────────────
// Free function
// ─────────────────────────────────────────────────────────────────────────────
std::string defenseTypeName(DefenseType d) {
    switch (d) {
        case DefenseType::NONE:       return "None";
        case DefenseType::DELAY:      return "Delay";
        case DefenseType::CAPTCHA:    return "CAPTCHA";
        case DefenseType::TEMP_BLOCK: return "Temp-Block";
    }
    return "Unknown";
}

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────
Account::Account(std::string username, int attackIntensity)
    : username_(std::move(username)), attackIntensity_(attackIntensity) {}

// ─────────────────────────────────────────────────────────────────────────────
// Simulation helpers
// ─────────────────────────────────────────────────────────────────────────────
void Account::simulateAttacks(int numAttempts, time_t startTime, int intervalSeconds) {
    // We use a seeded RNG derived from the username so each account is unique
    std::size_t seed = std::hash<std::string>{}(username_) ^ static_cast<std::size_t>(startTime);
    std::mt19937 rng(static_cast<unsigned>(seed));

    // Higher attack intensity → more failed attempts, shorter jitter intervals
    double failProb = 0.60 + (attackIntensity_ / 100.0) * 0.35;   // 60%–95%
    std::bernoulli_distribution bDist(failProb);

    // IP pool – attackers cycle through several addresses
    std::vector<std::string> ips;
    int ipCount = 2 + attackIntensity_ / 30;
    for (int i = 0; i < ipCount; ++i) {
        std::ostringstream ss;
        ss << "192.168." << (rng() % 255) << "." << (rng() % 255);
        ips.push_back(ss.str());
    }
    std::uniform_int_distribution<int> ipDist(0, static_cast<int>(ips.size()) - 1);

    // Jitter on interval: high intensity → tighter clustering
    int jitter = std::max(1, intervalSeconds / 2);
    std::uniform_int_distribution<int> jDist(-jitter, jitter);

    time_t t = startTime;
    for (int i = 0; i < numAttempts; ++i) {
        LoginAttempt a;
        a.timestamp = t;
        a.success   = !bDist(rng);   // most are failures
        a.sourceIP  = ips[ipDist(rng)];
        attempts_.push_back(a);

        int gap = std::max(1, intervalSeconds + jDist(rng));
        t += gap;
    }
}

void Account::addAttempt(const LoginAttempt& attempt) {
    attempts_.push_back(attempt);
}

// ─────────────────────────────────────────────────────────────────────────────
// Risk computation
//
//  Let F  = total failed attempts
//  Let R  = failure ratio (F / total)  ∈ [0, 1]
//  Let B  = burst penalty = F / max(avgGap, 1)
//
//  risk = w_fail * F  +  w_ratio * R * 100  +  w_burst * B
//
//  Weights: w_fail=2.0,  w_ratio=3.0,  w_burst=1.5
// ─────────────────────────────────────────────────────────────────────────────
double Account::computeRisk() const {
    if (attempts_.empty()) return 0.0;

    const double W_FAIL  = 2.0;
    const double W_RATIO = 3.0;
    const double W_BURST = 1.5;

    double F     = static_cast<double>(failedAttempts());
    double total = static_cast<double>(attempts_.size());
    double R     = F / total;
    double gap   = avgTimeBetweenAttempts();
    double B     = F / std::max(gap, 1.0);

    return W_FAIL * F  +  W_RATIO * R * 100.0  +  W_BURST * B;
}

double Account::effectiveRisk() const {
    return computeRisk() * (1.0 - effectFactor(defense_));
}

// ─────────────────────────────────────────────────────────────────────────────
// Defense management
// ─────────────────────────────────────────────────────────────────────────────
void Account::applyDefense(DefenseType d) {
    defense_ = d;
}

bool Account::isProtected() const {
    return defense_ != DefenseType::NONE;
}

DefenseType Account::currentDefense() const {
    return defense_;
}

// ─────────────────────────────────────────────────────────────────────────────
// Accessors
// ─────────────────────────────────────────────────────────────────────────────
int Account::failedAttempts() const {
    int count = 0;
    for (const auto& a : attempts_) if (!a.success) ++count;
    return count;
}

// ─────────────────────────────────────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────────────────────────────────────
double Account::avgTimeBetweenAttempts() const {
    if (attempts_.size() < 2) return 1.0;

    double total = 0.0;
    for (std::size_t i = 1; i < attempts_.size(); ++i) {
        total += std::abs(static_cast<double>(attempts_[i].timestamp - attempts_[i-1].timestamp));
    }
    return total / static_cast<double>(attempts_.size() - 1);
}

double Account::effectFactor(DefenseType d) {
    switch (d) {
        case DefenseType::NONE:       return 0.00;
        case DefenseType::DELAY:      return 0.20;
        case DefenseType::CAPTCHA:    return 0.50;
        case DefenseType::TEMP_BLOCK: return 0.85;
    }
    return 0.0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Display
// ─────────────────────────────────────────────────────────────────────────────
void Account::printSummary() const {
    std::cout << std::left
              << "  Account   : " << username_ << "\n"
              << "  Intensity : " << attackIntensity_ << "/100\n"
              << "  Attempts  : " << attempts_.size()
              << "  (failed: " << failedAttempts() << ")\n"
              << std::fixed << std::setprecision(2)
              << "  Raw Risk  : " << computeRisk() << "\n"
              << "  Eff. Risk : " << effectiveRisk() << "\n"
              << "  Defense   : " << defenseTypeName(defense_) << "\n";
}
