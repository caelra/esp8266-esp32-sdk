#ifndef _SINRICWINDOWAC_H_
#define _SINRICWINDOWAC_H_

#include "SinricProDevice.h"
#include <ArduinoJson.h>

class SinricProWindowAC :  public SinricProDevice {
  public:
	  SinricProWindowAC(const char* deviceId, unsigned long eventWaitTime=30000);
    // callback
	  typedef std::function<bool(const String, bool&)> PowerStateCallback; 
    typedef std::function<bool(const String, int&)> RangeValueCallback;
    typedef std::function<bool(const String, float&)> TargetTemperatureCallback;
    typedef std::function<bool(const String, String&)> ThermostatModeCallback;

    void onPowerState(PowerStateCallback cb) { powerStateCallback = cb; }
    void onRangeValue(RangeValueCallback cb) { rangeValueCallback = cb; }
    void onAdjustRangeValue(RangeValueCallback cb) { adjustRangeValueCallback = cb; }
    void onTargetTemperatue(TargetTemperatureCallback cb) { targetTemperatureCallback = cb; }
    void onThermostatMode(ThermostatModeCallback cb) { thermostatModeCallback = cb; }

    // event
    bool sendPowerStateEvent(bool state, String cause = "PHYSICAL_INTERACTION");
    bool sendRangeValueEvent(int rangeValue, String cause = "PHYSICAL_INTERACTION");
    void sendTemperatureEvent(float temperature, float humidity = -1, String cause = "PERIODIC_POLL");
    void sendTargetTemperatureEvent(float temperature, String cause = "PHYSICAL_INTERACTION");
    void sendThermostatModeEvent(String thermostatMode, String cause = "PHYSICAL_INTERACTION");

    // handle
    bool handleRequest(const char* deviceId, const char* action, JsonObject &request_value, JsonObject &response_value) override;
  private:
    PowerStateCallback powerStateCallback;
    RangeValueCallback rangeValueCallback; 
    RangeValueCallback adjustRangeValueCallback; 
    TargetTemperatureCallback targetTemperatureCallback;
    ThermostatModeCallback thermostatModeCallback;

};

SinricProWindowAC::SinricProWindowAC(const char* deviceId, unsigned long eventWaitTime) : SinricProDevice(deviceId, eventWaitTime),
  powerStateCallback(nullptr),
  rangeValueCallback(nullptr),
  adjustRangeValueCallback(nullptr),
  targetTemperatureCallback(nullptr),
  thermostatModeCallback(nullptr) {}

bool SinricProWindowAC::handleRequest(const char* deviceId, const char* action, JsonObject &request_value, JsonObject &response_value) {
  if (strcmp(deviceId, this->deviceId) != 0) return false;
  bool success = false;
  String actionString = String(action);

  if (actionString == "setPowerState" && powerStateCallback) {
    bool powerState = request_value["state"]=="On"?true:false;
    success = powerStateCallback(String(deviceId), powerState);
    response_value["state"] = powerState?"On":"Off";
    return success;
  }

  if (actionString == "setRangeValue" && rangeValueCallback) {
    int rangeValue = request_value["rangeValue"] | 0;
    success = rangeValueCallback(String(deviceId), rangeValue);
    response_value["rangeValue"] = rangeValue;
    return success;
  }

  if (actionString == "adjustRangeValue" && adjustRangeValueCallback) {
    int rangeValueDelta = request_value["rangeValueDelta"] | 0;
    success = adjustRangeValueCallback(String(deviceId), rangeValueDelta);
    response_value["rangeValue"] = rangeValueDelta;
    return success;
  }


  if (actionString == "targetTemperature" && targetTemperatureCallback) {
    float temperature = request_value["temperature"] | -1;
    success = targetTemperatureCallback(String(deviceId), temperature);
    response_value["temperature"] = temperature;
    return success;
  }

  if (actionString == "setThermostatMode" && thermostatModeCallback) {
    String thermostatMode = request_value["thermostatMode"] | "";
    success = thermostatModeCallback(String(deviceId), thermostatMode);
    response_value["thermostatMode"] = thermostatMode;
    return success;
  }

  return success;
}

bool SinricProWindowAC::sendPowerStateEvent(bool state, String cause) {
  DynamicJsonDocument eventMessage = prepareEvent(deviceId, "setPowerState", cause.c_str());
  JsonObject event_value = eventMessage["payload"]["value"];
  event_value["state"] = state?"On":"Off";
  return sendEvent(eventMessage);
}

bool SinricProWindowAC::sendRangeValueEvent(int rangeValue, String cause) {
  DynamicJsonDocument eventMessage = prepareEvent(deviceId, "setRangeValue", cause.c_str());
  JsonObject event_value = eventMessage["payload"]["value"];
  event_value["rangeValue"] = rangeValue;
  return sendEvent(eventMessage);
}

void SinricProWindowAC::sendTemperatureEvent(float temperature, float humidity, String cause) {
  DynamicJsonDocument eventMessage = prepareEvent(deviceId, "currentTemperature", cause.c_str());
  JsonObject event_value = eventMessage["payload"]["value"];
  event_value["humidity"] = humidity;
  event_value["temperature"] = temperature;
  sendEvent(eventMessage);
}

void SinricProWindowAC::sendTargetTemperatureEvent(float temperature, String cause) {
  DynamicJsonDocument eventMessage = prepareEvent(deviceId, "targetTemperature", cause.c_str());
  JsonObject event_value = eventMessage["payload"]["value"];
  event_value["temperature"] = temperature;
  sendEvent(eventMessage);
}

void SinricProWindowAC::sendThermostatModeEvent(String thermostatMode, String cause) {
  DynamicJsonDocument eventMessage = prepareEvent(deviceId, "setThermostatMode", cause.c_str());
  JsonObject event_value = eventMessage["payload"]["value"];
  event_value["thermostatMode"] = thermostatMode;
  sendEvent(eventMessage);
}

#endif