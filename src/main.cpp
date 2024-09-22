    #include <iostream>
    #include <fstream>
    #include <string>
    #include <unordered_map>
    #include <vector>
    #include <tuple>
    #include <algorithm>
    #include <regex>
    #include <array>
    #include "nlohmann/json.hpp"

    using json = nlohmann::json;
    using namespace std;

    constexpr int NUM_FIGHTS = 14;
    constexpr int STATE_ID_INDEX = NUM_FIGHTS;
    constexpr int TIME_REMAIN_INDEX = NUM_FIGHTS + 1;
    constexpr int STATE_VECTOR_SIZE = NUM_FIGHTS + 2;
    constexpr int TIME_SHRINK = 5;

    struct PlayerState {
        array<short, NUM_FIGHTS> fightTimes;
        char fightStateId;
        short timeRemaining;

        bool operator==(const PlayerState& other) const {
            return fightTimes == other.fightTimes &&
                fightStateId == other.fightStateId &&
                timeRemaining == other.timeRemaining;
        }
    };

    struct FightTime {
        char id;
        string name;
        short resetToNextFight;
        short resetToResetWithAttempt;
        short resetToResetWithWin;
        short fightStartToResetWithAttempt;
        short fightStartToResetWithWin;
        short fightStartToNextFight;
        short loss;
    };

    struct FightOption {
        char fightId;
        char winStateId = 255;
        char loseStateId = 255;
    };

    struct FightState {
        char id;
        string name;
        vector<FightOption> fightOptions;
    };

    struct Outcome {
        double probability;
        short fightTime;
    };

    struct FightStrategy {
        string name;
        char fightId;
        vector<Outcome> outcomes;
    };

    struct OptionFromFightState {
        const FightStrategy* fightStrategy;
        bool canWin;
        bool canLose;
    };

    struct Configuration {
        vector<FightState> fightStates;
        vector<FightTime> fightTimes;
        vector<FightStrategy> fightStrategies;
        PlayerState initialState;

        unordered_map<char, const FightTime*> fightTimeMap;
        unordered_map<string, const FightOption*> nextStateIdMap;
        unordered_map<char, vector<OptionFromFightState>> optionFromFightStateMap;
        vector<vector<short>> fightTimesPossible;
        vector<int> bases;
    };

    

    void from_json(const nlohmann::json& j, PlayerState& s) {
        j.at("fightStateId").get_to(s.fightStateId);
        j.at("fightTimes").get_to(s.fightTimes);
        s.timeRemaining = j.at("timeRemaining").get<int>() / TIME_SHRINK;
    }

    void from_json(const nlohmann::json& j, FightTime& f) {
        j.at("id").get_to(f.id);
        j.at("name").get_to(f.name);
        f.resetToNextFight = j.value("resetToNextFight", 32000) / TIME_SHRINK;
        f.resetToResetWithAttempt = j.value("resetToResetWithAttempt", 32000) / TIME_SHRINK;
        f.resetToResetWithWin = j.value("resetToResetWithWin", 32000) / TIME_SHRINK;
        f.fightStartToResetWithAttempt = j.value("fightStartToResetWithAttempt", 32000) / TIME_SHRINK;
        f.fightStartToResetWithWin = j.value("fightStartToResetWithWin", 32000) / TIME_SHRINK;
        f.fightStartToNextFight = j.value("fightStartToNextFight", 32000) / TIME_SHRINK;
        f.loss = j.value("loss", 32000) / TIME_SHRINK;
    }

    void from_json(const nlohmann::json& j, FightOption& f) {
        j.at("fightId").get_to(f.fightId);
        f.winStateId = j.value("winStateId", 255);
        f.loseStateId = j.value("loseStateId", 255);
    }

    void from_json(const nlohmann::json& j, FightState& f) {
        j.at("id").get_to(f.id);
        j.at("name").get_to(f.name);
        j.at("fightOptions").get_to(f.fightOptions);
    }

    void from_json(const nlohmann::json& j, Outcome& o) {
        j.at("probability").get_to(o.probability);
        j.at("fightTime").get_to(o.fightTime);
    }

    void from_json(const nlohmann::json& j, FightStrategy& f) {
        j.at("name").get_to(f.name);
        j.at("fightId").get_to(f.fightId);
        j.at("outcomes").get_to(f.outcomes);
    }

    void from_json(const nlohmann::json& j, Configuration& c) {
        j.at("fightStates").get_to(c.fightStates);
        j.at("fightTimes").get_to(c.fightTimes);
        j.at("fightStrategies").get_to(c.fightStrategies);
        j.at("initialState").get_to(c.initialState);
    }

    /////////////////

    // Load configuration from a JSON file
    json load_config(const string& file_path) {
        ifstream file(file_path);
        json config;
        file >> config;
        return config;
    }

    vector<short> getNextStateVector(const Configuration& config, const vector<short>& stateVector, char fightId, int fightTimeIndex, const pair<char, char>& resetStrategy) {
        vector<short> nextStateVector = stateVector;
        const FightTime* timeTaken = config.fightTimeMap.find(fightId)->second;
        const FightOption* fightOption = config.nextStateIdMap.find(string() + char(stateVector[STATE_ID_INDEX]) + fightId)->second;
        if (fightTimeIndex < stateVector[fightId]) {
            nextStateVector[fightId] = fightTimeIndex;
            if (resetStrategy.first == 'p') { // proceed
                nextStateVector[STATE_ID_INDEX] = fightOption->winStateId;
                if (stateVector[STATE_ID_INDEX] == 0) {
                    nextStateVector[TIME_REMAIN_INDEX] -= timeTaken->resetToNextFight;
                } else {
                    nextStateVector[TIME_REMAIN_INDEX] -= timeTaken->fightStartToNextFight;
                }
            } else { // reset
                nextStateVector[STATE_ID_INDEX] = 0;
                if (stateVector[STATE_ID_INDEX] == 0) {
                    nextStateVector[TIME_REMAIN_INDEX] -= timeTaken->resetToResetWithWin;
                } else {
                    nextStateVector[TIME_REMAIN_INDEX] -= timeTaken->fightStartToResetWithWin;
                }
                
            }
            return nextStateVector;
        }
        // Not a PB
        if (resetStrategy.second == 'p') { // proceed
            nextStateVector[STATE_ID_INDEX] = fightOption->winStateId;
            if (stateVector[STATE_ID_INDEX] == 0) {
                nextStateVector[TIME_REMAIN_INDEX] -= timeTaken->resetToNextFight;
            } else {
                nextStateVector[TIME_REMAIN_INDEX] -= timeTaken->fightStartToNextFight;
            }
        } else if (resetStrategy.second == 'r') { // reset
            nextStateVector[STATE_ID_INDEX] = 0;
            if (stateVector[STATE_ID_INDEX] == 0) {
                nextStateVector[TIME_REMAIN_INDEX] -= timeTaken->resetToResetWithAttempt;
            } else {
                nextStateVector[TIME_REMAIN_INDEX] -= timeTaken->fightStartToResetWithAttempt;
            }
        } else { // lose
            nextStateVector[STATE_ID_INDEX] = fightOption->loseStateId;
            nextStateVector[TIME_REMAIN_INDEX] -= timeTaken->loss;
        }

        return nextStateVector;
    }

    vector<pair<char, char>> getAvailableResetStrategies(bool canWin, bool canLose) {
        vector<pair<char, char>> result;
        result.push_back({'r', 'r'});
        if (canWin) {
            result.push_back({'p', 'p'});
            result.push_back({'p', 'r'});
        }
        if (canLose) {
            result.push_back({'r', 'l'});
        }
        if (canWin && canLose) {
            result.push_back({'p', 'l'});
        }
        return result;
    }

    vector<short> getVectorFromPlayerState(const Configuration& config, const PlayerState& playerState) {
        vector<short> result (STATE_VECTOR_SIZE, 0);
        result[STATE_ID_INDEX] = playerState.fightStateId;
        result[TIME_REMAIN_INDEX] = playerState.timeRemaining;
        for (int i = 0; i < NUM_FIGHTS; i++) {
            bool found = false;
            for (int j = 0; j < config.fightTimesPossible[i].size(); j++) {
                if (playerState.fightTimes[i] == config.fightTimesPossible[i][j]) {
                    result[i] = j;
                    found = true;
                    break;
                }
            }
            if (!found) {
                cout << "ERROR: NOT FOUND TIME: " << playerState.fightTimes[i] << endl ;
            }
        }
        return result;
    }

    double getTotalTime(const Configuration& config, const vector<short>& stateVector) {
        int totalTime = 0;
        for (int i = 0; i < NUM_FIGHTS; i++) {
            totalTime += config.fightTimesPossible[i][stateVector[i]];
        }
        return totalTime;
    }

    int getIndex(const Configuration& config, const vector<short>& stateVector) {
        int index = 0;
        for (int i = 0; i < STATE_VECTOR_SIZE; i++) {
            index += config.bases[i] * stateVector[i];
        }
        return index;
    }

    int getFightTimeIndex(const Configuration& config, char fightId, short fightTime) {
        for (int i = 0; i < config.fightTimesPossible[fightId].size(); i++) {
            if (fightTime == config.fightTimesPossible[fightId][i]) {
                return i;
            }
        }
        // We don't have this time, meaning it is higher than current value
        // So return high num (above any valid index)
        return 100;
    }

    void calculateExpectedTime(const Configuration& config, vector<double>& dpArray, const vector<short>& currentVector) {
        double minExpectedTime = numeric_limits<double>::infinity();
        vector<OptionFromFightState> options = config.optionFromFightStateMap.find(currentVector[STATE_ID_INDEX])->second;
        for (const OptionFromFightState& option : options) {
            vector<pair<char, char>> availableResetStrategies = getAvailableResetStrategies(option.canWin, option.canLose);
            for (const pair<char, char>& resetStrategy: availableResetStrategies) {
                double total = 0;
                const FightStrategy* fightStrategy = option.fightStrategy;
                for (int i = 0; i < fightStrategy->outcomes.size(); i++) {
                    int fightTimeIndex = getFightTimeIndex(config, fightStrategy->fightId, fightStrategy->outcomes[i].fightTime);
                    vector<short> nextStateVector = getNextStateVector(config, currentVector, fightStrategy->fightId, fightTimeIndex, resetStrategy);
                    double expectedTime, probability;
                    if (nextStateVector[TIME_REMAIN_INDEX] < 0) {
                        expectedTime = getTotalTime(config, nextStateVector);
                    } else {
                        expectedTime = dpArray[getIndex(config, nextStateVector)];
                    }
                    if (i == 0) {
                        probability = fightStrategy->outcomes[i].probability;
                    } else {
                        probability = fightStrategy->outcomes[i].probability - fightStrategy->outcomes[i-1].probability;
                    }
                    total += probability * expectedTime;
                }

                if (total < minExpectedTime) {
                    minExpectedTime = total;
                }
            }
        }
        dpArray[getIndex(config, currentVector)] = minExpectedTime;
    }

    vector<pair<string, double>> analyze(const Configuration& config, vector<double>& dpArray, const vector<short>& stateVector) {
        vector<pair<string, double>> results;
        vector<OptionFromFightState> options = config.optionFromFightStateMap.find(stateVector[STATE_ID_INDEX])->second;
        for (const OptionFromFightState& option : options) {
            vector<pair<char, char>> availableResetStrategies = getAvailableResetStrategies(option.canWin, option.canLose);
            for (const pair<char, char>& resetStrategy: availableResetStrategies) {
                double total = 0;
                const FightStrategy* fightStrategy = option.fightStrategy;
                for (int i = 0; i < fightStrategy->outcomes.size(); i++) {
                    int fightTimeIndex = getFightTimeIndex(config, fightStrategy->fightId, fightStrategy->outcomes[i].fightTime);
                    vector<short> nextStateVector = getNextStateVector(config, stateVector, fightStrategy->fightId, fightTimeIndex, resetStrategy);
                    double expectedTime, probability;
                    if (nextStateVector[TIME_REMAIN_INDEX] < 0) {
                        expectedTime = getTotalTime(config, nextStateVector);
                    } else {
                        expectedTime = dpArray[getIndex(config, nextStateVector)];
                    }
                    if (i == 0) {
                        probability = fightStrategy->outcomes[i].probability;
                    } else {
                        probability = fightStrategy->outcomes[i].probability - fightStrategy->outcomes[i-1].probability;
                    }
                    total += probability * expectedTime;
                }
                results.push_back({option.fightStrategy->name + " " + resetStrategy.first + resetStrategy.second, total});
            }
        }
        return results;
    }

    int main() {
        string config_path = "config.json";
        json jsonData = load_config(config_path);

        Configuration config;
        try {
            config = jsonData.get<Configuration>();
        } catch (const nlohmann::json::exception& e) {
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            return 1;
        }
        std::cout << "Parsed Configuration successfully!" << std::endl;


        // HELPFUL MAPS

        unordered_map<char, const FightTime*> fightTimeMap;
        for (const FightTime& fightTime: config.fightTimes) {
            fightTimeMap[fightTime.id] = &fightTime;
        }

        unordered_map<string, const FightOption*> nextStateIdMap;
        for (const FightState& fightState : config.fightStates) {
            for (const FightOption& fightOption : fightState.fightOptions) {
                nextStateIdMap[string() + fightState.id + fightOption.fightId] = &fightOption;
            }
        }

        unordered_map<char, vector<OptionFromFightState>> optionFromFightStateMap;
        for (const FightState& fightState : config.fightStates) {
            vector<OptionFromFightState> options;
            for (const FightOption& fightOption : fightState.fightOptions) {
                for (const FightStrategy& fightStrategy : config.fightStrategies) {
                    if (fightOption.fightId == fightStrategy.fightId) {
                        OptionFromFightState option;
                        option.fightStrategy = &fightStrategy;
                        option.canWin = fightOption.winStateId != char(255);
                        option.canLose = fightOption.loseStateId != char(255);
                        options.push_back(option);
                    }
                }
            }
            optionFromFightStateMap[fightState.id] = options;
        }


        //////

        vector<vector<short>> fightTimesPossible;
        fightTimesPossible.resize(NUM_FIGHTS);

        for (int i = 0; i < NUM_FIGHTS; i++) {
            fightTimesPossible[i].push_back(config.initialState.fightTimes[i]);
        }

        for (const FightStrategy& strategy : config.fightStrategies) {
            for (const Outcome& outcome : strategy.outcomes) {
                if (outcome.fightTime < fightTimesPossible[strategy.fightId][0]) { // only add times less than initial time
                    fightTimesPossible[strategy.fightId].push_back(outcome.fightTime);
                }
            }
        }

        for (vector<short>& times : fightTimesPossible) {
            sort(times.begin(), times.end());
            times.erase(unique(times.begin(), times.end()), times.end());
        }


        vector<short> maxValues (STATE_VECTOR_SIZE, 0);
        for (int i = 0; i < NUM_FIGHTS; i++) {
            maxValues[i] = fightTimesPossible[i].size();
        }
        maxValues[STATE_ID_INDEX] = config.fightStates.size();
        maxValues[TIME_REMAIN_INDEX] = config.initialState.timeRemaining + 1;

        vector<int> bases (maxValues.size(), 0);
        bases[0] = 1;
        for (int i = 1; i < maxValues.size(); i++) {
            bases[i] = bases[i - 1] * maxValues[i - 1];
        }

        int totalSize = bases[maxValues.size() - 1] * maxValues[maxValues.size() - 1];

        cout << "Total size: " << totalSize << endl;

        if (totalSize > 50000000) {
            cout << "Total size above 50000000, terminating" << endl;
            return -1;
        }

        

        vector<double> dpArray (totalSize, 0);

        config.fightTimeMap = fightTimeMap;
        config.nextStateIdMap = nextStateIdMap;
        config.optionFromFightStateMap = optionFromFightStateMap;
        config.fightTimesPossible = fightTimesPossible;
        config.bases = bases;


        // CALCULATE EXPECTED TIME FOR EACH COMBINATION

        vector<short> current(maxValues.size(), 0);
        bool done = false;
        while (!done) {
            calculateExpectedTime(config, dpArray, current);
            for (int i = 0; i < maxValues.size(); ++i) {
                if (current[i] < maxValues[i] - 1) {
                    ++current[i];
                    break;
                } else {
                    current[i] = 0;
                    if (i == maxValues.size() - 1) {
                        done = true;
                    }
                }
            }
        }

        vector<short> startingStateVector = getVectorFromPlayerState(config, config.initialState);

        vector<pair<string, double>> results = analyze(config, dpArray, startingStateVector);
        sort(results.begin(), results.end(), [](const pair<string, double>& a, const pair<string, double>& b) {
                return a.second < b.second;
            });

        cout << "Results" << endl;
        for (const pair<string, double>& pair : results) {
            string decision = pair.first;
            double expectedTime = pair.second / 100; // stored as hundredths
            int minutes = static_cast<int>(expectedTime) / 60;
            double remainingSeconds = expectedTime - (minutes * 60);
            cout << "Decision \"" << decision << "\" - " << minutes << ":" << fixed << setprecision(2) << remainingSeconds << endl;
        }
        return 0;
    }
