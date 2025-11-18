#pragma once

#include "Game.h"
#include "Grid.h"
#include "BitBoard.h"

constexpr int pieceSize = 80;

// enum ChessPiece
// {
//     NoPiece,
//     Pawn,
//     Knight,
//     Bishop,
//     Rook,
//     Queen,
//     King
// };

enum AllBitboards
{
    WHITE_PAWN,
    WHITE_KNIGHT,
    WHITE_BISHOP,
    WHITE_ROOK,
    WHITE_QUEEN,
    WHITE_KING,
    WHITE_OCCUPANCY,
    BLACK_PAWN,
    BLACK_KNIGHT,
    BLACK_BISHOP,
    BLACK_ROOK,
    BLACK_QUEEN,
    BLACK_KING,
    BLACK_OCCUPANCY,
    ALL_OCCUPANCY,
    TOTAL_BITBOARDS,
};

// enum of indices for each color and each piece type, as well as white and black occupancy

class Chess : public Game
{
public:
    Chess();
    ~Chess();

    void setUpBoard() override;

    bool canBitMoveFrom(Bit &bit, BitHolder &src) override;
    bool canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
    bool actionForEmptyHolder(BitHolder &holder) override;

    void stopGame() override;

    Player *checkForWinner() override;
    bool checkForDraw() override;

    std::string initialStateString() override;
    std::string stateString() override;
    void setStateString(const std::string &s) override;

    Grid* getGrid() override { return _grid; }

    // General Helpers
        // don't need getOccupancy;
    uint64_t getOccupancy();

    // Kings
    void generateKingMoveBitBoard();
    void generateKingMoves(std::vector<BitMove>& moves, BitboardElement kingBoard, uint64_t emptySquares);

    // Knights
    void generateKnightMoveBitBoard();
    void generateKnightMoves(std::vector<BitMove>& moves, BitboardElement knightBoard, uint64_t emptySquares);

    // Pawns
    void generatePawnMoves(BitboardElement& singlePush, BitboardElement& doublePush, BitboardElement& attackLeft, BitboardElement& attackRight);
    BitboardElement generatePawnBitboardPerFile(BitboardElement& singlePush, BitboardElement& doublePush, BitboardElement& attackLeft, BitboardElement& attackRight, int file);

    std::vector<BitMove> generateAllMoves();

private:
    Bit* PieceForPlayer(const int playerNumber, ChessPiece piece);
    Player* ownerAt(int x, int y) const;
    void FENtoBoard(const std::string& fen);
    char pieceNotation(int x, int y) const;

    Grid* _grid;
    
    // Moves Vector (probably not stored here)
    // updated every turn!
    std::vector<BitMove> allMoves;

    // Bitboard arrays
    // precomputation ones
    BitboardElement _knightBitboards[64];
    BitboardElement _kingBitboards[64];
    // BitboardElement _whitePawnBitBoards[64], _blackPawnBitBoards[64];
    // BitboardElement _currentPawnMove;

    // File masks
    // BitboardElement
    // current state
    BitboardElement _tableBitBoards[TOTAL_BITBOARDS];
        // this represents current state of the table for each piece and both occupancies
    // create an int mapping size 'z'+1
        // this is to map for any letter that we need
        // is also const!
    int mapping['z' + 1];  
        // clear everything to be 0 first
        // populate in the constructor
};