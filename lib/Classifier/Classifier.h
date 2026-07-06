#pragma once

#include <Arduino.h>


enum WasteType
{
    UNKNOWN = 0,
    RECYCLABLE,
    NON_RECYCLABLE
};

class Classifier
{
public:

    bool begin();

    WasteType classify();

    float confidence();

private:

    static constexpr uint32_t RAW_W = 320;
    static constexpr uint32_t RAW_H = 240;
    static constexpr uint32_t BYTES_PER_PIXEL = 3;

    static bool initialized;

    static uint8_t *snapshot_buf;

    float m_confidence = 0;

    bool cameraInit();

    bool captureImage(
        uint32_t width,
        uint32_t height,
        uint8_t *out_buf);

    static int getData(
        size_t offset,
        size_t length,
        float *out_ptr);

    void cameraDeinit();
};