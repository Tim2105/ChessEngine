#include "OrderedMoveList.h"

#include <algorithm>

OrderedMoveList::OrderedMoveList() {

}

OrderedMoveList::OrderedMoveList(const OrderedMoveList& orderedMoveList) {
    moves = orderedMoveList.moves;
}

std::list<Move> OrderedMoveList::getMoves() const {
    std::list<Move> result;
    for(IntMovePair pair : moves) {
        result.push_back(pair.move);
    }
    return result;
}

void OrderedMoveList::addMove(const Move& move) {
    IntMovePair pair;
    pair.move = move;
    pair.value = scoreMove(move);
    moves.insert(std::upper_bound(moves.begin(), moves.end(), pair, [](const IntMovePair& a, const IntMovePair& b) {
        return a.value > b.value;
    }), pair);
}

void OrderedMoveList::addMoves(const std::list<Move>& moves) {
    for(Move move : moves) {
        addMove(move);
    }
}

void OrderedMoveList::removeMove(const Move& move) {
    moves.remove_if([&move](const IntMovePair& pair) {
        return pair.move == move;
    });
}

void OrderedMoveList::removeMoves(const std::list<Move>& moves) {
    for(Move move : moves) {
        removeMove(move);
    }
}

void OrderedMoveList::clear() {
    moves.clear();
}

bool OrderedMoveList::contains(const Move& move) const {
    for(IntMovePair pair : moves) {
        if(pair.move == move) {
            return true;
        }
    }
    return false;
}

bool OrderedMoveList::contains(const std::list<Move>& moves) const {
    for(Move move : moves) {
        if(!contains(move)) {
            return false;
        }
    }
    return true;
}

bool OrderedMoveList::isEmpty() const {
    return moves.empty();
}

int32_t OrderedMoveList::size() const {
    return moves.size();
}

int32_t scoreMove(const Move& move) {
    // TODO
    return 0;
}