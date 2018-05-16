/*
 * Copyright (c) Martin Kinkelin
 *
 * See the "License.txt" file in the root directory for infos
 * about permitted and prohibited uses of this code.
 */

#pragma once

#include "Thread.h"
#include "Parameters.h"
#include "AmdThermalDiode.h"
#include "IT87FanController.h"

#include <math.h>



/// <summary>
/// Adjusts periodically the fan speeds according to the CPU temperature.
/// </summary>
class FansThread : public Thread
{
public:

	/// <summary>
	/// Constructor.
	/// Beware that a std::exception is thrown if the AMD thermal diode and/or the
	/// IT87xxF controller are not found.
	/// </summary>
	FansThread(const Parameters& params) :
		Thread(),
		_params(&params)
	{
		_diode = new AmdThermalDiode();

		try
		{
			_controller = new IT87FanController();
		}
		catch (std::exception)
		{
			delete _diode;
			throw;
		}
	}

	virtual ~FansThread()
	{
		Finish();

		delete _diode;
		delete _controller;
	}


protected:

	virtual void Run()
	{
		static const int interval = 500; // in ms

		using namespace std;

		const Parameters& params = *_params;

		float speeds[3];

		bool firstIteration = true;
		while (Wait(interval))
		{
			const float temperature = _diode->GetTemp();

			// initialize the controller (setting all fans to 100% for an instant)
			if (firstIteration)
				_controller->Initialize();

			for (int i = 0; i < 3; i++)
			{
				const vector<pair<int, int>>& curve = params.FanCurves[i];

				if (!curve.empty())
				{
					float newSpeed = GetCurveValue(curve, temperature);

					if (!firstIteration)
					{
						const float delta = newSpeed - speeds[i];
						const float absDelta = abs(delta);

						// suppress tiny deltas (< 1%)
						if (absDelta < 1)
							continue;

						if (params.PwmRamp > 0 && absDelta > params.PwmRamp)
							newSpeed = speeds[i] + (delta < 0 ? -params.PwmRamp : params.PwmRamp);
					}

					_controller->SetSpeed(i, newSpeed);
					speeds[i] = newSpeed;
				}
			}

			firstIteration = false;
		}
	}


private:

	const Parameters* _params;
	AmdThermalDiode* _diode;
	IT87FanController* _controller;


	/// <summary>Performs linear interpolation between the control points.</summary>
	static float GetCurveValue(const std::vector<std::pair<int, int>>& curve, float temperature)
	{
		using namespace std;

		if (curve.empty())
			return 100;

		if (temperature <= curve[0].first)
			return (float)curve[0].second;
		if (temperature >= curve.back().first)
			return (float)curve.back().second;

		pair<int, int> left, right;
		for (size_t i = 1; i < curve.size(); i++)
		{
			if (temperature < curve[i].first)
			{
				left = curve[i-1];
				right = curve[i];
				break;
			}
		}

		const float distance = (temperature - left.first) / (right.first - left.first);

		return left.second + distance * (right.second - left.second);
	}
};
