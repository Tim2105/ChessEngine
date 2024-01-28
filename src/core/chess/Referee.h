#ifndef REFEREE_H
#define REFEREE_H

#include "core/chess/Board.h"

class Referee {

    public:
        /**
         * @brief Überprüft, ob das Spiel zwangsläufig unentschieden ist,
         * d.h. dass selbst kooperierende Spieler kein Schachmatt erzwingen können.
         * 
         * @param b Das Spielfeld, das überprüft werden soll.
         */
        static bool isDraw(Board& b);

        /**
         * @brief Überprüft, ob das Spiel unentschieden ist, weil nicht genug Material vorhanden ist.
         * 
         * @param b Das Spielfeld, das überprüft werden soll.
         */
        static bool isDrawByMaterial(Board& b);

        /**
         * @brief Überprüft, ob das Spiel beendet ist, weil ein Spieler Schachmatt ist.
         * 
         * @param b Das Spielfeld, das überprüft werden soll.
         */
        static bool isCheckmate(Board& b);

        /**
         * @brief Überprüft, ob das Spiel beendet ist (Schachmatt oder unentschieden).
         */
        static bool isGameOver(Board& b);

    private:
        /**
         * Die Bitboards für die weißen bzw. schwarzen Felder.
         */

        static constexpr Bitboard lightSquares = 0x55AA55AA55AA55AA;
        static constexpr Bitboard darkSquares = 0xAA55AA55AA55AA55;

};

#endif