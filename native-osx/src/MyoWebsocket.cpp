#include <stdio.h>
#include "MyoWebsocket.hpp"
#include <unistd.h>
#include "Constants.h"

MyoWebsocket::MyoWebsocket() {
};

MyoWebsocket::~MyoWebsocket(){
    CFNotificationCenterRemoveEveryObserver(CFNotificationCenterGetLocalCenter(), this);
}

#pragma mark NSNotification handlers

void MyoWebsocket::requestConnectivityStatusHandler(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo) {
    (static_cast<MyoWebsocket *>(observer))->requestConnectivityStatus();
}

void MyoWebsocket::requestRSSIHandler(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo) {
    (static_cast<MyoWebsocket *>(observer))->requestRSSI();
}

void MyoWebsocket::requestDeviceInfoHandler(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo) {
    (static_cast<MyoWebsocket *>(observer))->requestDeviceInfo();
}

void MyoWebsocket::vibrateHandler(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo) {
    
    NSDictionary *userInfoDictionary = (__bridge NSDictionary*)userInfo;
    int vibrateValue = (int)[[userInfoDictionary objectForKey:@"vibrateValue"] integerValue];
    (static_cast<MyoWebsocket *>(observer))->vibrate(vibrateValue);
}

void MyoWebsocket::unlockHandler(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo) {
    
    NSDictionary *userInfoDictionary = (__bridge NSDictionary*)userInfo;
    int unlockValue = (int)[[userInfoDictionary objectForKey:@"unlockValue"] integerValue];
    (static_cast<MyoWebsocket *>(observer))->unlock(unlockValue);
}

void MyoWebsocket::lockHandler(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo) {
    (static_cast<MyoWebsocket *>(observer))->lock();
}

void MyoWebsocket::notifyUserActionHandler(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo) {
    (static_cast<MyoWebsocket *>(observer))->notifyUserAction();
}

void MyoWebsocket::tryConnectHandler(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo) {
    (static_cast<MyoWebsocket *>(observer))->tryConnect();
}

#pragma mark Hub run loop

void MyoWebsocket::run() {
    std::cout <<  boost::this_thread::get_id() << " worker thread " << std::endl;
    isConnected = false;
    isDongleConnected = false;
    hasShownErrorOnce = false;
    
    // Listen for connectivity request
    CFNotificationCenterAddObserver(CFNotificationCenterGetLocalCenter(), this, MyoWebsocket::requestConnectivityStatusHandler, (CFStringRef)kRequestConnectivityStatus, NULL, CFNotificationSuspensionBehaviorDeliverImmediately);
    CFNotificationCenterAddObserver(CFNotificationCenterGetLocalCenter(), this, MyoWebsocket::requestRSSIHandler, (CFStringRef)kRequestRSSI, NULL, CFNotificationSuspensionBehaviorDeliverImmediately);
    
    // Listen for device information from websocket
    CFNotificationCenterAddObserver(CFNotificationCenterGetLocalCenter(), this, MyoWebsocket::requestDeviceInfoHandler, (CFStringRef)kCmdRequestDeviceInfo, NULL, CFNotificationSuspensionBehaviorDeliverImmediately);
    CFNotificationCenterAddObserver(CFNotificationCenterGetLocalCenter(), this, MyoWebsocket::vibrateHandler, (CFStringRef)kCmdVibrate, NULL, CFNotificationSuspensionBehaviorDeliverImmediately);
    CFNotificationCenterAddObserver(CFNotificationCenterGetLocalCenter(), this, MyoWebsocket::unlockHandler, (CFStringRef)kCmdUnlock, NULL, CFNotificationSuspensionBehaviorDeliverImmediately);
    CFNotificationCenterAddObserver(CFNotificationCenterGetLocalCenter(), this, MyoWebsocket::lockHandler, (CFStringRef)kCmdLock, NULL, CFNotificationSuspensionBehaviorDeliverImmediately);
    CFNotificationCenterAddObserver(CFNotificationCenterGetLocalCenter(), this, MyoWebsocket::notifyUserActionHandler, (CFStringRef)kCmdNotifyUserAction, NULL, CFNotificationSuspensionBehaviorDeliverImmediately);

    // Listen for device information from websocket
    CFNotificationCenterAddObserver(CFNotificationCenterGetLocalCenter(), this, MyoWebsocket::tryConnectHandler, (CFStringRef)kTryConnect, NULL, CFNotificationSuspensionBehaviorDeliverImmediately);

    tryConnect();
}

void MyoWebsocket::tryConnect() {
    try{
        myo::Hub hub("com.logotype.myodaemon");
        myo = hub.waitForMyo(5000);
        isDongleConnected = true;

        if (!myo) {
            isConnected = false;
            CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), (CFStringRef)kMyoDisconnected, NULL, NULL, YES);
        } else {
            isConnected = true;
            hub.addListener(&listener);

            CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), (CFStringRef)kMyoConnected, NULL, NULL, YES);
            
            while (1) {
                try {
                    boost::this_thread::sleep(boost::posix_time::milliseconds(1));
                    hub.run(1000/50);
                }
                catch (const std::runtime_error& error)
                {
                    // If error occurs while running, sleep for 100ms and retry again (silent error)
                    NSLog(@"Runloop error: %s", error.what());
                    usleep(100000);
                }
            }
        }
    }
    catch (const std::exception& error) {
        isDongleConnected = false;
        isConnected = false;
        NSLog(@"Error: %s", error.what());
        
        if(!hasShownErrorOnce) {
            hasShownErrorOnce = true;
            // Write error message to notification
            CFStringRef errorMessage = CFStringCreateWithCString(NULL, error.what(), kCFStringEncodingUTF8);
            CFDictionaryKeyCallBacks keyCallbacks = {0, NULL, NULL, CFCopyDescription, CFEqual, NULL};
            CFDictionaryValueCallBacks valueCallbacks  = {0, NULL, NULL, CFCopyDescription, CFEqual};
            CFMutableDictionaryRef dictionary = CFDictionaryCreateMutable(kCFAllocatorDefault, 1, &keyCallbacks, &valueCallbacks);
            CFDictionaryAddValue(dictionary, CFSTR("errorMessage"), errorMessage);
            CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), (CFStringRef)kGlobalError, NULL, dictionary, TRUE);
        }

        // If error occured, wait 1 second and retry connection
        sleep(1);
        CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), (CFStringRef)kTryConnect, NULL, NULL, YES);
        
        return;
    }
    catch(boost::thread_interrupted&)
    {
        isConnected = false;
        std::cout <<  boost::this_thread::get_id() << " thread stopped" << std::endl;
        CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), (CFStringRef)kMyoDisconnected, NULL, NULL, YES);
        return;
    }
}

void MyoWebsocket::requestConnectivityStatus() {
    if(!isDongleConnected) {
        CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), (CFStringRef)kRequestConnectivityStatusNoDongle, NULL, NULL, YES);
        return;
    }
    if (isConnected) {
        CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), (CFStringRef)kRequestConnectivityStatusConnected, NULL, NULL, YES);
        myo->requestRssi();
    } else {
        CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), (CFStringRef)kRequestConnectivityStatusDisconnected, NULL, NULL, YES);
    }
}

void MyoWebsocket::requestRSSI() {
    if (isConnected) {
        CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), (CFStringRef)kRequestConnectivityStatusConnected, NULL, NULL, YES);
        myo->requestRssi();
    }
}

void MyoWebsocket::requestDeviceInfo() {
    if (isConnected) {
        std::cout << "MyoWebsocket::requestDeviceInfo: Sending deviceInfo..." << std::endl;

        NSDictionary *deviceDictionary = [[NSMutableDictionary alloc] init];
        [deviceDictionary setValue:@"true" forKey:@"connected"];
        [listener.frame setValue:deviceDictionary forKey:@"deviceInfo"];
        listener.removeDataAfterSend = TRUE;
    }
}

void MyoWebsocket::vibrate(int vibrateLength) {
    if (!isConnected || myo == NULL) {
        std::cout << "MyoWebsocket::vibrate: not connected" << std::endl;
        return;
    }
    std::cout << "MyoWebsocket::vibrate: Length " << vibrateLength << std::endl;
    switch (vibrateLength) {
        case 0:
            myo->vibrate(myo::Myo::vibrationShort);
            break;
            
        case 1:
            myo->vibrate(myo::Myo::vibrationMedium);
            break;
            
        case 2:
            myo->vibrate(myo::Myo::vibrationLong);
            break;
            
        default:
            break;
    }
}

void MyoWebsocket::unlock(int unlockValue) {
    if (!isConnected || myo == NULL) {
        std::cout << "MyoWebsocket::unlock: not connected" << std::endl;
        return;
    }
    std::cout << "MyoWebsocket::unlock: Option " << unlockValue << std::endl;
    switch (unlockValue) {
        case 0:
            myo->unlock(myo::Myo::unlockTimed);
            break;
        case 1:
            myo->unlock(myo::Myo::unlockHold);
            break;
        default:
            break;
    }
}

void MyoWebsocket::lock() {
    if (!isConnected || myo == NULL) {
        std::cout << "MyoWebsocket::lock: not connected" << std::endl;
        return;
    }
    std::cout << "MyoWebsocket::lock: " << std::endl;
    myo->lock();
}

void MyoWebsocket::notifyUserAction() {
    if (!isConnected || myo == NULL) {
        std::cout << "MyoWebsocket::notifyUserAction: not connected" << std::endl;
        return;
    }
    std::cout << "MyoWebsocket::notifyUserAction: " << std::endl;
    myo->notifyUserAction();
}