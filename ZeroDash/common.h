#pragma once

#include <string>

using namespace std;

struct Config {
	string apiServerUrl;
	string apiServerPassword;
};

struct DateTime {
	int hour;
	string time;
	string seconds;
	string amPm;
	string date;
};

struct SystemStats {
	string ip;
	string cpuLoad;
	string cpuLoad2;
	double cpuLoadAvg[3];
	string memUsage;
	string diskUsage;
	string cpuTemp;
};

struct MemInfo {
	int totalKb;
	int freeKb;
	int availableKb;
	int cached;
};

struct StatsSummary {
	int total_queries;
	int blocked_queries;
	double percent_blocked;
	int unique_domains;
	int forwarded;
	int cached;
	double frequency;
	int active_clients;
	int total_clients;
	int domains_being_blocked;
	double took;

	string ip;
	string hostname;

	string error;
	string message;
};

struct AuthResponse {
	bool valid;
	bool totp_enabled;
	string sid;
	string csrf_token;
	int validity;  // Remaining session lifetime
};