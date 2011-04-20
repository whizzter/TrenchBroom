//
//  Grid.h
//  TrenchBroom
//
//  Created by Kristian Duske on 07.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>

extern NSString* const GridChanged;

@class Vector3f;

@interface Grid : NSObject {
    @private
    int size;
    BOOL draw;
    BOOL snap;
    GLuint texIds[9];
}

- (int)size;
- (int)actualSize;
- (BOOL)draw;
- (BOOL)snap;

- (void)setSize:(int)theSize;
- (void)setDraw:(BOOL)isDrawEnabled;
- (void)setSnap:(BOOL)isSnapEnabled;
- (void)toggleDraw;
- (void)toggleSnap;

- (void)snapToGrid:(Vector3f *)vector;
- (void)snapUpToGrid:(Vector3f *)vector;
- (void)snapDownToGrid:(Vector3f *)vector;
- (Vector3f *)gridOffsetOf:(Vector3f *)vector;

- (void)activateTexture;
- (void)deactivateTexture;
@end
