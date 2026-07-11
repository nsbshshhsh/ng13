// Định nghĩa thông tin Blynk
#define BLYNK_TEMPLATE_ID "TMPL6Y5gn9jax"
#define BLYNK_TEMPLATE_NAME "gas"
#define BLYNK_AUTH_TOKEN "zsgB3hBF7-3wm5lB75W1OQG7hbUikwLJ"

// Thêm các thư viện cần thiết
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <HardwareSerial.h>

// Thông tin WiFi
char ssid[] = "COFFEE KANA 24H L3";
char pass[] = "68686868";

// Cấu hình chân MQ2, Buzzer và ngưỡng
#define MQ2_PIN 39
#define BUZZER_PIN 23 // Buzzer trên GPIO23
#define GAS_THRESHOLD 800 // Ngưỡng rò rỉ khí gas (điều chỉnh sau khi đo)

// Khởi tạo LCD I2C (16x2, tự động phát hiện địa chỉ I2C)
hd44780_I2Cexp lcd;

// Khởi tạo UART2 cho sim7680cL
HardwareSerial sim7680c(2); // UART2: RX=16, TX=17

// Biến lưu trạng thái rò rỉ khí gas và buzzer
bool gasLeakDetected = false;
bool alertSent = false; // Tránh gửi thông báo Blynk liên tục
bool smsSent = false; // Tránh gửi SMS liên tục
bool buzzerEnabled = true; // Trạng thái bật/tắt buzzer
bool buzzerState = false; // Trạng thái thực tế của buzzer (true: kêu, false: không kêu)
bool buzzerCanBeTurnedOff = false; // Trạng thái cho phép tắt còi qua V2
bool isOnline = false; // Trạng thái kết nối WiFi/Blynk

// Số điện thoại để gửi SMS
String phoneNumber = "+84799113810"; // Số điện thoại của bạn

void sendBuzzerStatusToBlynk() {
  if (isOnline) {
    Blynk.virtualWrite(V2, buzzerState ? "ON" : "OFF"); // Gửi trạng thái "ON" hoặc "OFF" lên V2
  }
}

void sendSMS(String message) {
  Serial.println("Gửi SMS...");
  sim7680c.println("AT"); // Kiểm tra kết nối
  delay(100);
  sim7680c.println("AT+CMGF=1"); // Chế độ văn bản cho SMS
  delay(100);
  sim7680c.println("AT+CMGS=\"" + phoneNumber + "\""); // Số điện thoại nhận SMS
  delay(100);
  sim7680c.print(message); // Nội dung SMS
  delay(100);
  sim7680c.write(26); // Gửi Ctrl+Z để hoàn tất SMS (ASCII 26)
  delay(1000);
  Serial.println("Đã gửi SMS!");
}

// Blynk Virtual Pin V1 để reset cảnh báo
BLYNK_WRITE(V1) {
  int resetValue = param.asInt(); // Lấy giá trị từ Button "Reset Alert" (0 hoặc 1)
  if (resetValue == 1) {
    alertSent = false; // Reset trạng thái để gửi lại thông báo Blynk
    smsSent = false; // Reset trạng thái để gửi lại SMS
    gasLeakDetected = false; // Reset trạng thái rò rỉ
    buzzerEnabled = true; // Bật lại buzzer
    buzzerCanBeTurnedOff = false; // Vô hiệu hóa tắt còi
    digitalWrite(BUZZER_PIN, LOW); // Tắt buzzer khi reset
    buzzerState = false; // Cập nhật trạng thái buzzer
    sendBuzzerStatusToBlynk(); // Gửi trạng thái lên Blynk
    Serial.println("Cảnh báo đã được reset từ Blynk!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Canh bao reset");
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Khoi dong...");
    delay(2000);

  // Kết nối WiFi
  Serial.print("Đang kết nối WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nĐã kết nối WiFi!");
    Serial.print("Địa chỉ IP: ");
    Serial.println(WiFi.localIP());
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi OK");
    delay(2000);

    // Kết nối Blynk
    if (Blynk.connect()) {
      Serial.println("Đã kết nối với Blynk!");
      isOnline = true;
    } else {
      Serial.println("Không thể kết nối với Blynk!");
      isOnline = false;
    }
  } else {
    Serial.println("\nKết nối WiFi thất bại!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Loi!");
    delay(2000);
    isOnline = false;
  }

  }
}

// Blynk Virtual Pin V2 để điều khiển và hiển thị trạng thái buzzer
BLYNK_WRITE(V2) {
  int buzzerControlValue = param.asInt(); // Lấy giá trị từ Button V2 (0 hoặc 1)
  if (buzzerControlValue == 1 && buzzerCanBeTurnedOff) { // Chỉ cho phép tắt nếu buzzerCanBeTurnedOff = true
    buzzerEnabled = false; // Tắt buzzer
    digitalWrite(BUZZER_PIN, LOW); // Đảm bảo buzzer tắt
    buzzerState = false; // Cập nhật trạng thái buzzer
    sendBuzzerStatusToBlynk(); // Gửi trạng thái lên Blynk
    Serial.println("Buzzer đã được tắt từ Blynk!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Buzzer Tat");
    delay(2000);
  } else if (buzzerControlValue == 1 && !buzzerCanBeTurnedOff) {
    Serial.println("Không thể tắt còi: Nồng độ khí chưa vượt ngưỡng!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Khong tat duoc");
    delay(2000);
  }
}

void setup() {
  // Khởi tạo Serial để debug
  Serial.begin(115200);
  delay(1000);
  Serial.println("Khởi động ESP32...");

  // Khởi tạo UART2 cho sim7680cL
  sim7680c.begin(115200, SERIAL_8N1, 16, 17); // UART2: RX=16, TX=17
  delay(5000); // Chờ sim7680cL khởi động và kết nối mạng

  // Cấu hình chân MQ2 và Buzzer
  pinMode(MQ2_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); // Tắt buzzer khi khởi động
  buzzerState = false; // Trạng thái ban đầu của buzzer
  sendBuzzerStatusToBlynk(); // Gửi trạng thái ban đầu lên Blynk

  // Khởi tạo LCD
  Wire.begin();
  Wire.setClock(50000); // Giảm xung nhịp I2C để ổn định
  int status = lcd.begin(16, 2);
  if (status) {
    Serial.println("Khởi động LCD thất bại!");
    Serial.print("Mã lỗi: ");
    Serial.println(status);
    while (true);
  }
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Khoi dong...");
  delay(2000);

  // Cấu hình Blynk nhưng không chặn
  Blynk.config(BLYNK_AUTH_TOKEN);
  delay(1000);

  // Kết nối WiFi
  Serial.print("Đang kết nối WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nĐã kết nối WiFi!");
    Serial.print("Địa chỉ IP: ");
    Serial.println(WiFi.localIP());
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi OK");
    delay(2000);

    // Kết nối Blynk
    if (Blynk.connect()) {
      Serial.println("Đã kết nối với Blynk!");
      isOnline = true;
    } else {
      Serial.println("Không thể kết nối với Blynk!");
      isOnline = false;
    }
  } else {
    Serial.println("\nKết nối WiFi thất bại!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Loi!");
    delay(2000);
    isOnline = false;
  }
}

void loop() {
  // Kiểm tra kết nối WiFi và Blynk
  if (WiFi.status() == WL_CONNECTED && !isOnline) {
    if (Blynk.connect()) {
      Serial.println("Đã kết nối với Blynk!");
      isOnline = true;
    }
  } else if (WiFi.status() != WL_CONNECTED && isOnline) {
    Blynk.disconnect();
    isOnline = false;
    Serial.println("Mất kết nối WiFi/Blynk!");
  }

  if (isOnline) {
    Blynk.run();
  }

  // Đọc giá trị từ cảm biến MQ2
  int mq2Value = analogRead(MQ2_PIN) - 800;
  if (mq2Value < 0) mq2Value = 0;
  Serial.print("Giá trị MQ2: ");
  Serial.println(mq2Value);

  // Gửi dữ liệu lên Blynk (nếu kết nối)
  if (isOnline) {
    Blynk.virtualWrite(V0, mq2Value);
  }

  // Kiểm tra rò rỉ khí gas
  if (mq2Value > GAS_THRESHOLD) {
    Serial.println("CẢNH BÁO: Rò rỉ khí gas!");
    gasLeakDetected = true;
    buzzerCanBeTurnedOff = true; // Kích hoạt khả năng tắt còi
    if (buzzerEnabled) {
      digitalWrite(BUZZER_PIN, HIGH); // Bật buzzer nếu buzzerEnabled = true
      buzzerState = true; // Cập nhật trạng thái buzzer
    }
    if (!alertSent && isOnline) {
      String message = "CẢNH BÁO: Phát hiện rò rỉ khí gas! Giá trị MQ2: " + String(mq2Value);
      Blynk.logEvent("gas_leak", message);
      alertSent = true;
    }
    if (!smsSent) {
      sendSMS("Gas Leak Detected! MQ2 Value: " + String(mq2Value));
      smsSent = true;
    }
  } else {
    gasLeakDetected = false;
    digitalWrite(BUZZER_PIN, LOW); // Tắt buzzer
    buzzerState = false; // Cập nhật trạng thái buzzer
    buzzerCanBeTurnedOff = false; // Vô hiệu hóa tắt còi
    buzzerEnabled = true; // Đặt lại để còi kêu khi vượt ngưỡng lần sau
    alertSent = false; // Reset để gửi lại thông báo Blynk
    smsSent = false; // Reset để gửi lại SMS
  }

  // Gửi trạng thái buzzer lên Blynk qua V2
  sendBuzzerStatusToBlynk();

  // Hiển thị trên LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MQ2:");
  lcd.print(mq2Value);
  lcd.setCursor(10, 0); // Đặt vị trí cho trạng thái WiFi
  if (isOnline) {
    lcd.print("Wifi:1"); // "1" cho Online
  } else {
    lcd.print("Wifi:0"); // "0" cho Offline
  }
  lcd.setCursor(0, 1);
  if (gasLeakDetected) {
    lcd.print("Ro ri khi gas!");
  } else {
    lcd.print("Binh thuong");
  }

  // Delay để tránh đọc quá nhanh
  delay(1000);
}