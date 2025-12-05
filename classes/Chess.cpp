#include "Chess.h"
#include <limits>
#include <cmath>
#include "MagicBitBoards.h"
#include <array>

static BitBoardElement _pawnAttacks[2][64];

static const std::array<int *, 128> pieceSquareTables = []() {
        std::array<int *, 128> pieceSquare{};
        pieceSquare['P'] = (int *)&pawnTableW;   pieceSquare['p'] = (int *)&pawnTableB;
        pieceSquare['N'] = (int *)&knightTableW; pieceSquare['n'] = (int *)&knightTableB;
        pieceSquare['B'] = (int *)&bishopTableW; pieceSquare['b'] = (int *)&bishopTableB;
        pieceSquare['R'] = (int *)&rookTableW;   pieceSquare['r'] = (int *)&rookTableB;
        pieceSquare['Q'] = (int *)&queenTableW;  pieceSquare['q'] = (int *)&queenTableB;
        pieceSquare['K'] = (int *)&kingTableW;   pieceSquare['k'] = (int *)&kingTableB;
        pieceSquare['0'] = (int *)&emptyTable;
        return pieceSquare;
    }();

Chess::Chess()
{
    _grid = new Grid(8, 8);

    initMagicBitboards();

    // // populate mapping
    // alphabetical order: B, K, N, P, Q, R
    indexMapping['P'] = WHITE_PAWN;
    indexMapping['N'] = WHITE_KNIGHT;
    indexMapping['B'] = WHITE_BISHOP;
    indexMapping['R'] = WHITE_ROOK;
    indexMapping['Q'] = WHITE_QUEEN;
    indexMapping['K'] = WHITE_KING;
    // mapping['W'] = WHITE_OCCUPANCY;
    indexMapping['p'] = BLACK_PAWN;
    indexMapping['n'] = BLACK_KNIGHT;
    indexMapping['b'] = BLACK_BISHOP;
    indexMapping['r'] = BLACK_ROOK;
    indexMapping['q'] = BLACK_QUEEN;
    indexMapping['k'] = BLACK_KING;
    indexMapping['0'] = EMPTY_SQUARES; // = what?
    // maybe when finding 0, do nothing

    // map to vals to, 10p, 40kn, 40b, 50r, 90q, 900k
    // each unit is centipawns
            // do this but negative for black
    materialValsMapping['P'] = 100;
    materialValsMapping['N'] = 400;
    materialValsMapping['B'] = 400;
    materialValsMapping['R'] = 500;
    materialValsMapping['Q'] = 900;
    materialValsMapping['K'] = 9000;
    materialValsMapping['p'] = -100;
    materialValsMapping['n'] = -400;
    materialValsMapping['b'] = -400;
    materialValsMapping['r'] = -500;
    materialValsMapping['q'] = -900;
    materialValsMapping['k'] = -9000;
    materialValsMapping['0'] = 0;

    // map chars to pieces
    pieceMapping['p'] = Pawn;
    pieceMapping['n'] = Knight;
    pieceMapping['b'] = Bishop;
    pieceMapping['r'] = Rook;
    pieceMapping['q'] = Queen;
    pieceMapping['k'] = King;

    // new mapping for weighted tables
    // mapping will have length of TOTAL_BITBOARDS
    // will map pieces to length 64 weighted tables
    

}

Chess::~Chess()
{
    delete _grid;
}

char Chess::pieceNotation(int x, int y) const
{
    const char *wpieces = { "0PNBRQK" };
    const char *bpieces = { "0pnbrqk" };
    Bit *bit = _grid->getSquare(x, y)->bit();
    char notation = '0';
    if (bit) {
        notation = bit->gameTag() < 128 ? wpieces[bit->gameTag()] : bpieces[bit->gameTag()-128];
    }
    return notation;
}

Bit* Chess::PieceForPlayer(const int playerNumber, ChessPiece piece)
{
    const char* pieces[] = { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png" };

    Bit* bit = new Bit();
    // should possibly be cached from player class?
    const char* pieceName = pieces[piece - 1];
    std::string spritePath = std::string("") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
    bit->setGameTag(playerNumber == 0 ? piece : piece+128);
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    bit->setSize(pieceSize, pieceSize);

    return bit;
}

void Chess::init(const char* newState, int player)
{
    memcpy(state, newState, 64);
    color = player;
}

void Chess::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;

    // white on bottom
    _grid->initializeChessSquares(pieceSize, "boardsquare.png");
    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    // black on bottom
    // _grid->initializeSquares(pieceSize, "boardsquare.png");
    // FENtoBoard("rnbkqbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBKQBNR");
    
    init(initialStateString().c_str(), WHITE);
    if(gameHasAI())
    {
        setAIPlayer(1);
    }

    startGame();
    _allMoves = generateAllMoves();
    // generateKnightMoveBitBoard();
    // generateKingMoveBitBoard();
    // BitBoard singlePush, doublePush, attackLeft, attackRight;
    // generatePawnMoves(singlePush, doublePush, attackLeft, attackRight);
    // generatePawnBitBoardPerFile(singlePush, doublePush, attackLeft, attackRight, 0);
    // generatePawnBitBoardPerFile(singlePush, doublePush, attackLeft, attackRight, 2);
    // _knightBitBoards[45].printBitBoard();
}

void Chess::FENtoBoard(const std::string& fen) {
    // convert a FEN string to a board
    // FEN is a space delimited string with 6 fields
    // 1: piece placement (from white's perspective)
    // NOT PART OF THIS ASSIGNMENT BUT OTHER THINGS THAT CAN BE IN A FEN STRING
    // ARE BELOW
    // 2: active color (W or B)
    // 3: castling availability (KQkq or -)
    // 4: en passant target square (in algebraic notation, or -)
    // 5: halfmove clock (number of halfmoves since the last capture or pawn advance)

    // Implementation:
    // lower case are black pieces, upper case are white pieces, numbers are empty spaces (consider pieces with spaces inbetween)
    int squareIndex = 0;
    for(int i=0; i<fen.length(); ++i)
    {
        if(fen[i] == '/')
        {
            continue;
        }
        else if(fen[i]-'0' >= 0 && fen[i]-'0' <= 8 )
        {
            // i+=(fen[i]-'0')-1;
            squareIndex += (fen[i]-'0')-1;
        }
        else
        {
            int playerNumber = isupper(fen[i]) ? 1 : 0;
            Bit* bit = PieceForPlayer(playerNumber, pieceMapping[tolower(fen[i])]);
            ChessSquare* square = _grid->getSquareByIndex(squareIndex);
            square->setBit(bit);
            bit->setPosition(square->getPosition());
            bit->setParent(square);
        }
        ++squareIndex;

    }

}


bool Chess::actionForEmptyHolder(BitHolder &holder)
{
    return false;
}

bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    // need to implement friendly/unfriendly in bit so for now this hack
    int currentPlayer = getCurrentPlayer()->playerNumber() * 128;
    int pieceColor = bit.gameTag() & 128;

    ChessSquare* srcSquare = dynamic_cast<ChessSquare*>(&src);

    bool valid = false;

    // loop through all moves that were generated
        // if the move.from is the src square index, set bool to true, highlight destination in the grid according to move.to
    for(auto move : _allMoves)
    {
        if(move.from == srcSquare->getSquareIndex())
        {
            _grid->getSquareByIndex(move.to)->setHighlighted(true);
            valid = true;
        }
    }
    // this is done because calling this as often as canBitMoveFromTo causes FRAME DROPS
    // ChessPiece piece = (ChessPiece)(bit.gameTag() < 128 ? bit.gameTag() : bit.gameTag() - 128);
    // if(piece == Pawn)
    // {
    //     ChessSquare* srcSquare = dynamic_cast<ChessSquare*>(&src);
    //     BitBoard singlePush, doublePush, attackLeft, attackRight;
    //     generatePawnMoves(singlePush, doublePush, attackLeft, attackRight);
    //     // _currentPawnMove = generatePawnBitBoardPerFile(singlePush, doublePush, attackLeft, attackRight, srcSquare->getColumn());
    // }

    if (pieceColor == currentPlayer) return valid;
    return false;
}

void Chess::bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder& dst)
{
    // setting currentPlayer to the other player
    color = color == WHITE ? BLACK : WHITE;
    std::string playerColor = color == WHITE ? "WHITE" : "BLACK";
    std::cout << "currentPlayer: " << playerColor << std::endl;
    

    // moves are set to generateAllmoves
    // generateAllMoves might also set _allMoves to the vector
    memcpy(state, stateString().c_str(), 64);
    _allMoves = generateAllMoves();
    // clearBoardHighlights()
    clearBoardHighlights();
    // end turn
    endTurn();
}

void Chess::clearBoardHighlights()
{
    _grid->forEachSquare([&](ChessSquare* square, int x, int y)
    {
        square->setHighlighted(false);
    });
}

bool Chess::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{

    // looping through moves
    // get src and dest square indices
    // if current move has the same indices as src and dst, then return true;
    // ChessPiece piece = (ChessPiece)(bit.gameTag() < 128 ? bit.gameTag() : bit.gameTag() - 128);
    // int pieceColor = bit.gameTag() & 128;
    // bool valid = false;

    ChessSquare* srcSquare = dynamic_cast<ChessSquare*>(&src);
    ChessSquare* dstSquare = dynamic_cast<ChessSquare*>(&dst);
    // BitBoardElement* bb;
    for(auto move : _allMoves)
    {
        // clearBoardHighlights();
        if(move.from == srcSquare->getSquareIndex() && move.to == dstSquare->getSquareIndex())
        {
            _grid->getSquareByIndex(move.to)->setHighlighted(true);
            return true;
        }
    }
    return false;


    // get current bit piece type, we use the game tag system to store that data
    // depending on piece type, grab that bit board at src idx
    // branching behavior :(
    // get bit board for src, src needs to be casted into a chessSquare, will column and row be correct then?
    // chessSquare gets narrowed into bitholder and then expanded back into chessSquare, are we able to preserve col/row data?
        // we are because what's being passed in already has that data
    // switch(piece)
    // {
    //     case Knight:
    //         // set bb to the right BitBoard
    //         bb = &KnightAttacks[srcSquare->getSquareIndex()];
    //         break;
    //     case King:
    //         bb = &KingAzttacks[srcSquare->getSquareIndex()];
    //         break;
    //     case Pawn:
    //         // bb = &_currentPawnMove;
    //         break;
    //     default:
    //         bb = nullptr;
    //         break;

            
    // }
    // if(piece == Knight)
    // {
    //     bb = &_knightBitBoards[srcSquare->getSquareIndex()];
    // }
    // else if(piece == King)
    // {
    //     bb = &_kingBitBoards[srcSquare->getSquareIndex()];
    // }
    // we do the efficient iteration, where we look at only set bits
    // forEachBit should be doing that
    // if(bb)
    // {
    //     bb->forEachBit([&](int idx)
    //     {
    //         // if lambda is called, then current idx has a set bit
    //         // check if idx is equal to dst idx
    //         if(idx == dstSquare->getSquareIndex())
    //         {
    //             valid = true;
    //         }
    //     });
    // }
    // return valid;
}

void Chess::stopGame()
{
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

Player* Chess::ownerAt(int x, int y) const
{
    if (x < 0 || x >= 8 || y < 0 || y >= 8) {
        return nullptr;
    }

    auto square = _grid->getSquare(x, y);
    if (!square || !square->bit()) {
        return nullptr;
    }
    return square->bit()->getOwner();
}

Player* Chess::checkForWinner()
{
    return nullptr;
}

bool Chess::checkForDraw()
{
    return false;
}

std::string Chess::initialStateString()
{
    return stateString();
}

std::string Chess::stateString()
{
    std::string s;
    s.reserve(64);
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
            // we want to make the white chars lower case
            // the if condition is never being met
            if(square->bit() && square->bit()->getOwner()->playerNumber() == 0)
            {
                s += pieceNotation( x, y );
            }
            else
            {
                char c = pieceNotation(x,y);
                c = tolower(c);
                s += c;
            }
        }
    );
    return s;
}

void Chess::setStateString(const std::string &s)
{
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        int index = y * 8 + x;
        char playerNumber = s[index] - '0';
        if (playerNumber) {
            square->setBit(PieceForPlayer(playerNumber - 1, Pawn));
        } else {
            square->setBit(nullptr);
        }
    });
}



// Generate actual move objects from a BitBoard
// emptySquares is ~occupancy, bits that represent emptySquares are set
void Chess::generateKnightMoves(std::vector<BitMove>& moves, const BitBoardElement knightBoard, const BitBoardElement occupancy) {
    // check if knightVoard is zero
        // return immediately
    // branching behavior, but it cuts off processing, so it's better?
    if (knightBoard.getData() == 0)
        return;
    knightBoard.forEachBit([&](int fromSquare) {
        BitBoardElement moveBitBoard = BitBoardElement(KnightAttacks[fromSquare] & ~occupancy.getData());
        // Efficiently iterate through only the set bits
        moveBitBoard.forEachBit([&](int toSquare) {
           moves.emplace_back(fromSquare, toSquare, Knight);
        });
    });
}

// Alternative to generating knight moves, returns a BitBoard
BitBoardElement generateKnightMoveBitBoard(int square)
{
    BitBoardElement bb = 0ULL;
    int rank = square / 8, 
        file = square % 8;
    
    // knight offsets pair vector
    std::pair<int, int> offsets[] = {
        {-2, -1}, {-2, 1}, {2, -1}, {2, 1},
        {-1, -2}, {-1, 2}, {1, -2}, {1, 2}
    };

    // dr: delta rank, df: delta file
    for(auto [dr, df] : offsets)
    {
        // creating ints for current rank and file after applying deltas
        int r = rank + dr, f = file + df;
        // check for within bounds
        if(r >=0 && r < 8 && f >= 0 && f < 8)
        {
            // if in bounds, shift properly
            bb |= 1ULL << (r * 8 + f);
        }
    }

    return bb;
}

void Chess::generateKingMoves(std::vector<BitMove>& moves, const BitBoardElement kingBoard, const BitBoardElement friendlies)
{
    kingBoard.forEachBit([&](int fromSquare) {
        BitBoardElement moveBitBoard = BitBoardElement(KingAttacks[fromSquare] & ~friendlies.getData());
        // Efficiently iterate through only the set bits
        moveBitBoard.forEachBit([&](int toSquare) {
           moves.emplace_back(fromSquare, toSquare, King);
        });
    });
}

uint64_t Chess::generatePawnAttacksBitBoard(int square, char color) {
    uint64_t bitboard = 0ULL;
    int rank = square / 8;
    int file = square % 8;

    // Pawns can only attack diagonally forward
    // For white: up-right and up-left
    // For black: down-right and down-left
    const int direction = (color == WHITE) ? 1 : -1;
    
    // Check diagonal left attack
    if (file > 0) {  // Not on a-file
        int r = rank + direction;
        int f = file - 1;
        if (r >= 0 && r < 8) {  // Stay within board bounds
            bitboard |= 1ULL << (r * 8 + f);
        }
    }
    
    // Check diagonal right attack
    if (file < 7) {  // Not on h-file
        int r = rank + direction;
        int f = file + 1;
        if (r >= 0 && r < 8) {  // Stay within board bounds
            bitboard |= 1ULL << (r * 8 + f);
        }
    }
    return bitboard;
}

const BitBoardElement Chess::generatePawnAttacks(const BitBoardElement pawns, char color) {
    BitBoardElement result(0);

    pawns.forEachBit([&](int fromSquare) {
        // Using precomputed or dynamic logic
        result |= _pawnAttacks[color == WHITE ? 0 : 1][fromSquare];
    });

    return result;
}

void Chess::generatePawnMoveList(std::vector<BitMove>& moves, const BitBoardElement pawns, const BitBoardElement emptySquares, const BitBoardElement enemies, int color)
{
    if(pawns.getData() == 0)
        return;
    
    // make BitBoards to mask left and right
    BitBoardElement removeRight(notAFile), removeLeft(notHFile);

    // same logic as before, less branching, more ternary
    BitBoardElement singlePush = (color == WHITE) ? (pawns.getData() << 8) & emptySquares.getData() : (pawns.getData() >> 8) & emptySquares.getData();
    BitBoardElement promotePush = (color == WHITE) ? (singlePush.getData() & RANK_7) & emptySquares.getData() : (singlePush.getData() & RANK_2) & emptySquares.getData();
    BitBoardElement doublePush = (color == WHITE) ? ((singlePush.getData() & RANK_3) << 8) & emptySquares.getData() : ((singlePush.getData() & RANK_6) >> 8) & emptySquares.getData();
    BitBoardElement captureLeft = (color == WHITE) ? ((pawns.getData() & notAFile) << 7) & enemies.getData() : ((pawns.getData() & notAFile) >> 9) & enemies.getData();
    BitBoardElement captureRight = (color == WHITE) ? ((pawns.getData() & notHFile) << 9) & enemies.getData() : ((pawns.getData() & notHFile) >> 7) & enemies.getData();

    

    // defining shifts so we can get the from for our BitMove
    const int shiftForward = (color == WHITE) ? 8 : -8;
    const int doubleShift = (color == WHITE) ? 16 : -16;
    const int captureLeftShift = (color == WHITE) ? 7 : -9;
    const int captureRightShift = (color == WHITE) ? 9 : -7;

    // adding pawn moves to list
    addPawnBitBoardMovesToList(moves, singlePush, shiftForward);
    addPawnBitBoardMovesToList(moves, doublePush, doubleShift);
    addPawnBitBoardMovesToList(moves, captureLeft, captureLeftShift);
    addPawnBitBoardMovesToList(moves, captureRight, captureRightShift);
}

void Chess::addPawnBitBoardMovesToList(std::vector<BitMove>& moves, const BitBoardElement moveBitBoard, const int shift)
{
    if(moveBitBoard.getData() == 0)
        return;
    // this will add the BitBoard moves to the list
    moveBitBoard.forEachBit([&](int toSquareIdx){
        moves.emplace_back(toSquareIdx-shift, toSquareIdx, Pawn);
    });
}

void Chess::generateRookMoves(std::vector<BitMove>& moves, const BitBoardElement rookBoard, const BitBoardElement friendlies, const BitBoardElement occupancy)
{

    // magic is brute forced magic
    // index = ((occupancy & mask) * magic) >> shift[sq]
    // moveBitBoard = rmagic[sq][index]

    // sq == square?

    rookBoard.forEachBit([&](int fromSquare){
        uint64_t index = (uint64_t)((occupancy.getData() & RMasks[fromSquare]) * RMagic[fromSquare]) >> (uint64_t)RShifts[fromSquare];
        BitBoardElement moveBitBoard(RAttacks[fromSquare][index] & ~friendlies.getData());
        moveBitBoard.forEachBit([&](int toSquare){
            moves.emplace_back(fromSquare, toSquare, Rook);
        });
    });
    // int index = ((~emptySquares.getData() & RMasks[36]) * RMagic[36]) >> (uint64_t)RShifts[36];
    // BitBoard moveBitBoard(RAttacks[36][index] & ~friendlies.getData());
    // BitBoard bb((ratt(36, ~emptySquares.getData()) & _tableBitBoards[BLACK_OCCUPANCY].getData()) | (RMasks[36] & ~_tableBitBoards[ALL_OCCUPANCY].getData()));
    
    // this seems to work, just pass in friendlies
    // BitBoard bb2(ratt(16, _tableBitBoards[ALL_OCCUPANCY].getData()) & ~_tableBitBoards[WHITE_OCCUPANCY].getData());
    
    
    // BitBoard bb3(RAttacks[16][32]);

    // BitBoard bb2 = ;
    // std::cout << "magic";
    // moveBitBoard.printBitBoard();
}

void Chess::generateBishopMoves(std::vector<BitMove>& moves, const BitBoardElement bishopBoard, const BitBoardElement friendlies, const BitBoardElement occupancy)
{
    bishopBoard.forEachBit([&](int fromSquare){
        uint64_t index = (uint64_t)((occupancy.getData() & BMasks[fromSquare]) * BMagic[fromSquare]) >> (uint64_t)BShifts[fromSquare];
        BitBoardElement moveBitBoard(BAttacks[fromSquare][index] & ~friendlies.getData());
        moveBitBoard.forEachBit([&](int toSquare){
            moves.emplace_back(fromSquare, toSquare, Bishop);
        });
    });
}

void Chess::generateQueenMoves(std::vector<BitMove>& moves, const BitBoardElement queenBoard, const BitBoardElement friendlies, const BitBoardElement occupancy)
{
    queenBoard.forEachBit([&](int fromSquare){
        uint64_t rIndex = (uint64_t)((occupancy.getData() & RMasks[fromSquare]) * RMagic[fromSquare]) >> (uint64_t)RShifts[fromSquare];
        BitBoardElement rMoveBitBoard(RAttacks[fromSquare][rIndex] & ~friendlies.getData());
        rMoveBitBoard.forEachBit([&](int toSquare){
            moves.emplace_back(fromSquare, toSquare, Queen);
        });
        uint64_t bIndex = (uint64_t)((occupancy.getData() & BMasks[fromSquare]) * BMagic[fromSquare]) >> (uint64_t)BShifts[fromSquare];
        BitBoardElement bMoveBitBoard(BAttacks[fromSquare][bIndex] & ~friendlies.getData());
        bMoveBitBoard.forEachBit([&](int toSquare){
            moves.emplace_back(fromSquare, toSquare, Queen);
        });
    });
    
}

std::vector<BitMove> Chess::generateAllMoves()
{
    // create moves vec
    // reserve 32
    // so there will ever only be 32 moves

    // CREATE NEW MOVES VEC
    std::vector<BitMove> moves;
    moves.reserve(32);


    // clear allBitBoards
    for(int i=0; i<TOTAL_BITBOARDS; ++i)
    {
        _tableBitBoards[i].setData(0);
    }

    // loop through state string
    // set bitIndex to mapping[state[i]]
    // AllBitBoards[index] |= 1ULL << i
    for(int i=0; i<64; ++i)
    {
        int index = indexMapping[state[i]];
        _tableBitBoards[index] |= 1ULL << i;
        // if state[i] is less than equal to 90, 90 is Z
        _tableBitBoards[WHITE_OCCUPANCY] |= (state[i] <= 90 && state[i] != '0' ? 1ULL : 0) << i;
        // if state[i] is greater than equal to 97, 97 is a
        _tableBitBoards[BLACK_OCCUPANCY] |= (state[i] >= 97 ? 1ULL : 0) << i;
        _tableBitBoards[ALL_OCCUPANCY] |= (state[i] != '0' ? 1ULL : 0) << i;
    }

    // calling all generate moves functions, this is not efficient, there's a better way
    // there was something about bitIndex and oppIndex

    // the bitIndex is an offset depending on current player
    int bitIndex = color == WHITE ? WHITE_PAWN : BLACK_PAWN;
    int oppBitIndex = color == WHITE ? BLACK_PAWN : WHITE_PAWN;

    // i need to isolate all the specific pieces and get the BitBoard for that index of that piece
    generateKnightMoves(moves, _tableBitBoards[WHITE_KNIGHT + bitIndex], _tableBitBoards[WHITE_OCCUPANCY + bitIndex]);
    generateKingMoves(moves, _tableBitBoards[WHITE_KING + bitIndex], _tableBitBoards[WHITE_OCCUPANCY + bitIndex]);
    generatePawnMoveList(moves, _tableBitBoards[WHITE_PAWN + bitIndex], _tableBitBoards[EMPTY_SQUARES], _tableBitBoards[WHITE_OCCUPANCY + oppBitIndex], color);
    generateRookMoves(moves, _tableBitBoards[WHITE_ROOK + bitIndex], _tableBitBoards[WHITE_OCCUPANCY + bitIndex], _tableBitBoards[ALL_OCCUPANCY]);
    generateBishopMoves(moves, _tableBitBoards[WHITE_BISHOP + bitIndex], _tableBitBoards[WHITE_OCCUPANCY + bitIndex], _tableBitBoards[ALL_OCCUPANCY]);
    generateQueenMoves(moves, _tableBitBoards[WHITE_QUEEN + bitIndex], _tableBitBoards[WHITE_OCCUPANCY + bitIndex], _tableBitBoards[ALL_OCCUPANCY]);
    // Graeme's code
    // for(int i=0; i<64; ++i)
    // {
    //     _knightBitBoards[i] = generateKnightMoveBitBoard(i);
    // }
    filterOutIllegalMoves(moves);

    return moves;
}

bool Chess::isSquareAttacked(int square, char attackerColor, const BitBoardElement (&boards)[TOTAL_BITBOARDS]) {
	const int pawnIdx   = (attackerColor == WHITE) ? WHITE_PAWN : BLACK_PAWN;
	const int knightIdx = (attackerColor == WHITE) ? WHITE_KNIGHT : BLACK_KNIGHT;
	const int bishopIdx = (attackerColor == WHITE) ? WHITE_BISHOP : BLACK_BISHOP;
	const int rookIdx   = (attackerColor == WHITE) ? WHITE_ROOK : BLACK_ROOK;
	const int queenIdx  = (attackerColor == WHITE) ? WHITE_QUEEN : BLACK_QUEEN;
	const int kingIdx   = (attackerColor == WHITE) ? WHITE_KING : BLACK_KING;

	// Get Occupancy of all pieces for sliding checks
	BitBoardElement occ = boards[ALL_OCCUPANCY];

	// Check Pawn Attacks
	char targetColor = (attackerColor == WHITE) ? BLACK : WHITE; 
	if ((generatePawnAttacksBitBoard(square, targetColor) & boards[pawnIdx].getData()) != 0) return true;

	// Check Knight Attacks
	if ((KnightAttacks[square] & boards[knightIdx].getData()) != 0) return true;

	// Check King Attacks (Neighboring kings)
	if ((KingAttacks[square] & boards[kingIdx].getData()) != 0) return true;

	// Check Bishop/Queen (Diagonal) Attacks
	uint64_t diagonalAttacks = getBishopAttacks(square, occ.getData());
	if ((diagonalAttacks & (boards[bishopIdx].getData() | boards[queenIdx].getData())) != 0) return true;

	// Check Rook/Queen (Straight) Attacks
	uint64_t straightAttacks = getRookAttacks(square, occ.getData());
	if ((straightAttacks & (boards[rookIdx].getData() | boards[queenIdx].getData())) != 0) return true;

	return false;
}

void Chess::filterOutIllegalMoves(std::vector<BitMove>& moves) {
	if (moves.empty()) return;

    const char myColor = color;
	const char opponentColor = (color == WHITE) ? BLACK : WHITE;
	const int myKingIdx = (myColor == WHITE) ? WHITE_KING : BLACK_KING;

	// Remove moves that leave the king in check
	moves.erase(std::remove_if(moves.begin(), moves.end(), [&](const BitMove& move) {
		
		// Create a temporary copy of the board state
		BitBoardElement tempBoards[TOTAL_BITBOARDS];
		for (int i = 0; i < TOTAL_BITBOARDS; ++i) tempBoards[i] = _tableBitBoards[i];

		// Apply the move to the temporary boards
		// Note: We just need occupancy correct for check detection.
		
		const uint64_t fromMask = 1ULL << move.from;
		const uint64_t toMask   = 1ULL << move.to;
		
		// Helper to determine which bitboard a piece belongs to
		auto getPieceIdx = [&](ChessPiece p, char c) {
			if (p == Pawn) return c == WHITE ? WHITE_PAWN : BLACK_PAWN;
			if (p == Knight) return c == WHITE ? WHITE_KNIGHT : BLACK_KNIGHT;
			if (p == Bishop) return c == WHITE ? WHITE_BISHOP : BLACK_BISHOP;
			if (p == Rook) return c == WHITE ? WHITE_ROOK : BLACK_ROOK;
			if (p == Queen) return c == WHITE ? WHITE_QUEEN : BLACK_QUEEN;
			return c == WHITE ? WHITE_KING : BLACK_KING; // King
		};

		int moverIdx = getPieceIdx(static_cast<ChessPiece>(move.piece), myColor);
		
		// Remove from 'from'
		tempBoards[moverIdx] &= ~fromMask;
		tempBoards[ALL_OCCUPANCY] &= ~fromMask;

		// Handle Captures (Remove opponent piece at 'to')
		// We scan opponent boards to find what was captured (slower than lookup, but safe for generic bitboards)
		int startOpp = (opponentColor == WHITE) ? WHITE_PAWN : BLACK_PAWN;
		int endOpp   = (opponentColor == WHITE) ? WHITE_KING : BLACK_KING;
		
		// Specialized handling for En Passant
		if (move.flags & EnPassant) {
			int capSq = (myColor == WHITE) ? (move.to - 8) : (move.to + 8);
			uint64_t capMask = 1ULL << capSq;
			tempBoards[startOpp] &= ~capMask; // Opponent Pawns
			tempBoards[ALL_OCCUPANCY] &= ~capMask;
		} else {
			// Standard capture
			for (int i = startOpp; i <= endOpp; ++i) {
				tempBoards[i] &= ~toMask;
			}
			tempBoards[ALL_OCCUPANCY] &= ~toMask; // Clear strictly to ensure no overlap before adding
		}

		// Handle Promotion
		if ((move.flags & IsPromotion)) {
			moverIdx = getPieceIdx(Queen, myColor); // Assume Queen promotion for check safety (mostly covers it)
		}

		// Add to 'to'
		tempBoards[moverIdx] |= toMask;
		tempBoards[ALL_OCCUPANCY] |= toMask;

		// Handle King Move (Update King Index tracking)
		int currentKingSquare = -1;
		if (move.piece == King) {
			currentKingSquare = move.to;
		} else {
			// If king didn't move, find him
			currentKingSquare = tempBoards[myKingIdx].firstBit();
		}

		// If the King is attacked by the opponent after this move, the move is illegal.
		return isSquareAttacked(currentKingSquare, opponentColor, tempBoards);

	}), moves.end());
}

void Chess::updateAI()
{
    if(!gameHasAI())
    {
        return;
    }
    // get current time
    const auto searchStart = std::chrono::steady_clock::now();
    _countMoves = 0;


    int bestVal = -10000000;
    BitMove bestMove = BitMove();
    constexpr int negInfinite = -1000000000;

    // initialize alpha and beta to negative "infinity" and positive "infinity" respectively
    int alpha = -1000000000;
    int beta = 1000000000;

    // get state string
    // std::string state = stateString();
    auto newMoves = generateAllMoves();
    // assert(newMoves.size() > 0);
    if(newMoves.size() == 0)
    {
        std::cout << "CHECKMATE";
        return;
    }
    _countMoves = 0;
    for(auto move : newMoves)
    {
        // char boardSave = state[move.to];
        // char pieceMoving = state[move.from];
        pushMove(move);

        // state[move.to] = pieceMcoving;
        // state[move.from] = '0';
        int moveVal = -negamax(5, color, alpha, beta);
        // state[move.from] = pieceMoving;
        // state[move.to] = boardSave;
        
        if(moveVal > bestVal)
        {
            bestMove = move;
            bestVal = moveVal;
        }

        // bestMove = moveVal > bestVal ? &move : bestMove;
        // bestVal = moveVal > bestVal ? moveVal : bestVal;
        
        popState();
    }
    if(bestVal != negInfinite){
        const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - searchStart).count();
        const double boardsPerSecond = seconds > 0.0 ? static_cast<double>(_countMoves) / seconds : 0.0;
        std::cout << "Moves checked: " << _countMoves
                  << " (" << std::fixed << std::setprecision(2) << boardsPerSecond
                  << " boards/s)" << std::defaultfloat << std::endl;

        
        // if(!bestMove)
        // {
        //     std::cout << "CHECKMATE\n";
        //     endTurn();
        //     return; 
        // }
        int srcSquare = bestMove.from;
        int dstSquare = bestMove.to;
        BitHolder& src = getHolderAt(srcSquare&7, srcSquare/8);
        BitHolder& dst = getHolderAt(dstSquare&7, dstSquare/8);
        Bit* bit = src.bit();
        dst.dropBitAtPoint(bit, ImVec2(0, 0));
        src.setBit(nullptr);
        
        bitMovedFromTo(*bit, src, dst);
    }
 
}

int Chess::evaluateBoard(const std::string& state)
{

    // ========  Material Evaluation  ========
    // int boardValues['z'-1];
    // paste mapping init here but for board values
        // map to vals instead, 10p, 40kn, 40b, 50r, 90q, 900k
            // do this but negative for black
        // empty squares is = 0
    
    int value = 0;
    for(int i=0; i<64; ++i)
    {
        value += materialValsMapping[state[i]];
        value += pieceSquareTables[state[i]][i];
        // i don't like this >:(
        // switch (state[i])
        // {
        //     case 'P':
        //         value += whitePawnBoard[i]*10;
        //         break;

        //     case 'N':
        //         value += whiteKnightBoard[i]*10;
        //         break;

        //     case 'B':
        //         value += whiteBishopBoard[i]*10;
        //         break;

        //     case 'R':
        //         value += whiteRookBoard[i]*10;
        //         break;

        //     case 'Q':
        //         value += whiteQueenBoard[i]*10;
        //         break;

        //     case 'K':
        //         value += whiteKingBoard[i]*10;
        //         break;

        //     case 'p':
        //         value -= blackPawnBoard[i]*10;
        //         break;

        //     case 'n':
        //         value -= blackKnightBoard[i]*10;
        //         break;

        //     case 'b':
        //         value -= blackBishopBoard[i]*10;
        //         break;

        //     case 'r':
        //         value -= blackRookBoard[i]*10;
        //         break;

        //     case 'q':
        //         value -= blackQueenBoard[i]*10;
        //         break;

        //     case 'k':
        //         value -= blackKingBoard[i]*10;
        //         break;
        //     default:
        //         break;
        // }
    }
    // for char in state
        // value += boardValues[char]
    return value*color;
}

// playerColor is either 1 or -1
int Chess::negamax(int depth, int playerColor, int alpha, int beta) 
{
    _countMoves++;
    if(depth == 0) return evaluateBoard(state);
    
    std::vector<BitMove> newMoves = generateAllMoves();
    // if(newMoves.size() == 0) return evaluateBoard(state);
    // if newMoves.size == 0, checkmate has happened, return maximum value?
    if(newMoves.size() == 0) return 100000000*color;
    int bestVal = -100000000;
    
    for(const auto& move : newMoves)
    {
        // push move
        pushMove(move);
        bestVal = std::max(bestVal, -negamax(depth-1, -playerColor, -beta, -alpha));
        // Undo move
        popState();
        // alpha beta cut-off
        alpha = std::max(alpha, bestVal);
        if(alpha > beta) 
        {
            // popState();
            break;
        }
        // popState();
    }
    // return bestVal code
    return bestVal;
    

    
    return 0;
}

// =====================  DEPRECATED  ================================
// code to generate moves and setup negamax here
    // for(const auto& move : newMoves) {
    //     gamestate.pushMove(move);
    //     bestVal = std::max(bestVal, -negamax(gamestate, depth - 1, -beta, -alpha));
    //     // Undo the move
    //     gamestate.popState();
    //     // alpha beta cut-off
    //     alpha = std::max(alpha, bestVal);
    //     if (alpha >= beta) {
    //         break;
    //     }
    // }
    // code to return bestVal here

    // if depth ==0 return evaluateBoard(state) * playerColor
    // auto newMoves = generateAllMoves(state, playerColor);
    // int bestVal = -1000000000;
    // // loop through moves
    // for(auto move : newMoves)
    // {
    //     // boardSave = state at move.to
    //     char boardSave = state[move.to];
    //     // peiceMoving = state at move.from
    //     char pieceMoving = state[move.from];
    //     // make the move
    //     // state at move.to = pieceMoving
    //     state[move.to] = pieceMoving;
    //     // state at move.from = '0';
    //     state[move.from] = '0';
    //     bestVal = std::max(negamax(state, depth-1, playerColor*-1, -beta, -alpha), bestVal);
    //     // reset move
    //     state[move.from] = pieceMoving;
    //     state[move.to] = boardSave;
    //     // alpha beta stuff
    //     alpha = std::max(alpha, bestVal);
    //     if(alpha > beta) break; // ew branching behavior, maybe can be done with no branching and a bool? unsure
    // }
    // return bestVal;

   // endTurn();

    // at the end of processing all the code and finding the best move
        // Make the best move
    // if(bestVal != negInfinite) {
    // const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - searchStart).count();
    // const double boardsPerSecond = seconds > 0.0 ? static_cast<double>(_countMoves) / seconds : 0.0;
    // std::cout << "Moves checked: " << _countMoves
    //           << " (" << std::fixed << std::setprecision(2) << boardsPerSecond
    //           << " boards/s)" << std::defaultfloat << std::endl;
    
    // int srcSquare = bestMove.from;
    // int dstSquare = bestMove.to;
    // BitHolder& src = getHolderAt(srcSquare&7, srcSquare/8);
    // BitHolder& dst = getHolderAt(dstSquare&7, dstSquare/8);
    // Bit* bit = src.bit();
    // dst.dropBitAtPoint(bit, ImVec2(0, 0));
    // src.setBit(nullptr);
    // bitMovedFromTo(*bit, src, dst);
    // }

    // loop through moves
        // char boardSave = state[move.to]
        // char pieceMoving = state[move.from]

        // Make the move
        // state[move.to] = pieceMoving;
        // state[move.from] = '0';
        // countMoves = 0;
        // moveVal = negamax(state, 0, WHITE)
        // state[move.from] = pieceMoving
        // state at move.to = boardSave

        // if val of current move is better than best value, update
        // if moveVal > bestVal
            // bestVal = moveVal
            // bestMove = move

    // if best val != -10000
        // print countMoves
        // char pieceMoving = state[bestmove.form]
        // state at bestmove to = piecemoving;
        // state at bestmove from = '0'
        // setstatestring(state)
        // endturn
        
// do I even need this?
// I don't
// I never figured this out :(
    // it was to do with state strings right?
// uint64_t Chess::getOccupancy()
// {
//     uint64_t occupancy = 0;
//     // loop through 64 bits to get data at each bit
//     // how would I get the data? stateString? BitHolder data?
//         // there shouldn't be a way to access board data with bit boards because this func is technically doing that
//     // would I have a white occupancy and black occupancy? i.e. this function would be called twice per game state?
//         // this is because white moves will get restricted by white occupancy and vice versa
//     return occupancy;
// }

// Kings

// Generate King BitBoards
// void Chess::generateKingMoveBitBoard()
// {
//     // offsets
//     std::pair<int, int> offsets[] = {
//         {-1, 1}, {0, 1}, {1, 1},
//         {-1, 0},         {1, 0},
//         {-1, -1}, {0, -1}, {1, -1}
//     };

//     for(int y=0; y<8; ++y)
//     {
//         for(int x=0; x<8; ++x)
//         {
//             for(auto [dx, dy] : offsets)
//             {
//                 // naiive implementation, not considering out of bounds
//                 // setting a bit: value |= (1 << n)

//                 // shifting by x then shifting by y
//                 // index is y level multiplied by 8 added x
//                 if(y + dy >= 0 && y + dy < 8 && x + dx >= 0 && x + dx < 8)
//                 {
//                     _kingBitBoards[(y*8)+x] |= (1ULL << ((dx+x) + 8*(dy+y)));
//                 }
//             }
//         }
//     }
// }

// Knights

// Generate Knight BitBoards
// void Chess::generateKnightMoveBitBoard()
// {
//     offsets are vector of pairs
//     constexpr doesn't work for this, but it doesn't have any operations?
//     by passing a literal array, probably trying to use it with a constructor for vector of pairs
//     std::pair<int, int> offsets[] = {
//         {-2, -1}, {-2, 1}, {2, -1}, {2, 1},
//         {-1, -2}, {-1, 2}, {1, -2}, {1, 2}
//     };

//     // {x, y}
//     // x offset is shifting by x
//     // y offset is shifting by 8*y

//     // for every index 
//     for(int y=0; y<8; ++y)
//     {
//         for(int x=0; x<8; ++x)
//         {
//             for(auto [dx, dy] : offsets)
//             {
//                 // naiive implementation, not considering out of bounds
//                 // setting a bit: value |= (1 << n)

//                 // shifting by x then shifting by y
//                 // index is y level multiplied by 8 added x
//                 if(y + dy >= 0 && y + dy < 8 && x + dx >= 0 && x + dx < 8)
//                 {
//                     _knightBitBoards[(y*8)+x] |= (1ULL << ((dx+x) + 8*(dy+y)));
//                 }
//             }
//         }
//     }
// }

// Pawns (deprecated)

// void Chess::generatePawnMoves(BitBoard& singlePush, BitBoard& doublePush, BitBoard& attackLeft, BitBoard& attackRight)
// {
//     // consider 3 possibles, move 1 rank, move 2 ranks, capture
//     // all should be put into move bit boards?
//     // different for white and black, shouldn't be like this?

//     // Final Implementation, check current turn and get movement/captures for that color
//     // try with ternaries
//     // getting occupancy
//     constexpr char whitePawnChar = 'P', blackPawnChar = 'p';
//     std::string state = stateString();
//     // you need full occupancy, white occupancy, black occupancy, pawns for current player, we want the current player number to determine some behavior
//         // current player number determines which side we're operating for and which occupancy we want to try and capture
//     BitBoard occupancy, pawns, whiteOccupancy, blackOccupancy;
//     char pawnChar = getCurrentPlayer()->playerNumber() == 0 ? whitePawnChar : blackPawnChar;
//     // making occupancy, whitePawns, and blackPawns, since we need different ones for each color
//     // uint64_t occupancy, whitePawns, blackPawns, whiteOccupancy, blackOccupancy;
//     for(int i=0; i<state.length(); ++i)
//     {
//         if(state[i] != '0')
//         {
//             occupancy |= 1ULL<<i;
//         }
//         if(state[i] == pawnChar)
//         {
//             pawns |= 1ULL<<i;
//         }
//         if(state[i] == whitePawnChar) // TODO: white occupancy is just the pawns here
//         {
//             whiteOccupancy |= 1ULL<<i;
//         }
//         if(state[i] == blackPawnChar) // TODO: black occupancy is just the pawns here
//         {
//             blackOccupancy |= 1ULL<<i;
//         }
//     }
//     uint64_t emptySquares = ~occupancy.getData();

//     // can't print const BitBoards? what in the print function does that?
//     BitBoard onSecondRank, 
//                     notAFilePawn(pawns.getData() & notAFile), 
//                     notHFilePawn(pawns.getData() & notHFile);

//     if(pawnChar == whitePawnChar)
//     {
//         singlePush.setData(pawns.getData() << 8);
//         singlePush &= emptySquares;
//         onSecondRank.setData((singlePush.getData() & RANK_3));
//         doublePush.setData(onSecondRank.getData() << 8);
//         doublePush &= emptySquares;
//         attackLeft = notAFilePawn.getData() << 7;
//         attackRight = notHFilePawn.getData() << 9;
//         attackLeft &= blackOccupancy.getData();
//         attackRight &= blackOccupancy.getData();
//     }
//     else
//     {
//         singlePush.setData(pawns.getData() >> 8);
//         singlePush &= emptySquares;
//         onSecondRank.setData((singlePush.getData() & RANK_6));
//         doublePush.setData(onSecondRank.getData() >> 8);
//         doublePush &= emptySquares;
//         attackLeft = notAFilePawn.getData() >> 7;
//         attackRight = notHFilePawn.getData() >> 9;
//         attackLeft &= whiteOccupancy.getData();
//         attackRight &= whiteOccupancy.getData();
//     }
    
//     Rank3 bit mask

//     attacking
//     A file mask
//     H file mask
//     masking current pawns
//     getting actual attacks
//     need to & these with enemy occupancy
    

//     uint64_t attackLeft = notAFilePawns << 7;
//     uint64_t attackRight = notHFilePawns << 9;
    



//     // white pawns operations
//     // deriving from notes and slides
//     // each hex bit represents 4 binary bits
//     // startingPawns doesn't mean currentPawns
//     uint64_t startingPawns = 0x000000000000FF00;
//     uint64_t singlePush = startingPawns << 8;
//     // sets bit to 0 if there's something there
//     singlePush &= emptySquares;
//     // Rank3 bit mask
//     uint64_t RANK_3 = 0x0000000000FF0000;
//     // if we did a single push and we're on rank 3, we were at starting pos
//     uint64_t onSecondRank = singlePush & RANK_3;
//     uint64_t doublePush = onSecondRank << 8;
//     // if something's there
//     doublePush &= emptySquares;

//     // attacking
//     // we need to get a BitBoard of current pawns?
//     // using starting pawns for now
//     uint64_t pawns = startingPawns;
//     // we need some masks to check if left or right edge
//     // A file mask
//     uint64_t notAFile = 0xFEFEFEFEFEFEFEFEULL;
//     // this in binary is:
//     /*
//         1111 1110 * 8
//     */
//     // H file mask
//     uint64_t notHFile = 0x7F7F7F7F7F7F7F7FULL;
//     // this in binary is:
//     /*
//         0111 1111 * 8
//     */
//     // isn't A on the left? Maybe something with how the nums are parsed onto the board
//         // yeah, parsed from right to left and projected onto board from left to right

//     // we want to & these file masks with current pawns before shifting for attacking
//     uint64_t notAFilePawns = pawns & notAFile;
//     uint64_t notHFilePawns = pawns & notHFile;

//     // these should be the valid attack BitBoards
//     uint64_t attackLeft = notAFilePawns << 7;
//     uint64_t attackRight = notHFilePawns << 9;
    
//     need to be BitBoards
//     need to get current pawns somehow
//         loop through stateString and get all p's and P's?
//         black needs to do right shifting, so we can differentiate those because of the different p's
    

//     just doing it for now
//     similar structure as the others
//     we're getting all possible moves from every index
//     Rank3 bit mask
//     uint64_t RANK_3 = 0x0000000000FF0000;
//     for(int y=0; y<8; ++y)
//     {
//         for(int x=0; x<8; ++x)
//         {
//             // shifting by x then shifting by y
//             // index is y level multiplied by 8 added x
//             // _knightBitBoards[(y*8)+x] |= (1ULL << ((dx+x) + 8*(dy+y)));
//             // just do single pushes first
//             // maybe need if statement?
//             if(y+1 < 8)
//             {
//                 // 1 shifted by 8 * y
//                 _pawnBitBoards[(y*8)+x] |= 1ULL << (8*y);
//             }
//         }
//     }
// }

// BitBoardElement Chess::generatePawnBitBoardPerFile(BitBoardElement& singlePush, BitBoardElement& doublePush, BitBoardElement& attackLeft, BitBoardElement& attackRight, int file)
// {
//     BitBoardElement defaultbb(0);
//     return defaultbb;
//     // given these, generatePawnMoves should've already been called and BitBoards should be valid
//     uint64_t currentFileMask = 0, leftFileMask = 0, rightFileMask = 0;
//     // file+=1;
//     for(int i=0; i<8; ++i)
//     {
//         // currentFileMask |= (7ULL << (file-1));
//         currentFileMask = currentFileMask << 8;
//         leftFileMask = leftFileMask << 8;
//         rightFileMask = rightFileMask << 8;
//         currentFileMask |= (1ULL << (file));
//         leftFileMask |= (1ULL << (file-1));
//         rightFileMask |= (1ULL << (file+1));
//     }
//     BitBoardElement pushes(currentFileMask & (singlePush.getData() | doublePush.getData())),
//                     leftCaptures(attackLeft.getData() & leftFileMask),
//                     rightCaptures(attackRight.getData() & rightFileMask);
//     BitBoardElement move(pushes.getData() | leftCaptures.getData() | rightCaptures.getData());
//     return move;
// }