#pragma once
#ifdef WIN32
#include <windows.h>
#endif
#include <vector>
#include <string>
#include <stdarg.h>

class Profiler
{
public:
	Profiler()
	{
#ifdef WIN32
		QueryPerformanceFrequency(&freq);
#endif
	}

	double time() const
	{
#ifdef WIN32
		LARGE_INTEGER t;
		QueryPerformanceCounter(&t);
		return double(t.QuadPart) / double(freq.QuadPart);
#else
		// TODO: gettimeofday()
		return 0;
#endif
	}

private:
#ifdef WIN32
	LARGE_INTEGER freq;
#endif
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