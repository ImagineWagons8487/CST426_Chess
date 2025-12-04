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

