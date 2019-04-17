#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

#include "types/TypedValue.hpp"
#include "histogram/HypedValue.hpp"
#include "types/TypeID.hpp"

namespace quickstep {


bool operator==(const HypedValue &h1, const HypedValue &h2) {
    TypedValue v1 = h1.v;
    TypedValue v2 = h2.v;
    TypeID t = v1.getTypeID();
    assert(t == v2.getTypeID());
    switch(t) {
      case kInt:
        return v1.getLiteral<int>() == v2.getLiteral<int>();
      case kLong:
        return v1.getLiteral<std::int64_t>() == v2.getLiteral<std::int64_t>();
      case kFloat:
        return v1.getLiteral<float>() == v2.getLiteral<float>();
      case kDouble:
        return v1.getLiteral<double>() == v2.getLiteral<double>();
      case kDate:
        return v1.getLiteral<DateLit>() == v2.getLiteral<DateLit>();
      case kDatetime:
        return v1.getLiteral<DatetimeLit>() == v2.getLiteral<DatetimeLit>();
      case kDatetimeInterval:
        return v1.getLiteral<DatetimeIntervalLit>() ==
            v2.getLiteral<DatetimeIntervalLit>();
      case kYearMonthInterval:
        return v1.getLiteral<YearMonthIntervalLit>() ==
            v2.getLiteral<YearMonthIntervalLit>();
      default:
        return false; // unimplemented
    }
}

bool operator!=(const HypedValue &h1, const HypedValue &h2) {
    return !(h1 == h2);
}

bool operator<(const HypedValue &h1, const HypedValue &h2) {
    TypedValue v1 = h1.v;
    TypedValue v2 = h2.v;
    TypeID t = v1.getTypeID();
    assert(t == v2.getTypeID());
    switch(t) {
      case kInt:
        return v1.getLiteral<int>() < v2.getLiteral<int>();
      case kLong:
        return v1.getLiteral<std::int64_t>() < v2.getLiteral<std::int64_t>();
      case kFloat:
        return v1.getLiteral<float>() < v2.getLiteral<float>();
      case kDouble:
        return v1.getLiteral<double>() < v2.getLiteral<double>();
      case kDate:
        return v1.getLiteral<DateLit>() < v2.getLiteral<DateLit>();
      case kDatetime:
        return v1.getLiteral<DatetimeLit>() < v2.getLiteral<DatetimeLit>();
      case kDatetimeInterval:
        return v1.getLiteral<DatetimeIntervalLit>() <
            v2.getLiteral<DatetimeIntervalLit>();
      case kYearMonthInterval:
        return v1.getLiteral<YearMonthIntervalLit>() <
            v2.getLiteral<YearMonthIntervalLit>();
      default:
        return false; // unimplemented
    }
}

bool operator<=(const HypedValue &h1, const HypedValue &h2) {
    return h1 < h2 || h1 == h2;
}

bool operator>(const HypedValue &h1, const HypedValue &h2) {
    return !(h1 <= h2);
}

bool operator>=(const HypedValue &h1, const HypedValue &h2) {
    return h1 > h2 || h1 == h2;
}

/* Shouldn't need this.
double operator+(const HypedValue &h1, const HypedValue &h2) {
    TypedValue v1 = h1.v;
    TypedValue v2 = h2.v;
    TypeID t = v1.getTypeID();
    assert(t == v2.getTypeID());
    switch(t) {
      case kInt:
        return v1.getLiteral<int>() + v2.getLiteral<int>();
      case kFloat:
        return v1.getLiteral<float>() + v2.getLiteral<float>();
      case kLong:
        return v1.getLiteral<std::int64_t>() + v2.getLiteral<std::int64_t>();
      case kDouble:
        return v1.getLiteral<double>() + v2.getLiteral<double>();
      default:
        return 0.0; // unimplemented
    }
}

double operator-(const HypedValue &h1, const HypedValue &h2) {
    TypedValue v1 = h1.v;
    TypedValue v2 = h2.v;
    TypeID t = v1.getTypeID();
    assert(t == v2.getTypeID());
    switch(t) {
      case kInt:
        return v1.getLiteral<int>() - v2.getLiteral<int>();
      case kFloat:
        return v1.getLiteral<float>() - v2.getLiteral<float>();
      case kLong:
        return v1.getLiteral<std::int64_t>() - v2.getLiteral<std::int64_t>();
      case kDouble:
        return v1.getLiteral<double>() - v2.getLiteral<double>();
      default:
        return 0.0; // unimplemented
    }
}
*/

std::ostream& operator<<(std::ostream &os, const HypedValue &h) {
    os << "HypedValue{";
    switch(h.v.getTypeID()) {
        case kInt:
            os << h.v.getLiteral<int>();
			break;
        case kLong:
            os << h.v.getLiteral<std::int64_t>();
			break;
        case kFloat:
            os << h.v.getLiteral<float>();
			break;
        case kDouble:
            os << h.v.getLiteral<double>();
			break;
        default: // unimplemented
            break;
    }
    os << "}";
    return os;
}
} //namespace
