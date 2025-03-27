#include "InfoManager.h"
#include <unistd.h>  // for usleep()

InfoManager::InfoManager(const Config& config)
    : display(240, 135), image(240, 135), statsProvider(config) {
    dateBgColor = image.GetColor(0x55, 0x55, 0x00);
    dateFgColor = image.GetColor(0xfe, 0xf7, 0xa0);
    timeColor = image.GetColor(0xdd, 0xdd, 0xdd);
    blackColor = image.GetColor(0, 0, 0);

    color1 = image.GetColor(0xff, 0xff, 0x00);
    color2 = image.GetColor(0xfc, 0xb7, 0x6d);
    color3 = image.GetColor(0x00, 0xff, 0x00);
    color4 = image.GetColor(0x00, 0x00, 0xff);
    color5 = image.GetColor(0xff, 0x00, 0xff);
}

void InfoManager::Run() {
    time_t t0 = time(nullptr);
    time_t buttonInfoTime = 0;
    int infoShowTime = 10;
    uint8_t buttonPressed = 0;
    bool sleep = false;
    uint8_t infoType = 0;

    display.SetBackLightOn();

    while (true) {
        time_t t1 = time(nullptr);

        buttonPressed = display.IsButtonAPressed() ? buttonPressed | 0x01 : buttonPressed & 0xfe;
        buttonPressed = display.IsButtonBPressed() ? buttonPressed | 0x02 : buttonPressed & 0xfd;

        if (buttonPressed == 3) {
            if (sleep) {
                display.SetBackLightOn();
                sleep = false;
            }
            else {
                display.SetBackLightOff();
                sleep = true;
            }
        }
        else if (buttonPressed == 1) {
            buttonInfoTime = time(nullptr);
            infoType = 1;
        }
        else if (buttonPressed == 2) {
            buttonInfoTime = time(nullptr);
            infoType = 2;
        }

        if (!sleep && t1 > t0) {
            t0 = t1;

            if (buttonInfoTime > 0 && time(nullptr) - buttonInfoTime > infoShowTime) {
                buttonInfoTime = 0;
                infoType = 0;
            }

            image.FillRectangle(0, 0, image.GetWidth(), image.GetHeight(), blackColor);

            switch (infoType) {
            case 0: DrawDateTime(); break;
            case 1: DrawSystemStats(); break;
            case 2: DrawPiStats(); break;
            default:
                DrawSystemStats(); break;
            }

            display.FillRectangle(0, 0, display.GetWidth(), display.GetHeight(),
                image.GetImageBytes());
        }

        usleep(250000);
    }
}

void InfoManager::DrawDateTime() {
    DateTime* dateTime = statsProvider.GetDateTime();
    int hourDisplayOffset = dateTime->hour < 10 ? 31 : 0;

    image.WriteText(dateTime->time.c_str(), 33 + hourDisplayOffset, 83, 38, timeColor);
    image.WriteText(dateTime->seconds.c_str(), 178, 83, 20, timeColor);
    image.WriteText(dateTime->amPm.c_str(), 178, 50, 20, timeColor);
    image.FillRectangle(0, 101, 240, 28, dateBgColor);
    image.WriteText(dateTime->date.c_str(), 40, 125, 20, dateFgColor);
}

void InfoManager::DrawSystemStats() {
    SystemStats* systemStats = statsProvider.GetSystemStats();

    int bbox[8];
    int posY = 17;
    int lineSpacing = 5;
    int fontSize = 17;

    image.WriteText(systemStats->ip.c_str(), 0, posY, fontSize, color1, bbox);
    posY = posY + (bbox[1] - bbox[7]) + lineSpacing;
    image.WriteText("Load: ", 0, posY, fontSize, color2, bbox);
    posY = posY + (bbox[1] - bbox[7]) + lineSpacing;
    image.WriteText(systemStats->cpuLoad2.c_str(), 20, posY, fontSize, color2, bbox);
    posY = posY + (bbox[1] - bbox[7]) + lineSpacing;
    image.WriteText(systemStats->memUsage.c_str(), 0, posY, fontSize, color3, bbox);
    posY = posY + (bbox[1] - bbox[7]) + lineSpacing;
    image.WriteText(systemStats->diskUsage.c_str(), 0, posY, fontSize, color4, bbox);
    posY = posY + (bbox[1] - bbox[7]) + lineSpacing;
    image.WriteText(systemStats->cpuTemp.c_str(), 0, posY, fontSize, color5, bbox);
}

void InfoManager::DrawPiStats() {
    StatsSummary* pi = statsProvider.GetPiSummary();

    int bbox[8];
    int posY = 17;
    int lineSpacing = 4;
    int fontSize = 17;

    std::string blocked = "Ads Blocked: " + std::to_string(pi->blocked_queries);
    std::string clients = "Clients: " + std::to_string(pi->active_clients);
    std::string queries = "DNS Queries: " + std::to_string(pi->total_queries);
    std::string cached = "Cached: " + std::to_string(pi->cached);

    if (pi->error.empty() && !pi->message.empty()) {
        image.WriteText(pi->message.c_str(), 20, 60, fontSize, color3, bbox);
    }
    else if (pi->error.empty() && pi->message.empty()) {
        image.WriteText(pi->ip.c_str(), 0, posY, fontSize, color1, bbox);
        posY = posY + (bbox[1] - bbox[7]) + lineSpacing;
        image.WriteText(pi->hostname.c_str(), 0, posY, fontSize, color2, bbox);
        posY = posY + (bbox[1] - bbox[7]) + lineSpacing;
        image.WriteText(blocked.c_str(), 0, posY, fontSize, color3, bbox);
        posY = posY + (bbox[1] - bbox[7]) + lineSpacing;
        image.WriteText(clients.c_str(), 0, posY, fontSize, color4, bbox);
        posY = posY + (bbox[1] - bbox[7]) + lineSpacing;
        image.WriteText(queries.c_str(), 0, posY, fontSize, color5, bbox);
        posY = posY + (bbox[1] - bbox[7]) + lineSpacing;
        image.WriteText(cached.c_str(), 0, posY, fontSize, color3, bbox);
    }
    else {
        image.WriteText(pi->error.c_str(), 0, posY, fontSize, color1, bbox);
    }
}