#include <bits/stdc++.h>
using namespace std;

/*
Ranks: Initiate (0), Seeker (1), Acolyte (2), Sentinel (3), Mystic (4), Ritualist (5), Emissary (6), Oracle (7), Phantom (8), Ascendant (9), Eternus (10)
Initiate 1 = 0 points, Eternus 1 = 10 * (1000 * 5 + 2000) = 10 * 7000 = 70000 
(I capped it at 77k, the ceiling of a theoretical (non % based) E6)
*/

// Do not change
int RANK_SHIELD = 5;
int SUBRANK_SHIELD = 2;
int RANK_REQUIREMENT = 2000;
int SUBRANK_REQUIREMENT = 1000;
int GAIN = 300;
int WINSTREAK_BONUSES[] = {70, 90, 110, 130};
int POINTS_MAX = 77000;
vector<string> RANK_NAMES = {"Initiate", "Seeker", "Acolyte", "Sentinel", "Mystic", "Ritualist", "Emissary", "Oracle", "Phantom", "Ascendant", "Eternus"};

// Defaults
float DEFAULT_WIN_PERCENTAGE = 50;
float DEFAULT_WIN_DEPRECIATION = 0.40; // higher rank = harder games, set as you wish
int DEFAULT_NUMBER_OF_GAMES = 50;
int DEFAULT_NUMBER_OF_SIMULATIONS = 25000;
string CSV_FILENAME = "dl_SimulationsOverview.csv";

// Debugging
bool DEBUG = false; // per-game summaries, mostly deprecated
bool HIGHLEVEL_DEBUG = false; // per-simulation summaries
bool ENABLE_TERMINAL_OUTPUT = true; // shows statistical values in terminal
bool DELETE_PREVIOUS_CSV = true; // deletes the previous CSV with the same name (edit CSV_FILENAME, if you want it differently named)
int ERRORS = 0;

struct Player {
    int points;
    int rank;
    int subrank;
    int subpoints;
    int shields = 0;
    int winstreak = 0;
};

float average(vector<vector<int>> &v, int ix) {
    float r = 0;
    for(int i = 0; i < v.size(); i++) {
        r += v[i][ix];
    }
    r /= (float)v.size(); // divide by amt of stats, same as NUMBER_OF_SIMULATIONS
    r = round(r * 100) / 100;
    return r;
}

float standard_deviation(vector<vector<int>> &v, float avg, int ix) {
    float r = 0;
    for(int i = 0; i < v.size(); i++) {
        float diff = (v[i][ix] - avg);
        r += pow(diff, 2);
    }
    r /= (float)v.size();
    r = sqrt(r);
    r = round(r * 100) / 100;
    return r;
}

int stringToRank(string s) {
    for(int i = 0; i < RANK_NAMES.size(); i++) {
        if(s == RANK_NAMES[i]) return i;
    }
    return -1;
}

string rankToString(int points, bool showSubpoints) {
    int rank = points / 7000;
    int subrank = (points % 7000) / 1000;
    if (subrank < 6) { subrank++; }
    string s = RANK_NAMES[rank] + " " + to_string(subrank);
    if(showSubpoints) {
        if(subrank < 6) { s = s + " (" + to_string(points % 1000) + "/1000)"; }
        else { s = s + " (" + to_string(points % 7000 - 5000) + "/2000)"; }
    }
    return s;
}

void calculate_rank(Player &p, bool printResult) {
    p.rank = p.points / 7000;
    p.subrank = (p.points % 7000) / 1000;
    p.subpoints = p.points % 7000;
    if (p.subrank < 6) { p.subrank++; p.subpoints %= 1000; } else { p.subpoints -= 5000; }
    if(printResult)
    cout << RANK_NAMES[p.rank] << " " << p.subrank << "\nBased on the given information:\nTotal points: " << p.points << "\nRank enumerated: " << p.rank << " (" << p.points - p.points % 7000 << ")\nSubrank enumerated: " << p.subrank << " (" << p.points % 7000 << ")\n\n";
}

vector<int> simulate(int rank, int subrank, int subrankPoints, float receivedWR, float winDepreciation, int gamesNumber) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> pts(1, 7000);
    uniform_int_distribution<int> game(1, 10000);

    Player t;
    float currentWinrate = receivedWR;
    if(rank == -1) {
        t.points = (pts(gen) - 1) * 10;
        if(receivedWR == -1) currentWinrate = DEFAULT_WIN_PERCENTAGE;
    } else {
        t.points = 7000 * rank + 1000 * (subrank-1) + subrankPoints;
    }
    calculate_rank(t, DEBUG);
    t.shields = (t.subrank == 1) ? RANK_SHIELD : SUBRANK_SHIELD;

    // Store for later
    int startingPoints = t.points, startingRank = t.rank, startingSubrank = t.subrank, startingSubpoints = t.subpoints;

    // Run stats
    int wins = 0, netRankups = 0, winstreakBonuses = 0, totalLossPoints = 0, totalWinPoints = 0, shieldsBroken = 0, shieldMitigatedLosses = 0, gamesPlayed = gamesNumber;

    for (int i = 1; i <= gamesNumber; i++) {
        if(float(game(gen))/100 <= currentWinrate) {
            bool endGamePreemptively = false;

            // Winstreak, saving run data
            wins++; t.winstreak++; 
            int winstreakBonus = 0;
            if (t.winstreak < 3) { // not eligle for winstreak
            } else if(t.winstreak >= 3 && t.winstreak <= 6) { // winstreak between 3-6
                winstreakBonus = WINSTREAK_BONUSES[t.winstreak - 3];
            } else { // winstreak over 6
                winstreakBonus = WINSTREAK_BONUSES[3];
            }

            // Calculate points gain
            int pointsGained = GAIN + winstreakBonus;
            if(t.points + pointsGained >= POINTS_MAX) { // reached E6 floor threshold
                pointsGained = POINTS_MAX - t.points; // get difference, did winstreak play a role?
                if(pointsGained >= GAIN) winstreakBonuses += pointsGained - GAIN;
                t.subpoints = 0; t.subrank++; 
                netRankups++; 
                gamesPlayed = i;
                endGamePreemptively = true;
            } 

            // Update stats
            totalWinPoints += pointsGained;

            if(endGamePreemptively) break;
            winstreakBonuses += winstreakBonus; t.subpoints += pointsGained; t.points += pointsGained;

            // Check rankup
            if(t.subrank < 6 && t.subpoints >= 1000 || t.subpoints >= 2000) {
                netRankups++; 
                currentWinrate -= winDepreciation;
                if(t.subrank == 6) {
                    t.rank++; t.subrank = 1; t.shields = RANK_SHIELD;
                } else {
                    t.subrank++; t.shields = SUBRANK_SHIELD;
                }
                t.subpoints %= 1000;
            }

        
        //cout << "Match result #" << i << " - WIN: " << rankToString(t.points, true) << " (" << t.points << ")\n";

        } else { // Loss

            // Save data
            t.winstreak = 0;
            int actualLoss = GAIN;
            if(t.subpoints - GAIN < 0) { // about to derank
                if(t.rank == 0 && t.subrank == 1) { // initiate 1
                    actualLoss = t.subpoints;
                    t.subpoints = 0;
                } else if(t.shields > 0) { // use shield instead
                    int mitigated = GAIN - t.subpoints;
                    actualLoss = t.subpoints; 
                    t.shields--; shieldsBroken++;
                    shieldMitigatedLosses += mitigated;
                    t.subpoints = 0;
                } else { // derank
                    int subpointLoss = GAIN - t.subpoints;
                    if(t.subrank == 1) {
                        t.rank--;
                        t.subrank = 6;
                        t.subpoints = 2000;
                    } else {
                        t.subrank--;
                        t.subpoints = 1000;
                    }
                    t.shields = (t.subrank == 1) ? RANK_SHIELD : SUBRANK_SHIELD;
                    netRankups--; currentWinrate += winDepreciation;
                    t.subpoints -= subpointLoss;
                }
            } else {
                t.subpoints -= actualLoss;
            }
            t.points -= actualLoss;
            totalLossPoints -= actualLoss;

        //cout << "Match result #" << i << " - LOSS: " << rankToString(t.points, true) << " (" << t.points << ")\n";
        }
    }

    if(HIGHLEVEL_DEBUG || DEBUG) { cout << "\n<---- SUMMARY OF " << gamesPlayed << " GAMES ---->\nRank Change: " << rankToString(startingPoints, true) << " -> " << rankToString(t.points, true) << ((netRankups > 0) ? " (Net: +" : " (Net: ") << netRankups << ")\nPoints Change: " << startingPoints << " -> " << t.points << " (Net: " << t.points - startingPoints << ")\nWinrate: " << wins << "/" << gamesPlayed << " (" << (float)wins/(float)gamesPlayed << "%)\nWinstreak Bonuses: " << ((winstreakBonuses > 0) ? "+" : "") << winstreakBonuses << "\nErrors (debug): " << ERRORS << "\n\n"; }
    vector<int> result = {startingPoints, startingRank, startingSubrank, startingSubpoints, t.points, t.rank, t.subrank, t.subpoints, wins, totalWinPoints, totalLossPoints, netRankups, winstreakBonuses, shieldsBroken, shieldMitigatedLosses, gamesPlayed};
    return result;
}

int main(int argc, char *argv[]) { // (programName, rank, subrank, subrankPoints, winrate, gamesNumber, deleteCSV, winDepreciation, simulationsNumber, terminalOutput) = len of 10
    float winrate = -1;
    int gamesNumber = DEFAULT_NUMBER_OF_GAMES;
    int simulationsNumber = DEFAULT_NUMBER_OF_SIMULATIONS;
    bool deletePreviousCSV = DELETE_PREVIOUS_CSV;
    bool terminalOutput = ENABLE_TERMINAL_OUTPUT;
    string _rank = "Random";
    int rank = -1; int subrank = -1; int subrankPoints = -1; int debugRankLimit = 0;
    float winDepreciation = DEFAULT_WIN_DEPRECIATION;

    // Note to self: Don't do dynamic command line args like this
    if(argc == 1) {
        cout << "[INFO] No arguments passed, running random rank simulations.\n";
    } else if (argc == 4) { winrate = atof(argv[1]); deletePreviousCSV = ((atoi(argv[2]) == 0) ? false : true); terminalOutput = ((atoi(argv[3]) == 0) ? false : true); // care about only the winrate
    } else if (argc >= 6) {
        _rank = argv[1]; subrank = atoi(argv[2]);
        rank = stringToRank(_rank);
        if(subrank > 6 || subrank <= 0) { cout << "\033[1;31m[ERROR] Subrank number is invalid according to the template.\033[0m\n"; return 0; }
        if(argc >= 4) subrankPoints = atoi(argv[3]);
        if(argc >= 5) winrate = atof(argv[4]);
        if(argc >= 6) gamesNumber = atoi(argv[5]);
        if(argc >= 7 && atoi(argv[6]) == 0) deletePreviousCSV = false;
        if(argc >= 8) winDepreciation = atof(argv[7]);
        if(argc >= 9) simulationsNumber = atoi(argv[8]);
        if(argc >= 10 && atoi(argv[9]) == 0) terminalOutput = false; // Purely so multiple runs don't mess spam our terminal
        if(subrank == -1) {} else if (subrank < 6) { debugRankLimit = 1000; } else { debugRankLimit = 2000; }
        if((subrank < 6 && subrankPoints >= 1000) || (subrank == 6 && subrankPoints >= 2000) || subrankPoints < 0) { cout << "\033[1;31m[ERROR] Points value is invalid according to the ranked template.\033[0m\n"; return 0; }

    } else {
        cout << "\033[1;31m[ERROR] Argument number is invalid according to the template.\033[0m\n";
        return 0;
    }

    vector<vector<int>> results;

    // CSV output
    fstream fout; // file pointer
    if(deletePreviousCSV) {
        //cout << "[INFO] Requested previous CSV file deletion.\n";
        int status = remove(CSV_FILENAME.c_str());
        //if(status != 0) { cout << "[INFO] Previous CSV file does not exist.\n"; } else { cout << "[INFO] Successfully deleted previous CSV file.\n"; }
    } 
    fout.open(CSV_FILENAME, ios::out | ios::app);
    if(deletePreviousCSV) { fout << "winrate,startingPoints,startingRank,startingSubrank,startingSubpoints,endingPoints,endingRank,endingSubrank,endingSubpoints,wins,totalWinPoints,totalLossPoints,netRankups,winstreakBonuses,shieldsBroken,shieldMitigatedLosses,successfulGamesPlayed,wdr,totalPointsGained\n"; }
    if(terminalOutput) cout << "[INFO] Running simulations with arguments: [" << _rank << " " << subrank << ": " << subrankPoints << "/" << debugRankLimit << " points, WR = " << ((winrate == -1) ? DEFAULT_WIN_PERCENTAGE : winrate) << "%, WD = " << winDepreciation << ", Games = " << gamesNumber << ", Simulations: " << simulationsNumber << ", TerminalOutput = " << ((terminalOutput) ? "On" : "Off, ") << "DeletePreviousCSV = " << ((deletePreviousCSV) ? "On" : "Off") << "]\n";

    for(int i = 0; i < simulationsNumber; i++) {
        vector<int> r = simulate(rank, subrank, subrankPoints, winrate, winDepreciation, gamesNumber);
        results.push_back(r);
        results[i].push_back(winDepreciation * 100); // because results vector is int, simple workaround (even if not ideal)
        results[i].push_back(r[4] - r[0]);
        fout << winrate;
        for(int j = 0; j < results[i].size(); j++) {
            fout << "," << results[i][j];
        } fout << "\n";
    }
    
    if (ERRORS > 0) { cout << "\033[1;31m[ERROR] Multiple (" << ERRORS << ") errors have been detected during the runtime, please try again or if this persists, contact me.\033[0m\n\n"; return 0; }

    if (terminalOutput) {
        // Averages
        vector<float> avgs;
        for(int i = 0; i < results[0].size(); i++) {
            avgs.push_back(average(results, i)); // returns the average of the stat
        }
        // Standard deviations (the smaller the simulations × games the less accurate the results are)
        vector<float> stdDevs;
        for(int i = 0; i < results[0].size(); i++) {
            stdDevs.push_back(standard_deviation(results, avgs[i], i)); // standard deviations from mean
        }

        // Note to self: trying to list values from a table that I keep adding info to isn't the best idea
        cout << "\n<-------  Statistical overview of the experiment  ------->\n";
        cout << "Winrate: " << ((winrate == -1) ? DEFAULT_WIN_PERCENTAGE : winrate) << "%";
        if(winDepreciation != 0) { cout << " with a win depreciation of " << winDepreciation << "%\n"; } else { cout << "\n"; }
        cout << "Simulations: " << simulationsNumber << "\n";
        cout << "Games per Simulation: " << avgs[15] << "\n";
        if(rank != -1) cout << "Predicted Rank Progress: \033[4;37m" << rankToString(avgs[0], true) << " -> " << rankToString(avgs[4], true) << "\033[0m\n";
        cout << "Average Rankups: " << avgs[11] << " ± " << stdDevs[11] << "\n";
        cout << "Average Wins: " << avgs[8] << "/" << avgs[15] << "\n";
        cout << "Average Points Change: " << avgs[17] << " ± " << stdDevs[17] << "\n";
        cout << "Average Points Gained: " << avgs[9] << " ± " << stdDevs[9] << " (of those " << avgs[12] << " ± " << stdDevs[12] << " from winstreak bonuses)" << "\n";
        cout << "Average Points Lost: " << avgs[10] << " ± " << stdDevs[10] << "\n";
        cout << "Average Shield Usage: " << avgs[13] << " ± " << stdDevs[13] << " (of those have mitigated losses of " << avgs[14] << " ± " << stdDevs[14] << " points)\n";
        cout << "<-------------------------------------------------------->\n";
        return 0;
    }
}