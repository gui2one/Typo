#pragma once
#include <chrono>
struct Timer {

	std::chrono::duration<double> elapsed;
	double seconds, old_seconds;
	double delta_seconds;
	std::chrono::high_resolution_clock::time_point start_time;
	void Start() {

		seconds = 0.0f;
		old_seconds = 0.0f;
		start_time = std::chrono::high_resolution_clock::now();
	}
	void Update() {


		auto current_time = std::chrono::high_resolution_clock::now();
		delta_seconds = seconds - old_seconds;
		elapsed = (current_time - start_time);
		old_seconds = seconds;
		seconds = (double)(elapsed.count());

	}
};