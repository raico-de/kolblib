# kolblib

Shared embedded libraries for S32K344 / FRDM-A projects under `wsnxp`.

```
kolblib/
  tcpip/          NXP lwIP stack (moved from T102test; junction remains)
  freertos/       FreeRTOS Kernel v11.1.0 (NXP S32DS package copy)
  mbedtls/        Mbed TLS 3.6.7 LTS (include/ + library/)
  isr_sched/      Parallel PIT multi-rate ISR scheduler
  config/         FreeRTOSConfig.h, mbedtls_config.h
  future/         Placeholder for project-local libs
  VERSIONS.md     Exact origins / checksums
```

## FreeRTOS ↔ ISR scheduler

- **PIT0** drives 62.5 µs / 205 µs / 1 ms / 10 ms callbacks (short ISR: flags + optional callbacks).
- All PIT0 channels share **one** NVIC vector (`PIT0_IRQn`); software order services fastest rate first.
- FreeRTOS runs as the **lowest** layer: `vTaskStartScheduler()` in `main` (idle + tasks). SysTick remains the FreeRTOS tick.
- High-rate ISRs (62.5 / 205 µs) must **not** call FreeRTOS APIs (keep NVIC priority numerically lower / higher urgency than `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY`).
- 1 ms / 10 ms may use `xTaskNotifyFromISR` / `xSemaphoreGiveFromISR` **only if** PIT0 NVIC priority is at or below (numerically ≥) the FreeRTOS max-syscall priority. Default setup uses volatile flags; app code defers work in a FreeRTOS task by polling `IsrSched_GetFlags()`.

## Mbed TLS scope

- Sources live under `mbedtls/library`; **not** added to project source folders by default (keeps build light).
- Includes + `kolblib/config/mbedtls_config.h` are wired; define `MBEDTLS_CONFIG_FILE` to that header when compiling TLS.
- When Eth/lwIP is stable: add `mbedtls/library` sources, then set `LWIP_ALTCP_TLS` / `LWIP_ALTCP_TLS_MBEDTLS` to 1.

## Projects

| Project | FreeRTOS | ISR sched | Notes |
|---------|----------|-----------|-------|
| **T102test** | Linked + started in `main` | PIT0 wired + vector | Full integration |
| **dm-motion-…** | Linked sources/includes | Sources linked | Main still FreeMASTER loop; start scheduler when ready |
| **a344_0** | Includes + links only | Includes only | See `a344_0/FREERTOS_NOTE.md` (SVC clash) |

## S32DS Config Tools (user)

1. Add **FreeRTOS** component (`platform.os.freertos` + CM7 + heap) if regenerating from `.mex` — or keep kolblib-linked sources.
2. Complete **Eth_43_GMAC** / EthIf / Phy, then **Update Code**.
3. Optionally add **Gpt** if you prefer RTD GPT instead of the register-level PIT driver in `isr_sched`.
4. Ensure vector table entry for `PIT0_IRQn` points at `IsrSched_PIT0_IRQHandler` (done for T102test).
5. Refresh / re-index projects in S32DS after pulling these Eclipse link changes.
