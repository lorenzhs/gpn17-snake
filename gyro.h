#pragma once

#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

#define BNO055_SAMPLERATE_DELAY_MS (100)

class Gyro {
    using Vec = imu::Vector<3>;

    Adafruit_BNO055 bno = Adafruit_BNO055(BNO055_ID, BNO055_ADDRESS_B);

    Vec neutral;

public:
    void init() {
        bno.begin();
    }

    void calibrate() {
        // TODO more fanciness
        neutral = get_euler();
    }

    Vec get_euler() {
        return bno.getVector(Adafruit_BNO055::VECTOR_EULER);
    }

    JoystickState get_joystick() {
        Vec state = get_euler();
        Vec diff = state - neutral;

        // "Enter" if centered
        if (abs(diff.y()) < 10 && abs(diff.z()) < 10) {
            pixels.setPixelColor(0, pixels.Color(0, 10, 0));
            pixels.setPixelColor(1, pixels.Color(0, 10, 0));
            pixels.setPixelColor(2, pixels.Color(0, 10, 0));
            pixels.setPixelColor(3, pixels.Color(0, 10, 0));

            float r = 0, c;
            if (abs(diff.y()) > 7 && abs(diff.z()) > 7) {
                // Two large deviations, yellow warning
                r = 1;
            }

            if (diff.y() < 0) {
                c = 10-5*diff.y();
                pixels.setPixelColor(1, pixels.Color(r*c, c, 0));
            } else {
                c = 10 + 5*diff.y();
                pixels.setPixelColor(2, pixels.Color(r*c, c, 0));
            }

            if (diff.z() < 0) {
                c = 10-5*diff.z();
                pixels.setPixelColor(0, pixels.Color(r * c, c, 0));
            } else {
                c = 10+5*diff.z();
                pixels.setPixelColor(3, pixels.Color(r * c, c, 0));
            }

            return JoystickState::BTN_NOTHING;
        }

        if (diff.y() > 10 && abs(diff.z()) < 8) {
            return JoystickState::BTN_RIGHT;
        }
        if (diff.y() < -10 && abs(diff.z()) < 8) {
            return JoystickState::BTN_LEFT;
        }
        if (abs(diff.y()) < 8 && diff.z() > 10) {
            return JoystickState::BTN_DOWN;
        }
        if (abs(diff.y()) < 8 && diff.z() < -10) {
            return JoystickState::BTN_UP;
        }

        return JoystickState::BTN_NOTHING;
    }
};
