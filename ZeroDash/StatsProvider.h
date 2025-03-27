#pragma once

#include "PiHoleClient.h"

using namespace std;

class StatsProvider {
public:
	StatsProvider(const Config& config);

	int GetCpuTemp();
	string GetIp();
	string GetHostname();
	string GetCpuLoad();
	int GetCpuLoadAvg(double* avg);
	string GetMemUsage();
	void GetMemUsage2(MemInfo& memInfo);
	string GetDiskUsage();

	SystemStats* GetSystemStats();
	StatsSummary* GetPiSummary();

	DateTime* GetDateTime();

private:
	PiHoleClient client;
	bool piClientLoadingSent = false;
	string RunCommand(const std::string& cmd);
};