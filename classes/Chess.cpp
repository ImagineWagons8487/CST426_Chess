#include "Chess.h"
#include <limits>
#include <cmath>

Chess::Chess()
{
    _grid = new Grid(8, 8);
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

void Chess::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;

    _grid->initializeChessSquares(pieceSize, "boardsquare.png");
    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

    startGame();
    generateKnightMoveBitBoard();
    generateKingMoveBitBoard();
    // BitboardElement singlePush, doublePush, attackLeft, attackRight;
    // generatePawnMoves(singlePush, doublePush, attackLeft, attackRight);
    // generatePawnBitboardPerFile(singlePush, doublePush, attackLeft, attackRight, 0);
    // generatePawnBitboardPerFile(singlePush, doublePush, attackLeft, attackRight, 2);
    // _knightBitboards[45].printBitboard();
}

ChessPiece charToPiece(const char c)
{
    // branching behavior :(
    switch(c)
    {
        case 'p':
            return Pawn;
        case 'r':
            return Rook;
        case 'n':
            return Knight;
        case 'b':
            return Bishop;
        case 'q':
            return Queen;
        case 'k':
            return King;
    }
    return NoPiece;
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
            Bit* bit = PieceForPlayer(playerNumber, charToPiece(tolower(fen[i])));
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
    // this is done because calling this as often as canBitMoveFromTo causes FRAME DROPS
    ChessPiece piece = (ChessPiece)(bit.gameTag() < 128 ? bit.gameTag() : bit.gameTag() - 128);
    if(piece == Pawn)
    {
        ChessSquare* srcSquare = dynamic_cast<ChessSquare*>(&src);
        BitboardElement singlePush, doublePush, attackLeft, attackRight;
        generatePawnMoves(singlePush, doublePush, attackLeft, attackRight);
        // _currentPawnMove = generatePawnBitboardPerFile(singlePush, doublePush, attackLeft, attackRight, srcSquare->getColumn());
    }

    if (pieceColor == currentPlayer) return true;
    return false;
}

bool Chess::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    // get current bit piece type, we use the game tag system to store that data
    ChessPiece piece = (ChessPiece)(bit.gameTag() < 128 ? bit.gameTag() : bit.gameTag() - 128);
    int pieceColor = bit.gameTag() & 128;
    bool valid = false;
    // depending on piece type, grab that bit board at src idx
    // branching behavior :(
    // get bit board for src, src needs to be casted into a chessSquare, will column and row be correct then?
    // chessSquare gets narrowed into bitholder and then expanded back into chessSquare, are we able to preserve col/row data?
        // we are because what's being passed in already has that data
    ChessSquare* srcSquare = dynamic_cast<ChessSquare*>(&src);
    ChessSquare* dstSquare = dynamic_cast<ChessSquare*>(&dst);
    BitboardElement* bb;
    switch(piece)
    {
        case Knight:
            // set bb to the right bitboard
            bb = &_knightBitboards[srcSquare->getSquareIndex()];
            break;
        case King:
            bb = &_kingBitboards[srcSquare->getSquareIndex()];
            break;
        case Pawn:
            // bb = &_currentPawnMove;
            break;
        default:
            bb = nullptr;
            break;

            
    }
    // if(piece == Knight)
    // {
    //     bb = &_knightBitboards[srcSquare->getSquareIndex()];
    // }
    // else if(piece == King)
    // {
    //     bb = &_kingBitboards[srcSquare->getSquareIndex()];
    // }
    // we do the efficient iteration, where we look at only set bits
    // forEachBit should be doing that
    if(bb)
    {
        bb->forEachBit([&](int idx)
        {
            // if lambda is called, then current idx has a set bit
            // check if idx is equal to dst idx
            if(idx == dstSquare->getSquareIndex())
            {
                valid = true;
            }
        });
    }
    return valid;
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

// Kings

// Generate King BitBoards
void Chess::generateKingMoveBitBoard()
{
    // offsets
    std::pair<int, int> offsets[] = {
        {-1, 1}, {0, 1}, {1, 1},
        {-1, 0},         {1, 0},
        {-1, -1}, {0, -1}, {1, -1}
    };

    for(int y=0; y<8; ++y)
    {
        for(int x=0; x<8; ++x)
        {
            for(auto [dx, dy] : offsets)
            {
                // naiive implementation, not considering out of bounds
                // setting a bit: value |= (1 << n)

                // shifting by x then shifting by y
                // index is y level multiplied by 8 added x
                if(y + dy >= 0 && y + dy < 8 && x + dx >= 0 && x + dx < 8)
                {
                    _kingBitboards[(y*8)+x] |= ((uint64_t)1 << ((dx+x) + 8*(dy+y)));
                }
            }
        }
    }
}

// Knights

// Generate Knight BitBoards
void Chess::generateKnightMoveBitBoard()
{
    // offsets are vector of pairs
    // constexpr doesn't work for this, but it doesn't have any operations?
    // by passing a literal array, probably trying to use it with a constructor for vector of pairs
    std::pair<int, int> offsets[] = {
        {-2, -1}, {-2, 1}, {2, -1}, {2, 1},
        {-1, -2}, {-1, 2}, {1, -2}, {1, 2}
    };

    // {x, y}
    // x offset is shifting by x
    // y offset is shifting by 8*y

    // for every index 
    for(int y=0; y<8; ++y)
    {
        for(int x=0; x<8; ++x)
        {
            for(auto [dx, dy] : offsets)
            {
                // naiive implementation, not considering out of bounds
                // setting a bit: value |= (1 << n)

                // shifting by x then shifting by y
                // index is y level multiplied by 8 added x
                if(y + dy >= 0 && y + dy < 8 && x + dx >= 0 && x + dx < 8)
                {
                    _knightBitboards[(y*8)+x] |= ((uint64_t)1 << ((dx+x) + 8*(dy+y)));
                }
            }
        }
    }
}

// Pawns

void Chess::generatePawnMoves(BitboardElement& singlePush, BitboardElement& doublePush, BitboardElement& attackLeft, BitboardElement& attackRight)
{
    // consider 3 possibles, move 1 rank, move 2 ranks, capture
    // all should be put into move bit boards?
    // different for white and black, shouldn't be like this?

    // Final Implementation, check current turn and get movement/captures for that color
    // try with ternaries
    // getting occupancy
    constexpr char whitePawnChar = 'P', blackPawnChar = 'p';
    std::string state = stateString();
    // you need full occupancy, white occupancy, black occupancy, pawns for current player, we want the current player number to determine some behavior
        // current player number determines which side we're operating for and which occupancy we want to try and capture
    BitboardElement occupancy, pawns, whiteOccupancy, blackOccupancy;
    char pawnChar = getCurrentPlayer()->playerNumber() == 0 ? whitePawnChar : blackPawnChar;
    // making occupancy, whitePawns, and blackPawns, since we need different ones for each color
    // uint64_t occupancy, whitePawns, blackPawns, whiteOccupancy, blackOccupancy;
    for(int i=0; i<state.length(); ++i)
    {
        if(state[i] != '0')
        {
            occupancy |= (uint64_t)1<<i;
        }
        if(state[i] == pawnChar)
        {
            pawns |= (uint64_t)1<<i;
        }
        if(state[i] == whitePawnChar)
        {
            whiteOccupancy |= (uint64_t)1<<i;
        }
        if(state[i] == blackPawnChar)
        {
            blackOccupancy |= (uint64_t)1<<i;
        }
    }
    uint64_t emptySquares = ~occupancy.getData();
    
    const BitboardElement RANK_3(0x0000000000FF0000), 
                            RANK_6(0x0000FF0000000000), 
                            notAFile(0xFEFEFEFEFEFEFEFEULL), 
                            notHFile(0x7F7F7F7F7F7F7F7FULL);
    // can't print const BitboardElements? what in the print function does that?
    BitboardElement onSecondRank, 
                    notAFilePawn(pawns.getData() & notAFile.getData()), 
                    notHFilePawn(pawns.getData() & notHFile.getData());

    if(pawnChar == whitePawnChar)
    {
        singlePush.setData(pawns.getData() << 8);
        singlePush &= emptySquares;
        onSecondRank.setData((singlePush.getData() & RANK_3.getData()));
        doublePush.setData(onSecondRank.getData() << 8);
        doublePush &= emptySquares;
        attackLeft = notAFilePawn.getData() << 7;
        attackRight = notHFilePawn.getData() << 9;
        attackLeft &= blackOccupancy.getData();
        attackRight &= blackOccupancy.getData();
    }
    else
    {
        singlePush.setData(pawns.getData() >> 8);
        singlePush &= emptySquares;
        onSecondRank.setData((singlePush.getData() & RANK_6.getData()));
        doublePush.setData(onSecondRank.getData() >> 8);
        doublePush &= emptySquares;
        attackLeft = notAFilePawn.getData() >> 7;
        attackRight = notHFilePawn.getData() >> 9;
        attackLeft &= whiteOccupancy.getData();
        attackRight &= whiteOccupancy.getData();
    }
    
    // Rank3 bit mask

    // attacking
    // A file mask
    // H file mask
    // masking current pawns
    // getting actual attacks
    // need to & these with enemy occupancy
    

    // uint64_t attackLeft = notAFilePawns << 7;
    // uint64_t attackRight = notHFilePawns << 9;
    



    // // white pawns operations
    // // deriving from notes and slides
    // // each hex bit represents 4 binary bits
    // // startingPawns doesn't mean currentPawns
    // uint64_t startingPawns = 0x000000000000FF00;
    // uint64_t singlePush = startingPawns << 8;
    // // sets bit to 0 if there's something there
    // singlePush &= emptySquares;
    // // Rank3 bit mask
    // uint64_t RANK_3 = 0x0000000000FF0000;
    // // if we did a single push and we're on rank 3, we were at starting pos
    // uint64_t onSecondRank = singlePush & RANK_3;
    // uint64_t doublePush = onSecondRank << 8;
    // // if something's there
    // doublePush &= emptySquares;

    // // attacking
    // // we need to get a bitboard of current pawns?
    // // using starting pawns for now
    // uint64_t pawns = startingPawns;
    // // we need some masks to check if left or right edge
    // // A file mask
    // uint64_t notAFile = 0xFEFEFEFEFEFEFEFEULL;
    // // this in binary is:
    // /*
    //     1111 1110 * 8
    // */
    // // H file mask
    // uint64_t notHFile = 0x7F7F7F7F7F7F7F7FULL;
    // // this in binary is:
    // /*
    //     0111 1111 * 8
    // */
    // // isn't A on the left? Maybe something with how the nums are parsed onto the board
    //     // yeah, parsed from right to left and projected onto board from left to right

    // // we want to & these file masks with current pawns before shifting for attacking
    // uint64_t notAFilePawns = pawns & notAFile;
    // uint64_t notHFilePawns = pawns & notHFile;

    // // these should be the valid attack bitboards
    // uint64_t attackLeft = notAFilePawns << 7;
    // uint64_t attackRight = notHFilePawns << 9;
    
    // need to be bitboardelements
    // need to get current pawns somehow
        // loop through stateString and get all p's and P's?
        // black needs to do right shifting, so we can differentiate those because of the different p's
    

    // just doing it for now
    // similar structure as the others
    // we're getting all possible moves from every index
    // Rank3 bit mask
    // uint64_t RANK_3 = 0x0000000000FF0000;
    // for(int y=0; y<8; ++y)
    // {
    //     for(int x=0; x<8; ++x)
    //     {
    //         // shifting by x then shifting by y
    //         // index is y level multiplied by 8 added x
    //         // _knightBitboards[(y*8)+x] |= ((uint64_t)1 << ((dx+x) + 8*(dy+y)));
    //         // just do single pushes first
    //         // maybe need if statement?
    //         if(y+1 < 8)
    //         {
    //             // 1 shifted by 8 * y
    //             _pawnBitBoards[(y*8)+x] |= (uint64_t)1 << (8*y);
    //         }
    //     }
    // }
}

BitboardElement Chess::generatePawnBitboardPerFile(BitboardElement& singlePush, BitboardElement& doublePush, BitboardElement& attackLeft, BitboardElement& attackRight, int file)
{
    // given these, generatePawnMoves should've already been called and bitboards should be valid
    uint64_t currentFileMask = 0, leftFileMask = 0, rightFileMask = 0;
    // file+=1;
    for(int i=0; i<8; ++i)
    {
        currentFileMask = currentFileMask << 8;
        leftFileMask = leftFileMask << 8;
        rightFileMask = rightFileMask << 8;
        currentFileMask |= ((uint64_t)1 << (file));
        leftFileMask |= ((uint64_t)1 << (file-1));
        rightFileMask |= ((uint64_t)1 << (file+1));
    }
    BitboardElement pushes(currentFileMask & (singlePush.getData() | doublePush.getData())),
                    leftCaptures(attackLeft.getData() & leftFileMask),
                    rightCaptures(attackRight.getData() & rightFileMask);
    BitboardElement move(pushes.getData() | leftCaptures.getData() | rightCaptures.getData());
    return move;
}

// Generate actual move objects from a bitboard
// emptySquares is ~occupancy, bits that represent emptySquares are set
void Chess::generateKnightMoves(std::vector<BitMove>& moves, BitboardElement knightBoard, uint64_t emptySquares) {
    // check if knightVoard is zero
        // return immediately
    knightBoard.forEachBit([&](int fromSquare) {
        BitboardElement moveBitboard = BitboardElement(_knightBitboards[fromSquare].getData() & emptySquares);
        // Efficiently iterate through only the set bits
        moveBitboard.forEachBit([&](int toSquare) {
           moves.emplace_back(fromSquare, toSquare, Knight);
        });
    });
}

void Chess::generateKingMoves(std::vector<BitMove>& moves, BitboardElement kingBoard, uint64_t emptySquares)
{
    kingBoard.forEachBit([&](int fromSquare) {
        BitboardElement moveBitboard = BitboardElement(_kingBitboards[fromSquare].getData() & emptySquares);
        // Efficiently iterate through only the set bits
        moveBitboard.forEachBit([&](int toSquare) {
           moves.emplace_back(fromSquare, toSquare, Knight);
        });
    });
}

std::vector<BitMove> Chess::generateAllMoves()
{
    
}

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