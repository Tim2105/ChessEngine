#ifndef HCE_PARAMETERS_H
#define HCE_PARAMETERS_H

#include <fstream>
#include <stdint.h>

#include "core/chess/BoardDefinitions.h"

class HCEParameters {

    public:
        /**
         * Faktoren für das Polynom zweiten Grades zur
         * Berechnung des Materialwertes einer Farbe.
         */

        int16_t pieceValues[20];

        /**
         * Faktoren für die Funktion zur Berechnung
         * des Materialungleichgewichts.
         */

        int16_t pieceImbalanceValues[10];

        /**
         * Positionstabellen für alle Figuren im Mittel- und Endspiel.
         */

        int16_t mgPSQTPawn[64];

        int16_t mgPSQTPacked[5][32];

        int16_t egPSQTPawn[64];

        int16_t egPSQTPacked[5][32];

        /**
         * Bonus für das Zugrecht im Mittel- und Endspiel.
         */

        int16_t mgTempoBonus;
        int16_t egTempoBonus;

        /**
         * Bonus für angegriffene Figuren im Mittel- und Endspiel.
         * Angriffe auf kleinere Figuren, die von einem
         * Bauern gedeckt werden, werden nicht berücksichtigt.
         */

        int16_t mgAttackByMinorPieceBonus[5];

        int16_t mgAttackByRookBonus[5];

        int16_t egAttackByMinorPieceBonus[5];

        int16_t egAttackByRookBonus[5];

        /**
         * Bewertungen für verschiedene Merkmale in
         * Bauernstrukturen im Mittel- und Endspiel.
         */

        int16_t mgConnectedPawnBonus[6]; // pro Rang (2 - 7)
        int16_t egConnectedPawnBonus[6]; // pro Rang (2 - 7)
        int16_t mgDoubledPawnPenalty[4]; // pro Linie (1 - 4, gespiegelt)
        int16_t egDoubledPawnPenalty[4]; // pro Linie (1 - 4, gespiegelt)
        int16_t mgIsolatedPawnPenalty[4]; // pro Linie (1 - 4, gespiegelt)
        int16_t egIsolatedPawnPenalty[4]; // pro Linie (1 - 4, gespiegelt)
        int16_t mgBackwardPawnPenalty[5]; // pro Rang (2 - 6)
        int16_t egBackwardPawnPenalty[5]; // pro Rang (2 - 6)
        int16_t mgPassedPawnBonus[6]; // pro Rang (2 - 7)
        int16_t egPassedPawnBonus[6]; // pro Rang (2 - 7)
        int16_t mgCandidatePassedPawnBonus[5]; // pro Rang (2 - 6)
        int16_t egCandidatePassedPawnBonus[5]; // pro Rang (2 - 6)
        int16_t mgConnectedPassedPawnBonus[6]; // pro Rang (2 - 7)
        int16_t egConnectedPassedPawnBonus[6]; // pro Rang (2 - 7)

        /**
         * Bewertungen für starke Felder im Mittelspiel.
         * Starke Felder sind Felder, die von einem eigenen Bauern
         * gedeckt sind und nie von einem gegnerischen Bauern
         * angegriffen werden können.
         */

        int16_t mgCenterOutpostBonus; // Linien C - F
        int16_t mgEdgeOutpostBonus; // Linien A - B und G - H

        /**
         * Bewertungen für die Sicherheit des Königs im Mittel- und Endspiel.
         */

        int16_t mgAttackWeight[4]; // Gewichtung der Angreifer (Springer, Läufer, Turm, Dame)
        int16_t egAttackWeight[4]; // Gewichtung der Angreifer (Springer, Läufer, Turm, Dame)
        int16_t mgUndefendedAttackWeight[4]; // Gewichtung der unverteidigten Angriffe (Springer, Läufer, Turm, Dame)
        int16_t egUndefendedAttackWeight[4]; // Gewichtung der unverteidigten Angriffe (Springer, Läufer, Turm, Dame)
        int16_t mgSafeCheckWeight[4]; // (Springer, Läufer, Turm, Dame)
        int16_t egSafeCheckWeight[4]; // (Springer, Läufer, Turm, Dame)
        int16_t mgSafeContactCheckWeight[2]; // (Turm, Dame)
        int16_t egSafeContactCheckWeight[2]; // (Turm, Dame)
        int16_t mgDefenseWeight[4]; // Gewichtung der Verteidiger (Springer, Läufer, Turm, Dame)
        int16_t egDefenseWeight[4]; // Gewichtung der Verteidiger (Springer, Läufer, Turm, Dame)
        int16_t mgPawnShieldSizeWeight[3]; // Gewichtung der Größe des Bauernschildes (defensiv)
        int16_t mgKingOpenFileWeight[3]; // Gewichtung der offenen Linien (offensiv)
        int16_t mgPawnStormWeight[5]; // pro Rang (2 - 6)

        /**
         * Bewertungen für die Mobilität der Figuren
         * im Mittel- und Endspiel.
         * (Springer, Läufer, Turm, Dame)
         */

        int16_t mgPieceMobilityBonus[4];
        int16_t egPieceMobilityBonus[4];

        int16_t mgPieceNoMobilityPenalty[4];
        int16_t egPieceNoMobilityPenalty[4];

        /**
         * Ein Bonus für jede Leichtfigur,
         * die auf einem starken Feld steht oder
         * im nächsten Zug erreichen kann.
         */

        int16_t mgKnightOnCenterOutpostBonus;
        int16_t mgKnightOnEdgeOutpostBonus;
        int16_t mgBishopOnCenterOutpostBonus;
        int16_t mgBishopOnEdgeOutpostBonus;

        /**
         * Eine Bestrafung für Läufer, die durch eigene
         * unbewegbare Bauern oder gegnerische, durch andere
         * Bauern gedeckte, Bauern blockiert werden.
         */

        int16_t mgBadBishopPenalty;
        int16_t egBadBishopPenalty;

        /**
         * Ein Bonus für einen viel besseren Läufer auf einer Farbe.
         */

        int16_t mgBishopDominanceBonus;
        int16_t egBishopDominanceBonus;

        /**
         * Ein Bonus für jede offene oder halboffene
         * Linie, auf der sich ein Turm befindet.
         */

        int16_t mgRookOnOpenFileBonus;
        int16_t mgRookOnSemiOpenFileBonus;

        /**
         * Ein Bonus für gedoppelte Türme auf
         * halboffenen oder offenen Linien.
         */
        int16_t mgDoubledRooksOnOpenFileBonus;
        int16_t mgDoubledRooksOnSemiOpenFileBonus;

        /**
         * Ein Bonus für jeden Freibauern, der von
         * einem Turm auf derselben Linie verteidigt wird.
         */

        int16_t egRookBehindPassedPawnBonus;

        /**
         * @brief Bonus für jede Figur, die einen
         * gegnerischen Freibauern blockiert.
         */

        int16_t egBlockedEnemyPassedPawnBonus;

        /**
         * @brief Bonus für jeden Angriff auf den
         * Promotionspfad eines Freibauern.
         */
        int16_t egAttackOnPassedPawnPathBonus;

        /**
         * Gewichtungsparameter für die Bewertung der Distanz
         * des Königs zu Bauern im Endspiel.
         */

        int16_t egKingProximityPawnWeight;
        int16_t egKingProximityBackwardPawnWeight;
        int16_t egKingProximityPassedPawnWeight;

        /**
         * @brief Bonus für Freibauern in einem Bauernendspiel,
         * die nicht mehr vom gegnerischen König erreicht werden können.
         */

        int16_t ruleOfTheSquareBonus;

        /**
         * "Draw Conditions"
         */

        /**
         * @brief Bestrafung für ein Endspiel mit Läufern
         * unterschiedlicher Farbe. Die Bestrafung wird
         * der ansonsten führenden Seite angerechnet und
         * kann das Vorzeichen der Bewertung nicht ändern.
         */
        int16_t oppositeColorBishopsEndgameWinnablePenalty;

        /**
         * @brief Bestrafung, wenn beide Seiten einen Läufer
         * auf unterschiedlichen Farben haben. Die Bestrafung
         * wird der ansonsten führenden Seite angerechnet und
         * kann das Vorzeichen der Bewertung nicht ändern.
         */

        int16_t mgOppositeColorBishopsWinnablePenalty;
        int16_t egOppositeColorBishopsWinnablePenalty;

        /**
         * @brief Bestrafung für ein Endspiel mit Türmen
         * und Bauern (und maximal einer weiteren Leichtfigur).
         * Die Bestrafung wird der ansonsten führenden Seite
         * angerechnet und kann das Vorzeichen der Bewertung nicht ändern.
         */
        int16_t rookEndgameWinnablePenalty;

        /**
         * @brief Konstante Bestrafung Richtung 0 für
         * Mittelspiel und Endspiel. Die Bestrafung wird
         * der ansonsten führenden Seite angerechnet und
         * kann das Vorzeichen der Bewertung nicht ändern.
         */

        int16_t mgDefaultWinnablePenalty;
        int16_t egDefaultWinnablePenalty;

        /**
         * @brief Bonus für ein Endspiel mit nur Bauern
         * und Königen. Der Bonus kann die Vorzeichen-
         * erhaltenden Bestrafungen abschwächen, aber nicht
         * die tatsächliche Bewertung erhöhen.
         */
        int16_t kingAndPawnEndgameWinnableBonus;

        /**
         * @brief Bonus für jeden Bauern. Der Bonus kann
         * die Vorzeichen-erhaltenden Bestrafungen
         * abschwächen, aber nicht die tatsächliche
         * Bewertung erhöhen.
         */

        int16_t mgPawnWinnableBonus;
        int16_t egPawnWinnableBonus;

        /**
         * @brief Bonus für jeden Freibauern(kandidat).
         * Der Bonus kann die Vorzeichen-erhaltenden
         * Bestrafungen abschwächen, aber nicht
         * die tatsächliche Bewertung erhöhen.
         */

        int16_t mgPassedPawnWinnableBonus;
        int16_t egPassedPawnWinnableBonus;

        /**
         * @brief Bonus für einen König, der
         * auf der gegnerischen Seite steht.
         * Der Bonus kann die Vorzeichen-erhaltenden
         * Bestrafungen abschwächen, aber nicht
         * die tatsächliche Bewertung erhöhen.
         */

        int16_t kingInfiltrationWinnableBonus;

        /**
         * Parameter für die Mop-Up Evaluation.
         */

        int16_t egMopupBaseBonus;
        int16_t egMopupProgressBonus;

        /**
         * Hier werden die entpackten Positionstabellen
         * für die Mittel- und Endspielbewertung gespeichert.
         */

        int16_t mgPSQT[6][64];
        int16_t egPSQT[6][64];


    public:
        HCEParameters();
        HCEParameters(std::istream& stream);

        ~HCEParameters();

        void unpackPSQT();

        void loadParameters(std::istream& stream);
        void saveParameters(std::ostream& stream) const;

        /**
         * @brief Gibt die Parameter in lesbarer Form auf
         * einem Ausgabe-Stream aus.
         */
        void displayParameters(std::ostream& stream) const;

        int16_t& operator[](size_t index);
        int16_t operator[](size_t index) const;
        size_t indexOf(int16_t* ptr) const;

        static constexpr size_t size() { return (sizeof(HCEParameters) - sizeof(mgPSQT) - sizeof(egPSQT)) / sizeof(int16_t); }

        /**
         * @brief Dieses Objekt enthält einige wenige Parameter,
         * die nie abgerufen werden (Positionstabellen für Bauern
         * auf der ersten und letzten Reihe). Diese Methode prüft,
         * ob ein Parameter an einem bestimmten Index tot ist.
         */
        bool isParameterDead(size_t index) const;

        /**
         * @brief Prüft, ob ein Parameter optimiert werden kann.
         */
        bool isOptimizable(size_t index) const;

        inline int getLinearPieceValue(int piece) const { return pieceValues[piece - 1]; }
        inline int getQuadraticPieceValue(int piece) const { return pieceValues[piece + 4]; }

        inline int getCrossedPieceValue(int piece1, int piece2) const {
            int minPiece = std::min(piece1, piece2) - 1;
            int maxPiece = std::max(piece1, piece2) - 2;

            return pieceValues[10 + maxPiece * (maxPiece + 1) / 2 + minPiece];
        }

        inline int getPieceImbalanceValue(int piece1, int piece2) const {
            int minPiece = std::min(piece1, piece2) - 1;
            int maxPiece = std::max(piece1, piece2) - 2;

            return pieceImbalanceValues[maxPiece * (maxPiece + 1) / 2 + minPiece];
        }

        inline int getMGPSQT(int piece, int square) const { return mgPSQT[piece - 1][square]; }
        inline int getEGPSQT(int piece, int square) const { return egPSQT[piece - 1][square]; }

        inline int getMGTempoBonus() const { return mgTempoBonus; }
        inline int getEGTempoBonus() const { return egTempoBonus; }

        inline int getMGAttackByMinorPieceBonus(int piece) const { return mgAttackByMinorPieceBonus[piece - 1]; }
        inline int getMGAttackByRookBonus(int piece) const { return mgAttackByRookBonus[piece - 1]; }
        inline int getEGAttackByMinorPieceBonus(int piece) const { return egAttackByMinorPieceBonus[piece - 1]; }
        inline int getEGAttackByRookBonus(int piece) const { return egAttackByRookBonus[piece - 1]; }

        inline int getMGConnectedPawnBonus(int rank) const { return mgConnectedPawnBonus[rank - 1]; }
        inline int getEGConnectedPawnBonus(int rank) const { return egConnectedPawnBonus[rank - 1]; }

        inline int getMGDoubledPawnPenalty(int file) const { return mgDoubledPawnPenalty[file & 4 ? file ^ 7 : file]; }
        inline int getEGDoubledPawnPenalty(int file) const { return egDoubledPawnPenalty[file & 4 ? file ^ 7 : file]; }

        inline int getMGIsolatedPawnPenalty(int file) const { return mgIsolatedPawnPenalty[file & 4 ? file ^ 7 : file]; }
        inline int getEGIsolatedPawnPenalty(int file) const { return egIsolatedPawnPenalty[file & 4 ? file ^ 7 : file]; }

        inline int getMGBackwardPawnPenalty(int rank) const { return mgBackwardPawnPenalty[rank - 1]; }
        inline int getEGBackwardPawnPenalty(int rank) const { return egBackwardPawnPenalty[rank - 1]; }

        inline int getMGPassedPawnBonus(int rank) const { return mgPassedPawnBonus[rank - 1]; }
        inline int getEGPassedPawnBonus(int rank) const { return egPassedPawnBonus[rank - 1]; }

        inline int getMGCandidatePassedPawnBonus(int rank) const { return mgCandidatePassedPawnBonus[rank - 1]; }
        inline int getEGCandidatePassedPawnBonus(int rank) const { return egCandidatePassedPawnBonus[rank - 1]; }

        inline int getMGConnectedPassedPawnBonus(int rank) const { return mgConnectedPassedPawnBonus[rank - 1]; }
        inline int getEGConnectedPassedPawnBonus(int rank) const { return egConnectedPassedPawnBonus[rank - 1]; }

        inline int getMGCenterOutpostBonus() const { return mgCenterOutpostBonus; }
        inline int getMGEdgeOutpostBonus() const { return mgEdgeOutpostBonus; }

        inline int getMGAttackWeight(int piece) const { return mgAttackWeight[piece - 2]; }
        inline int getEGAttackWeight(int piece) const { return egAttackWeight[piece - 2]; }
        inline int getMGUndefendedAttackWeight(int piece) const { return mgUndefendedAttackWeight[piece - 2]; }
        inline int getEGUndefendedAttackWeight(int piece) const { return egUndefendedAttackWeight[piece - 2]; }
        inline int getMGSafeCheckWeight(int piece) const { return mgSafeCheckWeight[piece - 2]; }
        inline int getEGSafeCheckWeight(int piece) const { return egSafeCheckWeight[piece - 2]; }
        inline int getMGSafeContactCheckWeight(int piece) const { return mgSafeContactCheckWeight[piece - 4]; }
        inline int getEGSafeContactCheckWeight(int piece) const { return egSafeContactCheckWeight[piece - 4]; }
        inline int getMGDefenseWeight(int piece) const { return mgDefenseWeight[piece - 2]; }
        inline int getEGDefenseWeight(int piece) const { return egDefenseWeight[piece - 2]; }

        inline int getMGPawnShieldSizeWeight(int size) const { return size == 0 ? 0 : mgPawnShieldSizeWeight[size - 1]; }
        inline int getMGKingOpenFileWeight(int numFiles) const { return numFiles == 0 ? 0 : mgKingOpenFileWeight[numFiles - 1]; }
        inline int getMGPawnStormWeight(int rank) const { return mgPawnStormWeight[rank - 1]; }

        inline int getMGPieceMobilityBonus(int piece) const { return mgPieceMobilityBonus[piece - 2]; }
        inline int getEGPieceMobilityBonus(int piece) const { return egPieceMobilityBonus[piece - 2]; }

        inline int getMGPieceNoMobilityPenalty(int piece) const { return mgPieceNoMobilityPenalty[piece - 2]; }
        inline int getEGPieceNoMobilityPenalty(int piece) const { return egPieceNoMobilityPenalty[piece - 2]; }

        inline int getMGKnightOnCenterOutpostBonus() const { return mgKnightOnCenterOutpostBonus; }
        inline int getMGKnightOnEdgeOutpostBonus() const { return mgKnightOnEdgeOutpostBonus; }
        inline int getMGBishopOnCenterOutpostBonus() const { return mgBishopOnCenterOutpostBonus; }
        inline int getMGBishopOnEdgeOutpostBonus() const { return mgBishopOnEdgeOutpostBonus; }

        inline int getMGBadBishopPenalty() const { return mgBadBishopPenalty; }
        inline int getEGBadBishopPenalty() const { return egBadBishopPenalty; }

        inline int getMGBishopDominanceBonus() const { return mgBishopDominanceBonus; }
        inline int getEGBishopDominanceBonus() const { return egBishopDominanceBonus; }

        inline int getMGRookOnOpenFileBonus() const { return mgRookOnOpenFileBonus; }
        inline int getMGRookOnSemiOpenFileBonus() const { return mgRookOnSemiOpenFileBonus; }

        inline int getMGDoubledRooksOnOpenFileBonus() const { return mgDoubledRooksOnOpenFileBonus; }
        inline int getMGDoubledRooksOnSemiOpenFileBonus() const { return mgDoubledRooksOnSemiOpenFileBonus; }

        inline int getEGRookBehindPassedPawnBonus() const { return egRookBehindPassedPawnBonus; }
        inline int getEGBlockedEnemyPassedPawnBonus() const { return egBlockedEnemyPassedPawnBonus; }

        inline int getEGAttackOnPassedPawnPathBonus() const { return egAttackOnPassedPawnPathBonus; }

        inline int getEGKingProximityPawnWeight() const { return egKingProximityPawnWeight; }
        inline int getEGKingProximityBackwardPawnWeight() const { return egKingProximityBackwardPawnWeight; }
        inline int getEGKingProximityPassedPawnWeight() const { return egKingProximityPassedPawnWeight; }

        inline int getRuleOfTheSquareBonus() const { return ruleOfTheSquareBonus; }

        inline int getOppositeColorBishopsEndgameWinnablePenalty() const { return oppositeColorBishopsEndgameWinnablePenalty; }

        inline int getMGOppositeColorBishopsWinnablePenalty() const { return mgOppositeColorBishopsWinnablePenalty; }
        inline int getEGOppositeColorBishopsWinnablePenalty() const { return egOppositeColorBishopsWinnablePenalty; }

        inline int getRookEndgameWinnablePenalty() const { return rookEndgameWinnablePenalty; }

        inline int getMGDefaultWinnablePenalty() const { return mgDefaultWinnablePenalty; }
        inline int getEGDefaultWinnablePenalty() const { return egDefaultWinnablePenalty; }

        inline int getKingAndPawnEndgameWinnableBonus() const { return kingAndPawnEndgameWinnableBonus; }

        inline int getMGPawnWinnableBonus() const { return mgPawnWinnableBonus; }
        inline int getEGPawnWinnableBonus() const { return egPawnWinnableBonus; }

        inline int getMGPassedPawnWinnableBonus() const { return mgPassedPawnWinnableBonus; }
        inline int getEGPassedPawnWinnableBonus() const { return egPassedPawnWinnableBonus; }

        inline int getKingInfiltrationWinnableBonus() const { return kingInfiltrationWinnableBonus; }

        inline int getMopupBaseBonus() const { return egMopupBaseBonus; }
        inline int getMopupProgressBonus() const { return egMopupProgressBonus; }
};

extern HCEParameters HCE_PARAMS;

#endif