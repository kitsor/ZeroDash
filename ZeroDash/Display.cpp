#include "Display.h"
#include <wiringPi.h>
#include <wiringPiSPI.h>

#ifndef DISPLAY_DEBUG
#define DISPLAY_DEBUG 0
#endif // DISPLAY_DEBUG

#if DISPLAY_DEBUG == 1
#include <iostream>
#endif // DISPLAY_DEBUG

Display::Display(uint16_t width, uint16_t height) {
    screenWidth = width;
    screenHeight = height;
    Init();
}

uint16_t Display::GetWidth() {
    return screenWidth;
}

uint16_t Display::GetHeight() {
    return screenHeight;
}

bool Display::IsButtonAPressed() {
    return !digitalRead(BUTTON_A);
}

bool Display::IsButtonBPressed() {
    return !digitalRead(BUTTON_B);
}

void Display::Init() {
    wiringPiSetupGpio();  // Use GPIO numbering
    pinMode(DC_PIN, OUTPUT);
    pinMode(BACKLIGHT_PIN, OUTPUT);
    pinMode(BUTTON_A, INPUT);
    pinMode(BUTTON_B, INPUT);

    wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED);

    std::vector<uint8_t> initCommands = { SWRESET, SLPOUT };
    SendCommand(initCommands);
    SendCommandData(COLMOD, ColModValue);
    SendCommandData(
        CASET, 
        std::vector<uint16_t>{ ScreenWidthOffset, 
        static_cast<uint16_t>(ScreenWidthOffset + screenWidth - 1) });
    SendCommandData(
        RASET, 
        std::vector<uint16_t>{ ScreenHeightOffset, 
        static_cast<uint16_t>(ScreenHeightOffset + screenHeight - 1) });
    SendCommand({ INVON, NORON, DISPON });
    SendCommandData(MADCTL, 0x60);
}

void Display::SetBackLightOn() {
    digitalWrite(BACKLIGHT_PIN, HIGH);
}

void Display::SetBackLightOff() {
    digitalWrite(BACKLIGHT_PIN, LOW);
}

// Converts RGB to 16-bit 565 color format
uint16_t Display::Color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

void Display::Send(const std::vector<uint8_t>& data, uint8_t pinMode) {
#if DISPLAY_DEBUG == 1
    // Print whether we are sending commands or data
    printf("Sending %d %s: ", data.size(), (pinMode == LOW) ? "commands" : "data");

    // Print vector contents in hex format
    if (data.size() > 10) {
        printf("%d bytes ", data.size());
        printf("%02X %02X...", data[0], data[1]);
    }
    else {
        for (uint8_t byte : data) {
            printf("%02X ", byte);  // Print each byte as a two-digit hex number
        }
    }
    printf("\n");
#endif // DISPLAY_DEBUG == 1

    // Set the pin mode
    digitalWrite(DC_PIN, (pinMode == LOW ? LOW : HIGH));

    // Send data over SPI
    if (pinMode == LOW)
    {
        // sending commands
        for (uint8_t command : data) {
            wiringPiSPIDataRW(SPI_CHANNEL, &command, 1);
        }
    }
    else {
        wiringPiSPIDataRW(SPI_CHANNEL, const_cast<uint8_t*>(data.data()),
            static_cast<int>(data.size()));
    }
}

void Display::SendCommand(const std::vector<uint8_t>& cmd) {
    Send(cmd, LOW); // LOW indicates command mode
}

void Display::SendCommand(uint8_t cmd) {
    SendCommand(std::vector<uint8_t>{cmd});
}

void Display::SendData(const std::vector<uint8_t>& data) {
    Send(data, HIGH); // HIGH indicates data mode
}

void Display::SendData(uint8_t data) {
    SendData(std::vector<uint8_t>{data});
}

void Display::SendData(uint16_t* data, size_t length) {
    std::vector<uint8_t> buffer(length * 2); // Each uint16_t is 2 bytes

    for (size_t i = 0; i < length; i++) {
        buffer[i * 2] = static_cast<uint8_t>(data[i] >> 8);        // High byte
        buffer[i * 2 + 1] = static_cast<uint8_t>(data[i] & 0xFF);  // Low byte
    }

    SendData(buffer);
}

void Display::SendData(const std::vector<uint16_t>& data) {
    std::vector<uint8_t> buffer(data.size() * 2); // Each uint16_t is 2 bytes

    for (size_t i = 0; i < data.size(); i++) {
        buffer[i * 2] = static_cast<uint8_t>(data[i] >> 8);        // High byte
        buffer[i * 2 + 1] = static_cast<uint8_t>(data[i] & 0xFF);  // Low byte
    }

    SendData(buffer);
}

void Display::SendCommandData(uint8_t cmd, const std::vector<uint16_t>& data) {
    SendCommand(cmd);
    SendData(data);
}

void Display::SendCommandData(uint8_t cmd, const std::vector<uint8_t>& data) {
    SendCommand(cmd);
    SendData(data);
}

void Display::SendCommandData(uint8_t cmd, uint8_t data) {
    SendCommand(cmd);
    SendData(data);
}

void Display::Block(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    x0 = ScreenWidthOffset + x0;
    x1 = ScreenWidthOffset + x1;
    y0 = ScreenHeightOffset + y0;
    y1 = ScreenHeightOffset + y1;
    SendCommandData(CASET, std::vector<uint16_t>{ x0, x1 });
    SendCommandData(RASET, std::vector<uint16_t>{ y0, y1 });
}

void Display::FillRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, 
    uint8_t r, uint8_t g, uint8_t b) {
    x = static_cast<uint16_t>(std::min(screenWidth - 1, std::max(0, int(x))));
    y = static_cast<uint16_t>(std::min(screenHeight - 1, std::max(0, int(y))));
    width = static_cast<uint16_t>(std::min(screenWidth - x, std::max(1, int(width))));
    height = static_cast<uint16_t>(std::min(screenHeight - y, std::max(1, int(height))));

    Block(x, y, static_cast<uint16_t>(x + width - 1), static_cast<uint16_t>(y + height - 1));
    uint16_t color = Color565(r, g, b);

    const int totalPixels = width * height;
    const int chunkSize = BUFFER_SIZE <= totalPixels ? BUFFER_SIZE : totalPixels;
    uint16_t pixels[chunkSize];
    // Fill buffer with color
    for (int i = 0; i < chunkSize; i++) {
        pixels[i] = color;
    }

    SendCommand(RAMWR);

    // Send full chunks
    int i;
    for (i = 0; i + chunkSize <= totalPixels; i += chunkSize) {
        SendData(pixels, chunkSize);
    }

    // Send remaining pixels
    int remaining = totalPixels % chunkSize;
    if (remaining > 0) {
        SendData(pixels, remaining);
    }
}

void Display::FillRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, 
    const std::vector<uint8_t>& rgbData) {
    x = static_cast<uint16_t>(std::min(screenWidth - 1, std::max(0, int(x))));
    y = static_cast<uint16_t>(std::min(screenHeight - 1, std::max(0, int(y))));
    width = static_cast<uint16_t>(std::min(screenWidth - x, std::max(1, int(width))));
    height = static_cast<uint16_t>(std::min(screenHeight - y, std::max(1, int(height))));

    if (rgbData.size() < width * height * 3) {
        return; // Invalid data size, prevent buffer overflow
    }

    Block(x, y, static_cast<uint16_t>(x + width - 1), static_cast<uint16_t>(y + height - 1));

    const int totalPixels = width * height;
    const int chunkSize = BUFFER_SIZE <= totalPixels ? BUFFER_SIZE : totalPixels;
    uint16_t pixels[chunkSize];

    SendCommand(RAMWR);

    int i;
    for (i = 0; i < totalPixels; i += chunkSize) {
        int currentChunkSize = std::min(chunkSize, totalPixels - i);

        // Convert RGB888 to RGB565 for the current chunk
        for (int j = 0; j < currentChunkSize; j++) {
            int index = (i + j) * 3;
            uint8_t r = rgbData[index];
            uint8_t g = rgbData[index + 1];
            uint8_t b = rgbData[index + 2];
            pixels[j] = Color565(r, g, b);
        }

        SendData(pixels, currentChunkSize);
    }
}

// Fill the screen with a solid color
void Display::FillScreen(uint8_t r, uint8_t g, uint8_t b) {
    FillRectangle(0, 0, screenWidth, screenHeight, r, g, b);
}

void Display::HLine(uint16_t x, uint16_t y, uint16_t width, 
    uint8_t r, uint8_t g, uint8_t b) {
    FillRectangle(x, y, width, 1, r, g, b);
}

void Display::VLine(uint16_t x, uint16_t y, uint16_t height, 
    uint8_t r, uint8_t g, uint8_t b) {
    FillRectangle(x, y, 1, height, r, g, b);
}

void Display::Rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, 
    uint8_t r, uint8_t g, uint8_t b) {
    HLine(x, y, width, r, g, b);
    VLine(x, y, height, r, g, b);
    HLine(x, y + height, width, r, g, b);
    VLine(x + width, y, height, r, g, b);
}