#pragma once

#include <cstdint> // uint8_t, uint16_t, ..
#include <vector>
#include <cstddef> // size_t

class Display {
public:
    Display(uint16_t width, uint16_t height);  // Constructor with width & height

    uint16_t GetWidth();
    uint16_t GetHeight();

    void SetBackLightOn();
    void SetBackLightOff();

    bool IsButtonAPressed();
    bool IsButtonBPressed();

    void FillScreen(uint8_t r, uint8_t g, uint8_t b);
    void FillRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, 
        uint8_t r, uint8_t g, uint8_t b);

    void HLine(uint16_t x, uint16_t y, uint16_t width, uint8_t r, uint8_t g, uint8_t b);
    void VLine(uint16_t x, uint16_t y, uint16_t height, uint8_t r, uint8_t g, uint8_t b);
    void Rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, 
        uint8_t r, uint8_t g, uint8_t b);
    void FillRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, 
        const std::vector<uint8_t>& rgbData);
private:
    static const int SWRESET = 0x01;
    static const int SLPOUT = 0x11;
    static const int NORON = 0x13;
    static const int INVON = 0x21;
    static const int DISPON = 0x29;
    static const int CASET = 0x2A;
    static const int RASET = 0x2B;
    static const int RAMWR = 0x2C;
    static const int COLMOD = 0x3A;
    static const int MADCTL = 0x36;

    static const int DC_PIN = 25;          // GPIO25 (Data/Command)
    static const int BACKLIGHT_PIN = 22;   // GPIO22
    static const int SPI_CHANNEL = 0;      // CE0
    static const int SPI_SPEED = 64000000; // 64 MHz

    static const int BUTTON_A = 23;        // GPIO23
    static const int BUTTON_B = 24;        // GPIO24

    static const int BUFFER_SIZE = 2048;   // This is the size of the buffer to be used for fill operations, in 16-bit units

    static const uint16_t ScreenWidthOffset = 40;
    static const uint16_t ScreenHeightOffset = 53;
    static const uint8_t ColModValue = 0x55;

    uint16_t screenWidth, screenHeight;    // Screen size

    void Init();

    void Send(const std::vector<uint8_t>& cmd, uint8_t pinMode);
    void SendCommand(const std::vector<uint8_t>& cmd);
    void SendCommand(uint8_t cmd);
    void SendData(const std::vector<uint8_t>& data);
    void SendData(const std::vector<uint16_t>& data);
    void SendData(uint8_t data);
    void SendData(uint16_t* data, size_t length);

    void SendCommandData(uint8_t cmd, const std::vector<uint16_t>& data);
    void SendCommandData(uint8_t cmd, const std::vector<uint8_t>& data);
    void SendCommandData(uint8_t cmd, uint8_t data);

    void Block(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

    uint16_t Color565(uint8_t r, uint8_t g, uint8_t b);
};