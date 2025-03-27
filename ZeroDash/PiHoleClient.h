#pragma once

#include "common.h"
#include <chrono>

using namespace std;

class PiHoleClient {
public:
    PiHoleClient(const std::string& server, const std::string& pwd);
    bool login();
    StatsSummary getStats();
    // Refresh session if needed
    bool isSessionValid();

private:
    string server_url;
    string password;
    string session_id;
    int session_validity;
    chrono::steady_clock::time_point session_start_time;

    // Perform a POST request for authentication
    AuthResponse authenticate();

    // Perform a GET request with authentication
    string getRequest(const std::string& url);
};