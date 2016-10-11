#include <pswmgr/keymgmt.h>
#include <pswmgr/cryptotoolkit.h>

int main()
{
	keymgmt mgmt;
	mgmt.load_keys("C:\\public_key.pem", "C:\\private_key.pem");
	cryptotoolkit::encrypt(mgmt, "This is an awesome test");
	cryptotoolkit::decrypt(mgmt, "");
	return 0;
}