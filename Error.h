#ifndef __ERROR_H__
#define __ERROR_H__

#include "Headers.h"

class Error : public std::exception
{
private:
	std::wstring m_errorString;
public:
	Error(std::wstring_view Function, std::wstring_view File, int Line, HRESULT Hr)
	{
		m_errorString += Function;
		m_errorString += L"\nFailed in: "; 
		m_errorString += File;
		m_errorString += L" \nLine "; 
		m_errorString += std::to_wstring(Line) + L"\n";
		m_errorString += _com_error(Hr).ErrorMessage();
	}
		
	const wchar_t* ErrorString() const noexcept
	{	
		return m_errorString.c_str();
	}
};

#endif