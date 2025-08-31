#include <Arduino.h>

#if defined(CONFIG_TINYUSB_ENABLED)
#include "USBHID.h"
#include "USB.h"
#include "USBHIDMouse.h"
#include "USBHIDKeyboard.h"
#include "USBHIDGamepad.h"
#include "USBHIDConsumerControl.h"
#include "USBHIDSystemControl.h"
#endif

#if defined(CONFIG_SOC_BLE_SUPPORTED)
#include "BleConnectionStatus.h"
#include "BleCompositeHID.h"
#include "KeyboardDevice.h"
#include "MouseDevice.h"
#include <NimBLEDevice.h>
#endif

#if defined(CONFIG_TINYUSB_ENABLED)
USBHIDSystemControl UsbSystemControl;
USBHIDConsumerControl UsbConsumerControl;
#endif

#if defined(CONFIG_SOC_BLE_SUPPORTED)
KeyboardDevice* keyboard;
MouseDevice* mouse;
BleCompositeHID compositeHID("CompositeHID Keyboard-Mouse", "Mystfit", 100);
#endif
bool pressed = false;
bool pinBootState = false;

void setup() {
    Serial.begin(115200);
  
#if defined(CONFIG_SOC_BLE_SUPPORTED)
    // Set up keyboard
    KeyboardConfiguration keyboardConfig;
    // Media keys are not enabled by default
    keyboardConfig.setUseMediaKeys(true);
    // SystemControl keys are not enabled by default
    keyboardConfig.setUseSystemControl(true);
    keyboardConfig.setAutoReport(true);
    keyboard = new KeyboardDevice(keyboardConfig);

    // Set up mouse
    MouseConfiguration mouseConfig;
    mouseConfig.setAutoReport(true);
    mouse = new MouseDevice(mouseConfig);

     // Add both devices to the composite HID device to manage them
    compositeHID.addDevice(keyboard);
    compositeHID.addDevice(mouse);

    // Start the composite HID device to broadcast HID reports
    compositeHID.begin();
#endif
#if defined(CONFIG_TINYUSB_ENABLED)
    UsbSystemControl.begin(); 
    UsbConsumerControl.begin();
    USB.begin();
#endif
    pinMode(BOOT_PIN,INPUT_PULLUP);
    pinMode(LED_BUILTIN,OUTPUT);
    pinBootState = digitalRead(BOOT_PIN);
    delay(3000);
}
void loop() {
  if(digitalRead(BOOT_PIN) != pinBootState){
    pressed = !pressed;
    pinBootState = digitalRead(BOOT_PIN);
    if(pinBootState==LOW){
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      Serial.println("Button pressed, new state: " + String(pressed));
#if defined(CONFIG_SOC_BLE_SUPPORTED)      
      keyboard->mediaKeyPress(KEY_MEDIA_MUTE); // KEY_MEDIA_MUTE and CONSUMER_CONTROL_MUTE are not the same
      delay(50);
      keyboard->mediaKeyRelease(KEY_MEDIA_MUTE);
      delay(1000);
      
      keyboard->SystemControlPress(SYSTEM_CONTROL_STANDBY);
      delay(50);
      keyboard->SystemControlRelease();
#endif
#if defined(CONFIG_TINYUSB_ENABLED)
      UsbConsumerControl.press(CONSUMER_CONTROL_MUTE);
      delay(50);
      UsbConsumerControl.release();
      delay(1000);
      // If using USB HID
      UsbSystemControl.press(SYSTEM_CONTROL_STANDBY);
      delay(50);
      UsbSystemControl.release();
#endif  
    } 
  }
}