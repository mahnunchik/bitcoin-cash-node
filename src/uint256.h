// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_UINT256_H
#define BITCOIN_UINT256_H

#include <cassert>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

/** Template base class for fixed-sized opaque blobs. */
template <unsigned int BITS> class base_blob {
protected:
    static constexpr int WIDTH = BITS / 8;
    uint8_t data[WIDTH];

public:
    constexpr base_blob() noexcept : data{0} {}

    /// type tag + convenience member for uninitialized c'tor
    static constexpr struct Uninitialized_t {} Uninitialized{};

    /// Uninitialized data constructor -- to be used when we want to avoid a
    /// redundant zero-initialization in cases where we know we will fill-in
    /// the data immediately anyway (e.g. for random generators, etc).
    /// Select this c'tor with e.g.: uint256 foo{uint256::Uninitialized}
    explicit constexpr base_blob(Uninitialized_t /* type tag to select this c'tor */) noexcept {}

    explicit base_blob(const std::vector<uint8_t> &vch) noexcept;

    bool IsNull() const {
        for (int i = 0; i < WIDTH; i++) {
            if (data[i] != 0) {
                return false;
            }
        }
        return true;
    }

    void SetNull() { memset(data, 0, sizeof(data)); }

    inline int Compare(const base_blob &other) const {
        for (size_t i = 0; i < sizeof(data); i++) {
            uint8_t a = data[sizeof(data) - 1 - i];
            uint8_t b = other.data[sizeof(data) - 1 - i];
            if (a > b) {
                return 1;
            }
            if (a < b) {
                return -1;
            }
        }

        return 0;
    }

    friend inline bool operator==(const base_blob &a, const base_blob &b) {
        return a.Compare(b) == 0;
    }
    friend inline bool operator!=(const base_blob &a, const base_blob &b) {
        return a.Compare(b) != 0;
    }
    friend inline bool operator<(const base_blob &a, const base_blob &b) {
        return a.Compare(b) < 0;
    }
    friend inline bool operator<=(const base_blob &a, const base_blob &b) {
        return a.Compare(b) <= 0;
    }
    friend inline bool operator>(const base_blob &a, const base_blob &b) {
        return a.Compare(b) > 0;
    }
    friend inline bool operator>=(const base_blob &a, const base_blob &b) {
        return a.Compare(b) >= 0;
    }

    std::string GetHex() const;
    void SetHex(const char *psz);
    void SetHex(const std::string &str);
    std::string ToString() const { return GetHex(); }

    uint8_t *begin() { return &data[0]; }

    uint8_t *end() { return &data[WIDTH]; }

    const uint8_t *begin() const { return &data[0]; }

    const uint8_t *end() const { return &data[WIDTH]; }

    static constexpr unsigned int size() { return sizeof(data); }

    uint64_t GetUint64(int pos) const {
        const uint8_t *ptr = data + pos * 8;
        return uint64_t(ptr[0]) | (uint64_t(ptr[1]) << 8) |
               (uint64_t(ptr[2]) << 16) | (uint64_t(ptr[3]) << 24) |
               (uint64_t(ptr[4]) << 32) | (uint64_t(ptr[5]) << 40) |
               (uint64_t(ptr[6]) << 48) | (uint64_t(ptr[7]) << 56);
    }

    template <typename Stream> void Serialize(Stream &s) const {
        s.write((char *)data, sizeof(data));
    }

    template <typename Stream> void Unserialize(Stream &s) {
        s.read((char *)data, sizeof(data));
    }
};

/**
 * 160-bit opaque blob.
 * @note This type is called uint160 for historical reasons only. It is an
 * opaque blob of 160 bits and has no integer operations.
 */
class uint160 : public base_blob<160> {
public:
    using base_blob<160>::base_blob; ///< inherit constructors
};

/**
 * 256-bit opaque blob.
 * @note This type is called uint256 for historical reasons only. It is an
 * opaque blob of 256 bits and has no integer operations. Use arith_uint256 if
 * those are required.
 */
class uint256 : public base_blob<256> {
public:
    using base_blob<256>::base_blob; ///< inherit constructors
};

/**
 * uint256 from const char *.
 * This is a separate function because the constructor uint256(const char*) can
 * result in dangerously catching uint256(0).
 */
inline uint256 uint256S(const char *str) {
    uint256 rv{uint256::Uninitialized};
    rv.SetHex(str);
    return rv;
}

/**
 * uint256 from std::string.
 * This is a separate function because the constructor uint256(const std::string
 * &str) can result in dangerously catching uint256(0) via std::string(const
 * char*).
 */
inline uint256 uint256S(const std::string &str) {
    uint256 rv{uint256::Uninitialized};
    rv.SetHex(str);
    return rv;
}

inline uint160 uint160S(const char *str) {
    uint160 rv{uint160::Uninitialized};
    rv.SetHex(str);
    return rv;
}
inline uint160 uint160S(const std::string &str) {
    uint160 rv{uint160::Uninitialized};
    rv.SetHex(str);
    return rv;
}

#endif // BITCOIN_UINT256_H
