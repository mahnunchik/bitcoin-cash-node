// Copyright 2014 BitPay Inc.
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://opensource.org/licenses/mit-license.php.

#include <cstring>
#include <sstream>
#include <stdio.h>
#include "univalue.h"
#include "univalue_escapes.h"

// Opaque type used for writing. This can be further optimized later.
struct UniValue::Stream {
    std::string & str; // this is a reference for RVO to always work in UniValue::write() below
    inline void put(char c) { str.push_back(c); }
    inline void put(char c, size_t nFill) { str.append(nFill, c); }
    inline void write(const char *s, size_t len) { str.append(s, len); }
    inline Stream & operator<<(const char *s) { str.append(s); return *this; }
    inline Stream & operator<<(const std::string &s) { str.append(s); return *this; }
};

/* static */
void UniValue::jsonEscape(Stream & ss, const std::string & inS)
{
    for (const auto ch : inS) {
        const char * const escStr = escapes[uint8_t(ch)];

        if (escStr)
            ss << escStr;
        else
            ss.put(ch);
    }
}

std::string UniValue::write(unsigned int prettyIndent, unsigned int indentLevel) const
{
    std::string s; // we do it this way for RVO to work on all compilers
    Stream ss{s};
    s.reserve(1024);
    writeStream(ss, prettyIndent, indentLevel);
    return s;
}

void UniValue::writeStream(Stream & ss, unsigned int prettyIndent, unsigned int indentLevel) const
{
    unsigned int modIndent = indentLevel;
    if (modIndent == 0)
        modIndent = 1;

    switch (typ) {
    case VNULL:
        ss.write("null", 4); // .write() is slightly faster than operator<<
        break;
    case VOBJ:
        writeObject(ss, prettyIndent, modIndent);
        break;
    case VARR:
        writeArray(ss, prettyIndent, modIndent);
        break;
    case VSTR:
        ss.put('"'); jsonEscape(ss, val); ss.put('"');
        break;
    case VNUM:
        ss << val;
        break;
    case VBOOL:
        if (val == "1")
            ss.write("true", 4);
        else
            ss.write("false", 5);
        break;
    }
}

/* static */
inline void UniValue::indentStr(Stream & ss, unsigned int prettyIndent, unsigned int indentLevel)
{
    ss.put(' ', prettyIndent * indentLevel);
}

void UniValue::writeArray(Stream & ss, unsigned int prettyIndent, unsigned int indentLevel) const
{
    ss.put('[');
    if (prettyIndent)
        ss.put('\n');

    for (size_t i = 0, nValues = values.size(); i < nValues; ++i) {
        if (prettyIndent)
            indentStr(ss, prettyIndent, indentLevel);
        values[i].writeStream(ss, prettyIndent, indentLevel + 1);
        if (i != (nValues - 1)) {
            ss.put(',');
        }
        if (prettyIndent)
            ss.put('\n');
    }

    if (prettyIndent)
        indentStr(ss, prettyIndent, indentLevel - 1);
    ss.put(']');
}

void UniValue::writeObject(Stream & ss, unsigned int prettyIndent, unsigned int indentLevel) const
{
    ss.put('{');
    if (prettyIndent)
        ss.put('\n');

    // Note: if typ == VOBJ, then keys.size() == values.size() always, so we can
    // use the non-bounds-checking operator[]() for both keys and values here safely.
    for (size_t i = 0, nItems = keys.size(); i < nItems; ++i) {
        if (prettyIndent)
            indentStr(ss, prettyIndent, indentLevel);
        ss.put('"'); jsonEscape(ss, keys[i]); ss.write("\":", 2);
        if (prettyIndent)
            ss.put(' ');
        values[i].writeStream(ss, prettyIndent, indentLevel + 1);
        if (i != (nItems - 1))
            ss.put(',');
        if (prettyIndent)
            ss.put('\n');
    }

    if (prettyIndent)
        indentStr(ss, prettyIndent, indentLevel - 1);
    ss.put('}');
}
