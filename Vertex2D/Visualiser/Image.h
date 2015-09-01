//
//  Image.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 22/08/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#ifndef __Vertex2D__Image__
#define __Vertex2D__Image__

#include <cstdint>
#include <string>
#include <vector>

class PngLoader
{
public:
    PngLoader(const std::string & name);

    const uint8_t * Data() const;
    int Width() const;
    int Height() const;
    bool HasAlpha() const;

private:
    int mWidth;
    int mHeight;
    bool mHasAlpha;

    std::vector<uint8_t> mData;
};

#endif /* defined(__Vertex2D__Image__) */
