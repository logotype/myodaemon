#include <stdio.h>
#include "DeviceListener.hpp"
#include "Constants.h"

void DeviceListener::toggleEulerHandler(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo) {
    
    NSDictionary *userInfoDictionary = (__bridge NSDictionary*)userInfo;
    int toggleEulerValue = (int)[[userInfoDictionary objectForKey:@"toggleEuler"] integerValue];
    (static_cast<DeviceListener *>(observer))->toggleEuler(toggleEulerValue);
}

DeviceListener::DeviceListener() {
    removeDataAfterSend = FALSE;
    isCalculatingEuler = TRUE;
    wrapper = [[NSMutableDictionary alloc] init];
    frame = [[NSMutableDictionary alloc] init];
    
    lastTime = clock();
    
    // Listen for notifications from Settings
    CFNotificationCenterAddObserver(CFNotificationCenterGetLocalCenter(), this, DeviceListener::toggleEulerHandler, (CFStringRef)kToggleEuler, NULL, CFNotificationSuspensionBehaviorDeliverImmediately);
    
    // Start a thread to run the processing loop
    processMessagesThread = thread((bind(&DataHandler::process_messages,&server)));
    
    // Run the asio loop in a separate thread
    dataHandlerThread = thread((bind(&DataHandler::run,&server)));
}

DeviceListener::~DeviceListener(){
    CFNotificationCenterRemoveEveryObserver(CFNotificationCenterGetLocalCenter(), this);
}

/// Called when a Myo has been paired.
void DeviceListener::onPair(myo::Myo *myo, uint64_t timestamp)
{
    NSLog(@"onPair");
    NSDictionary *eventDictionary = [[NSMutableDictionary alloc] init];
    [eventDictionary setValue:@"onPair" forKey:@"type"];
    [frame setValue:eventDictionary forKey:@"event"];
    sendData();
}

/// Called when a Myo has been un-paired.
void DeviceListener::onUnpair(myo::Myo *myo, uint64_t timestamp)
{
    NSLog(@"onUnpair");
    NSDictionary *eventDictionary = [[NSMutableDictionary alloc] init];
    [eventDictionary setValue:@"onUnpair" forKey:@"type"];
    [frame setValue:eventDictionary forKey:@"event"];
    sendData();
}

/// Called when a paired Myo has been connected.
void DeviceListener::onConnect(myo::Myo *myo, uint64_t timestamp)
{
    NSLog(@"onConnect");
    NSDictionary *eventDictionary = [[NSMutableDictionary alloc] init];
    [eventDictionary setValue:@"onConnect" forKey:@"type"];
    [frame setValue:eventDictionary forKey:@"event"];
    sendData();
}

/// Called when a paired Myo has been disconnected.
void DeviceListener::onDisconnect(myo::Myo *myo, uint64_t timestamp)
{
    NSLog(@"onDisconnect");
    NSDictionary *eventDictionary = [[NSMutableDictionary alloc] init];
    [eventDictionary setValue:@"onDisconnect" forKey:@"type"];
    [frame setValue:eventDictionary forKey:@"event"];
    sendData();
}

/// Called when a paired Myo recognizes that it is on an arm.
void DeviceListener::onArmRecognized(myo::Myo* myo, uint64_t timestamp, myo::Arm arm, myo::XDirection xDirection)
{
    NSLog(@"onArmRecognized");
    NSString* whichArm;
    switch (arm) {
        case myo::armLeft:
            whichArm = [NSString stringWithFormat:@"armLeft"];
            break;
        case myo::armRight:
            whichArm = [NSString stringWithFormat:@"armRight"];
            break;
        case myo::armUnknown:
            whichArm = [NSString stringWithFormat:@"armUnknown"];
            break;
        default:
            break;
    }
    
    NSString* whichDirection;
    switch (xDirection) {
        case myo::xDirectionTowardWrist:
            whichDirection = [NSString stringWithFormat:@"xDirectionTowardWrist"];
            break;
        case myo::xDirectionTowardElbow:
            whichDirection = [NSString stringWithFormat:@"xDirectionTowardElbow"];
            break;
        case myo::xDirectionUnknown:
            whichDirection = [NSString stringWithFormat:@"xDirectionUnknown"];
            break;
        default:
            break;
    }
    
    NSDictionary *eventDictionary = [[NSMutableDictionary alloc] init];
    [eventDictionary setValue:@"onArmRecognized" forKey:@"type"];

    [eventDictionary setValue:whichArm forKey:@"arm"];
    [eventDictionary setValue:whichDirection forKey:@"xDirection"];

    [frame setValue:eventDictionary forKey:@"event"];
    sendData();
}

/// Called when a paired Myo is moved or removed from the arm.
void DeviceListener::onArmLost(myo::Myo* myo, uint64_t timestamp)
{
    NSLog(@"onArmLost");
    NSDictionary *eventDictionary = [[NSMutableDictionary alloc] init];
    [eventDictionary setValue:@"onArmLost" forKey:@"type"];
    [frame setValue:eventDictionary forKey:@"event"];
    sendData();
}

/// Called when a paired Myo has provided a new pose.
void DeviceListener::onPose(myo::Myo *myo, uint64_t timestamp, myo::Pose pose)
{
    NSDictionary *poseDictionary = [[NSMutableDictionary alloc] init];
    NSNumber *poseType = @(pose.type());
    [poseDictionary setValue:poseType forKey:@"type"];
    
    [frame setValue:poseDictionary forKey:@"pose"];
    sendData();
}

/// Called when a paired Myo has provided new orientation data.
void DeviceListener::onOrientationData(myo::Myo *myo, uint64_t timestamp, const myo::Quaternion<float>& rotation)
{
    NSArray *rotationArray = [[NSArray alloc] initWithObjects:@(rotation.x()), @(rotation.y()), @(rotation.z()), @(rotation.w()), nil];
    [frame setValue:rotationArray forKey:@"rotation"];

    if (isCalculatingEuler) {
        using std::atan2;
        using std::asin;
        using std::sqrt;
        
        // Calculate Euler angles (roll, pitch, and yaw) from the unit quaternion.
        float roll = atan2(2.0f * (rotation.w() * rotation.x() + rotation.y() * rotation.z()),
                           1.0f - 2.0f * (rotation.x() * rotation.x() + rotation.y() * rotation.y()));
        float pitch = asin(2.0f * (rotation.w() * rotation.y() - rotation.z() * rotation.x()));
        float yaw = atan2(2.0f * (rotation.w() * rotation.z() + rotation.x() * rotation.y()),
                          1.0f - 2.0f * (rotation.y() * rotation.y() + rotation.z() * rotation.z()));
        
        NSMutableDictionary *eulerDictionary = [[NSMutableDictionary alloc] init];
        [eulerDictionary setValue:@(roll) forKey:@"roll"];
        [eulerDictionary setValue:@(pitch) forKey:@"pitch"];
        [eulerDictionary setValue:@(yaw) forKey:@"yaw"];
        [frame setValue:eulerDictionary forKey:@"euler"];
    }
    
    [frame setValue:@(timestamp) forKey:@"timestamp"];
    sendData(FALSE);
}

/// Called when a paired Myo has provided new accelerometer data in units of g.
void DeviceListener::onAccelerometerData(myo::Myo *myo, uint64_t timestamp, const myo::Vector3<float>& accel)
{
    NSArray *accelArray = [[NSArray alloc] initWithObjects:@(accel.x()), @(accel.y()), @(accel.z()), nil];
    [frame setValue:accelArray forKey:@"accel"];
}

/// Called when a paired Myo has provided new gyroscope data in units of deg/s.
void DeviceListener::onGyroscopeData(myo::Myo *myo, uint64_t timestamp, const myo::Vector3<float>& gyro)
{
    NSArray *gyroArray = [[NSArray alloc] initWithObjects:@(gyro.x()), @(gyro.y()), @(gyro.z()), nil];
    [frame setValue:gyroArray forKey:@"gyro"];
}

/// Called when a paired Myo has provided a new RSSI value.
void DeviceListener::onRssi(myo::Myo *myo, uint64_t timestamp, int8_t rssi)
{
    NSNumber *rssiValue = @(rssi);
    [frame setValue:rssiValue forKey:@"rssi"];

    CFStringRef rssiString = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d"), rssi);
    CFDictionaryKeyCallBacks keyCallbacks = {0, NULL, NULL, CFCopyDescription, CFEqual, NULL};
    CFDictionaryValueCallBacks valueCallbacks  = {0, NULL, NULL, CFCopyDescription, CFEqual};
    CFMutableDictionaryRef dictionary = CFDictionaryCreateMutable(kCFAllocatorDefault, 1, &keyCallbacks, &valueCallbacks);
    CFDictionaryAddValue(dictionary, CFSTR("rssi"), rssiString);
    CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), (CFStringRef)kMyoRSSI, NULL, dictionary, TRUE);
    CFRelease(rssiString);
    CFRelease(dictionary);

    sendData();
}

void DeviceListener::sendData(bool removeData)
{
    // Calculate framerate
    if (frameId % 50 == 0) {
        float delta = (clock() - lastTime) / (float) CLOCKS_PER_SEC;
        lastTime = clock();
        float frameRate = 50 / delta;
        
        CFStringRef frameRateString = CFStringCreateWithFormat(NULL, NULL, CFSTR("%.2f"), frameRate);
        CFDictionaryKeyCallBacks keyCallbacks = {0, NULL, NULL, CFCopyDescription, CFEqual, NULL};
        CFDictionaryValueCallBacks valueCallbacks  = {0, NULL, NULL, CFCopyDescription, CFEqual};
        CFMutableDictionaryRef dictionary = CFDictionaryCreateMutable(kCFAllocatorDefault, 1, &keyCallbacks, &valueCallbacks);
        CFDictionaryAddValue(dictionary, CFSTR("fps"), frameRateString);
        CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), (CFStringRef)kMyoFPS, NULL, dictionary, TRUE);
        CFRelease(frameRateString);
        CFRelease(dictionary);
    }
    
    NSNumber *frameIDNumber = @(frameId);
    [frame setValue:frameIDNumber forKey:@"id"];

    [wrapper setValue:frame forKey:@"frame"];
    
    // Encode JSON
    NSError *error;
    NSData *jsonData = [NSJSONSerialization dataWithJSONObject:wrapper options:NSJSONWritingPrettyPrinted error:&error];
    if (!jsonData) {
        NSLog(@"Error in JSON: %@", error);
    } else {
        NSString *jsonString = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
        server.send_message(std::string([jsonString UTF8String]));
    }

    if(removeData || removeDataAfterSend) {
        removeDataAfterSend = FALSE;
        [frame removeAllObjects];
    }
    
    frameId++;
}

void DeviceListener::toggleEuler(int toggleEulerValue) {
    std::cout << "MyoWebsocket::toggleEuler: " << toggleEulerValue << std::endl;
    switch (toggleEulerValue) {
        case 0:
            isCalculatingEuler = FALSE;
            break;
        case 1:
            isCalculatingEuler = TRUE;
            break;
        default:
            break;
    }
}