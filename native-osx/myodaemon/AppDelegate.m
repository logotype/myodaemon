#import "AppDelegate.h"
#import "../src/Constants.h"
#import <boost/thread/xtime.hpp>

@implementation AppDelegate

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)applicationDidFinishLaunching: (NSNotification*) aNotification
{
    isConnected = FALSE;
    
    // Listen for sleep and wake events
    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self selector: @selector(receiveSleepNote:) name: NSWorkspaceWillSleepNotification object: NULL];
    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self selector: @selector(receiveWakeNote:) name: NSWorkspaceDidWakeNotification object: NULL];

    // Listen for various notifications
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(receiveMyoNotification:) name:kMyoConnected object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(receiveMyoNotification:) name:kMyoDisconnected object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(receiveMyoNotification:) name:kMyoNoDongle object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(receiveMyoNotification:) name:kWindowJSONVisualizerClose object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(receiveMyoNotification:) name:kWindowSettingsClose object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(receiveError:) name:kGlobalError object:nil];
    
    // Start the main Myo integration in separate thread
    myoThread = thread(bind(&MyoWebsocket::run,&myowebsocket));
}

- (void)alertError:(NSString *)errorMessage {
    NSAlert *alert = [[NSAlert alloc] init];
    [alert addButtonWithTitle:NSLocalizedString(@"kOK", @"")];
    [alert setMessageText:NSLocalizedString(@"kErrorOccured", @"")];
    [alert setInformativeText:errorMessage];
    [alert setAlertStyle:NSWarningAlertStyle];

    isConnected = FALSE;
    
    // Disable connect button
    [connectionButton setTitle:NSLocalizedString(@"kMenuButtonConnectString", @"")];
    [connectionButton setAction:nil];
    isConnected = FALSE;

    // Set focus
    [[NSRunningApplication currentApplication] activateWithOptions:NSApplicationActivateIgnoringOtherApps];
    [alert runModal];
}

- (void)setConnectionState: (id)sender {
    if (isConnected) {
        [self disconnect];
    } else {
        [self connect];
    }
}

- (void)openSettings: (id)sender {
    
    if (settingsWindow != NULL) {
        [[settingsWindow window] makeMainWindow];
        [[settingsWindow window] center];
        [[settingsWindow window] makeKeyAndOrderFront:self];
        [NSApp activateIgnoringOtherApps:YES];
        return;
    }
    
    settingsWindow = [[SettingsWindow alloc] initWithWindowNibName:@"SettingsWindow"];
    [settingsWindow showWindow:self];
}

- (void)openVisualizer: (id)sender {
    if (visualizerWindow != NULL) {
        [[visualizerWindow window] makeMainWindow];
        [[visualizerWindow window] center];
        [[visualizerWindow window] makeKeyAndOrderFront:self];
        [NSApp activateIgnoringOtherApps:YES];
        return;
    }
    
    visualizerWindow = [[JSONVisualizerWindow alloc] initWithWindowNibName:@"JSONVisualizerWindow"];
    [visualizerWindow showWindow:self];
}

- (void)awakeFromNib {
    statusItem = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];
    [statusItem setMenu:statusMenu];
    [statusItem setImage:[[NSImage alloc] initWithContentsOfFile: [[NSBundle mainBundle] pathForResource:@"Myo" ofType:@"pdf" inDirectory:@"assets-bundled"]]];
    [statusItem setAlternateImage:[[NSImage alloc] initWithContentsOfFile: [[NSBundle mainBundle] pathForResource:@"MyoActive" ofType:@"pdf" inDirectory:@"assets-bundled"]]];
    [statusItem setHighlightMode:YES];
    
    NSMenu *menu = [[NSMenu alloc] init];
    connectionButton = [menu addItemWithTitle:NSLocalizedString(@"kMenuButtonConnectString", @"") action:@selector(setConnectionState:) keyEquivalent:@""];
    [menu addItem:[NSMenuItem separatorItem]];
    settingsButton = [menu addItemWithTitle:NSLocalizedString(@"kMenuButtonSettingsString", @"") action:@selector(openSettings:) keyEquivalent:@""];
    visualizerButton = [menu addItemWithTitle:NSLocalizedString(@"kMenuButtonVisualizerString", @"") action:@selector(openVisualizer:) keyEquivalent:@""];
    [menu addItem:[NSMenuItem separatorItem]];
    [menu addItemWithTitle:NSLocalizedString(@"kMenuButtonQuitString", @"") action:@selector(terminate:) keyEquivalent:@""];
    statusItem.menu = menu;
}

#pragma mark Computer sleep/wakeup

- (void)receiveSleepNote:(NSNotification*)note {
    [self disconnect];
}

- (void)receiveWakeNote:(NSNotification*)note {
    [self connect];
}

#pragma mark Enable/disable service

-(void)connect {
    // Resume service when computer wakes up
    NSLog(@"resuming worker thread");
    myoThread = thread(bind(&MyoWebsocket::run,&myowebsocket));
}

-(void)disconnect {
    // Stop service when computer goes to sleep
    myoThread.interrupt();
    boost::posix_time::time_duration timeout = boost::posix_time::milliseconds(100);
    if (myoThread.joinable()) {
        myoThread.join();
        NSLog(@"successfully interrupted worker thread");
    } else {
        NSLog(@"failed to interrupt worker thread");
    }
}

#pragma mark Notifications

- (void)receiveMyoNotification:(NSNotification *) notification {
    if(statusItem == NULL)
        return;
    
    if ([[notification name] isEqualToString:kMyoConnected]) {
        [statusItem setImage:[[NSImage alloc] initWithContentsOfFile: [[NSBundle mainBundle] pathForResource:@"MyoConnected" ofType:@"pdf" inDirectory:@"assets-bundled"]]];
        [connectionButton setTitle:NSLocalizedString(@"kMenuButtonDisconnectString", @"")];
        [connectionButton setAction:@selector(setConnectionState:)];
        isConnected = TRUE;
    }else if ([[notification name] isEqualToString:kMyoDisconnected]) {
        [statusItem setImage:[[NSImage alloc] initWithContentsOfFile: [[NSBundle mainBundle] pathForResource:@"Myo" ofType:@"pdf" inDirectory:@"assets-bundled"]]];
        [connectionButton setTitle:NSLocalizedString(@"kMenuButtonConnectString", @"")];
        [connectionButton setAction:@selector(setConnectionState:)];
        isConnected = FALSE;
    }else if ([[notification name] isEqualToString:kWindowJSONVisualizerClose]) {
        visualizerWindow = nil;
    }else if ([[notification name] isEqualToString:kMyoNoDongle]) {
        [connectionButton setTitle:NSLocalizedString(@"kMenuButtonConnectString", @"")];
        [connectionButton setAction:nil];
        isConnected = FALSE;
    }else if ([[notification name] isEqualToString:kWindowSettingsClose]) {
        settingsWindow = nil;
    }
}

- (void)receiveError:(NSNotification *) notification {

    NSDictionary *userInfo = [notification userInfo];
    NSString *errorMessage = [userInfo objectForKey:@"errorMessage"];
    [self performSelectorOnMainThread:@selector(alertError:) withObject:errorMessage waitUntilDone:YES];
}

@end