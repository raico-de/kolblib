# kolblib — vendored versions

| Component | Version / origin | License | Path |
|-----------|------------------|---------|------|
| **lwIP / NXP TCPIP** | From T102test `stacks/tcpip` (NXP PlatformSDK_S32K3 TCPIP / RTD 7.0 era) | NXP / lwIP BSD | `kolblib/tcpip/` |
| **FreeRTOS Kernel** | **v11.1.0** — copied from `C:\NXP\S32DS.3.6.5\S32DS\software\PlatformSDK_S32K3\FreeRTOS\Source` (`platform.os.freertos`). **Not** present under S32DS 3.6.9 PlatformSDK. | MIT | `kolblib/freertos/Source/` |
| **Mbed TLS** | **3.6.7** LTS — official release `mbedtls-3.6.7.tar.bz2` (SHA256 `a7e8bcbec0e6f761b4af24f25677626b35f762f68eef79c08677a363212d11f6`) | Apache-2.0 OR GPL-2.0-or-later | `kolblib/mbedtls/` |
| **ISR multi-rate scheduler** | kolblib original | project | `kolblib/isr_sched/` |

## Notes

- S32DS **3.6.9** has RTD 7.0 + TCPIP, but **no** FreeRTOS package tree. FreeRTOS was taken from the installed **3.6.5** PlatformSDK path above (verified on disk).
- Component IDs in NXP manifest: `platform.os.freertos`, `platform.os.freertosCM7`, `platform.os.freertos_heap_*` (not `platform.middleware.freertos`).
- `T102test/stacks/tcpip` is a **junction** → `kolblib/tcpip` so existing relative paths keep working.
- `LWIP_ALTCP_TLS_MBEDTLS` stays **0** until Eth/lwIP build is coherent; Mbed TLS is vendored + configured for later enable.
