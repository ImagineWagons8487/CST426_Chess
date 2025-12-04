#pragma once

#ifdef _MSC_VER
#include <intrin.h>
#endif
#include <iostream>

enum ChessPiece
{
    NoPiece,
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};
class BitBoardElement {
  public:
    // Constructors
    BitBoardElement()
        : _data(0) { }
    BitBoardElement(uint64_t data)
        : _data(data) { }

    // Getters and Setters
    uint64_t getData() const { return _data; }
    void setData(uint64_t data) { _data = data; }

    BitBoardElement& operator|=(const uint64_t other) {
        _data |= other;
        return *this;
    }

    BitBoardElement& operator&=(const uint64_t other) {
        _data &= other;
        return *this;
    }

    BitBoardElement& operator^=(const uint64_t other) {
        _data ^= other;
        return *this;
    }
        
    BitBoardElement operator<<(const int shift) const {
        return BitBoardElement(_data << shift);
    }
    BitBoardElement operator>>(const int shift) const {
        return BitBoardElement(_data >> shift);
    }

    bool anyCommonBits(const BitBoardElement& other) const {
        return (_data & other._data) != 0;
    }

    BitBoardElement operator|(const BitBoardElement& other) const {
        return BitBoardElement(_data | other._data);
    }
    BitBoardElement operator&(const BitBoardElement& other) const {
        return BitBoardElement(_data & other._data);
    }
    BitBoardElement operator&(const uint64_t other) const {
        return BitBoardElement(_data & other);
    }
    BitBoardElement& operator&=(const BitBoardElement& other) {
        _data &= other._data;
        return *this;
    }
    BitBoardElement& operator|=(const BitBoardElement& other) {
        _data |= other._data;
        return *this;
    }
    BitBoardElement operator~() const {
        return BitBoardElement(~_data);
    } 

    const int firstBit() const {
        return bitScanForward(_data);
    }
    
    // Method to loop through each bit in the element and perform an operation on it.
    template <typename Func>
    void forEachBit(Func func) const {
        if (_data != 0) {
            uint64_t tempData = _data;
            while (tempData) {
                int index = bitScanForward(tempData);
                func(index);
                tempData &= tempData - 1;
            }
        }
    }


    void printBitboard() {
        std::cout << "\n  a b c d e f g h\n";
        for (int rank = 7; rank >= 0; rank--) {
            std::cout << (rank + 1) << " ";
            for (int file = 0; file < 8; file++) {
                int square = rank * 8 + file;
                if (_data & (1ULL << square)) {
                    std::cout << "X ";
                } else {
                    std::cout << ". ";
                }
            }
            std::cout << (rank + 1) << "\n";
            std::cout << std::flush;
        }
        std::cout << "  a b c d e f g h\n";
        std::cout << std::flush;
    }

    inline int bitScanForward(uint64_t bb) const {
#if defined(_MSC_VER) && !defined(__clang__)
        unsigned long index;
        _BitScanForward64(&index, bb);
        return index;
#else
        return __builtin_ffsll(bb) - 1;
#endif
    };

private:
    uint64_t    _data;

};
// class BitboardElement {
//   public:
//     // Constructors
//     BitboardElement()
//         : _data(0) { }
//     BitboardElement(uint64_t data)
//         : _data(data) { }

//     // Getters and Setters
//     uint64_t getData() const { return _data; }
//     void setData(uint64_t data) { _data = data; }

//     // Method to loop through each bit in the element and perform an operation on it.
//     template <typename Func>
//     void forEachBit(Func func) const {
//         if (_data != 0) {
//             uint64_t tempData = _data;
//             while (tempData) {
//                 int index = bitScanForward(tempData);
//                 func(index);
//                 tempData &= tempData - 1;
//             }
//         }
//     }

//     BitboardElement& operator|=(const uint64_t other) {
//         _data |= other;
//         return *this;
//     }
    
//     // need to make the other bitwise operators

//     // bitwise and equals
//     BitboardElement& operator&=(const uint64_t other) {
//         _data &= other;
//         return *this;
//     }
    
//     void printBitboard() {
//         std::cout << "\n  a b c d e f g h\n";
//         for (int rank = 7; rank >= 0; rank--) {
//             std::cout << (rank + 1) << " ";
//             for (int file = 0; file < 8; file++) {
//                 int square = rank * 8 + file;
//                 if (_data & (1ULL << square)) {
//                     std::cout << "X ";
//                 } else {
//                     std::cout << ". ";
//                 }
//             }
//             std::cout << (rank + 1) << "\n";
//             std::cout << std::flush;
//         }
//         std::cout << "  a b c d e f g h\n";
//         std::cout << std::flush;
//     }

// private:
//     uint64_t    _data;

//     inline int bitScanForward(uint64_t bb) const {
// #if defined(_MSC_VER) && !defined(__clang__)
//         unsigned long index;
//         _BitScanForward64(&index, bb);
//         return index;
// #else
//         return __builtin_ffsll(bb) - 1;
// #endif
//     };

// };
#pragma pack(push, 1) // make everything aligned on 1 byte
struct BitMove {
    unsigned char from;
    unsigned char to;
    unsigned char piece;
    unsigned char flags;

    BitMove(int from, int to, ChessPiece piece, int flags = 0)
        : from(from), to(to), piece(piece), flags(flags) { }
        
    BitMove() : from(0), to(0), piece(NoPiece), flags(0) { }
    
    bool operator==(const BitMove& other) const {
        return from == other.from && 
               to == other.to && 
               piece == other.piece &&
               flags == other.flags;
    }
};
#pragma pack(pop) // stop packing things
// #pragma pack(push, 1)
// struct BitMove {
//     uint8_t from;
//     uint8_t to;
//     uint8_t piece; 
//     // pieces are stored as values in binary number
//     // but if they're stored as values instead of bits, it all fits within 3 bits
//     // would I be saving space or being more efficient by making piece a uint4_t?
    
//     BitMove(int from, int to, ChessPiece piece)
//         : from(from), to(to), piece(piece) { }
        
//     BitMove() : from(0), to(0), piece(NoPiece) { }
    
//     bool operator==(const BitMove& other) const {
//         return from == other.from && 
//                to == other.to && 
//                piece == other.piece;
//     }
// };

// #pragma pack(pop)