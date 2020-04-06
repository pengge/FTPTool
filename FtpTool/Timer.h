/********************************
 Scope:		Microsecond (us) level timer
 Author:	Walnut
 Updated:	2012-08-11
********************************/
#pragma once

class CTimer
{
public:
	CTimer(void);
	~CTimer(void);

	bool Start(void);
	bool Stop(void);
	bool Pause(void);
	bool Restore(void);
	bool Reset(void);
	double GetElapsedSeconds(void);

private:
	bool m_bRun;
	double m_fPaused;
	LARGE_INTEGER m_nFrequency;
	LARGE_INTEGER m_nStartTime;
	LARGE_INTEGER m_nPauseTime;
};
