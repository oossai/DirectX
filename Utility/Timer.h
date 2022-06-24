/*
 * Implementation of a high resolution timer
 * Timing is measured using QueryPerformanceCounter
 * Call Reset() once of initial startup of the app to be timed
 */

#ifndef __TIMER_H__
#define __TIMER_H__
#include "../Headers.h"

class Timer
{
private:
	LARGE_INTEGER m_frequency{}; // Cpu ticks per second
	LARGE_INTEGER m_baseTime{}; // Application start time
	LARGE_INTEGER m_previousTime{}; // Time tick was last called
	LARGE_INTEGER m_currentTime{}; // Current time
	LARGE_INTEGER m_deltaTime{}; // Difference between current and previous
	LARGE_INTEGER m_pausedTime{}; // Cumulative sum of time the timer was paused 
	LARGE_INTEGER m_stoppedTime{}; // Time when Pause() was last called
	bool m_paused = false; // Is timer paused
public:
	
	/*
 	 * Constructor, query's CPU ticks per second
 	 */
	Timer()
	{
		QueryPerformanceFrequency(&m_frequency);
	}
	
	/*
 	 * Set application start time
	 * Set previous time to current time, 
	 * this is important to make sure first call to tick 
	 * produces accurate result
 	 */
	void Reset()
	{
		QueryPerformanceCounter(&m_baseTime);
		QueryPerformanceCounter(&m_previousTime);
	}

	/*
 	 * Set delta time to the time taken since last call
	 * If timer is paused delta time is set to 0
 	 */
	void Tick()
	{	
		QueryPerformanceCounter(&m_currentTime);
		m_deltaTime.QuadPart = (m_currentTime.QuadPart - m_previousTime.QuadPart);
		m_previousTime = m_currentTime;
		if (m_paused)
		{
			// No work is done, previous time is left counting as it is used 
			// by TotalTime
			m_deltaTime.QuadPart = 0;
		}
	}

	/*
 	 * Set stopped time to current time
	 * Set paused to true
 	 */	
	void Pause()
	{
		if (!m_paused)
		{
			QueryPerformanceCounter(&m_stoppedTime);
			m_paused = true;
		}
	}
	
	/*
 	 * Calculate length of time timer was paused
 	 */	
	void UnPause()
	{
		if (m_paused)
		{
			LARGE_INTEGER startTime;
			QueryPerformanceCounter(&startTime);
			// Keep track of all the time the app has been paused
			m_pausedTime.QuadPart += (startTime.QuadPart - m_stoppedTime.QuadPart);
			// Previous time not accurate for GameTime, since it was last 
			// updated while paused, therefore update previous time
			m_previousTime = startTime;
			// No longer stopped
			// m_stoppedTime.QuadPart = 0;
			m_paused = false;
		}
	}

	/*
 	 * @return float, the difference in time since tick was
	 * last called
 	 */	
	float DeltaTime()
	{
		float delta = m_deltaTime.QuadPart * 1'000.0f;
		return delta / m_frequency.QuadPart;
	}

	/*
 	 * @return float, the difference in time since app started
 	 */	
	float TotalTime()
	{
		float totalTime = static_cast<float>(m_currentTime.QuadPart - 
			m_baseTime.QuadPart);
		return totalTime / m_frequency.QuadPart;
	}

	/*
 	 * @return float, the difference in time since app started
	 * excluding the accumulated paused time
 	 */	
	float GameTime()
	{
		if (m_paused)
		{
			// Don't count time that has passed since app was been paused
			float gameTime = static_cast<float>(m_stoppedTime.QuadPart - m_pausedTime.QuadPart - m_baseTime.QuadPart);
			return gameTime / m_frequency.QuadPart;
		}
		// Remove any accumulated paused time from total time
		return TotalTime() - ((float)m_pausedTime.QuadPart / m_frequency.QuadPart);
	}
};

#endif