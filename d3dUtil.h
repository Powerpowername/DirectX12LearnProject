#pragma once
#include <windows.h>
#include <string>
#include <stdexcept> 
std::string HrToString(HRESULT hr);

class HrException : public std::runtime_error
{
public:
	HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
	HRESULT Error() const { return m_hr; }
private:
	const HRESULT m_hr;

};

void ThrowIfFailed(HRESULT hr);

std::string HrToString(HRESULT hr);


void ThrowIfFailed(HRESULT hr);

#ifndef ReleaseCom
#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }
#endif