//
//  JSONVisualizerWindow.h
//  myodaemon
//
//  Created by logotype on 11/6/14.
//  Copyright (c) 2014 logotype. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>

@interface JSONVisualizerWindow : NSWindowController <NSWindowDelegate> {
    
}

@property (strong) IBOutlet WebView *jsonWebView;

@end
