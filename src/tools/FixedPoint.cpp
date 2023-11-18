#include "FixedPoint.hpp"
#include <cfenv>
#include "logger.hpp"

namespace
{
    std::string setPrecision(unsigned int precision, const std::string& value)
    {
        if(precision != 0)
        {
            ++precision;
            if(auto pointPosition = value.find(".", 0); pointPosition != std::string::npos
                && pointPosition + precision < value.size())
                return value.substr(0, pointPosition + precision);
        }

        return value;
    }
}

namespace tools
{
std::ostream& operator<<(std::ostream& os, const FixedPoint& other)
{
    os << other.toString();
    return os;
}

std::string FixedPoint::toString() const
{ 
    std::ostringstream os;
    os << setPrecision(minPrecision, toStringExact());
    return os.str();
};

std::string FixedPoint::toStringExact() const
{
    std::ostringstream os;
    os << std::fixed << std::setprecision(std::numeric_limits<ValueType>::max_digits10) << value;
    return os.str();
};

bool FixedPoint::nearZero()
{
    return *this < FixedPoint("0.0001") && *this > FixedPoint("-0.0001");
}

}