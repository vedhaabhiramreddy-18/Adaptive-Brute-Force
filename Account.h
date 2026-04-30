#pragma once

#include <string>
#include <vector>
#include <ctime>

// ─────────────────────────────────────────────────────────────────────────────
// DefenseType – the three available mitigation strategies
// ─────────────────────────────────────────────────────────────────────────────
enum class DefenseType {
    NONE,
    DELAY,          // low-cost,  low-effect:   adds artificial latency
    CAPTCHA,        // mid-cost,  mid-effect:   forces human verification
    TEMP_BLOCK      // high-cost, high-effect:  temporarily locks the account
};

std::string defenseTypeName(DefenseType d);

// ─────────────────────────────────────────────────────────────────────────────
// LoginAttempt – a single login event recorded for an account
// ─────────────────────────────────────────────────────────────────────────────
struct LoginAttempt {
    time_t      timestamp;   // epoch seconds
    bool        success;     // true = successful login
    std::string sourceIP;    // originating IP address
};

// ─────────────────────────────────────────────────────────────────────────────
// Account – represents one user account being monitored
//
// Risk score formula (computed by computeRisk()):
//
//   Let F  = total failed attempts
//   Let R  = failure ratio  (F / total)  ∈ [0, 1]
//   Let B  = burst penalty  = F / max(avgGap, 1)   (higher when gaps are short)
//
//   risk = w_fail * F  +  w_ratio * R * 100  +  w_burst * B
//
//   Default weights: w_fail=2.0, w_ratio=3.0, w_burst=1.5
//
// After a defense is applied the score is damped by the defense's
// effectiveness factor (DELAY=0.20, CAPTCHA=0.50, TEMP_BLOCK=0.85).
// ─────────────────────────────────────────────────────────────────────────────
class Account {
public:
    // ── Construction ────────────────────────────────────────────────────────
    explicit Account(std::string username, int attackIntensity = 0);

    // ── Simulation helpers ───────────────────────────────────────────────────
    void simulateAttacks(int numAttempts, time_t startTime, int intervalSeconds);
    void addAttempt(const LoginAttempt& attempt);

    // ── Risk computation ─────────────────────────────────────────────────────
    double computeRisk() const;          // raw risk score
    double effectiveRisk() const;        // after defense damp

    // ── Defense management ───────────────────────────────────────────────────
    void applyDefense(DefenseType d);
    bool isProtected() const;
    DefenseType currentDefense() const;

    // ── Accessors ────────────────────────────────────────────────────────────
    const std::string& username()      const { return username_; }
    int                attackIntensity() const { return attackIntensity_; }
    int                totalAttempts()  const { return static_cast<int>(attempts_.size()); }
    int                failedAttempts() const;
    const std::vector<LoginAttempt>& attempts() const { return attempts_; }

    // ── Display ──────────────────────────────────────────────────────────────
    void printSummary() const;

private:
    std::string              username_;
    int                      attackIntensity_;
    std::vector<LoginAttempt> attempts_;
    DefenseType               defense_  = DefenseType::NONE;

    // Compute average inter-attempt gap (seconds)
    double avgTimeBetweenAttempts() const;

    // Defense effectiveness factors
    static double effectFactor(DefenseType d);
};
