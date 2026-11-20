/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Mbed TLS (dependencies/mbedtls), as the Windows and Linux LAN transport
// uses it, compiled from a few units (its sources reuse static names, so they
// can't all share one) so every build system only needs these files, the
// Mbed TLS include folder and src-core on its include path. Apple builds use
// Network.framework instead and compile nothing here, and so does a build
// against an installed Mbed TLS (XL_MBEDTLS_EXTERNAL).

#if (!defined(__APPLE__) || defined(XL_COLLAB_PORTABLE_NET)) && !defined(XL_MBEDTLS_EXTERNAL)

#define MBEDTLS_CONFIG_FILE "collab/CollabTlsConfig.h"

#if defined(_MSC_VER) && !defined(XLIGHTS_CMAKE_BUILD)
#pragma comment(lib, "bcrypt.lib") // entropy_poll.c: BCryptGenRandom
#endif

#include "../../dependencies/mbedtls/library/platform.c"
#include "../../dependencies/mbedtls/library/platform_util.c"
#include "../../dependencies/mbedtls/library/constant_time.c"
#include "../../dependencies/mbedtls/library/error.c"
#include "../../dependencies/mbedtls/library/aes.c"
#include "../../dependencies/mbedtls/library/aesni.c"
#include "../../dependencies/mbedtls/library/aesce.c"
#include "../../dependencies/mbedtls/library/block_cipher.c"
#include "../../dependencies/mbedtls/library/gcm.c"
#include "../../dependencies/mbedtls/library/cipher.c"
#include "../../dependencies/mbedtls/library/cipher_wrap.c"
#include "../../dependencies/mbedtls/library/md.c"
#include "../../dependencies/mbedtls/library/sha256.c"
#include "../../dependencies/mbedtls/library/pkcs5.c"
#include "../../dependencies/mbedtls/library/entropy.c"
#include "../../dependencies/mbedtls/library/entropy_poll.c"
#include "../../dependencies/mbedtls/library/ctr_drbg.c"

#endif
