#pragma once

#include <string>

class keymgmt
{
public:
	class autounlocker
	{
	public:
		autounlocker(keymgmt& mgmt);
		~autounlocker();

	private:
		keymgmt& m_mgmt;
	};
public:
	keymgmt();
	~keymgmt();

	void load_keys(const std::string& publicKey, const std::string& privateKey);

	const char* get_private_key() const { return m_pszPrivateKey; }
	const std::string& get_public_key() const { return m_publicKey; }
protected:
	void lock_key();
	void unlock_key();
private:
	char* m_pszPrivateKey;
	unsigned int m_uPrivateKeySize;
	std::string m_publicKey;
};
