<img src="https://logotype.se/myo/logo_daemon.png">

This is a Websockets daemon for Thalmic Labs Myo.

The Myo armband measures the electrical activity from your muscles to detect what gesture your hand is making. It also senses all of the motions and rotations of your hand and forearm.

[thalmic.com](http://www.thalmic.com)

<img src="https://logotype.se/myo/myodaemon.png?v4">

Quick start
-----------

[Download the app](https://github.com/logotype/myodaemon/blob/master/build/myodaemon.zip?raw=true), then run the examples in the MyoJS repository.

Features
-----------

+ Data rate up to 230 FPS
+ Enables web-applications easy access to Myo data
+ Connect multiple computers/devices to the same Myo
+ Packages Myo data into "frames" (MyoJS stores frames in a circular buffer, to determine movements over time)
+ Clean, lightweight and documented code
+ Reports framerate and RSSI
+ Resumes service after system wake-up

MyoJS - JavaScript framework for the Myo
-----------

[Check out MyoJS here](https://github.com/logotype/myoJS-VN).


JSON protocol
--------

```javascript
    {
      "frame" : {
        "id" : 43928,
        "rssi" : 53,
        "event" : {
          "type" : "onConnect"
        },
        "rotation" : [
          -0.4093628,
          -0.1088257,
          0.1548462,
          0.8925171
        ],
        "euler" : {
          "roll" : 1.34422,
          "pitch" : -1.428455,
          "yaw" : 2.271631
        },
        "pose" : {
          "type" : 5
        },
        "gyro" : [
          2.868652,
          -2.868652,
          2.563476
        ],
        "accel" : [
          0.04736328,
          -0.7241211,
          0.6367188
        ]
      }
    }
```


Authors
-------

**Victor Norgren**

+ https://twitter.com/logotype
+ https://github.com/logotype
+ https://logotype.se


Copyright and license
---------------------

Copyright © 2014 logotype

Author: Victor Norgren

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:  The above copyright
notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
