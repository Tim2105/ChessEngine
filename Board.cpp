#include "Board.h"
#include <iostream>
#include <stdio.h>

Board::Board() {
    initMailbox();
}

Board::~Board() {

}

void Board::initMailbox() {
    for(int i = 0; i < 120; i++)
        mailbox[i] = -1;
    
    int index = 0;
    for(int rank = RANK_1; rank <= RANK_8; rank++) {
        for(int file = FILE_A; file <= FILE_H; file++) {
            int square = FR2SQ(file, rank);
            mailbox[square] = index;
            mailbox64[index] = square;
            index++;
        }
    }

    
}