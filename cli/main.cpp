#include <pswmgr/keymgmt.h>

int main()
{
	keymgmt mgmt;
	mgmt.load_keys("C:\\public_key.pem", "C:\\private_key.pem");
	return 0;
}