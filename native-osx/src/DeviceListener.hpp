#ifndef __DeviceListener__
#define __DeviceListener__

#include <cassert>
#include <iostream>
#include <myo/myo.hpp>
#include "DataHandler.hpp"

class DeviceListener : public myo::DeviceListener {
public:
    DeviceListener();
    ~DeviceListener();

    NSMutableDictionary *wrapper;
    NSMutableDictionary *frame = [[NSMutableDictionary alloc] init];
    bool removeDataAfterSend;
    bool isCalculatingEuler;

    static void toggleEulerHandler(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo);
    
    /// Called when a Myo has been paired.
    void onPair(myo::Myo *myo, uint64_t timestamp);
    
    /// Called when a Myo has been paired.
    void onUnpair(myo::Myo *myo, uint64_t timestamp);
    
    /// Called when a paired Myo has been connected.
    void onConnect(myo::Myo *myo, uint64_t timestamp);
    
    /// Called when a paired Myo has been disconnected.
    void onDisconnect(myo::Myo *myo, uint64_t timestamp);
    
    /// Called when a paired Myo recognizes that it is on an arm.
    void onArmRecognized(myo::Myo* myo, uint64_t timestamp, myo::Arm arm, myo::XDirection xDirection);
    
    /// Called when a paired Myo is moved or removed from the arm.
    void onArmLost(myo::Myo* myo, uint64_t timestamp);
    
    /// Called when a paired Myo has provided a new pose.
    void onPose(myo::Myo *myo, uint64_t timestamp, myo::Pose pose);
    
    /// Called when a paired Myo has provided new orientation data.
    void onOrientationData(myo::Myo *myo, uint64_t timestamp, const myo::Quaternion<float> &rotation);
    
    /// Called when a paired Myo has provided new accelerometer data in units of g.
    void onAccelerometerData(myo::Myo *myo, uint64_t timestamp, const myo::Vector3<float> &accel);
    
    /// Called when a paired Myo has provided new gyroscope data in units of deg/s.
    void onGyroscopeData(myo::Myo *myo, uint64_t timestamp, const myo::Vector3<float> &gyro);
    
    /// Called when a paired Myo has provided a new RSSI value.
    void onRssi(myo::Myo *myo, uint64_t timestamp, int8_t rssi);

    /// Sends all data in the dictionary, optional parameter for clearing dictionary
    void sendData(bool removeData = TRUE);
    
    void toggleEuler(int toggleEulerValue);

private:
    DataHandler server;
    int frameId = 0;
    thread processMessagesThread;
    thread dataHandlerThread;
    clock_t lastTime;
};
#endif /* defined(__DeviceListener__) */
