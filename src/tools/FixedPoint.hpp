#pragma once
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <iostream>
#include <iomanip>

namespace tools{
struct FixedPoint
{
    using ValueType = boost::multiprecision::cpp_dec_float_50;
    static inline size_t minPrecision = 4;

    explicit FixedPoint():value(0) {} ;
    explicit FixedPoint(std::string str):value(str) {} ;
    FixedPoint(int value):value(value) {} ;
    FixedPoint(ValueType value):value(value) {} ;

    friend std::ostream& operator<<(std::ostream& os, const FixedPoint& other);
    explicit operator double() const{ return value.convert_to<double>();};
    FixedPoint operator*(const FixedPoint& other) const{ return FixedPoint(value * other.value); };
    FixedPoint operator+(const FixedPoint& other) const{ return FixedPoint(value + other.value); };
    FixedPoint operator-(const FixedPoint& other) const{ return FixedPoint(value - other.value); };
    FixedPoint operator/(const FixedPoint& other) const{ return FixedPoint(value / other.value); };
    bool operator==(const FixedPoint& other) const{ return value == other.value; };
    bool operator<(const FixedPoint& other) const{ return value < other.value; };
    bool operator>(const FixedPoint& other) const{ return value > other.value; };
    bool operator>=(const FixedPoint& other) const{ return value >= other.value; };
    bool operator<=(const FixedPoint& other) const{ return value <= other.value; };
    
    bool nearZero();
    std::string toString() const;
    std::string toStringExact() const;

    ValueType value;
};

std::ostream& operator<<(std::ostream& os, const FixedPoint& other);

}

using Price = tools::FixedPoint;
using Quantity = tools::FixedPoint;