//
//  Brush.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Vector3i.h"
#import "Face.h"

extern NSString* const BrushFaceAdded;
extern NSString* const BrushFaceRemoved;

@interface Brush : NSObject {
	NSMutableSet* faces;
}

- (id)initCuboidAt:(Vector3i *)position with:(Vector3i *)dimensions;

- (NSSet *)faces;
- (NSSet *)polygons;
@end
