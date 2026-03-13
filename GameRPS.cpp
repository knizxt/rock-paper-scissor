#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <thread>
#include <chrono>
#include <algorithm>
#include <sstream>

using namespace std;

// ANSI escape codes for colored terminal output
namespace Color {
    const string RESET   = "\033[0m";
    const string RED     = "\033[1;31m";
    const string GREEN   = "\033[1;32m";
    const string YELLOW  = "\033[1;33m";
    const string BLUE    = "\033[1;34m";
    const string MAGENTA = "\033[1;35m";
    const string CYAN    = "\033[1;36m";
    const string WHITE   = "\033[1;37m";
    const string DIM     = "\033[2m";
    const string BOLD    = "\033[1m";
}

// ASCII hand art for each choice
namespace Art {
    const string ROCK =
        "    _______\n"
        "---'   ____)\n"
        "      (_____)\n"
        "      (_____)\n"
        "      (____)\n"
        "---.__(___)";

    const string PAPER =
        "     _______\n"
        "---'    ____)____\n"
        "           ______)\n"
        "          _______)\n"
        "         _______)\n"
        "---.__________)\n";

    const string SCISSORS =
        "    _______\n"
        "---'   ____)____\n"
        "          ______)\n"
        "       __________)\n"
        "      (____)\n"
        "---.__(___)";

    const string LIZARD =
        "        _\n"
        "       / \\\n"
        "      / _ \\\n"
        "     |  / \\ |\n"
        "      \\/ | |\n"
        "    /\\___|/\n"
        "   /  ~~~~\\\n"
        "~~~~~~~~~~";

    const string SPOCK =
        "    _______\n"
        "---'   ____)____\n"
        "          ______)\n"
        "        __)\n"
        "       (__)\n"
        "---.__(___)   [LLAP]";

    const string TITLE =
        "\n"
        "  ╔══════════════════════════════════════════════════╗\n"
        "  ║                                                  ║\n"
        "  ║     ██████   ██████   ███████   ██     ██        ║\n"
        "  ║     ██   ██  ██   ██  ██        ██     ██        ║\n"
        "  ║     ██████   ██████   ███████   ██     ██        ║\n"
        "  ║     ██   ██  ██            ██    ██   ██         ║\n"
        "  ║     ██   ██  ██       ███████     █████          ║\n"
        "  ║                                                  ║\n"
        "  ║     ROCK  ·  PAPER  ·  SCISSORS  ·  V2.0        ║\n"
        "  ║                                                  ║\n"
        "  ╚══════════════════════════════════════════════════╝\n";
}

// each possible move (INVALID is used for quit / bad input)
enum class Move { ROCK, PAPER, SCISSORS, LIZARD, SPOCK, INVALID };

struct MoveInfo {
    string name;
    string art;
    char key;
};

// holds all the scores and settings for the current session
struct GameState {
    int playerWins   = 0;
    int computerWins = 0;
    int ties         = 0;
    int totalRounds  = 0;
    int currentStreak = 0;
    int bestStreak    = 0;
    bool extendedMode = false;       // RPSLS mode
    int difficultyLevel = 1;         // 1=Easy, 2=Medium, 3=Hard
    int bestOfN = 0;                 // 0 = unlimited
    vector<pair<Move, Move>> history; // (player, computer)
};


void sleepMs(int ms) {
    this_thread::sleep_for(chrono::milliseconds(ms));
}

void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

map<Move, MoveInfo> getMoveInfoMap(bool extended) {
    map<Move, MoveInfo> m;
    m[Move::ROCK]     = {"Rock",     Art::ROCK,     'r'};
    m[Move::PAPER]    = {"Paper",    Art::PAPER,    'p'};
    m[Move::SCISSORS] = {"Scissors", Art::SCISSORS, 's'};
    if (extended) {
        m[Move::LIZARD] = {"Lizard", Art::LIZARD, 'l'};
        m[Move::SPOCK]  = {"Spock",  Art::SPOCK,  'k'};
    }
    return m;
}

vector<Move> getValidMoves(bool extended) {
    vector<Move> moves = {Move::ROCK, Move::PAPER, Move::SCISSORS};
    if (extended) {
        moves.push_back(Move::LIZARD);
        moves.push_back(Move::SPOCK);
    }
    return moves;
}

Move charToMove(char c) {
    c = tolower(c);
    switch (c) {
        case 'r': return Move::ROCK;
        case 'p': return Move::PAPER;
        case 's': return Move::SCISSORS;
        case 'l': return Move::LIZARD;
        case 'k': return Move::SPOCK;
        default:  return Move::INVALID;
    }
}

string moveToString(Move m) {
    switch (m) {
        case Move::ROCK:     return "Rock";
        case Move::PAPER:    return "Paper";
        case Move::SCISSORS: return "Scissors";
        case Move::LIZARD:   return "Lizard";
        case Move::SPOCK:    return "Spock";
        default:             return "?";
    }
}

string getArt(Move m) {
    switch (m) {
        case Move::ROCK:     return Art::ROCK;
        case Move::PAPER:    return Art::PAPER;
        case Move::SCISSORS: return Art::SCISSORS;
        case Move::LIZARD:   return Art::LIZARD;
        case Move::SPOCK:    return Art::SPOCK;
        default:             return "";
    }
}

// which moves beat which, and with what verb (e.g. "Rock crushes Scissors")
struct BeatInfo { Move loser; string verb; };

vector<BeatInfo> getBeatsMap(Move m) {
    switch (m) {
        case Move::ROCK:
            return {{Move::SCISSORS, "crushes"}, {Move::LIZARD, "crushes"}};
        case Move::PAPER:
            return {{Move::ROCK, "covers"}, {Move::SPOCK, "disproves"}};
        case Move::SCISSORS:
            return {{Move::PAPER, "cuts"}, {Move::LIZARD, "decapitates"}};
        case Move::LIZARD:
            return {{Move::SPOCK, "poisons"}, {Move::PAPER, "eats"}};
        case Move::SPOCK:
            return {{Move::SCISSORS, "smashes"}, {Move::ROCK, "vaporizes"}};
        default: return {};
    }
}

// Returns: 1 = player wins, -1 = computer wins, 0 = tie
int determineWinner(Move player, Move computer, string &reason) {
    if (player == computer) {
        reason = "It's a tie!";
        return 0;
    }
    // Check if player's move beats computer's
    for (auto &b : getBeatsMap(player)) {
        if (b.loser == computer) {
            reason = moveToString(player) + " " + b.verb + " " + moveToString(computer) + "!";
            return 1;
        }
    }
    // Computer wins
    for (auto &b : getBeatsMap(computer)) {
        if (b.loser == player) {
            reason = moveToString(computer) + " " + b.verb + " " + moveToString(player) + "!";
            return -1;
        }
    }
    reason = "Unknown outcome!";
    return 0;
}

// ──────────────────────────────────────────
// AI Computer Choice
// ──────────────────────────────────────────

Move computerChooseEasy(bool extended) {
    auto moves = getValidMoves(extended);
    return moves[rand() % moves.size()];
}

Move computerChooseMedium(bool extended, const vector<pair<Move, Move>> &history) {
    // Weighted random: slightly favors moves that would beat the player's most common move
    auto moves = getValidMoves(extended);
    if (history.size() < 3) return computerChooseEasy(extended);

    // Count player's recent move frequency
    map<Move, int> freq;
    int lookback = min((int)history.size(), 10);
    for (int i = (int)history.size() - lookback; i < (int)history.size(); i++) {
        freq[history[i].first]++;
    }

    // Find player's most common move
    Move mostCommon = moves[0];
    int maxCount = 0;
    for (auto &p : freq) {
        if (p.second > maxCount) {
            maxCount = p.second;
            mostCommon = p.first;
        }
    }

    // 60% chance: pick a move that beats the player's most common, 40% random
    if (rand() % 100 < 60) {
        for (auto &m : moves) {
            for (auto &b : getBeatsMap(m)) {
                if (b.loser == mostCommon) return m;
            }
        }
    }
    return computerChooseEasy(extended);
}

Move computerChooseHard(bool extended, const vector<pair<Move, Move>> &history) {
    // Pattern recognition: analyze sequences to predict next move
    auto moves = getValidMoves(extended);
    if (history.size() < 5) return computerChooseMedium(extended, history);

    // Look for repeating patterns (last 2-move sequence)
    Move last1 = history.back().first;
    Move last2 = history[history.size() - 2].first;

    // Search for this same sequence earlier in history
    map<Move, int> nextMoveFreq;
    for (int i = 1; i < (int)history.size() - 1; i++) {
        if (history[i - 1].first == last2 && history[i].first == last1) {
            if (i + 1 < (int)history.size()) {
                nextMoveFreq[history[i + 1].first]++;
            }
        }
    }

    // If pattern found, counter the predicted move
    if (!nextMoveFreq.empty()) {
        Move predicted = nextMoveFreq.begin()->first;
        int maxF = 0;
        for (auto &p : nextMoveFreq) {
            if (p.second > maxF) { maxF = p.second; predicted = p.first; }
        }
        // Pick a move that beats the predicted move
        for (auto &m : moves) {
            for (auto &b : getBeatsMap(m)) {
                if (b.loser == predicted) return m;
            }
        }
    }

    // Fallback to medium
    return computerChooseMedium(extended, history);
}

Move computerChoose(const GameState &state) {
    switch (state.difficultyLevel) {
        case 1: return computerChooseEasy(state.extendedMode);
        case 2: return computerChooseMedium(state.extendedMode, state.history);
        case 3: return computerChooseHard(state.extendedMode, state.history);
        default: return computerChooseEasy(state.extendedMode);
    }
}

// ──────────────────────────────────────────
// Display Helpers
// ──────────────────────────────────────────

void printDivider() {
    cout << Color::DIM << "  ─────────────────────────────────────────" << Color::RESET << endl;
}

void printScoreboard(const GameState &s) {
    cout << endl;
    cout << Color::CYAN << "  ┌─────────── SCOREBOARD ───────────┐" << Color::RESET << endl;
    cout << Color::CYAN << "  │" << Color::RESET;
    cout << Color::GREEN << "  You: " << setw(3) << s.playerWins << Color::RESET;
    cout << Color::DIM << "  |" << Color::RESET;
    cout << Color::RED << "  CPU: " << setw(3) << s.computerWins << Color::RESET;
    cout << Color::DIM << "  |" << Color::RESET;
    cout << Color::YELLOW << "  Ties: " << setw(3) << s.ties << Color::RESET;
    cout << Color::CYAN << "  │" << Color::RESET << endl;
    cout << Color::CYAN << "  └───────────────────────────────────┘" << Color::RESET << endl;
}

void printCountdown() {
    const string frames[] = {"3", "2", "1", "SHOOT!"};
    const string colors[] = {Color::RED, Color::YELLOW, Color::GREEN, Color::CYAN};
    for (int i = 0; i < 4; i++) {
        cout << "\r  " << colors[i] << ">>> " << frames[i] << " <<<" << Color::RESET << "   " << flush;
        sleepMs(i == 3 ? 500 : 700);
    }
    cout << endl << endl;
}

void printMoveComparison(Move player, Move computer) {
    // Print side by side
    cout << Color::CYAN << "  ╔═══ YOUR CHOICE ════╦══ CPU CHOICE ══╗" << Color::RESET << endl;

    // Split art into lines and print side by side
    auto splitLines = [](const string &s) -> vector<string> {
        vector<string> lines;
        istringstream iss(s);
        string line;
        while (getline(iss, line)) lines.push_back(line);
        return lines;
    };

    vector<string> playerLines  = splitLines(getArt(player));
    vector<string> cpuLines     = splitLines(getArt(computer));
    size_t maxLines = max(playerLines.size(), cpuLines.size());

    for (size_t i = 0; i < maxLines; i++) {
        string pLine = (i < playerLines.size()) ? playerLines[i] : "";
        string cLine = (i < cpuLines.size())    ? cpuLines[i]    : "";
        // Pad to 22 chars
        while (pLine.size() < 22) pLine += " ";
        while (cLine.size() < 22) cLine += " ";
        cout << "  " << Color::GREEN << pLine << Color::RESET
             << "  " << Color::RED   << cLine << Color::RESET << endl;
    }

    cout << "  " << Color::GREEN << moveToString(player) << Color::RESET;
    cout << string(22 - moveToString(player).size(), ' ');
    cout << "  " << Color::RED << moveToString(computer) << Color::RESET << endl;
    cout << endl;
}

void printHistory(const GameState &s) {
    if (s.history.empty()) {
        cout << Color::DIM << "  No moves played yet." << Color::RESET << endl;
        return;
    }
    cout << endl;
    cout << Color::MAGENTA << "  ┌──────────── MOVE HISTORY ────────────┐" << Color::RESET << endl;
    cout << Color::MAGENTA << "  │  #   You          CPU       Result   │" << Color::RESET << endl;
    cout << Color::MAGENTA << "  │  ─── ──────────  ─────────  ──────── │" << Color::RESET << endl;

    int start = max(0, (int)s.history.size() - 15); // show last 15
    for (int i = start; i < (int)s.history.size(); i++) {
        string pName = moveToString(s.history[i].first);
        string cName = moveToString(s.history[i].second);
        string reason;
        int result = determineWinner(s.history[i].first, s.history[i].second, reason);
        string rStr;
        string rColor;
        if (result == 1)       { rStr = "WIN";  rColor = Color::GREEN;  }
        else if (result == -1) { rStr = "LOSS"; rColor = Color::RED;    }
        else                   { rStr = "TIE";  rColor = Color::YELLOW; }

        while (pName.size() < 10) pName += " ";
        while (cName.size() < 10) cName += " ";

        cout << Color::MAGENTA << "  │  " << Color::RESET
             << Color::DIM << setw(3) << (i + 1) << " " << Color::RESET
             << Color::GREEN << pName << Color::RESET << " "
             << Color::RED << cName << Color::RESET << " "
             << rColor << rStr << Color::RESET;
        // Pad to fill the box
        int padLen = 8 - (int)rStr.size();
        cout << string(max(0, padLen), ' ');
        cout << Color::MAGENTA << "│" << Color::RESET << endl;
    }
    cout << Color::MAGENTA << "  └────────────────────────────────────────┘" << Color::RESET << endl;
}

void printGameSummary(const GameState &s) {
    cout << endl;
    cout << Color::BOLD << Color::CYAN
         << "  ╔════════════════════════════════════════╗\n"
         << "  ║          FINAL GAME SUMMARY            ║\n"
         << "  ╠════════════════════════════════════════╣"
         << Color::RESET << endl;

    double winPct = (s.totalRounds > 0) ? (100.0 * s.playerWins / s.totalRounds) : 0;
    double losePct = (s.totalRounds > 0) ? (100.0 * s.computerWins / s.totalRounds) : 0;
    double tiePct = (s.totalRounds > 0) ? (100.0 * s.ties / s.totalRounds) : 0;

    cout << Color::CYAN << "  ║" << Color::RESET
         << "  Total Rounds:   " << Color::WHITE << setw(5) << s.totalRounds << Color::RESET
         << string(17, ' ') << Color::CYAN << "║" << Color::RESET << endl;

    cout << Color::CYAN << "  ║" << Color::RESET
         << "  Your Wins:      " << Color::GREEN << setw(5) << s.playerWins
         << "  (" << fixed << setprecision(1) << winPct << "%)" << Color::RESET
         << string(max(0, 11 - (int)to_string((int)winPct).size()), ' ')
         << Color::CYAN << "║" << Color::RESET << endl;

    cout << Color::CYAN << "  ║" << Color::RESET
         << "  CPU Wins:       " << Color::RED << setw(5) << s.computerWins
         << "  (" << fixed << setprecision(1) << losePct << "%)" << Color::RESET
         << string(max(0, 11 - (int)to_string((int)losePct).size()), ' ')
         << Color::CYAN << "║" << Color::RESET << endl;

    cout << Color::CYAN << "  ║" << Color::RESET
         << "  Ties:           " << Color::YELLOW << setw(5) << s.ties
         << "  (" << fixed << setprecision(1) << tiePct << "%)" << Color::RESET
         << string(max(0, 11 - (int)to_string((int)tiePct).size()), ' ')
         << Color::CYAN << "║" << Color::RESET << endl;

    cout << Color::CYAN << "  ║" << Color::RESET
         << "  Best Win Streak:" << Color::MAGENTA << setw(5) << s.bestStreak << Color::RESET
         << string(17, ' ') << Color::CYAN << "║" << Color::RESET << endl;

    string diffStr = (s.difficultyLevel == 1) ? "Easy" : (s.difficultyLevel == 2) ? "Medium" : "Hard";
    cout << Color::CYAN << "  ║" << Color::RESET
         << "  Difficulty:     " << Color::WHITE << setw(8) << diffStr << Color::RESET
         << string(14, ' ') << Color::CYAN << "║" << Color::RESET << endl;

    string modeStr = s.extendedMode ? "RPSLS" : "Classic";
    cout << Color::CYAN << "  ║" << Color::RESET
         << "  Mode:           " << Color::WHITE << setw(8) << modeStr << Color::RESET
         << string(14, ' ') << Color::CYAN << "║" << Color::RESET << endl;

    cout << Color::BOLD << Color::CYAN
         << "  ╚════════════════════════════════════════╝"
         << Color::RESET << endl;

    // Verdict
    cout << endl << "  ";
    if (s.playerWins > s.computerWins) {
        cout << Color::GREEN << Color::BOLD
             << "★  CONGRATULATIONS! You are the CHAMPION!  ★" << Color::RESET;
    } else if (s.computerWins > s.playerWins) {
        cout << Color::RED << Color::BOLD
             << "✗  The computer wins this time. Better luck next time!  ✗" << Color::RESET;
    } else {
        cout << Color::YELLOW << Color::BOLD
             << "⊜  It's an even match! Well played!  ⊜" << Color::RESET;
    }
    cout << endl << endl;
}

// ──────────────────────────────────────────
// Menu & Input
// ──────────────────────────────────────────

int getMenuChoice(const string &prompt, int minVal, int maxVal) {
    int choice;
    while (true) {
        cout << prompt;
        if (cin >> choice && choice >= minVal && choice <= maxVal) {
            return choice;
        }
        cin.clear();
        cin.ignore(10000, '\n');
        cout << Color::RED << "  Invalid input. Please enter a number between "
             << minVal << " and " << maxVal << "." << Color::RESET << endl;
    }
}

Move getPlayerMove(bool extended) {
    while (true) {
        cout << endl;
        cout << Color::WHITE << "  Choose your move:" << Color::RESET << endl;
        cout << "    " << Color::BOLD << "[R]" << Color::RESET << "ock   "
             << Color::BOLD << "[P]" << Color::RESET << "aper   "
             << Color::BOLD << "[S]" << Color::RESET << "cissors";
        if (extended) {
            cout << "   " << Color::BOLD << "[L]" << Color::RESET << "izard   "
                 << "Spoc" << Color::BOLD << "[K]" << Color::RESET;
        }
        cout << endl;
        cout << "  " << Color::DIM << "(or 'h' for history, 'q' to quit)" << Color::RESET << endl;
        cout << "  > ";

        char c;
        cin >> c;
        c = tolower(c);

        if (c == 'q') return Move::INVALID; // signal quit
        if (c == 'h') {
            // return a sentinel — handled in main loop
            return static_cast<Move>(99);
        }

        Move m = charToMove(c);
        if (m == Move::INVALID) {
            cout << Color::RED << "  Invalid move! Try again." << Color::RESET << endl;
            continue;
        }
        if (!extended && (m == Move::LIZARD || m == Move::SPOCK)) {
            cout << Color::RED << "  That move is only available in RPSLS mode!" << Color::RESET << endl;
            continue;
        }
        return m;
    }
}


int main() {
    srand(static_cast<unsigned>(time(0)));

    GameState state;

    clearScreen();
    cout << Color::CYAN << Art::TITLE << Color::RESET << endl;

    // pick classic or RPSLS mode
    cout << Color::WHITE << "  Select Game Mode:" << Color::RESET << endl;
    cout << "    1. " << Color::GREEN  << "Classic" << Color::RESET << "  (Rock / Paper / Scissors)" << endl;
    cout << "    2. " << Color::MAGENTA << "Extended" << Color::RESET << " (Rock / Paper / Scissors / Lizard / Spock)" << endl;
    int modeChoice = getMenuChoice("  > ", 1, 2);
    state.extendedMode = (modeChoice == 2);
    cout << endl;

    // difficulty selection
    cout << Color::WHITE << "  Select Difficulty:" << Color::RESET << endl;
    cout << "    1. " << Color::GREEN  << "Easy"   << Color::RESET << "   — Random moves" << endl;
    cout << "    2. " << Color::YELLOW << "Medium" << Color::RESET << " — Adapts to your patterns" << endl;
    cout << "    3. " << Color::RED    << "Hard"   << Color::RESET << " — Pattern recognition AI" << endl;
    state.difficultyLevel = getMenuChoice("  > ", 1, 3);
    cout << endl;

    // how many rounds to play
    cout << Color::WHITE << "  Select Match Type:" << Color::RESET << endl;
    cout << "    1. " << Color::CYAN << "Unlimited" << Color::RESET << " — Play as long as you want" << endl;
    cout << "    2. " << Color::CYAN << "Best of 3" << Color::RESET << endl;
    cout << "    3. " << Color::CYAN << "Best of 5" << Color::RESET << endl;
    cout << "    4. " << Color::CYAN << "Best of 7" << Color::RESET << endl;
    int matchChoice = getMenuChoice("  > ", 1, 4);
    switch (matchChoice) {
        case 2: state.bestOfN = 3; break;
        case 3: state.bestOfN = 5; break;
        case 4: state.bestOfN = 7; break;
        default: state.bestOfN = 0;
    }
    cout << endl;

    // if RPSLS, show a quick rules reference
    if (state.extendedMode) {
        printDivider();
        cout << Color::MAGENTA << Color::BOLD << "  RPSLS Rules:" << Color::RESET << endl;
        cout << Color::DIM;
        cout << "    Scissors cuts Paper     |  Paper covers Rock" << endl;
        cout << "    Rock crushes Lizard     |  Lizard poisons Spock" << endl;
        cout << "    Spock smashes Scissors  |  Scissors decapitates Lizard" << endl;
        cout << "    Lizard eats Paper       |  Paper disproves Spock" << endl;
        cout << "    Spock vaporizes Rock    |  Rock crushes Scissors" << endl;
        cout << Color::RESET;
        printDivider();
        cout << endl;
    }

    // main game loop
    int winsNeeded = (state.bestOfN > 0) ? (state.bestOfN / 2 + 1) : 0;
    bool matchOver = false;

    while (!matchOver) {
        state.totalRounds++;

        // Show round header
        cout << Color::WHITE << Color::BOLD;
        if (state.bestOfN > 0) {
            cout << "  ══════ ROUND " << state.totalRounds
                 << " (Best of " << state.bestOfN << ") ══════";
        } else {
            cout << "  ══════ ROUND " << state.totalRounds << " ══════";
        }
        cout << Color::RESET << endl;

        printScoreboard(state);

        // Get player move
        Move playerMove;
        while (true) {
            playerMove = getPlayerMove(state.extendedMode);
            if (static_cast<int>(playerMove) == 99) {
                // Show history
                printHistory(state);
                continue;
            }
            break;
        }

        if (playerMove == Move::INVALID) {
            // Quit
            state.totalRounds--; // don't count aborted round
            break;
        }

        // Countdown
        printCountdown();

        // Computer chooses
        Move computerMove = computerChoose(state);

        // Display choices side by side
        printMoveComparison(playerMove, computerMove);

        // Determine winner
        string reason;
        int result = determineWinner(playerMove, computerMove, reason);

        // Display result
        if (result == 1) {
            cout << "  " << Color::GREEN << Color::BOLD << "★ YOU WIN! " << Color::RESET
                 << Color::GREEN << reason << Color::RESET << endl;
            state.playerWins++;
            state.currentStreak++;
            if (state.currentStreak > state.bestStreak) state.bestStreak = state.currentStreak;
        } else if (result == -1) {
            cout << "  " << Color::RED << Color::BOLD << "✗ YOU LOSE! " << Color::RESET
                 << Color::RED << reason << Color::RESET << endl;
            state.computerWins++;
            state.currentStreak = 0;
        } else {
            cout << "  " << Color::YELLOW << Color::BOLD << "⊜ TIE! " << Color::RESET
                 << Color::YELLOW << reason << Color::RESET << endl;
            state.ties++;
            // Streak continues on tie
        }

        // Record history
        state.history.push_back({playerMove, computerMove});

        cout << endl;

        // Check if best-of-N match is over
        if (state.bestOfN > 0) {
            if (state.playerWins >= winsNeeded) {
                cout << Color::GREEN << Color::BOLD
                     << "  ★★★  YOU WIN THE MATCH!  ★★★" << Color::RESET << endl;
                matchOver = true;
            } else if (state.computerWins >= winsNeeded) {
                cout << Color::RED << Color::BOLD
                     << "  ✗✗✗  COMPUTER WINS THE MATCH!  ✗✗✗" << Color::RESET << endl;
                matchOver = true;
            }
        }

        if (!matchOver && state.bestOfN == 0) {
            // Ask to continue
            cout << Color::DIM << "  Press Enter to continue, or 'q' to quit... " << Color::RESET;
            cin.ignore(10000, '\n');
            string line;
            getline(cin, line);
            if (!line.empty() && tolower(line[0]) == 'q') {
                break;
            }
        }

        printDivider();
    }

    // ── Final Summary ──
    printHistory(state);
    printGameSummary(state);

    cout << Color::DIM << "  Thanks for playing! Goodbye." << Color::RESET << endl << endl;

    return 0;
}
