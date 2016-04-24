#import "SettingsWindow.h"
#import "../src/Constants.h"

@interface SettingsWindow ()

@end

@implementation SettingsWindow

- (instancetype)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self) {
        [self setFieldsHidden:TRUE];
    }
    return self;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)windowDidLoad
{
    [self.window makeMainWindow];
    [self.window center];
    [self.window makeKeyAndOrderFront:self];
    [NSApp activateIgnoringOtherApps:YES];
    
    numClients = 0;
    
    // Update version string
    NSDictionary* infoDict = [[NSBundle mainBundle] infoDictionary];
    NSString* infoVersion = [infoDict objectForKey:@"CFBundleShortVersionString"];
    NSString* infoBuild = [infoDict objectForKey:@"CFBundleVersion"];
    NSString* infoXCodeBuild = [infoDict objectForKey:@"DTXcodeBuild"];
    
    [_labelVersion setStringValue:[NSString stringWithFormat:@"Version %@\nRev. %@ Build %@", infoVersion, infoBuild, infoXCodeBuild]];
    
    // Disable focus ring on tab view
    [_tabView setFocusRingType:NSFocusRingTypeNone];
    
    isConnected = false;
    [_labelServerURL setSelectable:YES];
    [_labelVersion setSelectable:YES];
    
    // Listen for connectivity status
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(receiveMyoNotification:) name:kRequestConnectivityStatusConnected object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(receiveMyoNotification:) name:kRequestConnectivityStatusDisconnected object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(receiveMyoNotification:) name:kRequestConnectivityStatusNoDongle object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(receiveMyoNotification:) name:kMyoRSSI object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(receiveMyoNotification:) name:kMyoFPS object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(receiveMyoNotification:) name:kMyoNumClients object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(receiveMyoNotification:) name:kMyoDisconnected object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(receiveMyoNotification:) name:kMyoConnected object:nil];

    // Request RSSI at specific intervals
    timerRSSI = [NSTimer scheduledTimerWithTimeInterval:1.0 target:self selector:@selector(updateRSSI) userInfo:nil repeats:YES];

    // Ask for connectivity status
    CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), (CFStringRef)kRequestConnectivityStatus, NULL, NULL, YES);

    [super windowDidLoad];
}

- (void)windowWillClose:(NSNotification *)notification
{
    if(timerRSSI)
    {
        [timerRSSI invalidate];
        timerRSSI = nil;
    }
    CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), (CFStringRef)kWindowSettingsClose, NULL, NULL, YES);
}

#pragma mark Notifications

- (void) receiveMyoNotification:(NSNotification *) notification {
    if ([[notification name] isEqualToString:kRequestConnectivityStatusConnected] || [[notification name] isEqualToString:kMyoConnected]) {
        [_stateConnection setImage:[[NSImage alloc] initWithContentsOfFile: [[NSBundle mainBundle] pathForResource:@"MyoConnConnected" ofType:@"pdf" inDirectory:@"assets-bundled"]]];
        [_labelConnection setStringValue:[NSString stringWithFormat:@"%@", NSLocalizedString(@"kMyoConnectedString", @"")]];
        [_labelServer setStringValue:NSLocalizedString(@"kServerRunningString", @"")];
        isConnected = true;
        [self setFieldsHidden:FALSE];
    } else if ([[notification name] isEqualToString:kRequestConnectivityStatusDisconnected] || [[notification name] isEqualToString:kMyoDisconnected]) {
        [_stateConnection setImage:[[NSImage alloc] initWithContentsOfFile: [[NSBundle mainBundle] pathForResource:@"MyoConnNotConnected" ofType:@"pdf" inDirectory:@"assets-bundled"]]];
        [_labelConnection setStringValue:NSLocalizedString(@"kMyoDisconnectedString", @"")];
        [self setFieldsHidden:TRUE];
    } else if ([[notification name] isEqualToString:kRequestConnectivityStatusNoDongle]) {
        [_stateConnection setImage:[[NSImage alloc] initWithContentsOfFile: [[NSBundle mainBundle] pathForResource:@"MyoConnNoDongle" ofType:@"pdf" inDirectory:@"assets-bundled"]]];
        [_labelConnection setStringValue:NSLocalizedString(@"kMyoNoDongleString", @"")];
        [self setFieldsHidden:TRUE];
    } else if ([[notification name] isEqualToString:kMyoFPS]) {
        NSDictionary *userInfo = [notification userInfo];
        NSString *fpsString = [userInfo objectForKey:@"fps"];
        NSString *fpsFormattedString = [NSString stringWithFormat:@"%@ FPS (%d clients connected)", fpsString, numClients];
        [_labelFPS setStringValue:NSLocalizedString(@"kDataRateString", @"")];
        [_labelFPSValue setStringValue:fpsFormattedString];
    } else if ([[notification name] isEqualToString:kMyoNumClients]) {
        NSDictionary *userInfo = [notification userInfo];
        numClients = (int)[[userInfo objectForKey:@"numClients"] integerValue];
    } else if ([[notification name] isEqualToString:kMyoRSSI]) {
        NSDictionary *userInfo = [notification userInfo];
        NSString *rssiString = [userInfo objectForKey:@"rssi"];
        NSString *rssidBString = [NSString stringWithFormat:@"RSSI %@dBm", rssiString];
        
        [_labelRSSI setStringValue:rssidBString];
        [_levelRSSI setIntValue:(int)[rssiString integerValue]];
    }
}

- (void)updateRSSI {
    if (isConnected) {
        CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), (CFStringRef)kRequestRSSI, NULL, NULL, YES);
    }
}

- (void)setFieldsHidden:(BOOL)isHidden {
    [_labelRSSI setHidden:isHidden];
    [_labelFPS setHidden:isHidden];
    [_labelFPSValue setHidden:isHidden];
    [_labelServer setHidden:isHidden];
    [_labelServerURL setHidden:isHidden];
    [_levelRSSI setHidden:isHidden];
}

#pragma mark IB Actions

- (IBAction)onButtonSourceClick:(id)sender {
    NSURL *url = [NSURL URLWithString:@"https://github.com/logotype/myodaemon"];
    if( ![[NSWorkspace sharedWorkspace] openURL:url] )
        NSLog(@"Failed to open url: %@",[url description]);
}

- (IBAction)onJSButtonSourceClick:(id)sender {
    NSURL *url = [NSURL URLWithString:@"https://github.com/logotype/MyoJS"];
    if( ![[NSWorkspace sharedWorkspace] openURL:url] )
        NSLog(@"Failed to open url: %@",[url description]);
}

- (IBAction)onWebAppsClick:(id)sender {
    NSString *enableWebappsString = [NSString stringWithFormat:@"%ld", (long)[_chkWebApps state]];
    
    CFDictionaryKeyCallBacks keyCallbacks = {0, NULL, NULL, CFCopyDescription, CFEqual, NULL};
    CFDictionaryValueCallBacks valueCallbacks  = {0, NULL, NULL, CFCopyDescription, CFEqual};
    CFMutableDictionaryRef dictionary = CFDictionaryCreateMutable(kCFAllocatorDefault, 1, &keyCallbacks, &valueCallbacks);
    CFDictionaryAddValue(dictionary, CFSTR("toggleWebApps"), (CFStringRef)enableWebappsString);
    CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), (CFStringRef)kToggleWebApps, NULL, dictionary, TRUE);
    CFRelease(dictionary);
}

- (IBAction)onEulerClick:(id)sender {
    NSString *calculateEulerString = [NSString stringWithFormat:@"%ld", (long)[_chkEuler state]];
    
    CFDictionaryKeyCallBacks keyCallbacks = {0, NULL, NULL, CFCopyDescription, CFEqual, NULL};
    CFDictionaryValueCallBacks valueCallbacks  = {0, NULL, NULL, CFCopyDescription, CFEqual};
    CFMutableDictionaryRef dictionary = CFDictionaryCreateMutable(kCFAllocatorDefault, 1, &keyCallbacks, &valueCallbacks);
    CFDictionaryAddValue(dictionary, CFSTR("toggleEuler"), (CFStringRef)calculateEulerString);
    CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), (CFStringRef)kToggleEuler, NULL, dictionary, TRUE);
    CFRelease(dictionary);
}

- (IBAction)onEMGClick:(id)sender {
    NSString *receiveEMGString = [NSString stringWithFormat:@"%ld", (long)[_chkEMG state]];
    
    CFDictionaryKeyCallBacks keyCallbacks = {0, NULL, NULL, CFCopyDescription, CFEqual, NULL};
    CFDictionaryValueCallBacks valueCallbacks  = {0, NULL, NULL, CFCopyDescription, CFEqual};
    CFMutableDictionaryRef dictionary = CFDictionaryCreateMutable(kCFAllocatorDefault, 1, &keyCallbacks, &valueCallbacks);
    CFDictionaryAddValue(dictionary, CFSTR("toggleEMG"), (CFStringRef)receiveEMGString);
    CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), (CFStringRef)kToggleEMG, NULL, dictionary, TRUE);
    CFRelease(dictionary);
}

- (IBAction)onVibrationButtonClick:(id)sender {
    
    NSString *vibrateLengthString = [NSString stringWithFormat:@"%ld", [_vibrationButton selectedSegment]];
    
    CFDictionaryKeyCallBacks keyCallbacks = {0, NULL, NULL, CFCopyDescription, CFEqual, NULL};
    CFDictionaryValueCallBacks valueCallbacks  = {0, NULL, NULL, CFCopyDescription, CFEqual};
    CFMutableDictionaryRef dictionary = CFDictionaryCreateMutable(kCFAllocatorDefault, 1, &keyCallbacks, &valueCallbacks);
    CFDictionaryAddValue(dictionary, CFSTR("vibrateValue"), (CFStringRef)vibrateLengthString);
    CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), (CFStringRef)kCmdVibrate, NULL, dictionary, TRUE);
    CFRelease(dictionary);
}

- (IBAction)labelVersion:(id)sender {
}
@end
