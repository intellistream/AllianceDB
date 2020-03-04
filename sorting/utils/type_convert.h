//
// Created by Shuhao Zhang on 25/11/19.
//

#ifndef ALLIANCEDB_TYPE_CONVERT_H
#define ALLIANCEDB_TYPE_CONVERT_H

#include <limits>
#include <cstdint>

const bool DoubleToInt64(const double& d, int64_t & i64)
{
    // double representation of max acceptable value
    // for double to int64 conversion
    static int64_t iMax(0x43DFFFFFFFFFFFFF);

    static const double dMax = *reinterpret_cast<double*>(&iMax);
    static const double dMin = -dMax;

    static const bool is_IEEE754 =    std::numeric_limits<double>::is_specialized
                                      && std::numeric_limits<double>::is_iec559;

    if(!is_IEEE754 || d > dMax || d < dMin)
    {
        return false;
    }
    else
    {
        // Can convert it.
        i64 = int64_t(d);
        return true;
    }

}

#endif //ALLIANCEDB_TYPE_CONVERT_H
