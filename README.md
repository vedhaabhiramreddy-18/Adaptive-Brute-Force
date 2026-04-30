# Adaptive Brute-Force Attack Mitigation Simulator

A C++17 simulator that models brute-force login attacks across multiple user
accounts and uses a **greedy algorithm** to allocate limited defensive
resources to the accounts that yield the greatest risk reduction per unit cost.

---

## Features

| Feature | Detail |
|---------|--------|
| Simulated accounts | Configurable number (1–12) with randomised attack intensity |
| Realistic login attempts | Failure ratio, IP cycling, burst clustering |
| Risk scoring | Weighted formula: failure count + ratio + burst penalty |
| Three defense strategies | Delay · CAPTCHA · Temp-Block |
| Greedy allocator | Efficiency = riskReduction / cost; max-heap selection |
| Full audit log | Every defense decision recorded with before/after risk |
| ASCII risk timeline | Visual bar showing reduction per account |
| CLI flags | `--accounts`, `--budget`, `--seed` |

---

## Quick Start

### Prerequisites

- GCC ≥ 9 or Clang ≥ 10 (C++17 support)
- GNU Make **or** CMake ≥ 3.16

### Build with Make

```bash
git clone https://github.com/<you>/adaptive-brute-force-mitigation.git
cd adaptive-brute-force-mitigation
make          # produces ./abfams
make run      # build + run immediately
```

### Build with CMake

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/abfams
```

### Run with options

```bash
./abfams --accounts 10 --budget 50 --seed 99
./abfams --help
```

---

## Project Structure

```
adaptive-brute-force-mitigation/
├── include/
│   ├── Account.h           # Account, LoginAttempt, DefenseType
│   ├── AttackLog.h         # Audit log, LogEntry
│   ├── DefenseManager.h    # Greedy allocator, DefenseCatalog
│   └── Simulator.h         # Orchestrator, SimulationConfig
├── src/
│   ├── Account.cpp
│   ├── AttackLog.cpp
│   ├── DefenseManager.cpp
│   ├── Simulator.cpp
│   └── main.cpp            # CLI entry point
├── docs/
│   └── DESIGN.md           # Risk formula, greedy justification, complexity
├── CMakeLists.txt
├── Makefile
└── README.md
```

---

## Risk Score Formula

```
risk = 2.0 × F  +  3.0 × R × 100  +  1.5 × B

  F       = failed attempts
  R       = failure ratio (F / total)
  B       = burst penalty = F / avg_gap_seconds
```

After applying a defense the effective risk is damped:

```
effective_risk = risk × (1 − effectFactor)
```

| Defense    | Cost | Effect |
|------------|:----:|:------:|
| Delay      | 2    | 20 %   |
| CAPTCHA    | 5    | 50 %   |
| Temp-Block | 8    | 85 %   |

---

## Greedy Algorithm

At each allocation step the engine computes:

```
efficiency(account, defense) = riskReduction / cost
```

and picks the highest-efficiency pair using a **max-heap
(`std::priority_queue`)**.  The loop repeats until the budget is
exhausted or all accounts are protected.

See [`docs/DESIGN.md`](docs/DESIGN.md) for full justification and
complexity analysis (**O(A² log A)** total).

---

## Sample Output

```
========================================================================
  Adaptive Brute-Force Attack Mitigation Simulator
  Greedy Defense Allocation Engine  |  C++17
========================================================================

------------------------------------------------------------------------
  INITIAL ACCOUNT STATE (before defenses)
------------------------------------------------------------------------

  Account       Intensity   Attempts  Failed    Raw Risk    Eff. Risk
  ------------------------------------------------------------------
  alice         57          18        14        132.3       132.3
  bob           82          27        24        231.8       231.8
  ...

------------------------------------------------------------------------
  GREEDY DEFENSE ALLOCATION
------------------------------------------------------------------------

  [Round  1]  bob               defense=Temp-Block  risk 231.8 → 34.8  efficiency=24.6  cost=8.0  budget_left=22.0
  [Round  2]  mallory           defense=Temp-Block  risk 198.4 → 29.8  ...
  ...
```

---

## Extending the Simulator

- **Add a new defense**: insert an entry in `DefenseManager::catalog()` and
  add the enum value to `DefenseType`.
- **Custom risk weights**: expose `W_fail`, `W_ratio`, `W_burst` as
  `Account` constructor parameters.
- **More accounts**: increase `--accounts` up to 12 (add names to the pool
  in `Simulator::buildAccounts()` to go further).

---

## License

MIT – see [LICENSE](LICENSE) for details.
# Adaptive-Brute-Force-Attack-Mitigation-Simulator
