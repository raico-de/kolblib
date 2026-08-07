/**
 * @file kolblib_entropy_stub.c
 * @ingroup kolblib_mbedtls
 * @brief Placeholder @c mbedtls_hardware_poll for TLS bring-up only.
 *
 * @warning Zeros are not secure. Replace with S32K3 TRNG/HSE before production TLS.
 * Linked only when mbedtls library sources are added to the build.
 *
 * @code
 * // Provided as MBEDTLS_ENTROPY_HARDWARE_ALT hook via mbedtls_config.h
 * @endcode
 */
#include "mbedtls/entropy.h"
#include <string.h>

/**
 * @brief Mbed TLS hardware entropy callback (STUB — returns zeros).
 * @param[in]  data   Unused context.
 * @param[out] output Entropy buffer.
 * @param[in]  len    Requested length.
 * @param[out] olen   Actual length written.
 * @return 0 on success, -1 on bad args.
 */
int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen)
{
    (void)data;
    if (output == NULL || olen == NULL)
    {
        return -1;
    }
    /* Non-secure placeholder: zeros. Must be replaced. */
    memset(output, 0, len);
    *olen = len;
    return 0;
}
