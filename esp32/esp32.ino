#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define RELAY_PIN_1 2

bool measuring = false;
String temperature = "--";
String humidity = "--";

// WiFi
const char* ssid = "WIFI GIANG VIEN";
const char* password = "dhdn7799";

// Flask server
const char* flaskServerUrl = "http://172.16.69.170:5000/upload";
const char* flaskStateUrl  = "http://172.16.69.170:5000/state";

unsigned long lastStateCheck = 0;
const unsigned long stateCheckInterval = 5000;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  pinMode(RELAY_PIN_1, OUTPUT);
  digitalWrite(RELAY_PIN_1, LOW);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("🔌 Đang kết nối WiFi...");
  }

  Serial.println("✅ Đã kết nối WiFi.");
  Serial.print("🌐 IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // 1. Đọc dữ liệu từ UNO
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    data.trim();
    Serial.println("📥 Dữ liệu nhận: " + data);  // LOG MỚI

    if (data.startsWith("T:") && data.indexOf(", H:") > 0) {
      int tIndex = 2;
      int cIndex = data.indexOf(" *C");
      int hIndex = data.indexOf("H:") + 2;
      int pIndex = data.indexOf(" %");

      if (cIndex > 0 && pIndex > hIndex) {
        temperature = data.substring(tIndex, cIndex);
        humidity = data.substring(hIndex, pIndex);

        Serial.println("🌡️ Nhiệt độ: " + temperature + " °C, 💧 Độ ẩm: " + humidity + " %");

        // 2. Gửi dữ liệu nếu measuring == true
        if (WiFi.status() == WL_CONNECTED && measuring) {
          HTTPClient http;
          http.begin(flaskServerUrl);
          http.addHeader("Content-Type", "application/json");

          float t = temperature.toFloat();
          float h = humidity.toFloat();

          String json = "{\"temperature\":" + String(t, 2) + ",\"humidity\":" + String(h, 2) + "}";
          Serial.println("📤 JSON sẽ gửi: " + json);

          int responseCode = http.POST(json);
          String response = http.getString();

          Serial.println("📨 Mã phản hồi: " + String(responseCode));
          Serial.println("📥 Phản hồi từ server: " + response);

          if (responseCode > 0) {
            Serial.println("✅ Gửi dữ liệu thành công");
          } else {
            Serial.println("❌ Lỗi gửi Flask: " + http.errorToString(responseCode));
          }

          http.end();
        } else {
          Serial.println("⏸️ Đang tạm dừng gửi vì measuring = false");
        }
      } else {
        Serial.println("⚠️ Lỗi cắt chuỗi nhiệt độ hoặc độ ẩm");
      }
    } else {
      Serial.println("⚠️ Dữ liệu không đúng định dạng: " + data);
    }
  }

  // 3. Cập nhật trạng thái đo từ Flask mỗi 5 giây
  if (WiFi.status() == WL_CONNECTED && millis() - lastStateCheck > stateCheckInterval) {
    lastStateCheck = millis();
    HTTPClient http;
    http.begin(flaskStateUrl);
    int httpCode = http.GET();

    if (httpCode == 200) {
      String payload = http.getString();
      DynamicJsonDocument doc(128);
      DeserializationError error = deserializeJson(doc, payload);
      if (!error && doc.containsKey("measuring")) {
        measuring = doc["measuring"];
        Serial.println("📡 Trạng thái đo từ Flask: " + String(measuring ? "Bật" : "Tắt"));
      } else {
        Serial.println("❌ Lỗi phân tích JSON hoặc thiếu trường measuring.");
        Serial.println("📦 Payload: " + payload);
      }
    } else {
      Serial.println("⚠️ Lỗi lấy trạng thái từ Flask: " + http.errorToString(httpCode));
    }
    http.end();
  }

  // 4. Điều khiển relay
  digitalWrite(RELAY_PIN_1, measuring ? HIGH : LOW);
  delay(100);
}
