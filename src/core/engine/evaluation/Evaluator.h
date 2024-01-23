#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "core/chess/Board.h"

#define UNUSED(x) (void)(x)

/**
 * @brief Eine Basisklasse für alle statische Evaluatoren von Schachpositionen.
 * 
 * Diese Klasse deklariert die Methode int32_t evaluate(), die eine statische Bewertung der Spielposition ausführt.
 * Diese Methode muss von allen abgeleiteten Klassen implementiert werden.
 * 
 * Diese Klasse deklariert und implementiert auch die Methode bool isDraw(), die überprüft, ob das Spiel ein Unentschieden ist.
 * Außerdem existieren bereits Methoden zur statischen Bewertung eines Zuges mit SEE und MVVLVA.
 */
class Evaluator {

    private:
        /**
         * @brief Die Werte der Figuren in Centipawns.
         * Wird nur für die Bewertung der Figuren in SEE und MVV-LVA verwendet.
         */
        static constexpr int32_t SIMPLE_PIECE_VALUE[7] = {0, 100, 300, 300, 500, 900, 0};

        /**
         * @brief Versucht, den Angreifer mit dem geringsten Wert zu finden, der das Feld to angreift.
         * 
         * @return Das Feld des Angreifers mit dem geringsten Wert.
         * Wenn kein Angreifer gefunden wurde, wird NO_SQ zurückgegeben.
         */
        int32_t getSmallestAttacker(int32_t to, int32_t side);

        /**
         * @brief Static Exchange Evaluation.
         * https://www.chessprogramming.org/Static_Exchange_Evaluation
         */
        int32_t see(Move& m);
    
    protected:
        Board* b;

    public:
        Evaluator(Board& b) : b(&b) {}

        virtual ~Evaluator() {}

        /**
         * @brief Setzt das Spielfeld, auf dem die statische Bewertung ausgeführt werden soll.
         */
        virtual inline void setBoard(Board& b) {
            this->b = &b;
        }

        virtual void updateBeforeMove(Move m) { UNUSED(m); }
        virtual void updateAfterMove() {}
        virtual void updateBeforeUndo() {}
        virtual void updateAfterUndo(Move m) { UNUSED(m); }

        /**
         * @brief Überprüft, ob das Spiel zwangsläufig ein Unentschieden ist.
         * Ein Spiel ist zwangsläufig ein Unentschieden, wenn kein Spieler Schachmatt gesetzt werden kann,
         * auch wenn beide Spieler kooperieren würden.
         * Andere Möglichkeiten für ein Unentschieden sind z.B. 50 Züge ohne Bauern- oder Schlagzug
         * oder dreimal dieselbe Stellung.
         */
        bool isDraw();

        /**
         * @brief Führt eine statische Bewertung der
         * Spielpositon aus der Sicht des Spielers, der am Zug ist, durch.
         * 
         * @return Die Bewertung der Spielposition.
         * Je größer der Wert, desto besser ist die Spielposition für den Spieler der am Zug ist.
         * Je kleiner der Wert, desto besser ist die Spielposition für den Gegner des Spielers der am Zug ist.
         * Eine Bewertung von 0 bedeutet ein ausgeglichenes Spiel.
         */
        virtual int32_t evaluate() = 0;

        /**
         * @brief Führt eine statische Bewertung eines Zugs mit SEE durch.
         */
        int16_t evaluateMoveSEE(Move m);

        /**
         * @brief Führt eine statische Bewertung eines Zugs mit MVVLVA durch.
         */
        int16_t evaluateMoveMVVLVA(Move m);

        /**
         * @brief Gibt eine Referenz auf das aktuelle Spielfeld zurück.
         */
        constexpr Board& getBoard() { return *b; }
};

#endif