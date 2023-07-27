#pragma once
#include <chrono>

namespace GDX11::Utils
{
	class Time
	{
	public:
		Time();
		void UpdateDeltaTime();

		float GetTime();
		float GetDeltaTime() { return m_deltaTime; };

	private:
		std::chrono::steady_clock::time_point m_startTime;
		std::chrono::steady_clock::time_point m_lastFrame;
		float m_deltaTime;
	};

	class Timer
	{
	public:
		Timer();
		float Mark();
		float Peek() const;

	private:
		std::chrono::steady_clock::time_point m_last;
	};
}