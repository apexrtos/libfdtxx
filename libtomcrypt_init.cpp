#define LTM_DESC
#include <tomcrypt.h>

namespace {

struct libtomcrypt_init {
	libtomcrypt_init();
} libtomcrypt_init;

libtomcrypt_init::libtomcrypt_init()
{
	init_LTM();
	register_all_ciphers();
	register_all_hashes();
	register_all_prngs();
}

}
