//
//  hfloat.h
//  Vortex
//
//  Created by Maximilian Maldacker on 28/04/2014.
//
//

#ifndef __Vortex__hfloat__
#define __Vortex__hfloat__

union hfloat
{
    unsigned short f;
    struct {
        unsigned short mantissa : 10;
        unsigned short exp  : 5;
        unsigned short sign : 1;
    } fields;
};

float convertHFloatToFloat(union hfloat hf);

#endif /* defined(__Vortex__hfloat__) */
