#pragma once
#include <windows.h>
#include <vector>
#include <string>

class Profiler
{
public:
	Profiler()
	{
		QueryPerformanceFrequency(&freq);
	}

	double time() const
	{
		LARGE_INTEGER t;
		QueryPerformanceCounter(&t);
		return double(t.QuadPart) / double(freq.QuadPart);
	}

private:
	LARGE_INTEGER freq;
};

class FpsCounter
{
public:
	FpsCounter(int average_length) :history(average_length, 0.0)
	{
	}

	double tick()
	{
		double t = prof.time();
		history[idx++ % history.size()] = t - prev;
		prev = t;

		double avg = 0;
		for (auto &v : history)avg += v;
		return history.size() / avg;
	}

private:
	double prev = 0;
	int idx = 0;
	std::vector<double> history;
	Profiler prof;
};

inline std::string ssprintf(const char *format, ...)
{
	va_list list;
	va_start(list, format);
	char buf[1024];
	vsnprintf(buf, sizeof(buf), format, list);
	va_end(list);
	return buf;
}

class RangeCache
{
public:
	RangeCache(int beg, int end);

	void set(int beg, int end);
	bool is_set(int beg, int end) const;

private:
	struct Chunk
	{
		int beg;
		int end;
	};
	std::vector<Chunk> chunks;
};