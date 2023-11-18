#include "LinearExtrapoler.hpp"

namespace tools
{

LinearExtrapoler::LinearExtrapoler(const Extrapolable& lowBound, const Extrapolable& highBound)
: _lowBound{.x=lowBound.getX(), .y=lowBound.getY()}
, _highBound{.x=highBound.getX(), .y=highBound.getY()}
{
    _a = (_highBound.y - _lowBound.y) / (_highBound.x - _lowBound.x);
    _b = _highBound.y - _a * _highBound.x;
}

double LinearExtrapoler::extrapolate(double value) const
{
    if(value < _lowBound.x)
        return _lowBound.y;
    if(value > _highBound.x)
        return _highBound.y;

    return _a * value + _b;
}

}