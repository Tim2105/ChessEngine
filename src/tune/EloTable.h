#ifndef ELO_TABLE_H
#define ELO_TABLE_H

#include <algorithm>
#include "core/utils/Random.h"
#include <cmath>
#include <iomanip>
#include <stdint.h>
#include <map>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

enum GameResult {
    WHITE_WIN,
    BLACK_WIN,
    DRAW
};

template <typename T>
class EloTable {
    private:
        struct Entry {
            T data;
            double elo;

            Entry(const T& data, double elo) : data(data), elo(elo) {}
            Entry(T&& data, double elo) : data(std::move(data)), elo(elo) {}
        };

        using EntryMap = std::map<std::string, Entry>;
        using EntryIterator = typename EntryMap::iterator;
        using ConstEntryIterator = typename EntryMap::const_iterator;

        EntryMap entries;
        EntryIterator current;
        size_t maxEntries;
        double kFactor;

        static double expectedScore(double eloA, double eloB) {
            return 1.0 / (1.0 + std::pow(10.0, (eloB - eloA) / 400.0));
        }

        std::vector<EntryIterator> getSortedEntries() {
            std::vector<EntryIterator> sortedEntries;
            for(auto it = entries.begin(); it != entries.end(); ++it)
                sortedEntries.push_back(it);

            std::sort(sortedEntries.begin(), sortedEntries.end(),
                      [](const auto& a, const auto& b) { return a->second.elo > b->second.elo; });

            return sortedEntries;
        }

        std::vector<ConstEntryIterator> getSortedEntries() const {
            std::vector<ConstEntryIterator> sortedEntries;
            for(auto it = entries.cbegin(); it != entries.cend(); ++it)
                sortedEntries.push_back(it);

            std::sort(sortedEntries.begin(), sortedEntries.end(),
                      [](const auto& a, const auto& b) { return a->second.elo > b->second.elo; });

            return sortedEntries;
        }

    public:
        inline EloTable(size_t maxEntries = 8, double kFactor = 16.0):
            maxEntries(maxEntries), kFactor(kFactor > 0.0 ? kFactor : 16.0) {}

        void addPlayer(const std::string& name, double elo, const T& data) {
            // Füge neuen Eintrag hinzu, ohne großes temporäres Entry-Objekt auf dem Stack zu bauen.
            auto [newEntryIt, inserted] = entries.try_emplace(name, data, elo);
            if(!inserted) {
                newEntryIt->second.data = data;
                newEntryIt->second.elo = elo;
            }

            if(name == "current")
                current = newEntryIt;

            // Wenn die Anzahl der Einträge das Limit überschreitet, entferne den Eintrag mit der niedrigsten Elo
            // Entferne aber nicht den current, selbst wenn er die niedrigste Elo hat
            if(maxEntries > 0 && entries.size() > maxEntries) {
                auto sortedEntries = getSortedEntries();
                auto lowestEntryIt = sortedEntries.back();
                if(lowestEntryIt != current)
                    entries.erase(lowestEntryIt);
                else
                    entries.erase(sortedEntries[sortedEntries.size() - 2]);
            }
        }

        bool hasPlayer(const std::string& name) const {
            return entries.find(name) != entries.end();
        }

        size_t size() const {
            return entries.size();
        }

        double getElo(const std::string& name) const {
            auto it = entries.find(name);
            if(it == entries.end())
                throw std::invalid_argument("Player does not exist in EloTable: " + name);

            return it->second.elo;
        }

        const T& getData(const std::string& name) const {
            auto it = entries.find(name);
            if(it == entries.end())
                throw std::invalid_argument("Player does not exist in EloTable: " + name);

            return it->second.data;
        }

        void setCurrentData(const T& data) {
            current->second.data = data;
        }

        std::string getRandomPlayerName(double temperature = 0.0) const {
            if(entries.empty())
                throw std::invalid_argument("Cannot sample player from empty EloTable");

            std::mt19937& rng = Random::generator<3>();
            if(temperature <= 0.0) {
                std::uniform_int_distribution<size_t> dist(0, entries.size() - 1);
                size_t target = dist(rng);

                auto it = entries.begin();
                std::advance(it, target);
                return it->first;
            }

            if(current == entries.end())
                throw std::invalid_argument("Current player not set in EloTable");

            double currentElo = current->second.elo;
            std::vector<ConstEntryIterator> candidates;
            candidates.reserve(entries.size());

            std::vector<double> weights;
            weights.reserve(entries.size());

            for(auto it = entries.cbegin(); it != entries.cend(); ++it) {
                candidates.push_back(it);
                double logit = -std::abs(it->second.elo - currentElo) / temperature;
                weights.push_back(std::exp(logit));
            }

            std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
            size_t picked = dist(rng);
            return candidates[picked]->first;
        }

        std::string getRandomPlayerNameExcluding(const std::string& exclude, double temperature = 0.0) const {
            auto excludeIt = entries.find(exclude);
            if(entries.empty() || (entries.size() == 1 && excludeIt != entries.end()) || excludeIt == entries.end())
                throw std::invalid_argument("Cannot sample player from EloTable: no players or only the excluded player exists");

            std::mt19937& rng = Random::generator<4>();
            // temperature <= 0 disables weighting and falls back to uniform sampling.
            if(temperature <= 0.0) {
                std::uniform_int_distribution<size_t> dist(0, entries.size() - 2);
                size_t target = dist(rng);

                auto it = entries.cbegin();
                for(size_t i = 0; i < target; i++) {
                    if(it->first == exclude)
                        it++;

                    it++;
                }

                if(it->first == exclude)
                    it++;

                return it->first;
            }

            double excludedElo = excludeIt->second.elo;

            std::vector<ConstEntryIterator> candidates;
            candidates.reserve(entries.size() - 1);
            double maxLogit = -std::numeric_limits<double>::infinity();

            for(auto it = entries.cbegin(); it != entries.cend(); ++it) {
                if(it->first == exclude)
                    continue;

                candidates.push_back(it);
                double logit = -std::abs(it->second.elo - excludedElo) / temperature;
                maxLogit = std::max(maxLogit, logit);
            }

            std::vector<double> weights;
            weights.reserve(candidates.size());
            for(const auto& it : candidates) {
                double logit = -std::abs(it->second.elo - excludedElo) / temperature;
                weights.push_back(std::exp(logit - maxLogit));
            }

            std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
            size_t picked = dist(rng);

            return candidates[picked]->first;
        }

        void addGameResult(const std::string& white, const std::string& black, GameResult result) {
            auto whiteIt = entries.find(white);
            auto blackIt = entries.find(black);

            if(whiteIt == entries.end() || blackIt == entries.end())
                throw std::invalid_argument("Both players must exist in EloTable before adding a game result");

            double whiteScore;
            double blackScore;

            switch(result) {
                case WHITE_WIN:
                    whiteScore = 1.0;
                    blackScore = 0.0;
                    break;
                case BLACK_WIN:
                    whiteScore = 0.0;
                    blackScore = 1.0;
                    break;
                case DRAW:
                default:
                    whiteScore = 0.5;
                    blackScore = 0.5;
                    break;
            }

            double whiteExpected = expectedScore(whiteIt->second.elo, blackIt->second.elo);
            double blackExpected = expectedScore(blackIt->second.elo, whiteIt->second.elo);

            whiteIt->second.elo += kFactor * (whiteScore - whiteExpected);
            blackIt->second.elo += kFactor * (blackScore - blackExpected);
        }

        void write(std::ostream& out) const {
            size_t rankWidth = std::max((size_t)4, std::to_string(entries.size()).size());
            size_t nameWidth = 6;
            for(const auto& [name, entry] : entries)
                nameWidth = std::max(nameWidth, name.size());

            out << std::left << std::setw((int)rankWidth) << "Rank" << " | "
                << std::left << std::setw((int)nameWidth) << "Name" << " | "
                << std::right << std::setw(8) << "Elo" << "\n";

            size_t separatorLen = rankWidth + nameWidth + 15;
            out << std::string(separatorLen, '-') << "\n";

            auto sortedEntries = getSortedEntries();

            std::streamsize currPrecision = out.precision();
            out << std::fixed << std::setprecision(2);
            for(size_t i = 0; i < sortedEntries.size(); i++) {
                out << std::left << std::setw((int)rankWidth) << (i + 1) << " | "
                    << std::left << std::setw((int)nameWidth) << sortedEntries[i]->first << " | "
                    << std::right << std::setw(8) << sortedEntries[i]->second.elo << "\n";
            }

            out.unsetf(std::ios::fixed);
            out.precision(currPrecision);
        }

        EntryIterator begin() {
            return entries.begin();
        }

        EntryIterator end() {
            return entries.end();
        }

        ConstEntryIterator begin() const {
            return entries.cbegin();
        }

        ConstEntryIterator end() const {
            return entries.cend();
        }

};

#endif