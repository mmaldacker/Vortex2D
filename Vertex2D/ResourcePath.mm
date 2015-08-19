//
//  ResourcePath.mm
//  Vortex
//
//  Created by Maximilian Maldacker on 06/04/2014.
//
//
#include "ResourcePath.h"
#import <Foundation/Foundation.h>

std::string getResourcePath(void)
{
    NSBundle* mainBundle = [NSBundle mainBundle];

    if (mainBundle != nil)
    {
        NSString* path = [mainBundle resourcePath];
        return [path UTF8String] + std::string("/");
    }

    return "";
}

std::string getDocumentsPath(void)
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    if(paths != nil)
    {
        NSString *documentsDirectory = [paths objectAtIndex:0];
        return [documentsDirectory UTF8String] + std::string("/");
    }

    return "";
}