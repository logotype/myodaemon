#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>

@interface JSONVisualizerWindow : NSWindowController <NSWindowDelegate> {
    
}

@property (strong) IBOutlet WebView *jsonWebView;

@end
