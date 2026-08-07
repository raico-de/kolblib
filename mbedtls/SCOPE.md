# Mbed TLS in kolblib (scoped)

Vendored: **mbedtls 3.6.7** under `include/` and `library/`.

## Not in default build

Project `.cproject` files get **include paths** + `MBEDTLS_CONFIG_FILE` ready, but do **not**
compile `library/*.c` until Eth/lwIP is coherent.

## To enable compilation later

1. Add source folder `../../kolblib/mbedtls/library` (exclude nothing essential; skip if any `*.c` under tests).
2. Add `../../kolblib/mbedtls/kolblib_entropy_stub.c` (replace with real TRNG ASAP).
3. Ensure `-DMBEDTLS_CONFIG_FILE=\"mbedtls_config.h\"` and `-I../../kolblib/config`.
4. Then set in `lwipopts.h`: `LWIP_ALTCP`, `LWIP_ALTCP_TLS`, `LWIP_ALTCP_TLS_MBEDTLS` to 1.
