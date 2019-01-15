#pragma once
#ifndef FILTER_H_INCLUDED
#define FILTER_H_INCLUDED

enum FilterType : std::uint16_t {
    NONE = 0,
    BUTTERWORTH = 1,
    CHEBYSHEV = 2
};

#pragma pack(push,1)
struct Filter {
    std::uint32_t cornerFreq;
    std::uint32_t order;
    FilterType type;
    friend std::ostream& operator<<(std::ostream& out, const Filter& filter) {
        switch(filter.type) {
            case NONE:
                out << "None";
                break;
            case BUTTERWORTH:
                out <<  "Butterworth (Corner Frequency: " << (filter.cornerFreq)/1000.0  << "Hz Order: " << filter.order << ')';
                break;
            case CHEBYSHEV:
                out << "Butterworth (Corner Frequency: " << filter.cornerFreq/1000.0 << "Hz Order: " << filter.order << ')';
        }
        return out;
    }
};
#pragma pack(pop)

#endif
