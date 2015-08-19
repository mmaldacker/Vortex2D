//
//  hfloat.cpp
//  Vortex
//
//  Created by Maximilian Maldacker on 28/04/2014.
//
//

#include "hfloat.h"

static unsigned int HALF_FLOAT_MIN_BIASED_EXP_AS_SINGLE_FP_EXP = 0x38000000;
static unsigned int HALF_FLOAT_MAX_BIASED_EXP = 0x1F << 10;
static unsigned int FLOAT_MAX_BIASED_EXP = 0xFF << 23;

float convertHFloatToFloat(union hfloat hf)
{
    unsigned int    sign = (unsigned int)(hf.f >> 15);
    unsigned int    mantissa = (unsigned int)(hf.f & ((1 << 10) - 1));
    unsigned int    exp = (unsigned int)(hf.f & HALF_FLOAT_MAX_BIASED_EXP);
    unsigned int    f;
    if (exp == HALF_FLOAT_MAX_BIASED_EXP)
    {
        // we have a half-float NaN or Inf
        // half-float NaNs will be converted to a single precision NaN
        // half-float Infs will be converted to a single precision Inf
        exp = FLOAT_MAX_BIASED_EXP;
        if (mantissa)
            mantissa = (1 << 23) - 1;   // set all bits to indicate a NaN
    }
    else if (exp == 0x0)
    {
        // convert half-float zero/denorm to single precision value
        if (mantissa)
        {
            mantissa <<= 1;
            exp = HALF_FLOAT_MIN_BIASED_EXP_AS_SINGLE_FP_EXP;
            // check for leading 1 in denorm mantissa
            while ((mantissa & (1 << 10)) == 0)
            {
                // for every leading 0, decrement single precision exponent by 1
                // and shift half-float mantissa value to the left
                mantissa <<= 1;
                exp -= (1 << 23);
            }
            // clamp the mantissa to 10-bits
            mantissa &= ((1 << 10) - 1);
            // shift left to generate single-precision mantissa of 23-bits
            mantissa <<= 13;
        }
    }
    else
    {
        // shift left to generate single-precision mantissa of 23-bits
        mantissa <<= 13;
        // generate single precision biased exponent value
        exp = (exp << 13) + HALF_FLOAT_MIN_BIASED_EXP_AS_SINGLE_FP_EXP;
    }
    f = (sign << 31) | exp | mantissa;
    return *((float *)&f);
}
