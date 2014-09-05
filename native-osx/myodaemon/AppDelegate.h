#import <Cocoa/Cocoa.h>
#import "MyoWebsocket.hpp"
#import "SettingsWindow.h"
#import "JSONVisualizerWindow.h"

@interface AppDelegate : NSObject <NSApplicationDelegate> {
    bool isConnected;
    IBOutlet NSMenu *statusMenu;
    NSStatusItem *statusItem;
    NSMenuItem *connectionButton;
    NSMenuItem *settingsButton;
    NSMenuItem *visualizerButton;
    thread myoThread;
    MyoWebsocket myowebsocket;
    SettingsWindow *settingsWindow;
    JSONVisualizerWindow *visualizerWindow;
}

- (void)receiveMyoNotification:(NSNotification *) notification;
- (void)receiveError:(NSNotification *) notification;
- (void)alertError:(NSString *) errorMessage;
- (void)setConnectionState: (id)sender;
- (void)openSettings: (id)sender;
- (void)openVisualizer: (id)sender;
- (void)connect;
- (void)disconnect;
@end
