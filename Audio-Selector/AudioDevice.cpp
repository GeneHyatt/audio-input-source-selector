/*
  AudioDevice.cpp - class to encapsulate an audio device for use in AudioSelector.
  Created by Gene Hyatt Jan. 20 2022.
*/

#include "Arduino.h"
#include "AudioDevice.h"

AudioDevice::AudioDevice(int pin, String label)
{
  pinMode(pin, OUTPUT);
  _pin = pin;
  _isActive = false;
  _label = label;
}

bool AudioDevice::isActive;

String AudioDevice::label = label;

void AudioDevice::on()
{
  digitalWrite(_pin, HIGH);
  _isActive = !_isActive;
}

void AudioDevice::off()
{
  digitalWrite(_pin, LOW);
  _isActive = !_isActive;
}
