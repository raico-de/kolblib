/**
 * @file mbedtls_config.h
 * @ingroup kolblib_config
 * @brief Conservative Mbed TLS 3.6.x config for Cortex-M7 / S32K344 (kolblib).
 *
 * Dual Apache-2.0 / GPL — choose Apache-2.0 for this project.
 * Compile with: `-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>` and `-I` to
 * `kolblib/config` plus `kolblib/mbedtls/include`.
 *
 * Do NOT enable @c LWIP_ALTCP_TLS_MBEDTLS until Eth/lwIP is coherent.
 *
 * @code
 * // arm-none-eabi-gcc ... -DMBEDTLS_CONFIG_FILE=\"mbedtls_config.h\"
 * @endcode
 */
/**
 * Conservative Mbed TLS 3.6.x config for Cortex-M7 / S32K344 (kolblib).
 * Dual Apache-2.0 / GPL — choose Apache-2.0 for this project.
 *
 * Compile with: -DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>
 * and -I to kolblib/config plus kolblib/mbedtls/include.
 *
 * Do NOT enable LWIP_ALTCP_TLS_MBEDTLS until lwIP/Eth build is coherent.
 */
#ifndef KOLBLIB_MBEDTLS_CONFIG_H
#define KOLBLIB_MBEDTLS_CONFIG_H

#define MBEDTLS_HAVE_ASM
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_NO_PLATFORM_ENTROPY
#define MBEDTLS_ENTROPY_HARDWARE_ALT
#define MBEDTLS_AES_ROM_TABLES

#define MBEDTLS_AES_C
#define MBEDTLS_CIPHER_MODE_CBC
#define MBEDTLS_CIPHER_MODE_CTR
#define MBEDTLS_GCM_C
#define MBEDTLS_CIPHER_C
#define MBEDTLS_MD_C
#define MBEDTLS_SHA1_C
#define MBEDTLS_SHA224_C
#define MBEDTLS_SHA256_C
#define MBEDTLS_SHA384_C
#define MBEDTLS_SHA512_C
#define MBEDTLS_MD5_C

#define MBEDTLS_BIGNUM_C
#define MBEDTLS_OID_C
#define MBEDTLS_RSA_C
#define MBEDTLS_PKCS1_V15
#define MBEDTLS_PKCS1_V21
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_BASE64_C
#define MBEDTLS_PEM_PARSE_C
#define MBEDTLS_PEM_WRITE_C
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_PK_WRITE_C
#define MBEDTLS_X509_USE_C
#define MBEDTLS_X509_CRT_PARSE_C
#define MBEDTLS_X509_CRL_PARSE_C
#define MBEDTLS_ECDSA_C
#define MBEDTLS_ECP_C
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#define MBEDTLS_ECDH_C

#define MBEDTLS_ENTROPY_C
#define MBEDTLS_CTR_DRBG_C
#define MBEDTLS_HMAC_DRBG_C

#define MBEDTLS_SSL_TLS_C
#define MBEDTLS_SSL_PROTO_TLS1_2
#define MBEDTLS_SSL_CLI_C
#define MBEDTLS_SSL_SRV_C
#define MBEDTLS_SSL_MAX_CONTENT_LEN                4096

#define MBEDTLS_KEY_EXCHANGE_RSA_ENABLED
#define MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED
#define MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED

#include "mbedtls/check_config.h"

#endif /* KOLBLIB_MBEDTLS_CONFIG_H */
