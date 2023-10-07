#ifndef MAGICS_FINDER_H
#define MAGICS_FINDER_H

#include <chrono>
#include <fstream>
#include <stdint.h>

class MagicsFinder {
    private:
        static constexpr uint32_t maxTries = 1000000;

    public:
        class NoMagicNumberFoundException : public std::exception {
            public:
                virtual const char* what() const throw() {
                    return "No suitable magic number found!";
                }
        };

        /**
         * @brief Generiert alle möglichen Rook-Masks und speichert die in der Datei.
        */
        static void findRookMasks(std::ofstream& resultFile);

        /**
         * @brief Generiert alle möglichen Bishop-Masks und speichert die in der Datei.
        */
        static void findBishopMasks(std::ofstream& resultFile);

        /**
         * @brief Generiert eine Magic-Number für einen Turm auf einem bestimmten Feld.
         * 
         * @param sq Das Feld, auf dem der Turm steht.
         * @param shift Die Anzahl an Shifts, die die Magic-Number benötigen soll.
        */
        static uint64_t findRookMagic(int32_t sq, int32_t shift);

        /**
         * @brief Sucht nach Magic-Numbers für Türme auf jedem Feld
        */
        static void searchForRookMagics(std::ofstream& resultFile, std::chrono::seconds time);

        /**
         * @brief Generiert eine Magic-Number für einen Läufer auf einem bestimmten Feld.
         * 
         * @param sq Das Feld, auf dem der Läufer steht.
         * @param shift Die Anzahl an Shifts, die die Magic-Number benötigen soll.
        */
        static uint64_t findBishopMagic(int32_t sq, int32_t shift);
};

#endif