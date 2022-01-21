/*
  AudioDevice.h - class to encapsulate an audio device for use in AudioSelector.
  Created by Gene Hyatt Jan. 20 2022.
*/
#ifndef AudioDevice_h
#define AudioDevice_h

#include "AudioDevice.h"

class AudioDevice
{
  public:
    AudioDevice(int pin, String label);

    static bool isActive;
    static String label;
    void on();
    void off();
  private:
    int _pin;
    int _isActive;
    String _label;
};

#endif
