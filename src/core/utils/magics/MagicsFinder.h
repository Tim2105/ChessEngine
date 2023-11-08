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
         * @brief Generiert alle möglichen Züge für einen Turm auf einem bestimmten Feld
         * mit einer bestimmten Belegung.
         * 
         * Diese Funktion ist nur für die Generierung der Magic-Numbers gedacht.
        */
        static uint64_t rookAttackMask(int32_t sq, uint64_t occupied);

        /**
         * @brief Generiert alle möglichen Züge für einen Läufer auf einem bestimmten Feld
         * mit einer bestimmten Belegung.
         * 
         * Diese Funktion ist nur für die Generierung der Magic-Numbers gedacht.
        */
        static uint64_t bishopAttackMask(int32_t sq, uint64_t occupied);

        /**
         * Generiert alle möglichen Züge in eine bestimmte Richtung für einen Turm auf einem bestimmten Feld.
         */

        static uint64_t rookAttackTopMask(int32_t sq);

        static uint64_t rookAttackRightMask(int32_t sq);

        static uint64_t rookAttackBottomMask(int32_t sq);

        static uint64_t rookAttackLeftMask(int32_t sq);

        /**
         * Generiert alle möglichen Züge in eine bestimmte Richtung für einen Läufer auf einem bestimmten Feld.
         */

        static uint64_t bishopAttackTopLeftMask(int32_t sq);

        static uint64_t bishopAttackTopRightMask(int32_t sq);

        static uint64_t bishopAttackBottomLeftMask(int32_t sq);

        static uint64_t bishopAttackBottomRightMask(int32_t sq);

        /**
         * @brief Generiert alle möglichen Rook-Masks und speichert die in der Datei.
        */
        static void findRookMasks(std::ofstream& resultFile);

        /**
         * @brief Generiert alle möglichen Bishop-Masks und speichert die in der Datei.
        */
        static void findBishopMasks(std::ofstream& resultFile);

        /**
         * @brief Generiert alle möglichen Kombinationen einer Bitmaske.
         * 
         * @param mask Die Maske, für die die Kombinationen generiert werden sollen.
         * @param occupancies Ein Array, in das die Kombinationen gespeichert werden sollen.
        */
        static void generateAllOccupancyCombinations(uint64_t mask, uint64_t* occupancies);

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

        /**
         * @brief Sucht nach Magic-Numbers für Läufer auf jedem Feld
        */
        static void searchForBishopMagics(std::ofstream& resultFile, std::chrono::seconds time);
};

#endif