#pragma once
#include <string>

namespace GDX11::Utils
{
	std::string LoadText(const std::string& filename);
	std::wstring ToWideString(const std::string& str);
}