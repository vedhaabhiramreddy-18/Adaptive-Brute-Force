# Design & Algorithm Documentation

## 1. Risk Function

### Formula

Given an account with a set of recorded login attempts, the **raw risk score**
is computed as:

```
risk = W_fail  × F
     + W_ratio × R × 100
     + W_burst × B

where:
  F       = total number of failed login attempts
  R       = failure ratio = F / total_attempts  ∈ [0, 1]
  avgGap  = average seconds between consecutive attempts
  B       = burst penalty = F / max(avgGap, 1)

Default weights:
  W_fail  = 2.0   – rewards reducing raw failure count
  W_ratio = 3.0   – penalises accounts with almost exclusively failed attempts
  W_burst = 1.5   – penalises short inter-attempt gaps (fast automated attacks)
```

### Intuition

| Component | Why it matters |
|-----------|----------------|
| `W_fail × F` | More failures = higher absolute threat volume |
| `W_ratio × R × 100` | A 95 % failure rate is a clearer brute-force signal than 40 % |
| `W_burst × B` | Rapid successive failures indicate automation; slow failures may be legitimate |

### Effective risk after defense

Each defense damps the raw score:

```
effective_risk = raw_risk × (1 − effectFactor(defense))
```

| Defense    | Cost | Effect factor | Effective reduction |
|------------|------|--------------|---------------------|
| Delay      | 2.0  | 0.20         | 20 %                |
| CAPTCHA    | 5.0  | 0.50         | 50 %                |
| Temp-Block | 8.0  | 0.85         | 85 %                |

---

## 2. Greedy Algorithm

### Problem statement

Given a fixed **resource budget** B and a set of accounts A each with
effective risk r_i, allocate **at most one** defense to each account to
**maximise total risk reduction** without exceeding B.

This is a variant of the **0-1 knapsack problem** (NP-hard in general).

### Greedy strategy: maximum efficiency first

At every allocation step:

1. For every **unprotected** account *i* and every defense *d*:
   ```
   efficiency(i, d) = riskReduction(i, d) / cost(d)
                    = (r_i × effectFactor(d)) / cost(d)
   ```
2. Build a **max-heap** keyed on `efficiency`.
3. Pop the top candidate `(i*, d*)` and apply it if `cost(d*) ≤ remaining budget`.
4. Repeat until budget exhausted or all accounts protected.

### Justification

* For the **fractional knapsack** this is **provably optimal** (classic result).
* For the **0-1 variant** greedy-by-efficiency is a strong heuristic that
  achieves near-optimal results when item weights (costs) are similar, which
  holds here (costs 2, 5, 8 are within one order of magnitude).
* The algorithm always makes a **locally optimal, irrevocable** choice —
  the defining property of a greedy algorithm.
* No backtracking is needed; the heap is rebuilt each round so updated risk
  scores (if the simulator were extended to dynamic re-scoring) are reflected.

---

## 3. Complexity Analysis

### Per round

| Step | Complexity |
|------|-----------|
| Iterate accounts × defenses | O(A × D) |
| Build max-heap | O(A × D × log(A × D)) |
| Extract top | O(log(A × D)) |

where A = number of accounts, D = 3 defense types (constant).

Simplified: **O(A log A)** per round.

### Total

With at most A rounds (one per account protected):

```
Total = O(A² log A)
```

For A = 12 (default) this is negligible.

### Space

* `accounts_` vector: O(A × T) where T = attempts per account.
* `log_` vector: O(A) entries (one per defense applied).
* Heap: O(A × D) = O(A).

Overall space: **O(A × T)**.

---

## 4. Class Responsibilities

```
Account          – models one user; computes risk; stores attempts and defense
AttackLog        – append-only audit ledger; risk-timeline report
DefenseManager   – greedy allocator; holds catalog and budget
Simulator        – orchestrator; builds accounts; runs allocation; prints report
```

### Key STL containers

| Container | Used in | Purpose |
|-----------|---------|---------|
| `std::vector<Account>` | Simulator | accounts collection |
| `std::vector<LoginAttempt>` | Account | attempt history |
| `std::vector<LogEntry>` | AttackLog | audit log |
| `std::priority_queue<DefenseOption>` | DefenseManager | greedy max-heap |
| `std::map<string,int>` | AttackLog | defenses per user |

---

## 5. Extension Points

* **Dynamic re-scoring** – call `computeRisk()` after each new batch of
  attempts and re-run the allocator; the greedy loop handles it naturally.
* **Custom weights** – expose `W_fail`, `W_ratio`, `W_burst` as constructor
  parameters of `Account` or a global `RiskConfig` struct.
* **More defenses** – add entries to `DefenseManager::catalog()` and update
  `DefenseType` enum; no other code changes needed.
* **Persistence** – serialise `AttackLog` entries to JSON/CSV for offline
  analysis.
