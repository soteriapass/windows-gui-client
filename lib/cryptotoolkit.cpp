#include <pswmgr/cryptotoolkit.h>
#include <pswmgr/keymgmt.h>
#include <sstream>

std::string cryptotoolkit::encrypt(keymgmt& mgmt, const std::string& text)
{
//http://unix.stackexchange.com/questions/12260/how-to-encrypt-messages-text-with-rsa-using-openssl
	//keymgmt::autounlocker unlocker(mgmt);
	/*echo 'Hi Alice! Please bring malacpörkölt for dinner!' |
		openssl rsautl - encrypt - pubin - inkey alice.pub > message.encrypted
		
		echo 'Hi Alice! Please bring malacpörkölt for dinner!' | openssl rsautl -encrypt -pubin -inkey public_key.pem > message.encrypted
		*/

	std::stringstream ss;
	ss << "echo " << text << " | openssl rsautl -encrypt -pubin -inkey C:\\public_key.pem > D:\\msg.enc";
	system(ss.str().c_str());
	return{};
}

std::string cryptotoolkit::decrypt(keymgmt& mgmt, const std::string& text)
{
	keymgmt::autounlocker unlocker(mgmt);
	system("openssl rsautl -decrypt -inkey C:\\private_key.pem -in D:\\msg.enc -out D:\\msg.dec");
	return{};
}

// rsautl -decrypt -inkey private_key.pem -in msg.enc -out msg.dec

