#pragma once

#include <vector>



struct CustomCnQProfile
{
	bool Ganged;          // keep all cores in the same P-state

	int MinPState;        // min hardware P-state, 0-4
	int MaxPState;        // max hardware P-state, 0-4

	int SamplingInterval; // in ms
	int ThresholdUp;      // average CPU load percentage
	int ThresholdDown;    // average CPU load percentage
	int NumSamplesUp;     // number of consecutive samples above UpThreshold   until we switch to lower  P-state
	int NumSamplesDown;   // number of consecutive samples below DownThreshold until we switch to higher P-state
	bool AggressiveUp;    // when switching to a lower P-state, switch directly to the lowest P-state
};

struct Parameters
{
	bool EnableCustomPStates;
	// lower 32 bits for each core's MSR, for all 5 P-states (P0 to P4)
	std::vector<unsigned int> Msrs[5];

	int TurboCores; // max number of cores in the Turbo state at a time

	bool EnableCustomCnQ;
	CustomCnQProfile Profiles[3]; // Balanced, High performance, Power saver

	bool EnableFanManagement;
	int PwmRamp; // max step (in percent) from one interval (500 ms) to the next
	std::vector<std::pair<int, int>> FanCurves[3]; // set of control points (CPU temperature, fan speed in percent) for all 3 fans
};
