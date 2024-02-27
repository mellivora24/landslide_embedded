#include <WiFi.h>
#include <HTTPClient.h>
#include <iterator>
#include <ArduinoJson.h>
#include <TinyGPSPlus.h>
#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

/*-----------------------------------------------------------------*/

const byte h = 35;
const byte water = 15;
const byte rain = 5;

const char* ssid = "xxx";
const char* password = "xxx";
// const char* mainURL = "xxx";
const char* updateURL = "xxx";

String ten = "TEST 1";  // Mảng lưu tên địa điểm

bool check = false;
bool isRaining = false;
unsigned long time_in_minutes = 0;
unsigned long start = millis();
/*-----------------------------------------------------------------*/

HardwareSerial GPS(1);
TinyGPSPlus gps;
Adafruit_MPU6050 mpu;

static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (GPS.available())
      gps.encode(GPS.read());
  } while (millis() - start < ms);
}

/*-----------------------------------------------------------------*/

// Hàm đọc giá trị độ ẩm
int get_h() {
  int gia_tri = ((4095 - analogRead(h)) * 100) / 4095;
  return gia_tri;
}

// Hàm đọc lưu lượng nước
int flow_rate() {
  int dem = 0;
  int count = 0;
  // Đọc trạng thái của chân đầu vào
  // Serial.println("start");
  while (dem < 2000000) {
    // Kiểm tra nếu mức logic là HIGH, tăng biến đếm
    if ((digitalRead(water) == HIGH) && check == 0) {
      count++;
      check = true;
    }
    if ((digitalRead(water) == LOW) && check == 1) {
      count++;
      check = false;
    }
    dem++;
  }
  // Serial.println(dem);
  // Serial.println("end");
  // Hiển thị số lần nhận mức logic 1
  // Serial.println(count);
  return count;
}

// Hàm tính độ nghiêng
float angle_percent() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float phan_tram_nghieng = a.acceleration.z*9;

  if (phan_tram_nghieng >= 0) return 90 - phan_tram_nghieng;
  else return phan_tram_nghieng;
}

// Hàm tính thời gian mưa
float rain_time() {
  if (digitalRead(rain)) {
    isRaining = false;
    time_in_minutes = 0;
  } else if (!digitalRead(rain) && !isRaining) {
    isRaining = true;
    time_in_minutes = millis()*0.0000166667;
    return 1.0;
  } else if (!digitalRead(rain) && isRaining) {
    return float(millis()*0.0000166667 - time_in_minutes);
  }
  return time_in_minutes;
}

//Hàm tính tỉ lệ sạt lở
byte landslide_rate() {
  byte a = get_h();
  byte b = digitalRead(rain);
  byte c = angle_percent();

  byte t = 0;
  if (c < 30) {
    if (20 > a > 10) {
      if (b == 1) t = 0;
      if (b == 0) t = 1;
    }
    if (40 > a > 20) t = 2;
    if (a > 40) {
      if (b == 1) t = 3;
      if (b == 0) t = 4;
    }
  } else t = 4;
  return t;
}

// Hàm gửi tín hiệu qua sóng lora
void loraSend() {
  if (landslide_rate() == 4) {
    Serial.println("+");
  } else Serial.println("A");
}

/*-----------------------------------------------------------------*/

// Hàm tạo mới một điểm trong Database -> OK!
/* void creatNewPoint() {
  if (WiFi.status() == WL_CONNECTED && GPS.available()) {

    String kinhdo;  // Mảng lưu vị trí
    String vido;    // theo kinh độ và vĩ độ

    smartDelay(1000);

    kinhdo = String(gps.location.lat(), 6);
    vido = String(gps.location.lng(), 6);

    WiFiClient client;

    DynamicJsonDocument doc(300);

    doc["name"] = ten;
    doc["kinhDo"] = kinhdo;
    doc["viDo"] = vido;
    doc["doAm"] = 50;
    doc["luongMua"] = 1500;
    doc["satLo"] = 30;
    doc["status"] = "An toàn";

    String dataToSend;
    serializeJson(doc, dataToSend);

    HTTPClient http;

    http.begin(mainURL);
    http.addHeader("Content-Type", "application/json");

    int httpCode = http.POST(dataToSend);

    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
        String response_data = http.getString();

        //Chuyển dữ liệu thành json
        DynamicJsonDocument json_data(300);
        DeserializationError error = deserializeJson(json_data, response_data);

        if (error) {
          Serial.print("Gặp lỗi khi chuyển dữ liệu sang JSON, mã: ");
          Serial.println(error.c_str());
          http.end();
          return;
        } else {
          // Trích xuất dữ liệu từ JSON
          if (json_data.containsKey("_id")) {
            ID = json_data["_id"].as<String>();
          } else {
            Serial.println("Không tìm được ID");
          }
          Serial.println("Điểm này chưa được thêm, đã thêm vào Database.");
        }
      } else {
        Serial.printf("=> Không thể thêm dữ liệu lên Database, mã lỗi: %s\n", http.errorToString(httpCode).c_str());
        Serial.println("--------------------------------------------------");
        http.end();
        return;
      }
      http.end();
    }
  } else {
    Serial.println("GPS chưa khởi động xong...");
    bool leap = true;
    while (!leap) {
      Serial.print(".");
      if (gps.location.lat() != 0.00) leap = false;
      smartDelay(1000);
    }
  }
}*/

// Cập nhật các giá trị lên DataBase
void updateValue() {
  // if (gps.location.lat() != 0 && gps.location.lng() != 0) {
    WiFiClient client;

    DynamicJsonDocument doc(1024);


    doc["name"] = ten;

    if (gps.location.lat() != 0 && gps.location.lng() != 0) {
      doc["kinhDo"] = String(gps.location.lat(), 5);
      doc["viDo"] = String(gps.location.lng(), 5);
    }

    int temp = landslide_rate();

    doc["doAm"] = get_h();
    doc["luongMua"] = flow_rate();
    doc["satLo"] = temp;
    if (temp < 2) doc["status"] = "An toàn";
    else doc["status"] = "Cảnh báo";
    doc["thoiGianMua"] = round(rain_time());
    doc["doNghieng"] = round(angle_percent());

    String dataToSend;
    serializeJson(doc, dataToSend);

    HTTPClient http;

    http.begin(updateURL);
    http.addHeader("Content-Type", "application/json");

    int httpCode = http.PUT(dataToSend);

    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
        // Serial.print("=> Cập nhật dữ liệu thành công: ");
        // String payload = http.getString();
        // Serial.println(payload);
        http.end();
        return;
      }
    } else {
      // Serial.printf("=> Không thể cập nhật dữ liệu, mã lỗi: %s\n", http.errorToString(httpCode).c_str());
      http.end();
      return;
    }
  /*} else if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println("Kiểm tra dây Module GPS!");
    Serial.println(" => Không thể lấy dữ liệu GPS!");
    Serial.print("    ");
    Serial.print(String(gps.location.lat(), 5));
    Serial.print(" - ");
    Serial.println(String(gps.location.lng(), 5));
    Serial.println("***************************");
    smartDelay(1000);
  }*/
}

/* Kiểm tra xem đã tồn tại điểm này trên database hay chưa
bool isAvailable() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    int response = http.begin(mainURL);
    response = http.GET();

    if (response > 0) {
      Serial.println("Nhận dữ liệu từ Database thành công!");

      String response_data = http.getString();

      //Chuyển dữ liệu thành json
      DynamicJsonDocument json_data(1024);
      DeserializationError error = deserializeJson(json_data, response_data);

      if (error) {
        Serial.print("Gặp lỗi khi chuyển dữ liệu sang JSON, mã: ");
        Serial.println(error.c_str());
        Serial.println("--------------------------------------------------");
        return true;
      } else {
        // Trích xuất dữ liệu từ JSON
        if (json_data.containsKey("name")) {

          String name = json_data["name"].as<String>();

          if (!strcmp(ten.c_str(), name.c_str())) {
            http.end();
            Serial.println("--------------------------------------------------");
            return false;
          }else {
            http.end();
            Serial.println("--------------------------------------------------");
            return true;
          }
        } else {
          http.end();
          Serial.println("--------------------------------------------------");
          return false;
        }
      }
    } else {
      Serial.println("Gặp lỗi trong quá trình nhận dữ liệu từ Database!");
      Serial.println("--------------------------------------------------");
      http.end();
    }
    http.end();
  }
  return true;
}*/

void connectWiFi() {
  // Serial.print("Đang kết nối tới WiFi ");
  // Serial.print(ssid);

  int count = 0;

  while (WiFi.status() != WL_CONNECTED) {
    count++;
    delay(1000);
    if (count == 20) {
      // Serial.println();
      // Serial.print("Không thể kết nối với WiFi ");
      // Serial.print(ssid);
      // Serial.println(", hãy thử lại sau vài phút.");
      while (1) {};
    } /*else Serial.print(".");*/
  }

  // Serial.println();
  // Serial.print("=> Đã kết nối với ");
  // Serial.println(ssid);
  // Serial.println("--------------------------------------------------");
}
/*-----------------------------------------------------------------*/

// Hàm khởi tạo
void setup() {
  pinMode(h, INPUT);
  pinMode(water, INPUT_PULLUP);
  pinMode(rain, INPUT);

  while (!mpu.begin()) { delay(10); }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.begin(9600);

  WiFi.begin(ssid, password);
  connectWiFi();

  GPS.begin(9600, SERIAL_8N1, 25, 26); //TX = 25, RX=26

  smartDelay(1000);

  // if(!isAvailable()) {
  //   delay(5000);
  //   creatNewPoint();
  // }
}

// Hàm chính
void loop() {
  if (millis() - start >= 10000) {
    start = millis();
    updateValue();
  }
  loraSend();
  Serial.print(gps.location.lat());
  Serial.print(" - ");
  Serial.println(gps.location.lng());
  delay(500);
}
