/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL6ywERumGa"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "0xqXFnSUNqEUFg5C5Dp0eIXL4B8BDbME"

#define BLYNK_PRINT Serial

String PPMcontent;
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <time.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <MQ135.h>
#include "MAX30100_PulseOximeter.h"
#include <BlynkSimpleEsp32.h>
#include "Adafruit_SHT31.h"
#include <WiFiManager.h>
#include <Adafruit_MLX90614.h>
WiFiManager wifiManager;
//// Wifi ////
#define SSID_WIFI "AmericanStudy"
#define PASS_WIFI "66668888"

// #define SSID_WIFI "Minhthuy"
// #define PASS_WIFI "68401184"
//// OpenWeather ////
#define LATITUDE "21.0787038"    // Ex: "10.77"
#define LONGITUDE "105.8656097"  // Ex: "106.65"
#define API_KEY "e6fa53bea61f102d5f3b16a80d468d22"
//
// Check the API call limits per hour/minute to avoid getting blocked/banned
unsigned long lastTime = 0;
#define TIME_UPDATE 30000UL  // Unit (ms)
String _time;
//// API "Current weather data" ////
#define NAME_URL "https://api.openweathermap.org/data/3.0/onecall"
#define PARAM_LAT "?lat="
#define PARAM_LON "&lon="
#define PARAM_EXCLUDE "&exclude=minutely,hourly,daily"
#define PARAM_API "&appid="

//String URL = String(NAME_URL) + String(PARAM_LAT) + String(LATITUDE) + String(PARAM_LON) + String(LONGITUDE) + String(PARAM_EXCLUDE)+ String(PARAM_API) + String(API_KEY) ;
String URL = "https://api.openweathermap.org/data/3.0/onecall?lat=21.018605&lon=105.7958367&appid=e6fa53bea61f102d5f3b16a80d468d22";

String asthmaCondition = "";

//// Oled ////
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
//
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  //
#define TIME_VIEW 3000       // Unit (ms)


TwoWire I2C_1 = TwoWire(0);
TwoWire I2C_2 = TwoWire(1);

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
//// Oled ////
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &I2C_1, OLED_RESET);
Adafruit_SHT31 sht31 = Adafruit_SHT31(&I2C_2);
//// JSON ////
String jsonBuffer;
JSONVar myObject;
//#define REPORTING_PERIOD_MS 1000

const float HEART_RATE_THRESHOLD = 100;    // Nhịp tim trên 100 BPM là bất thường
const float SPO2_THRESHOLD = 92;           // SpO2 dưới 92% là nguy hiểm
const float CO2_THRESHOLD = 1000;          // PPM của MQ135 trên 1000 là nguy hiểm
const float TEMP_LOW_THRESHOLD = 18;       // Nhiệt độ dưới 18°C
const float HUMIDITY_HIGH_THRESHOLD = 60;  // Độ ẩm trên 60%
const int UV_INDEX_THRESHOLD = 7;          // Chỉ số UV trên 7 là rất cao

double temperature, humidity, nhiptim, oxy, nhietdo, correctedPPM;
String quality = "";
double UV_index;

PulseOximeter pox;
MQ135 mq135_sensor(34);


bool check1 = true, check2 = true, check3 = true, check4 = true, check5 = true, StateWarning = false;

BLYNK_WRITE(V4) {
  StateWarning = param.asInt();
}

void setup() {
  Serial.begin(115200);
  // Wire.begin();
  I2C_1.begin(16, 17);
  I2C_2.begin(21, 22);
  pinMode(15, INPUT);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }
  display.clearDisplay();               // Clear the buffer
  display.setRotation(2);               // Rotate the screen vertically
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.cp437(true);                  // Use full 256 char 'Code Page 437' font

  display.clearDisplay();
  display.setCursor(0, 24);
  display.setTextSize(3);
  display.print(" HELLO  ");
  display.display();
  delay(5000);
  display.clearDisplay();
  //////////////////////////////////////
  display.setCursor(0, 10);
  display.setTextSize(1);
  display.print("WIFI: ");
  display.setCursor(0, 25);
  display.setTextSize(1);
  display.print("IP: ");
  display.setCursor(0, 40);
  display.setTextSize(1);
  display.print("Blynk: ");

  display.display();
  if (!mlx.begin()) {
    Serial.println("Error connecting to MLX sensor. Check wiring.");
    while (1);
  };
  if (!pox.begin()) {
    Serial.println("POX: KHÔNG THÀNH CÔNG");
    //failed();
    while (1)
      ;
  } else {
    Serial.println("POX: THÀNH CÔNG");
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_14_2MA);

  Wire.setClock(100000);
  /////////////////////////////////////////////
  WiFi.begin(SSID_WIFI, PASS_WIFI);

  Serial.println(F("Connecting"));

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
    display.setCursor(31, 10);
    display.print("connecting.");
    display.display();
    delay(1000);
    display.setCursor(31, 10);
    display.print("              ");
    display.setCursor(31, 10);
    display.print("connecting..");
    display.display();
    delay(1000);
    display.setCursor(31, 10);
    display.print("              ");
    display.setCursor(31, 10);
    display.print("connecting...");
    display.display();
    delay(500);
    display.setCursor(81, 10);
    display.print("   ");
    display.display();
    //  wifiManager.autoConnect("Asthma Device", "88888888");
  }
  display.setCursor(31, 10);
  display.print("              ");
  display.print("OK");
  display.display();
  Serial.println(F(""));
  //
  Serial.print(F("Connected to WiFi network with IP Address: "));
  Serial.println(WiFi.localIP());
  //
  display.setCursor(21, 25);
  display.setTextSize(1);
  display.print(WiFi.localIP());
  display.display();
  Serial.print(F("Pls, wait in about "));
  Serial.print(TIME_UPDATE / 1000);
  Serial.println(" seconds");
  Blynk.begin(BLYNK_AUTH_TOKEN, SSID_WIFI, PASS_WIFI);

  display.setCursor(35, 40);
  display.setTextSize(1);
  display.print("connected");
  display.display();
  delay(2000);
  Serial.println("SHT31 test");
  if (!sht31.begin(0x44)) {  // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }
}


unsigned long lastUpdate = 0;
void loop() {
  Blynk.run();
    pox.update();
  displayOled2();  // Show current weather information
  delay(5000);
  displayOled1();
  delay(5000);
  displayOled3();
  delay(5000);
  displayOled4();
  delay(5000);


  Blynk.virtualWrite(V0, "Hum:" + String(humidity) + "% | Temp:" + String(temperature) + "°C");  // Độ ẩm                                                             // Nhịp tim
  Blynk.virtualWrite(V1, "Heart Rate:" + String(nhiptim) + " | SpO2:" + String(oxy) + "% | Body Temp:" + String(nhietdo) + "°C");
  Blynk.virtualWrite(V2, "Air quality:" + PPMcontent + "PPM | UV index:" + String(UV_index));  // SpO2                                                                     // Nhiệt độ từ xa                                                          // Chỉ số UV
  Blynk.virtualWrite(V3, checkAsthmaCondition());
}


void displayOled1() {
  if (digitalRead(15) == HIGH) {
    nhietdo = mlx.readObjectTempC()0;
    oxy =  pox.getSpO2();
    nhiptim = pox.getHeartRate();
  } else {
    nhietdo = 0;
    oxy = 0;      // pox.getSpO2();
    nhiptim = 0;  //pox.getHeartRate();
  }


  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.print("  REPORT");

  display.setCursor(0, 18);
  display.setTextSize(1);
  display.print("  At this location");

  display.setCursor(0, 30);
  display.setTextSize(1);
  display.print("Heart Rate: ");
  display.print(int(nhiptim));
  display.print("BPM");
  display.setCursor(0, 42);
  display.setTextSize(1);
  display.print("SpO2: ");
  display.print(int(oxy));
  display.print("%");
  display.setCursor(0, 54);
  display.setTextSize(1);
  display.print("Body temp:");
  display.print(int(nhietdo));
  display.print("*C");

  display.display();
}


void displayOled2() {

  temperature = sht31.readTemperature();
  humidity = sht31.readHumidity();
  correctedPPM = mq135_sensor.getCorrectedPPM(temperature, humidity);
  ;
  if (correctedPPM < 500) {
    PPMcontent = "good";
  } else if (correctedPPM <= 1200 && correctedPPM >= 500) {
    PPMcontent = "fair";
  } else {
    PPMcontent = "poor";
  }
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.print("  REPORT");


  display.setCursor(0, 18);
  display.setTextSize(1);
  display.print("  At this location");

  display.setCursor(0, 30);
  display.setTextSize(1);
  display.print("Temp: ");
  display.print(temperature);
  display.setCursor(0, 42);
  display.setTextSize(1);
  display.print("Hum: ");
  display.print(humidity);
  display.setCursor(0, 54);
  display.print("Air quality: ");
  display.print(PPMcontent);

  display.display();
}
void displayOled4() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.print("  WARNING");
  display.setCursor(0, 20);
  display.setTextSize(1);
  display.print("Content: \n  " + checkAsthmaCondition());
  display.display();
}
void displayOled3() {

  /* ---------------- Send an HTTP GET request --------------- */
  if ((millis() - lastTime) >= TIME_UPDATE) {
    /* ------------- Check WiFi connection status ------------ */
    if (WiFi.status() == WL_CONNECTED) {
      jsonBuffer = httpGETRequest(URL.c_str());
      Serial.println(jsonBuffer);
      myObject = JSON.parse(jsonBuffer);

      // Check received JSON packet has data?
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println(F("Parsing input failed!"));
        return;
      }
      /// t = double(myObject["current"]["temp"]);
      //  h = double(myObject["current"]["humidity"]);

      UV_index = double(myObject["current"]["uvi"]);
      int32_t epochTime = int32_t(myObject["current"]["dt"]);
      struct tm *tm_struct = localtime((time_t *)&epochTime);

      // Lấy các giá trị ngày tháng và giờ
      int year = tm_struct->tm_year + 1900;
      int month = tm_struct->tm_mon + 1;
      int day = tm_struct->tm_mday;
      int hour = tm_struct->tm_hour + 7;
      int minute = tm_struct->tm_min;
      int second = tm_struct->tm_sec;
      int weekday = tm_struct->tm_wday;  // Thứ trong tuần (0=Chủ Nhật, 6=Thứ Bảy)

      _time = String(hour) + ":" + String(minute) + "-" + String(day) + "/" + String(month) + "/" + String(year);
      Serial.println(F(""));
    } else {
      Serial.println(F("WiFi Disconnected"));
    }
    /* ------------------------------------------------------- */
    lastTime = millis();
  }
  if (myObject["current"]["weather"][0]["description"] != null) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print("  -WEATHER STATION-");
    display.setCursor(0, 11);
    display.setTextSize(1);
    display.print("Weather: ");
    display.print(myObject["current"]["weather"][0]["main"]);
    display.setCursor(0, 22);
    display.setTextSize(1);
    display.print("Temp:");
    display.print(double(myObject["current"]["temp"]) - 273);
    display.print("C | Hum:");
    display.print(myObject["current"]["humidity"]);
    display.print("%");
    display.setCursor(0, 33);
    display.setTextSize(1);
    display.print("Wind:");
    display.print(myObject["current"]["wind_speed"]);
    display.print("m/s");
    display.setCursor(0, 44);
    display.setTextSize(1);
    display.print("Cloud:");
    display.print(myObject["current"]["clouds"]);
    display.print("% | UV:");
    display.print(myObject["current"]["uvi"]);
    display.setCursor(0, 55);
    display.setTextSize(1);
    display.print("Time:");
    display.print(_time);

    display.display();
  }
}



String httpGETRequest(const char *serverName) {
  WiFiClientSecure client;  // Sử dụng WiFiClientSecure cho HTTPS
  HTTPClient http;

  client.setInsecure();  // Tạm thời bỏ qua việc xác thực chứng chỉ SSL

  // Bắt đầu yêu cầu HTTPS
  http.begin(client, serverName);

  // Gửi yêu cầu GET
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    Serial.print(F("HTTP Response code: "));
    Serial.println(httpResponseCode);
    payload = http.getString();
  } else {
    Serial.print(F("Error code: "));
    Serial.println(httpResponseCode);
  }

  // Giải phóng tài nguyên
  http.end();

  return payload;
}


String checkAsthmaCondition() {
  // Kiểm tra nhịp tim

  if (nhiptim > HEART_RATE_THRESHOLD && digitalRead(15) == HIGH) {
    asthmaCondition = "Abnormal heart rate,\n possible asthma!";
    if (check1 && StateWarning == true) {
      Blynk.logEvent("Warning_1", "Abnormal heart rate, possible asthma!");
      check1 = false;
    }


  }

  // Kiểm tra nồng độ oxy trong máu
  else if (oxy < SPO2_THRESHOLD && digitalRead(15) == HIGH) {
    asthmaCondition = "Low oxygen levels,\n possible asthma!";
    if (check2 && StateWarning == true) {
      Blynk.logEvent("Warning_2", "Low oxygen levels, possible asthma!");
      check2 = false;
    }
  }

  // Kiểm tra chất lư)ợng không khí (VOC)
  else if (correctedPPM > CO2_THRESHOLD) {
    asthmaCondition = "Poor air quality,\n may trigger asthma!";
    if (check3 && StateWarning == true) {
      Blynk.logEvent("Warning_3", "Poor air quality, may trigger asthma!");
      check3 = false;
    }
  }

  // Kiểm tra điều kiện nhiệt độ và độ ẩm
  else if (temperature < TEMP_LOW_THRESHOLD && humidity > HUMIDITY_HIGH_THRESHOLD) {
    asthmaCondition = "Low temperature\n and high humidity,\n risk of asthma!";
    if (check4 && StateWarning == true) {
      Blynk.logEvent("Warning_4", "Low temperature and high humidity, risk of asthma!");
      check4 = false;
    }
  }

  // Kiểm tra chỉ số UV
  else if (UV_index > UV_INDEX_THRESHOLD) {
    asthmaCondition = "High UV index,\n may trigger asthma!";
    if (check5 && StateWarning == true) {
      Blynk.logEvent("Warning_5", "High UV index, may trigger asthma!");
      check5 = false;
    }
  }

  // Nếu không có yếu tố nào đáng lo ngại
  else {
    asthmaCondition = "Everything is fine!";
    check1 = true;
    check2 = true;
    check3 = true;
    check4 = true;
    check5 = true;
  }

  Serial.println(asthmaCondition);
  return asthmaCondition;
}