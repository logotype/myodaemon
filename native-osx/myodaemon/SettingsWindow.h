#import <Cocoa/Cocoa.h>

@interface SettingsWindow : NSWindowController <NSWindowDelegate> {
    NSTimer *timerRSSI;
    bool isConnected;
    int numClients;
}

@property (strong) IBOutlet NSTabView *tabView;
@property (strong) IBOutlet NSWindow *settingsWindow;
@property (strong) IBOutlet NSImageView *logoView;
@property (strong) IBOutlet NSTextField *labelConnection;
@property (strong) IBOutlet NSTextField *labelFPS;
@property (strong) IBOutlet NSTextField *labelFPSValue;
@property (strong) IBOutlet NSTextField *labelServer;
@property (strong) IBOutlet NSTextField *labelServerURL;
@property (strong) IBOutlet NSTextField *labelRSSI;
@property (strong) IBOutlet NSTextField *labelVersion;
@property (strong) IBOutlet NSImageView *stateConnection;
@property (strong) IBOutlet NSLevelIndicator *levelRSSI;
@property (strong) IBOutlet NSButton *linkButton;
@property (strong) IBOutlet NSButton *jsButton;
@property (strong) IBOutlet NSButton *updateButton;
@property (strong) IBOutlet NSSegmentedControl *vibrationButton;
@property (strong) IBOutlet NSButton *chkEuler;
@property (strong) IBOutlet NSButton *chkWebApps;

- (void) receiveMyoNotification:(NSNotification *) notification;
- (void)updateRSSI;
- (void)setFieldsHidden:(BOOL)isHidden;
- (IBAction)onButtonSourceClick:(id)sender;
- (IBAction)onJSButtonSourceClick:(id)sender;
- (IBAction)onVibrationButtonClick:(id)sender;

@end
