#pragma once

#include "Game.h"
#include "Grid.h"
#include "BitBoard.h"
// #include "GameState.h"
#include <chrono>
#include <iomanip>

constexpr int pieceSize = 80;
constexpr int WHITE = +1, BLACK = -1;
constexpr int MAX_DEPTH = 24;

const int pawnTableB[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    -500, -500, -500, -500, -500, -500, -500, -500,
    -100, -100, -200, -300, -300, -200, -100, -100,
    -50, -50, -100, -250, -250, -100, -50, -50,
    0, 0, 0, -200, -200, 0, 0, 0,
    -50, 50, 100, 0, 0, 100, 50, -50,
    -50, -100, -100, 200, 200, -100, -100, -50,
    0, 0, 0, 0, 0, 0, 0, 0
};

const int pawnTableW[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    50, 100, 100, -200, -200, 100, 100, 50,
    50, -50, -100, 0, 0, -100, -50, 50,
    0, 0, 0, 200, 200, 0, 0, 0,
    50, 50, 100, 250, 250, 100, 50, 50,
    100, 100, 200, 300, 300, 200, 100, 100,
    500, 500, 500, 500, 500, 500, 500, 500,
    0, 0, 0, 0, 0, 0, 0, 0
};

const int knightTableB[64] = {
    500, 400, 300, 300, 300, 300, 400, 500,
    400, 200, 0, 0, 0, 0, 200, 400,
    300, 0, -100, -150, -150, -100, 0, 300,
    300, -50, -150, -200, -200, -150, -50, 300,
    300, 0, -150, -200, -200, -150, 0, 300,
    300, -50, -100, -150, -150, -100, -50, 300,
    400, 200, 0, -50, -50, 0, 200, 400,
    500, 400, 300, 300, 300, 300, 400, 500
};

const int knightTableW[64] = {
    -500, -400, -300, -300, -300, -300, -400, -500,
    -400, -200, 0, 50, 50, 0, -200, -400,
    -300, 50, 100, 150, 150, 100, 50, -300,
    -300, 0, 150, 200, 200, 150, 0, -300,
    -300, 50, 150, 200, 200, 150, 50, -300,
    -300, 0, 100, 150, 150, 100, 0, -300,
    -400, -200, 0, 0, 0, 0, -200, -400,
    -500, -400, -300, -300, -300, -300, -400, -500
};

const int rookTableB[64] = {
    0, 0, 0, -50, -50, 0, 0, 0,
    50, 0, 0, 0, 0, 0, 0, 50,
    50, 0, 0, 0, 0, 0, 0, 50,
    50, 0, 0, 0, 0, 0, 0, 50,
    50, 0, 0, 0, 0, 0, 0, 50,
    50, 0, 0, 0, 0, 0, 0, 50,
    -50, -100, -100, -100, -100, -100, -100, -50,
    0, 0, 0, 0, 0, 0, 0, 0
};

const int rookTableW[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    50, 100, 100, 100, 100, 100, 100, 50,
    -50, 0, 0, 0, 0, 0, 0, -50,
    -50, 0, 0, 0, 0, 0, 0, -50,
    -50, 0, 0, 0, 0, 0, 0, -50,
    -50, 0, 0, 0, 0, 0, 0, -50,
    -50, 0, 0, 0, 0, 0, 0, -50,
    0, 0, 0, 50, 50, 0, 0, 0
};

const int queenTableB[64] = {
    200, 100, 100, 50, 50, 100, 100, 200,
    100, 0, 0, 0, 0, 0, 0, 100,
    100, 0, -50, -50, -50, -50, 0, 100,
    50, 0, -50, -50, -50, -50, 0, 50,
    0, 0, -50, -50, -50, -50, 0, 50,
    100, -50, -50, -50, -50, -50, 0, 100,
    100, 0, -50, 0, 0, 0, 0, 100,
    200, 100, 100, 50, 50, 100, 100, 200
};

const int queenTableW[64] = {
    -200, -100, -100, -50, -50, -100, -100, -200,
    -100, 0, 50, 0, 0, 0, 0, -100,
    -100, 50, 50, 50, 50, 50, 0, -100,
    0, 0, 50, 50, 50, 50, 0, -50,
    -50, 0, 50, 50, 50, 50, 0, -50,
    -100, 0, 50, 50, 50, 50, 0, -100,
    -100, 0, 0, 0, 0, 0, 0, -100,
    -200, -100, -100, -50, -50, -100, -100, -200
};

const int bishopTableB[64] = {
    200, 100, 100, 100, 100, 100, 100, 200,
    100, 0, 0, 0, 0, 0, 0, 100,
    100, 0, -50, -100, -100, -50, 0, 100,
    100, -50, -50, -100, -100, -50, -50, 100,
    100, 0, -100, -100, -100, -100, 0, 100,
    100, -100, -100, -100, -100, -100, -100, 100,
    100, -50, 0, 0, 0, 0, -50, 100,
    200, 100, 100, 100, 100, 100, 100, 200
};

const int bishopTableW[64] = {
    -200, -100, -100, -100, -100, -100, -100, -200,
    -100, 50, 0, 0, 0, 0, 50, -100,
    -100, 100, 100, 100, 100, 100, 100, -100,
    -100, 0, 100, 100, 100, 100, 0, -100,
    -100, 50, 50, 100, 100, 50, 50, -100,
    -100, 0, 50, 100, 100, 50, 0, -100,
    -100, 0, 0, 0, 0, 0, 0, -100,
    -200, -100, -100, -100, -100, -100, -100, -200
};

const int kingTableB[64] = {
    -200, -300, -100, 0, 0, -100, -300, -200,
    -200, -200, 0, 0, 0, 0, -200, -200,
    100, 200, 200, 200, 200, 200, 200, 100,
    200, 300, 300, 400, 400, 300, 300, 200,
    300, 400, 400, 500, 500, 400, 400, 300,
    300, 400, 400, 500, 500, 400, 400, 300,
    300, 400, 400, 500, 500, 400, 400, 300,
    300, 400, 400, 500, 500, 400, 400, 300
};

const int kingTableW[64] = {
    -300, -400, -400, -500, -500, -400, -400, -300,
    -300, -400, -400, -500, -500, -400, -400, -300,
    -300, -400, -400, -500, -500, -400, -400, -300,
    -300, -400, -400, -500, -500, -400, -400, -300,
    -200, -300, -300, -400, -400, -300, -300, -200,
    -100, -200, -200, -200, -200, -200, -200, -100,
    200, 200, 0, 0, 0, 0, 200, 200,
    200, 300, 100, 0, 0, 100, 300, 200
};

const int emptyTable[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

enum AllBitBoards
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
    EMPTY_SQUARES,
    TOTAL_BITBOARDS,
};

enum MoveFlags {
    EnPassant = 0x01, // 0000 0001
    IsCapture = 0x02, // 0000 0010
    KingSideCastle = 0x04, // 0000 0100
    QueenSideCastle = 0x08, // 0000 1000
    IsPromotion = 0x10 // 0001 0000
};

// enum of indices for each color and each piece type, as well as white and black occupancy

struct alignas(32) GameStateData {
    char state[64];                 // persisitent
    char color;                     // BLACK or WHITE
    int flags;

    GameStateData() : flags(0)
        , color(WHITE) {
        std::memset(state, '0', sizeof(state));
    }
    GameStateData(const GameStateData&) = default;
    GameStateData& operator=(const GameStateData&) = default;
};

class Chess : public Game, public GameStateData
{
public:
    Chess();
    ~Chess();

    void setUpBoard() override;
    void init(const char* newState, int player);

    bool canBitMoveFrom(Bit &bit, BitHolder &src) override;
    bool canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
    bool actionForEmptyHolder(BitHolder &holder) override;
    void bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder& dst) override;

    void stopGame() override;

    Player *checkForWinner() override;
    bool checkForDraw() override;

    std::string initialStateString() override;
    std::string stateString() override;
    void setStateString(const std::string &s) override;

    Grid* getGrid() override { return _grid; }

    // General Helpers
        // don't need getOccupancy;
    void clearBoardHighlights() override;

    // Kings
    void generateKingMoveBitBoard();
    void generateKingMoves(std::vector<BitMove>& moves, const BitBoardElement kingBoard, const BitBoardElement emptySquares);

    // // Knights
    void generateKnightMoveBitBoard();
    void generateKnightMoves(std::vector<BitMove>& moves, const BitBoardElement knightBoard, const BitBoardElement emptySquares);

    // // Pawns
    // void generatePawnMoves(BitBoardElement& singlePush, BitBoardElement& doublePush, BitBoardElement& attackLeft, BitBoardElement& attackRight);
    // BitBoardElement generatePawnBitBoardPerFile(BitBoardElement& singlePush, BitBoardElement& doublePush, BitBoardElement& attackLeft, BitBoardElement& attackRight, int file);
    const BitBoardElement generatePawnAttacks(const BitBoardElement pawns, char color);
    uint64_t generatePawnAttacksBitBoard(int square, char color);
    // There are more things to pass, unsure what they'd be
    void generatePawnMoveList(std::vector<BitMove>& moves, const BitBoardElement pawns, const BitBoardElement emptySquares, const BitBoardElement enemies, int color);
    void addPawnBitBoardMovesToList(std::vector<BitMove>& moves, const BitBoardElement moveBitBoard, const int shift);

    // // Rooks, Bishops, Queens
    void generateRookMoves(std::vector<BitMove>& moves, const BitBoardElement rookBoard, BitBoardElement friendlies, const BitBoardElement occupancy);
    void generateBishopMoves(std::vector<BitMove>& moves, const BitBoardElement bishopBoard, BitBoardElement friendlies, const BitBoardElement occupancy);
    void generateQueenMoves(std::vector<BitMove>& moves, const BitBoardElement queenBoard, BitBoardElement friendlies, const BitBoardElement occupancy);

    // return allmoves, or just store in all moves and clear at beginning?
    // adding params for AI evaluation
    std::vector<BitMove> generateAllMoves();
    bool isSquareAttacked(int square, char attackerColor, const BitBoardElement (&boards)[TOTAL_BITBOARDS]);
    void filterOutIllegalMoves(std::vector<BitMove>& moves);

    
    // AI
    void updateAI() override;
    int evaluateBoard(const std::string& state);
    bool gameHasAI()
    {
        return isAIEnabled;
    };

private:
    bool isAIEnabled = true;

    Bit* PieceForPlayer(const int playerNumber, ChessPiece piece);
    Player* ownerAt(int x, int y) const;
    void FENtoBoard(const std::string& fen);
    char pieceNotation(int x, int y) const;

    Grid* _grid;
    // keep track of current player
    int _currentPlayer;

    // GameState gameState;

    // Moves Vector (probably not stored here)
    // updated every turn!
    std::vector<BitMove> _allMoves;

    // BitBoard arrays
    // precomputation ones

    // These are handled in magicBitBoards.h, as knightAttacks and kingAttacks, precomputed already
    // BitBoard _knightBitBoards[64];
    // BitBoard _kingBitBoards[64];
    // BitBoard _whitePawnBitBoards[64], _blackPawnBitBoards[64];
    // BitBoard _currentPawnMove;

    // File masks
    // BitBoard
    // current state

    // Current state of the table for each piece and both occupancies.
    BitBoardElement _tableBitBoards[TOTAL_BITBOARDS];
    // create an int mapping size 'z'+1
        // this is to map for any letter that we need
        // is also const!
    int indexMapping['z'+1], materialValsMapping['z'+1];
    ChessPiece pieceMapping['z'+1]; 
    // int weightedTableMapping[TOTAL_BITBOARDS][64];
        // clear everything to be 0 first
        // populate in the constructor


    // Some Masks
    // uint64_t num = 0x0000000000FF0000;
    const uint64_t RANK_3 = 0x0000000000FF0000,
                    RANK_6 = 0x0000FF0000000000,
                    notAFile = 0xFEFEFEFEFEFEFEFEULL,
                    notHFile = 0x7F7F7F7F7F7F7F7FULL;


    // AI
    int         negamax(int depth, int playerColor, int alpha, int beta);
    int         _countMoves;

    GameStateData stateStack[MAX_DEPTH];
    int stackPtr = 0;

    uint64_t _zobristHash[2]; // when one hash value is made, the other is made as well because it's just a xor of the first by the color bit
    BitBoardElement _attackBitBoard;
    
    inline void pushMove(const BitMove& move) {
        pushState();
        unsigned char fromPiece = state[move.from];
        state[move.from] = '0';
        state[move.to] = fromPiece;
        if (move.flags & KingSideCastle) {
            state[move.to - 1] = state[move.to + 1];
            state[move.to + 1] = '0';
        } else if (move.flags & QueenSideCastle) {
            state[move.to + 1] = state[move.to - 2];
            state[move.to - 2] = '0';
        } else if (move.flags & EnPassant) {
            // check for color to determine which direction to capture
            if (fromPiece == 'P') {
                state[move.to - 8] = '0';
            } else {
                state[move.to + 8] = '0';
            }
        } else if (move.flags & IsPromotion) {
            state[move.to] = color == WHITE ? 'Q' : 'q';
        }
        // flip the _currentPlayer bit as it now becomes the other player's turn
        color = (color == WHITE) ? BLACK : WHITE;
        flags = 0; // invalidate all the flags
    }

    inline void pushState() {
        assert(stackPtr < MAX_DEPTH);
        stateStack[stackPtr++] = static_cast<const GameStateData&>(*this);
    }
    inline void popState() {
        assert(stackPtr > 0);
        static_cast<GameStateData&>(*this) = stateStack[--stackPtr];
    }
};
