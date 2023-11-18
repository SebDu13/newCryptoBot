#pragma once

namespace tools
{
    class Extrapolable
    {
        public:
        virtual double getX() const = 0;
        virtual double getY() const = 0;
    };

    class LinearExtrapoler
    {
        struct Bound
        {
            double x;
            double y;
        };

        public:
        // linear extrapolation according to two bounds of coordinates (x,y)
        LinearExtrapoler(const Extrapolable& lowBound, const Extrapolable& highBound);
        double extrapolate(double value) const;

        private:

        double _a, _b;
        Bound _highBound, _lowBound;
    };
}