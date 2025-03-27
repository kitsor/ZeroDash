#include "StatsProvider.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <unistd.h> // gethostname()

std::string doubleToStringWithPrecision(double value, int precision) {
    std::stringstream stream;
    stream << std::fixed << std::setprecision(precision) << value;
    return stream.str();
}

StatsProvider::StatsProvider(const Config& config)
    : client(config.apiServerUrl, config.apiServerPassword) {
}

SystemStats* StatsProvider::GetSystemStats() {
    static SystemStats retVal{};

    GetCpuLoadAvg(retVal.cpuLoadAvg);
    retVal.ip = "IP: " + GetIp();
    retVal.cpuLoad2 =
        doubleToStringWithPrecision(retVal.cpuLoadAvg[0], 2) + " " +
        doubleToStringWithPrecision(retVal.cpuLoadAvg[1], 2) + " " +
        doubleToStringWithPrecision(retVal.cpuLoadAvg[2], 2);
    MemInfo memInfo = { 0, 0, 0, 0 };  // Initialize struct with default values
    GetMemUsage2(memInfo);
    retVal.memUsage = "Mem: " +
        std::to_string((memInfo.totalKb - memInfo.availableKb) / 1024) + "/"
        + std::to_string(memInfo.totalKb / 1024) + " MB";

    retVal.diskUsage = GetDiskUsage();
    retVal.cpuTemp = "CPU Temp: " + std::to_string(GetCpuTemp()) + " °C";

    return &retVal;
}

StatsSummary* StatsProvider::GetPiSummary() {
    static StatsSummary retVal{};

    if (!client.isSessionValid() && !piClientLoadingSent) {
        retVal.message = "Loading...";
        piClientLoadingSent = true;
        return &retVal;
    }

    if (client.login()) {
        piClientLoadingSent = false;
        try {
            retVal = client.getStats();
            retVal.error = "";
            retVal.message = "";
            retVal.ip = "IP: " + GetIp();
            retVal.hostname = GetHostname();
        }
        catch (const std::exception& e) {
            retVal.error = "Error fetching stats";
        }
    }
    else {
        retVal.error = "Authentication failed!";
    }

    return &retVal;
}

int StatsProvider::GetCpuTemp() {
    std::ifstream file("/sys/class/thermal/thermal_zone0/temp");
    double temp;

    if (file.is_open()) {
        file >> temp;
        file.close();
        return static_cast<int>(temp / 1000.0);  // Convert millidegree Celsius to Celsius
    }
    else {
        return -1;
    }
}

std::string StatsProvider::GetIp() {
    std::string cmd = "hostname -I | cut -d\' \' -f1";
    return RunCommand(cmd);
}

std::string StatsProvider::GetHostname() {
    char hostname[1024];
    gethostname(hostname, 1024);
    return std::string(hostname);
}

std::string StatsProvider::GetCpuLoad() {
    std::string cmd = "top -bn1 | grep load | awk '{printf \"%.2f\", $(NF-2)}'";
    return RunCommand(cmd);
}

int StatsProvider::GetCpuLoadAvg(double* loadavg) {
    for (int i = 0; i < 3; ++i) {
        loadavg[i] = -1;
    }
    return getloadavg(loadavg, 3);
}

std::string StatsProvider::GetMemUsage() {
    std::string cmd = "free -m | awk 'NR==2{printf \"Mem: %s/%s MB\", $3,$2 }'";
    return RunCommand(cmd);
}

void StatsProvider::GetMemUsage2(MemInfo& memInfo) {
    std::ifstream file("/proc/meminfo");
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key;
        int value;

        iss >> key >> value;  // Read the key and the numeric value

        if (key == "MemTotal:") {
            memInfo.totalKb = value;
        }
        else if (key == "MemFree:") {
            memInfo.freeKb = value;
        }
        else if (key == "MemAvailable:") {
            memInfo.availableKb = value;
        }
        else if (key == "Cached:") {
            memInfo.cached = value;
        }
    }
}

std::string StatsProvider::GetDiskUsage() {
    std::string cmd = "df -h | awk '$NF==\"/\"{printf \"Disk: %d/%d GB  %s\", $3,$2,$5}'";
    return RunCommand(cmd);
}

DateTime* StatsProvider::GetDateTime() {
    static DateTime retVal;

    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);

    std::string monthArr[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    std::ostringstream timeStream;
    std::ostringstream secondsStream;
    std::ostringstream dateStream;

    int tm_hour;

    if (now->tm_hour > 12) {
        tm_hour = now->tm_hour - 12;
        retVal.amPm = "pm";
    }
    else {
        tm_hour = now->tm_hour == 0 ? 12 : now->tm_hour;
        retVal.amPm = "am";
    }

    timeStream << tm_hour << ":"
        << std::setw(2) << std::setfill('0') << now->tm_min;
    secondsStream << std::setw(2) << std::setfill('0') << now->tm_sec;
    dateStream << monthArr[now->tm_mon] << " "
        << now->tm_mday << " " << now->tm_year + 1900;

    retVal.hour = tm_hour;
    retVal.time = timeStream.str(); // Store in a variable
    retVal.seconds = secondsStream.str();
    retVal.date = dateStream.str();

    return &retVal;
}

// Function to execute shell commands and return the output as a string
std::string StatsProvider::RunCommand(const std::string& cmd) {
    char buffer[128];
    std::string result;

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        return "Error opening pipe!";
    }

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    pclose(pipe);
    return result;
}