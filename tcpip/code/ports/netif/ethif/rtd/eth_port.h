/*
 * Copyright 2019-2025 NXP
 * All rights reserved.
 *
 * This software is owned or controlled by NXP and may only be
 * used strictly in accordance with the applicable license terms. By expressly
 * accepting such terms or by downloading, installing, activating and/or otherwise
 * using the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms. If you do not agree to be
 * bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software. The production use license in
 * Section 2.3 is expressly granted for this software.
 *
 * This file is derived from the Ethernet Interface Skeleton in lwIP with the following copyright:
 *
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 */

/**
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 8.1, function has no explicit type
 * The type is defined in some place.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 8.2, function has no explicit type or class, int assumed
 * This is a fake finding.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 8.3, All declarations or an object or function shall use the
 * same names and type qualifiers.
 * Argument of function has the same name and type qualifier in both files. This is a fake finding.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Directive 4.9, A function should be used in preference
 * to a function-like macro where they are interchangeable.
 * Function-like macros are used instead of inline functions in order to ensure
 * that the performance will not be decreased if the functions will not be
 * inlined by the compiler.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 10.1, Unpermitted operand to operator '&&'
 * Variable is of essential boolean type
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 5.7, A tag name shall be a unique identifier.
 * Symbol redeclaration is declared at different local functions.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 8.4, A compatible declaration shall be
 * visible when an object or function with external linkage is defined.
 * These are symbols weak symbols defined in platform startup files (.s).
 *
 */
#ifndef ETH_PORT_H
#define ETH_PORT_H

#include "lwip/err.h"
#include "lwip/tcpip.h"
#include "lwip/ip_addr.h"

#include "OsIf.h"
#include "EthIf.h"
#include "EthIf_Cfg.h"
#include "OsIf_rtd_port.h"

#include "netifcfg.h"


/*! @brief Data alignment. */
#if defined ( __GNUC__ ) || defined ( __ghs__ ) || defined (__ARMCC_VERSION)
    #define ALIGNED(x)      __attribute__((aligned(x)))
#elif defined ( __ICCARM__ )
    #define stringify(s) tostring(s)
    #define tostring(s) #s
    #define ALIGNED(x)      _Pragma(stringify(data_alignment=x))
#else
    /* Keep compatibility with software analysis tools */
    #define ALIGNED(x)
#endif

/*! @brief Defines the maximum Ethernet frame size. */
#define TCPIP_FRAME_MAX_FRAMELEN         (1520U)

/*! @brief Definitions used for aligning the buffers */
#define TCPIP_BUFF_ALIGNMENT     64
#define TCPIP_BUFF_ALIGN(x)       (((uint32_t)(x) + (TCPIP_BUFF_ALIGNMENT - 1UL)) & ~(TCPIP_BUFF_ALIGNMENT - 1UL))

/*! @brief Receive buffer descriptors */
#define TCPIP_RXBUFF_SIZE    TCPIP_BUFF_ALIGN(TCPIP_FRAME_MAX_FRAMELEN)

/* Code returned by the pre-input handler in the case when the frame should be forwarded to the stack */
#define FORWARD_FRAME   (0U)

/* Also defined on driver Eth.c file */
#define TCPIP_FRAME_PAYLOAD_OFFSET    (14U)

#define TCPIP_FRAME_MACDST_LENGTH     (6U)
#define TCPIP_FRAME_MACSRC_LENGTH     (6U)
#define TCPIP_FRAME_ETHTYPE_LENGTH    (2U)
#define TCPIP_FRAME_HEADER_LENGTH     (TCPIP_FRAME_MACDST_LENGTH + TCPIP_FRAME_MACSRC_LENGTH + TCPIP_FRAME_ETHTYPE_LENGTH)

typedef unsigned int (*Tcpip_RxBuffProcessCondHandler_t)(uint8_t eth_instance, void *buff);

#if (NO_SYS == 0)
extern sys_mutex_t tcpip_TxLock;
#endif /* (NO_SYS == 0) */

err_t Tcpip_EthernetifInit(struct netif *netif);
void Tcpip_EthernetifShutdown(struct netif *netif);

#if (LWIP_IPV6 == 1)
err_t Tcpip_MldEthFilter (struct netif *netif, const ip6_addr_t *group, enum netif_mac_filter_action action);
#endif /* (LWIP_IPV6 == 1) */
#if (LWIP_IGMP == 1)
err_t Tcpip_IgmpEthFilter (struct netif *netif, const ip4_addr_t *group, enum netif_mac_filter_action action);
#endif /* (LWIP_IGMP == 1) */
#if (NO_SYS == 1)
err_t Tcpip_PollInterface(struct netif *netif);
#endif /* (NO_SYS == 1) */

void send_tx_pbuffs_dummy_char(void);
void Tcpip_RegisterRxBuffProcessConditionHandler(Tcpip_RxBuffProcessCondHandler_t handler);
void Tcpip_RxIndication(uint8 CtrlIdx, Eth_FrameType FrameType, boolean IsBroadcast, const uint8* PhysAddrPtr, const Eth_DataType* DataPtr, uint16 DataLen, TimeTupleType* IngressTimeTuplePtr, Eth_BufIdxType RxHandleId);
void Tcpip_TxConfirmation(uint8 CtrlIdx, Eth_BufIdxType BufIdx, Std_ReturnType Result);

#endif /* ETH_PORT_H */
