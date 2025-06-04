#include <DHT.h>

#define DHTPIN 8         // Cập nhật đúng chân DATA mới
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  Serial.println("Dang khoi dong DHT...");
  dht.begin();
}

void loop() {
  float temp = dht.readTemperature();
  float humi = dht.readHumidity();

  if (!isnan(temp) && !isnan(humi)) {
    Serial.print("T:");
    Serial.print(temp);
    Serial.print(" *C, H:");
    Serial.print(humi);
    Serial.println(" %");
  } else {
    Serial.println("ERROR: Khong doc duoc DHT11");
  }

  delay(3000);
}
