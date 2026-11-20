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

#include "../../dependencies/mbedtls/library/ssl_ciphersuites.c"
#include "../../dependencies/mbedtls/library/ssl_msg.c"
#include "../../dependencies/mbedtls/library/ssl_tls.c"
#include "../../dependencies/mbedtls/library/ssl_client.c"
#include "../../dependencies/mbedtls/library/ssl_tls12_client.c"

#endif
