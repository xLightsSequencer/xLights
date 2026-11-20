#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// The Mbed TLS build used by the LAN transport on Windows and Linux: just
// what TLS_PSK_WITH_AES_128_GCM_SHA256 over TLS 1.2 and the PBKDF2 pairing
// key need, so it interoperates with the Apple transport and nothing else is
// compiled in. Every file that includes an Mbed TLS header must see this same
// configuration (CollabTls.c and CollabNetPortable.cpp define
// MBEDTLS_CONFIG_FILE before their includes).

#define MBEDTLS_HAVE_ASM
#define MBEDTLS_HAVE_TIME

#define MBEDTLS_SSL_TLS_C
#define MBEDTLS_SSL_CLI_C
#define MBEDTLS_SSL_SRV_C
#define MBEDTLS_SSL_PROTO_TLS1_2
#define MBEDTLS_KEY_EXCHANGE_PSK_ENABLED
#define MBEDTLS_SSL_EXTENDED_MASTER_SECRET
#define MBEDTLS_SSL_ALL_ALERT_MESSAGES
#define MBEDTLS_PSK_MAX_LEN 32

#define MBEDTLS_CIPHER_C
#define MBEDTLS_AES_C
#define MBEDTLS_AESNI_C
#define MBEDTLS_AESCE_C
#define MBEDTLS_GCM_C
#define MBEDTLS_MD_C
#define MBEDTLS_SHA256_C
#define MBEDTLS_PKCS5_C

#define MBEDTLS_CTR_DRBG_C
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_ERROR_C
