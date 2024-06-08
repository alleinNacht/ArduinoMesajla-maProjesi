#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Ekran boyutu
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// OLED reset pini (OLED ekrana bağlı değilse -1 yapın)
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// NRF24L01 ayarları
RF24 radio(9, 10); // CE, CSN pinleri
const byte address[6] = "00001"; // İletişim için kullanılan adres

// Buton pinleri
const int button1 = 2; // Sol
const int button2 = 3; // Yukarı
const int button3 = 4; // Sağ
const int button4 = 5; // Aşağı
const int button5 = 6; // Silme
const int button6 = 7; // Seçimi onaylama
const int button7 = 8; // Gönderme

char alphabet[29] = "ABCÇDEFGHIİJKLMNOÖPQRSTUÜVWXYZ ";
int selectedCharIndex = 0;
String message = "";
bool messageReceived = false;
String receivedMessage = "";

void setup() {
  // Buton pinleri giriş olarak ayarlanıyor
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(button4, INPUT_PULLUP);
  pinMode(button5, INPUT_PULLUP);
  pinMode(button6, INPUT_PULLUP);
  pinMode(button7, INPUT_PULLUP);

  // Ekran başlatılıyor
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // 0x3C I2C adresi
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.display();

  // NRF24L01 başlatılıyor
  radio.begin();
  radio.openReadingPipe(1, address);
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void loop() {
  if (messageReceived) {
    displayReceivedMessage();
    if (digitalRead(button6) == LOW) { // Seçimi onaylama butonuna basıldığında
      messageReceived = false;
      delay(200); // Debounce
    }
  } else {
    // Ekranı güncelle
    updateDisplay();

    // Butonları kontrol et ve işlem yap
    if (digitalRead(button1) == LOW) { // Sol
      selectedCharIndex--;
      if (selectedCharIndex < 0) {
        selectedCharIndex = 28; // Boşluk karakteri dahil
      }
      delay(200); // Debounce
    }

    if (digitalRead(button2) == LOW) { // Yukarı
      selectedCharIndex -= 14;
      if (selectedCharIndex < 0) {
        selectedCharIndex += 29; // Satırdan çıkmayacak şekilde döndür
      }
      delay(200); // Debounce
    }

    if (digitalRead(button3) == LOW) { // Sağ
      selectedCharIndex++;
      if (selectedCharIndex > 28) {
        selectedCharIndex = 0;
      }
      delay(200); // Debounce
    }

    if (digitalRead(button4) == LOW) { // Aşağı
      selectedCharIndex += 14;
      if (selectedCharIndex > 28) {
        selectedCharIndex -= 29; // Satırdan çıkmayacak şekilde döndür
      }
      delay(200); // Debounce
    }

    if (digitalRead(button5) == LOW) { // Silme
      if (message.length() > 0) {
        message.remove(message.length() - 1);
      }
      delay(200); // Debounce
    }

    if (digitalRead(button6) == LOW) { // Seçimi onaylama
      message += alphabet[selectedCharIndex];
      delay(200); // Debounce
    }

    if (digitalRead(button7) == LOW) { // Gönderme
      radio.stopListening();
      char text[32];
      message.toCharArray(text, 32);
      radio.write(&text, sizeof(text));
      message = ""; // Mesajı gönderdikten sonra temizle
      delay(200); // Debounce
      radio.startListening();
    }
  }

  // Gelen mesajları kontrol et
  if (radio.available()) {
    char text[32] = "";
    radio.read(&text, sizeof(text));
    receivedMessage = String(text);
    messageReceived = true;
  }
}

void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Message: ");
  display.println(message);

  display.setCursor(0, 20);
  display.print("Seçilen: ");
  display.println(alphabet[selectedCharIndex]);

  // Klavyeyi çiz
  display.setCursor(0, 40);
  for (int i = 0; i < 29; i++) {
    if (i == selectedCharIndex) {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Seçili karakteri ters renk yap
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    display.print(alphabet[i]);
    display.print(" ");
  }

  display.display();
}

void displayReceivedMessage() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Received: ");
  display.println(receivedMessage);
  display.display();
}
