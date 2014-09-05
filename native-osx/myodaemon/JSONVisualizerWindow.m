#import "JSONVisualizerWindow.h"
#import "Constants.h"

@implementation JSONVisualizerWindow

- (instancetype)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self) {
    }
    return self;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    [_jsonWebView setDrawsBackground:NO];
    [self.window makeMainWindow];
    [self.window center];
    [self.window makeKeyAndOrderFront:self];
    [NSApp activateIgnoringOtherApps:YES];
    
    NSURLRequest *urlRequest = [NSURLRequest requestWithURL:[NSURL fileURLWithPath:[[NSBundle mainBundle] pathForResource:@"visualizer" ofType:@"html" inDirectory:@"visualizer"]]];
    WebPreferences *preferences = [_jsonWebView preferences];

    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wundeclared-selector"
    if([preferences respondsToSelector:@selector(setWebGLEnabled:)]){
        [preferences performSelector:@selector(setWebGLEnabled:) withObject:[NSNumber numberWithBool:YES]];
    }
    #pragma clang diagnostic pop

    [_jsonWebView setPreferences:preferences];
    [[_jsonWebView mainFrame] loadRequest:urlRequest];
}

- (void)windowWillClose:(NSNotification *)notification
{
    CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), (CFStringRef)kWindowJSONVisualizerClose, NULL, NULL, YES);
}

@end
