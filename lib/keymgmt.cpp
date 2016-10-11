#include <pswmgr/keymgmt.h>
#include <windows.h>

keymgmt::autounlocker::autounlocker(keymgmt& mgmt)
	: m_mgmt(mgmt)
{
	m_mgmt.unlock_key();
}

keymgmt::autounlocker::~autounlocker()
{
	m_mgmt.lock_key();
}

keymgmt::keymgmt()
	: m_pszPrivateKey(nullptr)
{
	m_pszPrivateKey = reinterpret_cast<char*>(VirtualAlloc(m_pszPrivateKey, 4096, MEM_COMMIT, PAGE_READWRITE));
	VirtualLock(m_pszPrivateKey, 4096);

	lock_key();
}

keymgmt::~keymgmt()
{
	memset(m_pszPrivateKey, 0, 4096);
	VirtualUnlock(m_pszPrivateKey, 4096);

	m_pszPrivateKey = nullptr;
}

void keymgmt::load_keys(const std::string& publicKey, const std::string& privateKey)
{
	FILE* file = nullptr;
	fopen_s(&file, publicKey.c_str(), "r");
	if (file == nullptr)
		return;

	char buffer[4096];
	memset(buffer, 0, 4096);
	fread(buffer, 1, 4096, file);
	fclose(file);
	m_publicKey = { buffer };

	file = nullptr;
	fopen_s(&file, publicKey.c_str(), "r");
	if (file == nullptr)
		return;

	{
		autounlocker lock(*this);
		memset(m_pszPrivateKey, 0, 4096);
		fread(m_pszPrivateKey, 1, 4096, file);
	}
	
	fclose(file);
}

void keymgmt::lock_key()
{
	DWORD sOldProtectValue;
	VirtualProtect(m_pszPrivateKey, 4096, PAGE_NOACCESS, &sOldProtectValue);
}

void keymgmt::unlock_key()
{
	DWORD sOldProtectValue;
	VirtualProtect(m_pszPrivateKey, 4096, PAGE_READWRITE, &sOldProtectValue);
}