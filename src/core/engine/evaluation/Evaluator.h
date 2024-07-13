#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "core/chess/Board.h"

#define UNUSED(x) (void)(x)

/**
 * @brief Eine Basisklasse für alle statische Evaluatoren von Schachpositionen.
 * 
 * Diese Klasse deklariert die Methode int evaluate(), die eine statische Bewertung der Spielposition ausführt.
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
        static constexpr int SIMPLE_PIECE_VALUE[7] = {0, 100, 300, 300, 500, 900, 0};

        /**
         * @brief Versucht, den Angreifer mit dem geringsten Wert zu finden, der das Feld to angreift.
         * 
         * @return Das Feld des Angreifers mit dem geringsten Wert.
         * Wenn kein Angreifer gefunden wurde, wird NO_SQ zurückgegeben.
         */
        int getSmallestAttacker(int to, int side);

        /**
         * @brief Static Exchange Evaluation.
         * https://www.chessprogramming.org/Static_Exchange_Evaluation
         */
        int see(Move m);

        /**
         * @brief Eine Kopie von isSEEGreaterEqual, die die Spezialfälle
         * Bauernaufwertung und En-Passant nicht berücksichtigt.
         */
        bool seeGreaterEqual(Move m, int threshold);

        /**
         * @brief Überprüft, ob eine gegebene Stellung eine KPK-Remisstellung ist.
         */
        bool isDrawnKPKEndgame();

    protected:
        Board& board;

    public:
        Evaluator(Board& b) : board(b) {}

        virtual ~Evaluator() {}

        /**
         * @brief Setzt das Spielfeld, auf dem die statische Bewertung ausgeführt werden soll.
         */
        virtual inline void setBoard(Board& b) {
            this->board = b;
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
         * Außer dem werden auch theoretische Remisstellungen wie bestimme KPK-Stellungen überprüft.
         * 
         * @note Patt wird hier nicht überprüft, weil das eine Aufgabe der Suche ist.
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
        virtual int evaluate() = 0;

        /**
         * @brief Führt eine statische Bewertung eines Zugs mit SEE durch.
         */
        int evaluateMoveSEE(Move m);

        /**
         * @brief Überprüft, ob die statische Bewertung eines Zugs mit SEE
         * größer oder gleich einem Schwellwert ist.
         */
        bool isSEEGreaterEqual(Move m, int threshold);

        /**
         * @brief Führt eine statische Bewertung eines Zugs mit MVVLVA durch.
         */
        int evaluateMoveMVVLVA(Move m);

        /**
         * @brief Gibt eine Referenz auf das aktuelle Spielfeld zurück.
         */
        constexpr Board& getBoard() { return board; }
};

#endif