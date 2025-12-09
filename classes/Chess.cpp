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
        setAIPlayer(0);
    }

    startGame();
    _allMoves.reserve(32);
    generateAllMoves(_allMoves);
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

    if (pieceColor == currentPlayer) return valid;
    return false;
}

void Chess::bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder& dst)
{
    // setting currentPlayer to the other player
    color = color == WHITE ? BLACK : WHITE;
    std::string playerColor = color == WHITE ? "WHITE" : "BLACK";
    std::cout << "currentPlayer: " << playerColor << std::endl;
    

    memcpy(state, stateString().c_str(), 64);
    // moves are set to generateAllmoves
    // generateAllMoves might also set _allMoves to the vector
    _allMoves.clear();
    generateAllMoves(_allMoves);
    if(_allMoves.size() == 0)
    {
        std::cout << "CheckMate!\n";
    }
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
void Chess::generateKnightMoves(std::vector<BitMove>& moves, const BitBoardElement knightBoard, const BitBoardElement enemies, const BitBoardElement occupancy) {
    // check if knightVoard is zero
        // return immediately
    // branching behavior, but it cuts off processing, so it's better?
    if (knightBoard.getData() == 0)
        return;
    knightBoard.forEachBit([&](int fromSquare) {
        BitBoardElement moveBitBoard = BitBoardElement(KnightAttacks[fromSquare] & ~occupancy.getData());
        // Efficiently iterate through only the set bits
        moveBitBoard.forEachBit([&](int toSquare) {
            enemies.getData() & 1ULL << toSquare ? moves.emplace_back(fromSquare, toSquare, Knight, IsCapture) : moves.emplace_back(fromSquare, toSquare, Knight);
        // moves.emplace_back(fromSquare, toSquare, Knight);
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

void Chess::generateKingMoves(std::vector<BitMove>& moves, const BitBoardElement kingBoard, const BitBoardElement enemies, const BitBoardElement friendlies)
{
    kingBoard.forEachBit([&](int fromSquare) {
        BitBoardElement moveBitBoard = BitBoardElement(KingAttacks[fromSquare] & ~friendlies.getData());
        // Efficiently iterate through only the set bits
        moveBitBoard.forEachBit([&](int toSquare) {
            enemies.getData() & 1ULL << toSquare ? moves.emplace_back(fromSquare, toSquare, King, IsCapture) : moves.emplace_back(fromSquare, toSquare, King);
        //    moves.emplace_back(fromSquare, toSquare, King);
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
        moveBitBoard.getData() & 1ULL << toSquareIdx ? moves.emplace_back(toSquareIdx-shift, toSquareIdx, Pawn, IsCapture) : moves.emplace_back(toSquareIdx-shift, toSquareIdx, Pawn);
        // moves.emplace_back(toSquareIdx-shift, toSquareIdx, Pawn);
    });
}

void Chess::generateRookMoves(std::vector<BitMove>& moves, const BitBoardElement rookBoard, const BitBoardElement friendlies, const BitBoardElement enemies, const BitBoardElement occupancy)
{

    rookBoard.forEachBit([&](int fromSquare){
        uint64_t index = (uint64_t)((occupancy.getData() & RMasks[fromSquare]) * RMagic[fromSquare]) >> (uint64_t)RShifts[fromSquare];
        BitBoardElement moveBitBoard(RAttacks[fromSquare][index] & ~friendlies.getData());
        moveBitBoard.forEachBit([&](int toSquare){
            enemies.getData() & 1ULL << toSquare ? moves.emplace_back(fromSquare, toSquare, Rook, IsCapture) : moves.emplace_back(fromSquare, toSquare, Rook);
            // moves.emplace_back(fromSquare, toSquare, Rook);
        });
    });
}

void Chess::generateBishopMoves(std::vector<BitMove>& moves, const BitBoardElement bishopBoard, const BitBoardElement friendlies, const BitBoardElement enemies, const BitBoardElement occupancy)
{
    bishopBoard.forEachBit([&](int fromSquare){
        uint64_t index = (uint64_t)((occupancy.getData() & BMasks[fromSquare]) * BMagic[fromSquare]) >> (uint64_t)BShifts[fromSquare];
        BitBoardElement moveBitBoard(BAttacks[fromSquare][index] & ~friendlies.getData());
        moveBitBoard.forEachBit([&](int toSquare){
            enemies.getData() & 1ULL << toSquare ? moves.emplace_back(fromSquare, toSquare, Bishop, IsCapture) : moves.emplace_back(fromSquare, toSquare, Bishop);
            // moves.emplace_back(fromSquare, toSquare, Bishop);
        });
    });
}

void Chess::generateQueenMoves(std::vector<BitMove>& moves, const BitBoardElement queenBoard, const BitBoardElement friendlies, const BitBoardElement enemies, const BitBoardElement occupancy)
{
    queenBoard.forEachBit([&](int fromSquare){
        uint64_t rIndex = (uint64_t)((occupancy.getData() & RMasks[fromSquare]) * RMagic[fromSquare]) >> (uint64_t)RShifts[fromSquare];
        BitBoardElement rMoveBitBoard(RAttacks[fromSquare][rIndex] & ~friendlies.getData());
        rMoveBitBoard.forEachBit([&](int toSquare){
            enemies.getData() & 1ULL << toSquare ? moves.emplace_back(fromSquare, toSquare, Queen, IsCapture) : moves.emplace_back(fromSquare, toSquare, Queen);
        });
        uint64_t bIndex = (uint64_t)((occupancy.getData() & BMasks[fromSquare]) * BMagic[fromSquare]) >> (uint64_t)BShifts[fromSquare];
        BitBoardElement bMoveBitBoard(BAttacks[fromSquare][bIndex] & ~friendlies.getData());
        bMoveBitBoard.forEachBit([&](int toSquare){
            enemies.getData() & 1ULL << toSquare ? moves.emplace_back(fromSquare, toSquare, Queen, IsCapture) : moves.emplace_back(fromSquare, toSquare, Queen);
            // get access to enemies, if toSquare is an enemy, add cap flag
            // _grid->getSquareByIndex(toSquare)->bit() ? moves.emplace_back(fromSquare, toSquare, Queen, IsCapture) : moves.emplace_back(fromSquare, toSquare, Queen);
            // moves.emplace_back(fromSquare, toSquare, Queen);
        });
    });
    
}

std::vector<BitMove> Chess::generateAllMoves(std::vector<BitMove>& moves)
{
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
    generateQueenMoves(moves, _tableBitBoards[WHITE_QUEEN + bitIndex], _tableBitBoards[WHITE_OCCUPANCY + bitIndex], _tableBitBoards[WHITE_OCCUPANCY + oppBitIndex], _tableBitBoards[ALL_OCCUPANCY]);
    generateRookMoves(moves, _tableBitBoards[WHITE_ROOK + bitIndex], _tableBitBoards[WHITE_OCCUPANCY + bitIndex], _tableBitBoards[WHITE_OCCUPANCY + oppBitIndex], _tableBitBoards[ALL_OCCUPANCY]);
    generateKnightMoves(moves, _tableBitBoards[WHITE_KNIGHT + bitIndex], _tableBitBoards[WHITE_OCCUPANCY + oppBitIndex], _tableBitBoards[WHITE_OCCUPANCY + bitIndex]);
    generateBishopMoves(moves, _tableBitBoards[WHITE_BISHOP + bitIndex], _tableBitBoards[WHITE_OCCUPANCY + bitIndex],_tableBitBoards[WHITE_OCCUPANCY + oppBitIndex],  _tableBitBoards[ALL_OCCUPANCY]);
    generatePawnMoveList(moves, _tableBitBoards[WHITE_PAWN + bitIndex], _tableBitBoards[EMPTY_SQUARES], _tableBitBoards[WHITE_OCCUPANCY + oppBitIndex], color);
    generateKingMoves(moves, _tableBitBoards[WHITE_KING + bitIndex], _tableBitBoards[WHITE_OCCUPANCY + oppBitIndex], _tableBitBoards[WHITE_OCCUPANCY + bitIndex]);
    
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
    _lastAIMove = BitMove();  // Reset last AI move
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
    std::vector<BitMove> newMoves;
    newMoves.reserve(32);
    generateAllMoves(newMoves);

    std::stable_partition(newMoves.begin(), newMoves.end(), [](const BitMove& m){ // prioritize captures
        return (m.flags & IsCapture) != 0;
    });

    // assert(newMoves.size() > 0);
    if(newMoves.size() == 0)
    {
        std::cout << "CHECKMATE";
        return;
    }
    _countMoves = 0;
    for(auto move : newMoves)
    {
        pushMove(move);

        int moveVal = -negamax(5, color, alpha, beta);
        
        if(moveVal > bestVal)
        {
            bestMove = move;
            bestVal = moveVal;
        }

        
        popState();
    }
    if(bestVal != negInfinite){
        const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - searchStart).count();
        const double boardsPerSecond = seconds > 0.0 ? static_cast<double>(_countMoves) / seconds : 0.0;
        std::cout << "Moves checked: " << _countMoves << " (~" << _countMoves / 1000000 << "M)"
                  << " (" << std::fixed << std::setprecision(2) << boardsPerSecond
                  << " boards/s)" << std::defaultfloat << std::endl;

        
        int srcSquare = bestMove.from;
        int dstSquare = bestMove.to;
        BitHolder& src = getHolderAt(srcSquare&7, srcSquare/8);
        BitHolder& dst = getHolderAt(dstSquare&7, dstSquare/8);
        Bit* bit = src.bit();
        dst.dropBitAtPoint(bit, ImVec2(0, 0));
        src.setBit(nullptr);
        _lastAIMove = bestMove;
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
    }
    return value*color;
}

// playerColor is either 1 or -1
int Chess::negamax(int depth, int playerColor, int alpha, int beta) 
{
    _countMoves++;
    if(depth == 0) return evaluateBoard(state);
    
    std::vector<BitMove> newMoves;
    newMoves.reserve(32);
    generateAllMoves(newMoves);

    std::stable_partition(newMoves.begin(), newMoves.end(), [](const BitMove& m){ // prioritize captures
        return (m.flags & IsCapture) != 0;
    });

    // if(newMoves.size() == 0) return evaluateBoard(state);
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
        if(alpha >= beta) 
        {
            break;
        }
    }
    // return bestVal code
    return bestVal;   
}

// Ideas for Optimization:
// Move Ordering!
    // we want to sort the moves given from generateAllMoves based on some criteria
    // we can just do based off capture
    // We can also try "Most Valuable Victim minues Least Valuable Attacker"
    // This allows for us to prune stuff more which means less moves checked

// Tournament Code
// Tournament support: Set board from FEN and reinitialize game state for AI
void Chess::setBoardFromFEN(const std::string& fen) {
    // Parse FEN string - can be full FEN or just piece placement
    std::string piecePlacement = fen;
    std::string activeColor = "w";
    std::string castling = "KQkq";
    std::string enPassant = "-";

    // Check if this is a full FEN string (has spaces)
    size_t spacePos = fen.find(' ');
    if (spacePos != std::string::npos) {
        // Parse full FEN
        std::istringstream fenStream(fen);
        fenStream >> piecePlacement >> activeColor >> castling >> enPassant;
    }

    // Set visual board from piece placement
    FENtoBoard(piecePlacement);

    // Determine current player from FEN
    color = (activeColor == "w" || activeColor == "W") ? WHITE : BLACK;

    // Reinitialize game state so AI sees correct board

    init(stateString().c_str(), color);
    // memcpy(state, stateString().c_str(), 64);
    // state = stateString().c_str();


    // TODO: Parse castling rights and en passant from FEN for more accurate state
    // For now, the basic state is sufficient for AI to calculate moves

    // Generate legal moves for the new position
    // _moves = _gamestate.generateAllMoves();
    _allMoves.clear();
    generateAllMoves(_allMoves);

    std::cout << "[Tournament] Board set from FEN. Player: "
              << (_currentPlayer == WHITE ? "White" : "Black")
              << ", Legal moves: " << _allMoves.size() << std::endl;
}

// Tournament support: Generate FEN string from current board
std::string Chess::getFEN() const {
    std::string fen;
    fen.reserve(90);

    // Piece placement (from rank 8 to rank 1)
    for (int rank = 7; rank >= 0; --rank) {
        int emptyCount = 0;
        for (int file = 0; file < 8; ++file) {
            char piece = pieceNotation(file, rank);
            if (piece == '0') {
                emptyCount++;
            } else {
                if (emptyCount > 0) {
                    fen += std::to_string(emptyCount);
                    emptyCount = 0;
                }
                fen += piece;
            }
        }
        if (emptyCount > 0) {
            fen += std::to_string(emptyCount);
        }
        if (rank > 0) {
            fen += '/';
        }
    }

    // Active color
    fen += ' ';
    fen += (_currentPlayer == WHITE) ? 'w' : 'b';

    // Castling availability (simplified - always report based on piece positions)
    fen += ' ';
    std::string castling;

    // Check if white can castle (king on e1, rooks on a1/h1)
    char e1 = pieceNotation(4, 0);
    char a1 = pieceNotation(0, 0);
    char h1 = pieceNotation(7, 0);
    if (e1 == 'K') {
        if (h1 == 'R') castling += 'K';
        if (a1 == 'R') castling += 'Q';
    }

    // Check if black can castle (king on e8, rooks on a8/h8)
    char e8 = pieceNotation(4, 7);
    char a8 = pieceNotation(0, 7);
    char h8 = pieceNotation(7, 7);
    if (e8 == 'k') {
        if (h8 == 'r') castling += 'k';
        if (a8 == 'r') castling += 'q';
    }

    fen += castling.empty() ? "-" : castling;

    // En passant target square (simplified - report as '-')
    fen += " -";

    // Halfmove clock (simplified)
    fen += " 0";

    // Fullmove number (simplified)
    fen += " 1";

    return fen;
}