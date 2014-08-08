#ifndef __MyoWebsocket__
#define __MyoWebsocket__

#include <cassert>
#include <iostream>
#include <myo/myo.hpp>
#include "DeviceListener.hpp"

class MyoWebsocket {
public:
    MyoWebsocket();
    ~MyoWebsocket();

    bool isConnected;
    bool isDongleConnected;
    bool hasShownErrorOnce;
    myo::Myo *myo;
    DeviceListener listener;

    static void requestConnectivityStatusHandler(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo);
    static void requestRSSIHandler(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo);
    static void requestDeviceInfoHandler(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo);
    static void vibrateHandler(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo);
    static void tryConnectHandler(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo);

    void run();
    void tryConnect();
    void requestConnectivityStatus();
    void requestRSSI();
    void requestDeviceInfo();
    void vibrate(int vibrateLength);
private:
};
#endif /* defined(__MyoWebsocket__) */