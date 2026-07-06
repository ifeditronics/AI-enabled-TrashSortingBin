#include <Arduino.h>
#include <WiFi.h>
#include "esp_camera.h"
#include <WebServer.h>

//==============================
// WiFi Credentials
//==============================
const char* ssid = "SSID_HERE";
const char* password = "PASS_HERE";

//==============================
// AI Thinker ESP32-CAM Pinout
//==============================
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0

#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5

#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Flash LED
#define FLASH_LED_PIN      4

WebServer server(80);

//==============================
// Camera Initialization
//==============================
bool initCamera()
{
    camera_config_t config;

    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer   = LEDC_TIMER_0;

    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;

    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;

    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;

    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;

    config.xclk_freq_hz = 20000000;

    config.pixel_format = PIXFORMAT_JPEG;

    if(psramFound())
    {
        config.frame_size = FRAMESIZE_QVGA;   // 320x240
        config.jpeg_quality = 10;
        config.fb_count = 2;
    }
    else
    {
        config.frame_size = FRAMESIZE_QQVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    esp_err_t err = esp_camera_init(&config);

    if(err != ESP_OK)
    {
        Serial.printf("Camera Init Failed: 0x%x\n", err);
        return false;
    }

    sensor_t * s = esp_camera_sensor_get();

    s->set_brightness(s, 0);
    s->set_contrast(s, 0);
    s->set_saturation(s, 0);
    s->set_whitebal(s, 1);
    s->set_gain_ctrl(s, 1);
    s->set_exposure_ctrl(s, 1);
    s->set_awb_gain(s, 1);

    Serial.println("Camera Initialized.");

    return true;
}

//==============================
// WiFi
//==============================
void connectWiFi()
{
    Serial.print("Connecting");

    WiFi.begin(ssid, password);

    while(WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }

    Serial.println();
    Serial.println("WiFi Connected!");

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

//======================================
// Capture WITHOUT Flash (Preview)
//======================================
void handlePreview()
{
    camera_fb_t *fb = esp_camera_fb_get();

    if (!fb)
    {
        server.send(500, "text/plain", "Capture Failed");
        return;
    }

    server.send_P(
        200,
        "image/jpeg",
        (const char *)fb->buf,
        fb->len
    );

    esp_camera_fb_return(fb);
}

//======================================
// Capture WITH Flash (Dataset Image)
//======================================
void handleCapture()
{
    digitalWrite(FLASH_LED_PIN, HIGH);
    delay(120);

    camera_fb_t *fb = esp_camera_fb_get();

    digitalWrite(FLASH_LED_PIN, LOW);

    if (!fb)
    {
        server.send(500, "text/plain", "Capture Failed");
        return;
    }

    server.send_P(
        200,
        "image/jpeg",
        (const char *)fb->buf,
        fb->len
    );

    esp_camera_fb_return(fb);
}

//==============================
// Root Page
//==============================
void handleRoot()
{
      server.send(200, "text/plain",
            "ESP32-CAM Dataset Server\n\n"
            "/preview - Live Preview\n"
            "/capture - Capture Dataset Image");
}

//==============================
// Setup
//==============================
void setup()
{
    Serial.begin(115200);

    pinMode(FLASH_LED_PIN, OUTPUT);
    digitalWrite(FLASH_LED_PIN, LOW);

    if (!initCamera())
    {
        while (true)
        {
            digitalWrite(FLASH_LED_PIN, HIGH);
            delay(250);
            digitalWrite(FLASH_LED_PIN, LOW);
            delay(250);
        }
    }

    connectWiFi();

    server.on("/", HTTP_GET, handleRoot);
    server.on("/preview", HTTP_GET, handlePreview);
    server.on("/capture", HTTP_GET, handleCapture);

    server.begin();

    Serial.println();
    Serial.println("==============================");
    Serial.println("Dataset Camera Ready");
    Serial.print("Open: http://");
    Serial.print(WiFi.localIP());
    Serial.println("/capture");
    Serial.println("==============================");
}

//==============================
// Loop
//==============================
void loop()
{
    server.handleClient();
}