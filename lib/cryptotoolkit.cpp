#include <pswmgr/cryptotoolkit.h>
#include <pswmgr/keymgmt.h>

std::string cryptotoolkit::encrypt(keymgmt& mgmt, const std::string& text)
{
//http://unix.stackexchange.com/questions/12260/how-to-encrypt-messages-text-with-rsa-using-openssl
	keymgmt::autounlocker unlocker(mgmt);
	/*echo 'Hi Alice! Please bring malacpörkölt for dinner!' |
		openssl rsautl - encrypt - pubin - inkey alice.pub > message.encrypted
		
		echo 'Hi Alice! Please bring malacpörkölt for dinner!' | openssl rsautl -encrypt -pubin -inkey public_key.pem > message.encrypted
		*/
	return{};
}


// rsautl -decrypt -inkey private_key.pem -in msg.enc -out msg.dec

