#include "PiHoleClient.h"
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Helper function to handle HTTP response data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb; // Calculate the total size of received data
    userp->append((char*)contents, totalSize); // Append data to the response string
    return totalSize; // Return the number of bytes handled
}

PiHoleClient::PiHoleClient(const std::string& server, const std::string& pwd)
    : server_url(server), password(pwd) {
}

bool PiHoleClient::login() {
    if (isSessionValid()) {
        return true;
    }

    AuthResponse auth = authenticate();

    if (auth.valid) {
        session_id = auth.sid;
        session_validity = auth.validity;
        session_start_time = std::chrono::steady_clock::now();
        return true;
    }
    return false;
}

StatsSummary PiHoleClient::getStats() {
    if (!isSessionValid()) {
        if (!login()) {
            throw std::runtime_error("Failed to re-authenticate.");
        }
    }

    std::string stats_url = server_url + "/api/stats/summary";
    std::string response = getRequest(stats_url);
    StatsSummary stats{};

    json j = json::parse(response);
    stats.total_queries = j["queries"]["total"];
    stats.blocked_queries = j["queries"]["blocked"];
    stats.percent_blocked = j["queries"]["percent_blocked"];
    stats.unique_domains = j["queries"]["unique_domains"];
    stats.forwarded = j["queries"]["forwarded"];
    stats.cached = j["queries"]["cached"];
    stats.frequency = j["queries"]["frequency"];
    stats.active_clients = j["clients"]["active"];
    stats.total_clients = j["clients"]["total"];
    stats.domains_being_blocked = j["gravity"]["domains_being_blocked"];
    stats.took = j["took"];

    return stats;
}

AuthResponse PiHoleClient::authenticate() {
    CURL* curl = curl_easy_init();
    std::string response;
    AuthResponse authResponse{ false, false, "", "", 0 };

    if (curl) {
        std::string auth_url = server_url + "/api/auth";
        std::string json_payload = json{ {"password", password} }.dump();

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, auth_url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            json j = json::parse(response);
            authResponse.valid = j["session"]["valid"];
            authResponse.totp_enabled = j["session"]["totp"];
            authResponse.sid = j["session"]["sid"];
            authResponse.csrf_token = j["session"]["csrf"];
            authResponse.validity = j["session"]["validity"];
            // getting some time buffer before session expires
            authResponse.validity = std::max(0, authResponse.validity - 5);
        }
        else {
            std::string curlError = curl_easy_strerror(res);
            throw std::runtime_error("Authentication request failed: " + curlError);
        }
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    return authResponse;
}

std::string PiHoleClient::getRequest(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("X-FTL-SID: " + session_id).c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::string curlError = curl_easy_strerror(res);
            throw std::runtime_error("GET request failed: " + curlError);
        }
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    return response;
}

bool PiHoleClient::isSessionValid() {
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - session_start_time).count();
    return elapsed < session_validity;
}