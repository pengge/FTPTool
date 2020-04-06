/********************************
 Scope:		Microsecond (us) level timer
 Author:	Walnut
 Updated:	2012-08-11
********************************/

#include "StdAfx.h"
#include "Timer.h"

CTimer::CTimer(void)
{
	m_bRun = false;
	m_fPaused = 0;
}

CTimer::~CTimer(void)
{
}

bool CTimer::Start(void)
{
	if(FALSE == QueryPerformanceFrequency(&this->m_nFrequency))
	{return false;}
	if(FALSE == QueryPerformanceCounter(&m_nStartTime))
	{return false;}

	m_bRun = true;
	return true;
}

bool CTimer::Stop(void)
{
	m_bRun = false;
	m_nStartTime.LowPart = 0;
	m_nPauseTime.LowPart = 0;
	return true;
}

bool CTimer::Pause(void)
{
	VERIFY(m_bRun);
	return (FALSE != QueryPerformanceCounter(&m_nPauseTime));
}

bool CTimer::Restore(void)
{
	VERIFY(m_bRun);

	LARGE_INTEGER nStopTime;
	if(FALSE == QueryPerformanceCounter(&nStopTime))
	{return false;}

	if(nStopTime.LowPart < m_nPauseTime.LowPart)
	{nStopTime.LowPart += 0xFFFFFFFF;}

	m_fPaused = (double)(nStopTime.LowPart - m_nPauseTime.LowPart)/(double)m_nFrequency.QuadPart;
	return true;
}

bool CTimer::Reset(void)
{
	m_fPaused = 0;
	m_nPauseTime.LowPart = 0;
	return (FALSE != QueryPerformanceCounter(&m_nStartTime));
}

double CTimer::GetElapsedSeconds(void)
{
	VERIFY(m_bRun);

	LARGE_INTEGER nStopTime;
	double fValue;
	
	if(FALSE == QueryPerformanceCounter(&nStopTime))
	{return 0;}

	if(nStopTime.LowPart < m_nStartTime.LowPart)
	{nStopTime.LowPart += 0xFFFFFFFF;}

	fValue = (double)(nStopTime.LowPart - m_nStartTime.LowPart)/(double)m_nFrequency.QuadPart;
	return (fValue - m_fPaused);
}