#pragma once
#include <string>

class keymgmt;

class cryptotoolkit
{
public:
	static std::string encrypt(keymgmt& mgmt, const std::string& text);
	static std::string decrypt(keymgmt& mgmt, const std::string& text);
};
