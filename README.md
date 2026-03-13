# 🎮 Rock-Paper-Scissors V2.0

A feature-rich, polished **Rock-Paper-Scissors** console game written in C++ — with an optional **Rock-Paper-Scissors-Lizard-Spock** extended mode!

---

## ✨ Features

| Feature | Description |
|---|---|
| 🎯 **Classic & RPSLS Modes** | Choose between standard RPS or the extended Lizard-Spock variant |
| 🤖 **3 AI Difficulty Levels** | *Easy* (random), *Medium* (frequency analysis), *Hard* (pattern recognition) |
| 🏆 **Score Tracking** | Running scoreboard with wins, losses, and ties |
| 📊 **Game Summary** | Detailed end-of-game stats: win %, streaks, total rounds |
| 🎨 **ASCII Art** | Visual art for every move |
| 🌈 **Colored Output** | ANSI-colored terminal output (green/red/yellow) |
| ⏱️ **Countdown Animation** | "3… 2… 1… SHOOT!" animation before each reveal |
| 📜 **Move History** | Review all moves played in the session |
| 🏅 **Best-of-N Matches** | Play unlimited, best-of-3, best-of-5, or best-of-7 |
| ✅ **Input Validation** | Graceful handling of invalid inputs |

---

## 🚀 Getting Started

### Prerequisites

- A C++ compiler with C++11 support (e.g., `g++`, `clang++`, MSVC)

### Compile

```bash
g++ -std=c++11 -o rps GameRPS.cpp
```

### Run

```bash
./rps
```

On Windows:
```cmd
rps.exe
```

---

## 🎮 How to Play

1. **Select game mode** — Classic (RPS) or Extended (RPSLS)
2. **Select difficulty** — Easy, Medium, or Hard
3. **Select match type** — Unlimited, Best of 3/5/7
4. **Play!** — Enter your move each round:
   - `r` — Rock
   - `p` — Paper
   - `s` — Scissors
   - `l` — Lizard *(RPSLS only)*
   - `k` — Spock *(RPSLS only)*
   - `h` — View move history
   - `q` — Quit

---

## 🧠 AI Difficulty Levels

| Level | Strategy |
|---|---|
| **Easy** | Completely random — fair 50/50 |
| **Medium** | Tracks your most frequent recent moves and counters them 60% of the time |
| **Hard** | Analyzes move sequences to predict your next play using pattern recognition |

---

## 📸 Sample Output

```
  ╔══════════════════════════════════════════════════╗
  ║     ROCK  ·  PAPER  ·  SCISSORS  ·  V2.0        ║
  ╚══════════════════════════════════════════════════╝

  ══════ ROUND 1 (Best of 5) ══════

  ┌─────────── SCOREBOARD ───────────┐
  │  You:   0  |  CPU:   0  |  Ties:   0  │
  └───────────────────────────────────┘

  >>> SHOOT! <<<

  ★ YOU WIN! Rock crushes Scissors!
```

---

## 📄 License

This project is open source and available for educational and personal use.