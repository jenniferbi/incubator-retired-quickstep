
#ifndef QUICKSTEP_HTREE_HYPEDVALUE_HPP_
#define QUICKSTEP_HTREE_HYPEDVALUE_HPP_

#include "types/TypedValue.hpp"
#include "types/TypeID.hpp"
namespace quickstep {

/*  _   _                      ___     __    _
 * | | | |_   _ _ __   ___  __| \ \   / /_ _| |_   _  ___
 * | |_| | | | | '_ \ / _ \/ _` |\ \ / / _` | | | | |/ _ \
 * |  _  | |_| | |_) |  __/ (_| | \ V / (_| | | |_| |  __/
 * |_| |_|\__, | .__/ \___|\__,_|  \_/ \__,_|_|\__,_|\___|
 *        |___/|_|
 *  FIGLET: HypedValue
 *
 *  A wrapper for the TypedValue class with comparison/arithmetic operators.
 */

class HypedValue {
    TypedValue v;

public:
    HypedValue() : v(TypedValue()) {}

    HypedValue(TypedValue v) : v(v) {}

    HypedValue(const HypedValue &h) : v(h.v) {}
	
	const TypedValue getTypedValue() const { return v; }

    friend bool operator==(const HypedValue &h1, const HypedValue &h2);
    friend bool operator!=(const HypedValue &h1, const HypedValue &h2);
    friend bool operator<(const HypedValue &h1, const HypedValue &h2);
    friend bool operator<=(const HypedValue &h1, const HypedValue &h2);
    friend bool operator>(const HypedValue &h1, const HypedValue &h2);
    friend bool operator>=(const HypedValue &h1, const HypedValue &h2);
    // shouldn't need this.
    //friend double operator+(const HypedValue &h1, const HypedValue &h2);
    //friend double operator-(const HypedValue &h1, const HypedValue &h2);
    //
    //static double width(const HypedValue &h1, const HypedValue &h2);
    friend std::ostream& operator<<(std::ostream &os, const HypedValue &h2);

};

bool operator==(const HypedValue &h1, const HypedValue &h2);
bool operator!=(const HypedValue &h1, const HypedValue &h2);
bool operator<(const HypedValue &h1, const HypedValue &h2);
bool operator<=(const HypedValue &h1, const HypedValue &h2);
bool operator>(const HypedValue &h1, const HypedValue &h2);
bool operator>=(const HypedValue &h1, const HypedValue &h2);
    // shouldn't need this.
    //friend double operator+(const HypedValue &h1, const HypedValue &h2);
    //friend double operator-(const HypedValue &h1, const HypedValue &h2);
    //
std::ostream& operator<<(std::ostream &os, const HypedValue &h2);

// For int values, the interval [x,y] contains y-x+1 points.
inline int width(int x, int y) {
    return y - x + 1;
}

inline double width(double x, double y) {
    return y - x;
}

inline double width(const HypedValue &h1, const HypedValue &h2) {
    if (h1 > h2) {
        return 0.0;
    }
    TypedValue v1 = h1.getTypedValue();
    TypedValue v2 = h2.getTypedValue();
    TypeID t = v1.getTypeID();
    assert(t == v2.getTypeID());
    // For integer values, the interval [x,y] contains y-x+1 points.
    switch(t) {
        case kInt:
            return v2.getLiteral<int>() - v1.getLiteral<int>() + 1;
        case kLong:
            return v2.getLiteral<std::int64_t>() -
                v1.getLiteral<std::int64_t>() + 1;
        case kFloat:
            return v2.getLiteral<float>() - v1.getLiteral<float>();
        case kDouble:
            assert(1 == 0);
            return v2.getLiteral<double>() - v1.getLiteral<double>();
        default:
            return 0.0; // unimplemented
    }
}


} // namespace quickstep
#endif  // QUICKSTEP_HTREE_HTREE_HPP_
