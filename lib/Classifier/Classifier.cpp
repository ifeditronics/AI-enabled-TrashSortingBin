#include "Classifier.h"
#include "esp_camera.h"
#include "ifeditronics-project-1_inferencing.h"
#include "edge-impulse-sdk/dsp/image/image.hpp"
#include "img_converters.h"

bool Classifier::initialized = false;
uint8_t *Classifier::snapshot_buf = nullptr;

/************************************************
 * AI Thinker ESP32-CAM Pinout
 ***********************************************/

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

bool Classifier::begin()
{
    return cameraInit();
}

bool Classifier::cameraInit()
{
    if (initialized)
        return true;

    camera_config_t config;

    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;

    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;

    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d0 = Y2_GPIO_NUM;

    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;

    config.xclk_freq_hz = 20000000;

    config.ledc_timer = LEDC_TIMER_0;
    config.ledc_channel = LEDC_CHANNEL_0;

    //
    // Same configuration as Edge Impulse example
    //
    config.pixel_format = PIXFORMAT_JPEG;

    config.frame_size = FRAMESIZE_QVGA;

    config.jpeg_quality = 12;

    config.fb_count = 1;

    config.fb_location = CAMERA_FB_IN_PSRAM;

    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

    esp_err_t err = esp_camera_init(&config);

    if (err != ESP_OK)
    {
        Serial.printf(
            "Camera Init Failed : 0x%x\n",
            err
        );

        return false;
    }

    sensor_t *s = esp_camera_sensor_get();

    if (s->id.PID == OV3660_PID)
    {
        s->set_vflip(s, 1);
        s->set_brightness(s, 1);
        s->set_saturation(s, 0);
    }

    s->set_brightness(s, 0);
    s->set_contrast(s, 0);
    s->set_saturation(s, 0);

    initialized = true;

    Serial.println("Classifier Camera Ready");

    return true;
}

void Classifier::cameraDeinit()
{
    if (!initialized)
        return;

    esp_camera_deinit();

    initialized = false;
}

/************************************************
 * Capture Image
 ***********************************************/

bool Classifier::captureImage(
    uint32_t img_width,
    uint32_t img_height,
    uint8_t *out_buf)
{
    if (!initialized)
    {
        Serial.println("Camera not initialized");
        return false;
    }

    camera_fb_t *fb = esp_camera_fb_get();

    if (!fb)
    {
        Serial.println("Camera capture failed");
        return false;
    }

    bool converted = fmt2rgb888(
        fb->buf,
        fb->len,
        PIXFORMAT_JPEG,
        snapshot_buf
    );

    esp_camera_fb_return(fb);

    if (!converted)
    {
        Serial.println("JPEG conversion failed");
        return false;
    }

    //
    // Resize from 320x240 -> 96x96
    //
    if ((img_width != RAW_W) || (img_height != RAW_H))
    {
        ei::image::processing::crop_and_interpolate_rgb888(
            out_buf,
            RAW_W,
            RAW_H,
            out_buf,
            img_width,
            img_height
        );
    }

    return true;
}

/************************************************
 * Edge Impulse callback
 ***********************************************/

int Classifier::getData(
    size_t offset,
    size_t length,
    float *out_ptr)
{
    size_t pixel_ix = offset * 3;

    size_t pixels_left = length;

    size_t out_ix = 0;

    while (pixels_left)
    {
        //
        // ESP32 Camera stores BGR
        // Edge Impulse expects RGB packed
        //

        out_ptr[out_ix] =
            (snapshot_buf[pixel_ix + 2] << 16) |
            (snapshot_buf[pixel_ix + 1] << 8) |
            snapshot_buf[pixel_ix];

        pixel_ix += 3;

        out_ix++;

        pixels_left--;
    }

    return 0;
}

/************************************************
 * Run Edge Impulse
 ***********************************************/

WasteType Classifier::classify()
{
    if (snapshot_buf)
    {
        free(snapshot_buf);
        snapshot_buf = nullptr;
    }

    snapshot_buf = (uint8_t *)malloc(
        RAW_W *
        RAW_H *
        BYTES_PER_PIXEL
    );

    if (!snapshot_buf)
    {
        Serial.println("Snapshot allocation failed");

        m_confidence = 0;

        return UNKNOWN;
    }

    if (!captureImage(
            EI_CLASSIFIER_INPUT_WIDTH,
            EI_CLASSIFIER_INPUT_HEIGHT,
            snapshot_buf))
    {
        free(snapshot_buf);
        snapshot_buf = nullptr;

        m_confidence = 0;

        return UNKNOWN;
    }

    signal_t signal;

    signal.total_length =
        EI_CLASSIFIER_INPUT_WIDTH *
        EI_CLASSIFIER_INPUT_HEIGHT;

    signal.get_data = &Classifier::getData;

    ei_impulse_result_t result = {0};

    EI_IMPULSE_ERROR err =
        run_classifier(
            &signal,
            &result,
            false);

    free(snapshot_buf);

    snapshot_buf = nullptr;

    if (err != EI_IMPULSE_OK)
    {
        Serial.printf(
            "Inference Error %d\n",
            err);

        m_confidence = 0;

        return UNKNOWN;
    }

    uint8_t best = 0;

    for (uint8_t i = 1;
         i < EI_CLASSIFIER_LABEL_COUNT;
         i++)
    {
        if (result.classification[i].value >
            result.classification[best].value)
        {
            best = i;
        }
    }

    m_confidence =
        result.classification[best].value;

    Serial.println();

    Serial.println("===== Prediction =====");

    for (uint8_t i = 0;
         i < EI_CLASSIFIER_LABEL_COUNT;
         i++)
    {
        Serial.printf(
            "%s : %.3f\n",
            ei_classifier_inferencing_categories[i],
            result.classification[i].value);
    }

    Serial.println("======================");

    //
    // Confidence threshold
    //

    if (m_confidence < 0.80f)
    {
        return UNKNOWN;
    }

    String label =
        String(
            ei_classifier_inferencing_categories[best]);

    label.toLowerCase();

    if (label == "recyclable")
    {
        return RECYCLABLE;
    }

    if (label == "non_recyclable")
    {
        return NON_RECYCLABLE;
    }

    if (label == "non-recyclable")
    {
        return NON_RECYCLABLE;
    }

    return UNKNOWN;
}

/************************************************
 * Confidence
 ***********************************************/

float Classifier::confidence()
{
    return m_confidence;
}

