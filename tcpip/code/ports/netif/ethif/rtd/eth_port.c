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
 * Violates MISRA 2012 Required Rule 1.3,  Taking address of near auto variable.
 * The code is not dynamically linked. An absolute stack address is obtained
 * when taking the address of the near auto variable.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 2.1, A project shall not contain unreachable code.
 * These are safety checks to avoid dereferencing NULL pointers.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 8.4, A compatible declaration shall be
 * visible when an object or function with external linkage is defined.
 * These are symbols weak symbols defined in platform startup files (.s).
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.9, Could define variable at block scope
 * The variable is used in pal c file, so it must remain global.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.13, Pointer parameter could be declared as pointing to const
 * Type definition is done in another file.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 10.1, Unpermitted operand to operator '&&'
 * Variable is of essential boolean type
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 10.3, The value of an expression shall not be assigned to an
 * object with a narrower essential type or a different essential type category.
 * This is a string that will be concatenated to a macro variable to define a new one.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 10.4, Both operands of an operator in which the usual arithmetic
 * conversions are performed shall have the same essential type category.
 * These are bitwise operations used to enable flags or check their state.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 10.8, Impermissible cast of composite expression
 * Required in comparisons between constants and numerical types.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.3, Cast performed between a pointer to object type
 * and a pointer to a different object type.
 * This is used to check if transmission is complete.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.2, Conversion between a pointer to incomplete type and another type
 * The is a fake finding
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 11.4, Conversion between a pointer and
 * integer type.
 * The cast is required to initialize a pointer with an unsigned long define,
 * representing an address.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 11.5, Conversion from pointer to void to pointer to other type.
 * The conversion is needed to allocate or free the memory.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.6, A cast shall not be performed between
 * pointer to void and an arithmetic type.
 * The cast is required to comply with the lwip API that mandates passing arguments
 * to threads using a pointer to void type.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 13.5, The right hand operand of a logical && or || operator shall
 * not contain persistent side effects.
 * This is required in order to reduce code complexity.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 14.3, Controlling expressions shall not be invariant
 * This behaviour is intentional, some functions must always be called in current implementation.
 * Condition is still checked in event of further modifications for other applications.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 14.4, Conditional expression should have essentially Boolean type.
 * This is required for macro constructs in form do {...} while(0).
 *
 * @section [global]
 * Violates MISRA 2012 Mandatory Rule 17.3, Symbol undeclared, assumed
 * to return int.
 * The symbol is defined in another file.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 18.2, Substract operator applied to pointers.
 * Operation is required to compute the aligned address of the memory zone that further
 * needs to be freed.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 18.3, Relational or substract operator applied
 * to pointers.
 * Substraction is needed to compute a memory address.
 *
 * @section [global]
 * Violates MISRA 2012 Mandatory Rule 17.4, All exit paths from a function with non-void
 * return type shall have an explicit return statement with an expression.
 * Return value is available but cannot be reached due to multi threads.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 5.3, An identifier declared in an inner scope
 * shall not hide an identifier declared in an outer scope.
 * Symbol redeclaration is declared at different local functions.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 5.6, A typedef name shall be a unique identifier.
 * Symbol redeclaration is declared at different local functions.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 5.7, A tag name shall be a unique identifier.
 * Symbol redeclaration is declared at different local functions.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 5.8, Identifiers that define objects or functions
 * with external linkage shall be unique.
 * Symbol redeclaration is declared at different local functions.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 5.9, Identifiers that define objects or functions
 * with internal linkage should be unique.
 * Symbol redeclaration is declared at different local functions.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 15.5, A function should have a single point of exit
 * at the end.
 * This is acceptable because of multi threading application.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 1.2, Language extensions should not be used.
 * This is required for debug symbol assert.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 1.1, The program shall contain no violations of the
 * standard C syntax and constraints, and shall not exceed the implementation's translation limits.
 * This is required for debug symbol assert.
 *
 */

#include "eth_port.h"
#include <string.h>
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/ethip6.h"
#include "lwip/etharp.h"
#include "lwip/tcpip.h"

#include "lwipcfg.h"
#include "lwip/sys.h"

#include "PlatformTypes.h"
#include "Std_Types.h"

#if !defined(USING_OS_FREERTOS) || !defined(USING_OS_AUTOSAROS)
#include "sys_arch.h"
#endif


#if defined(USING_OS_FREERTOS)
#include "FreeRTOS.h"
#endif /* defined(USING_OS_FREERTOS) */

#define IFNAME0 'e'
#define IFNAME1 'n'

/* Macros defining whether pbufs are chained or single */
#define TCPIP_SINGLE_PBUF 1
#define TCPIP_CHAINED_PBUF 0

struct netif * tcpip_gNetif[NETIF_NUMBER] = { NULL };

static Tcpip_RxBuffProcessCondHandler_t tcpip_RxBuffProcessHandler = NULL;

#if (NO_SYS == 0)
/* Lock to synchronize access on TX side, since the frames are sent from different threads */
sys_mutex_t tcpip_TxLock;
#endif /* (NO_SYS == 0) */

__attribute__ ((section (".tcpip_buffer")))
VAR_ALIGN(uint8_t tcpip_DataBuffer[NETIF_RXBD_NUM * NETIF_MAX_RXBUFFLEN_SUPPORTED], 128)
uint8_t tcpip_DataIndex;

typedef struct tx_pbufs_t{
    struct pbuf * volatile tx_pbuf;
    Eth_BufIdxType buf_idx;
} tx_pbufs_t;

__attribute__ ((section (".tcpip_buffer")))
tx_pbufs_t tcpip_TxPbufs[NETIF_TXBD_NUM];

#if (NO_SYS == 0)

/* In order to support zero-copy operation, on the RX side we are using custom pbufs, with the payload pointing to the
    receive buffer obtained from the driver. When the pbuf is eventually freed, the receive buffer is given back to the driver.
    On the TX side we are incrementing the reference count on the pbuf and giving its payload storage to the driver. Once we
    detect the transmission is complete, we are freeing our reference to the pbuf. */

/* Memory pool for RX custom pbufs
   The pool only holds the pbuf_custom structures, not the storage for actual payload */
LWIP_MEMPOOL_DECLARE(RX_POOL, NETIF_RXBD_NUM, sizeof(struct pbuf_custom), "Zero-copy RX PBUF pool")

/* Queue for holding pbufs which have been sent to the driver for transmission. They will be released once transmission is complete */
static sys_mbox_t tcpip_InFlightTxPbufs;

#if (NETIF_IRQ_ENABLED == STD_OFF)
static sys_thread_t tcpip_PollThread;
#endif /* (NETIF_IRQ_ENABLED == STD_OFF) */

/**
 * Callback function called when a custom pbuf is freed
 *
 * @param p - the custom pbuf structure
 * Implements ethif_pbuf_free_custom_Activity
 */
static void Tcpip_PbufFreeCustom(struct pbuf *p)
{
    LWIP_ASSERT("NULL pointer", p != NULL);
    struct pbuf_custom* pc = (struct pbuf_custom*)p;
    LWIP_MEMPOOL_FREE(RX_POOL, pc);
}

#endif /* (NO_SYS == 0) */
/**
 * Transmit a packet.
 * The packet is contained in the pbuf that is passed to the function. This pbuf might be chained.
 *
 * @param netif - the lwip network interface structure for this ethernetif
 * @param p - the pbuf structure
 * Implements ethif_low_level_output_Activity
 */
static err_t Tcpip_LowLevelOutput(struct netif *netif, struct pbuf *p)
{
    uint8_t pbuf_chain_type = TCPIP_SINGLE_PBUF;
    uint8_t *pQhelper;
    uint8_t *pMacDestAddr;
    uint8_t *pPayloadBuffer;
    uint16_t u16payloadLength;
    BufReq_ReturnType bufferStatus = BUFREQ_OK;
    Eth_FrameType frameType;
    Std_ReturnType status = BUFREQ_OK;
    err_t pbuf_status = ERR_BUF;
    struct pbuf *q;
    uint8_t i;


    LWIP_ASSERT("Output packet buffer empty", p);
#if defined(LWIP_DEBUG) && LWIP_NETIF_TX_SINGLE_PBUF && !(LWIP_IPV4 && IP_FRAG) && (LWIP_IPV6 && LWIP_IPV6_FRAG)
    LWIP_ASSERT("p->next == NULL && p->len == p->tot_len", p->next == NULL && p->len == p->tot_len);
#endif /* LWIP_DEBUG && LWIP_NETIF_TX_SINGLE_PBUF && !(LWIP_IPV4 && IP_FRAG) && (LWIP_IPV6 && LWIP_IPV6_FRAG */

    /* Increment our reference on p */
    pbuf_ref(p);

    /* Check whether this was single or a chained pbuf */
    if (NULL != p->next)
    {
        /* This is a chained pbuf, save info into a local variable, as p will be lost if allocation does not fail */
        pbuf_chain_type = TCPIP_CHAINED_PBUF;
    }

    /* If p was a pbuf chain instead, p's ref was decreased and we got another q pbuf with ref 1
    Either way, q has a +1 ref that we need to free in case we're not keeping the buffer - ie in case of errors*/

    q = pbuf_coalesce(p, PBUF_RAW);

    pbuf_status = ERR_BUF;
    while (ERR_BUF == pbuf_status)
    {
        for(i=0; i < NETIF_TXBD_NUM; i++)
        {
            sys_arch_protect();
            if (tcpip_TxPbufs[i].tx_pbuf == NULL)
            {
                tcpip_TxPbufs[i].tx_pbuf=q;
                pbuf_status=ERR_OK;
            }
            sys_arch_unprotect(0);
            if(ERR_OK == pbuf_status)
            {
                break;
            }
        }
    }

    /* If this was a chained pbuf, check allocation */
    if ((TCPIP_CHAINED_PBUF == pbuf_chain_type) && (q == p))
    {
        /* Memory allocation failed */
        pbuf_status = ERR_MEM;
    }
    else
    {
#if defined(D_CACHE_ENABLE) && (NETIF_CUSTOM_CACHE_MANAGEMENT == STD_ON) && defined(CPU_CORTEX_M7)
        DataCacheCleanbyAddr((uint32)bd.Data, bd.Length);
#endif /* defined(D_CACHE_ENABLE) && (NETIF_CUSTOM_CACHE_MANAGEMENT == STD_ON) && defined(CPU_CORTEX_M7) */
        u16payloadLength = (tcpip_TxPbufs[i].tx_pbuf)->tot_len - TCPIP_FRAME_HEADER_LENGTH;

        /* For pointer arithmetic */
        pQhelper = (tcpip_TxPbufs[i].tx_pbuf)->payload;
        /* Get Frame Type from buffer Header */
        frameType = ((uint16)pQhelper[12] << 8) | (uint16)pQhelper[13];
        do
        {
            bufferStatus = EthIf_ProvideTxBuffer(netif_cfg[netif->num]->num, frameType, 0U, &tcpip_TxPbufs[i].buf_idx, &pPayloadBuffer, &u16payloadLength);
        }
        while (BUFREQ_E_BUSY == bufferStatus);

        /* Copy payload data inside the buffer provided by the Eth Driver */
        memcpy(pPayloadBuffer, &pQhelper[TCPIP_FRAME_PAYLOAD_OFFSET], (tcpip_TxPbufs[i].tx_pbuf)->tot_len - TCPIP_FRAME_HEADER_LENGTH);

        pMacDestAddr = pQhelper;

        /* Keep trying to send the frame as long as the driver says there is not enough space in the queue */
        do
        {
            status = EthIf_Transmit(netif_cfg[netif->num]->num, tcpip_TxPbufs[i].buf_idx, frameType, TRUE, (tcpip_TxPbufs[i].tx_pbuf)->tot_len - TCPIP_FRAME_HEADER_LENGTH, pMacDestAddr);
        }
        while (BUFREQ_E_BUSY == status);

        if (E_OK != status)
        {
            /* Decrement the ref (either p's ref in case it was a single pbuf, or the coalesed q's ref) */
            (void)pbuf_free(q);
            tcpip_TxPbufs[i].tx_pbuf=NULL;
        }
    }

    return pbuf_status;
}

#if (NO_SYS == 0)
/**
 * This function is called when a packet is ready to be read from the interface.
 *
 * @param netif - the lwip network interface structure for this ethernetif
 * @param data - the pointer to the received data buffer
 * @param size - the length of received data buffer
 * @return ERR_OK if the packet is being handled (we take ownership of the data buffer)
 *         ERR_MEM if the packet cannot be handled (we don't take ownership of the data buffer,
 *         therefore the caller should release it)
 * Implements ethif_input_Activity
 */
static err_t Tcpip_Input(struct netif *netif, uint8_t * data, uint16_t size)
{
    err_t ret = ERR_MEM;

    /* Allocate a custom PBUF_REF pointing to the receive buffer */
    struct pbuf_custom* ethif_pbuf  = (struct pbuf_custom*)LWIP_MEMPOOL_ALLOC(RX_POOL);

    if (NULL != ethif_pbuf)
    {
        ethif_pbuf->custom_free_function = Tcpip_PbufFreeCustom;
        struct pbuf* p = pbuf_alloced_custom(PBUF_RAW, size, PBUF_REF, ethif_pbuf, data, size);

        p->if_idx = netif_get_index(netif);
        /* Saving receive buffer for further calling on provide Rx buffer */
        p->rx_buf = data;
        ret  = netif->input(p, netif);

        if (ERR_OK != ret)
        {
            LWIP_DEBUGF(NETIF_DEBUG, ("Tcpip_Input: IP input error\n"));
            (void)pbuf_free(p);
        }
    }

    return ret;
}

#else /* (NO_SYS == 1) */
/**
 * This function is called when a packet is ready to be read from the interface.
 *
 * @param netif - the lwip network interface structure for this ethernetif
 * @param data - the pointer to the received data buffer
 * @param size - the length of received data buffer
 * @return ERR_OK if the packet is being handled (we take ownership of the data buffer)
 *         ERR_MEM if the packet cannot be handled (we don't take ownership of the data buffer,
 *         therefore the caller should release it)
 * Implements ethif_input_Activity
 */
static err_t Tcpip_Input(struct netif *netif, uint8_t * data, uint16_t size)
{
    err_t ret = ERR_MEM;

    /* Allocate a PBUF_REF pointing to the receive buffer */
    struct pbuf* p  = pbuf_alloc(PBUF_RAW, size, PBUF_REF);
    if (NULL != p)
    {
        ret = ERR_OK;
        p->payload = data;
        if (ERR_OK != netif->input(p, netif))
        {
            LWIP_DEBUGF(NETIF_DEBUG, ("Tcpip_Input: IP input error\n"));
            (void)pbuf_free(p);
        }
    }

    return ret;
}

#endif /* (NO_SYS == 0) */

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif - the already initialized lwip network interface structure
 *        for this ethernetif
 * Implements ethif_low_level_init_Activity
 */
static void Tcpip_LowLevelInit(struct netif *netif)
{
    uint8_t i = 0U;
    /* set MAC hardware address length */
    netif->hwaddr_len = NETIF_MAX_HWADDR_LEN;

    /* set MAC hardware address */
    for (i = 0U; i < NETIF_MAX_HWADDR_LEN; i++)
    {
        netif->hwaddr[i] = netif_cfg[netif->num]->hwaddr[i];
    }

    /* maximum transfer unit */
    netif->mtu = 1500;

    /* device capabilities */
    /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
    netif->flags = (u8_t)(NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET);
#if (LWIP_IGMP == 1)
    netif->flags = netif->flags | (u8_t)NETIF_FLAG_IGMP;
    /* Will add the function igmp_mac_filter to the netif */
    (netif)->igmp_mac_filter = Tcpip_IgmpEthFilter;
#endif /* (LWIP_IGMP == 1) */

    NETIF_SET_CHECKSUM_CTRL(netif, NETIF_CHECKSUM_SETTING);

    tcpip_gNetif[netif->num] = netif;
    tcpip_DataIndex = 0;
    EthIf_SetControllerMode(netif_cfg[netif->num]->num, ETH_MODE_ACTIVE); /* to add declaration in EthIf.c */
    EthIf_UpdatePhysAddrFilter(netif_cfg[netif->num]->num, &netif_cfg[netif->num]->hwaddr[0], ETH_ADD_TO_FILTER);

#if (LWIP_IPV6 == 1) && (LWIP_IPV6_MLD == 1)
    if (netif_cfg[netif->num]->has_IPv6)
    {
        netif->flags = netif->flags | (u8_t)NETIF_FLAG_MLD6;
        /*If flag MLD6 flag is set, add mdl_ETH_filter  function to netif*/
        (netif)->mld_mac_filter = Tcpip_MldEthFilter;
        /*
        * For hardware/netifs that implement MAC filtering.
        * All-nodes link-local is handled by default, so we must let the hardware know
        * to allow multicast packets in.
        * Should set mld_mac_filter previously. */
         ip6_addr_t ip6_allnodes_ll;
         ip6_addr_set_allnodes_linklocal(&ip6_allnodes_ll);
         (void)netif->mld_mac_filter(netif, &ip6_allnodes_ll, NETIF_ADD_MAC_FILTER);
    }
#endif /* (LWIP_IPV6 == 1) && (LWIP_IPV6_MLD == 1) */
    netif_set_link_up(netif);
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif - the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 * Implements ethif_ethernetif_init_Activity
 */
err_t Tcpip_EthernetifInit(struct netif *netif)
{
    err_t ret = ERR_OK;
    uint8_t i;
    LWIP_ASSERT("netif != NULL", (netif != NULL));

#if (NO_SYS == 0)
    LWIP_MEMPOOL_INIT(RX_POOL);
    err_t status = sys_mutex_new(&tcpip_TxLock);
    ret = sys_mbox_new((sys_mbox_t *)&tcpip_InFlightTxPbufs, NETIF_TXBD_NUM);

    LWIP_ASSERT("ret == ERR_OK", ret == ERR_OK);
    LWIP_ASSERT("status == E_OK", status == ERR_OK);

#endif /* (NO_SYS == 0) */
    for (i=0;i<NETIF_TXBD_NUM;i++)
    {
        tcpip_TxPbufs[i].tx_pbuf = NULL;
        tcpip_TxPbufs[i].buf_idx = 255;
    }

    netif->name[0] = netif_cfg[netif->num]->name[0];
    netif->name[1] = netif_cfg[netif->num]->name[1];

#if (LWIP_IPV4 == 1)
#if (LWIP_ARP == 1)
    /* We directly use etharp_output() here to save a function call.
     * You can instead declare your own function an call etharp_output()
     * from it if you have to do some checks before sending (e.g. if link
     * is available...) */
    netif->output = etharp_output;
#else /* (LWIP_ARP == 0) */
    netif->output = NULL; /* not used for PPPoE */
#endif /* (LWIP_ARP == 1) */
#endif /* (LWIP_IPV4 == 1) */
#if (LWIP_IPV6 == 1)
    if (netif_cfg[netif->num]->has_IPv6)
    {
        netif->output_ip6 = ethip6_output;
    }
#endif /* (LWIP_IPV6 == 1) */
    netif->linkoutput = Tcpip_LowLevelOutput;
#if (LWIP_NETIF_HOSTNAME == 1)
    /* Initialize interface hostname */
    if (NULL != netif_cfg[netif->num]->hostname)
    {
        netif->hostname = netif_cfg[netif->num]->hostname;
    }
#endif /* (LWIP_NETIF_HOSTNAME == 1) */
#if (LWIP_SNMP == 1)
    /*
    * Initialize the snmp variables and counters inside the struct netif.
    * The last argument should be replaced with your link speed, in units
    * of bits per second.
    */
    NETIF_INIT_SNMP(netif, (u8_t)snmp_ifType_ethernet_csmacd, (u32_t)100000000);
#endif /* (LWIP_SNMP == 1) */

    /* initialize the hardware */
    Tcpip_LowLevelInit(netif);

    return ret;
}

/**
 * Clean up network interface and internal structures
 *
 * @param netif - the lwip network interface structure for this ethernetif
 * Implements ethif_ethernetif_shutdown_Activity
 */
void Tcpip_EthernetifShutdown(struct netif *netif)
{
#if (NO_SYS == 0)
    struct pbuf *p;
    LWIP_ASSERT("netif != NULL", (netif != NULL));

    /* Empty and free the mboxes */
    while (0 == sys_arch_mbox_tryfetch((sys_mbox_t *)&tcpip_InFlightTxPbufs, (void**)&p))
    {
        (void)pbuf_free_callback(p);
    }
    sys_mbox_free((sys_mbox_t *)&tcpip_InFlightTxPbufs);

    EthIf_SetControllerMode(netif_cfg[netif->num]->num, ETH_MODE_DOWN);

    (void)sys_mutex_free(&tcpip_TxLock);
#else /* (NO_SYS == 1) */
    EthIf_SetControllerMode(netif_cfg[netif->num]->num, ETH_MODE_DOWN);
#endif /* (NO_SYS == 0) */
}

#if (LWIP_IPV6 == 1)
/**
 * @ingroup netif_ip6
 * Modify/Configure eth driver setting  to forward (or stop forwarding) multicast packet for MLD (ICMPv6)
 * if "action" = NETIF_ADD_MAC_FILTER , eth module will forward multicast packet of the group corresponding to "group"
 * if "action" = NETIF_DEL_MAC_FILTER , eth module will stop forwarding multicast packet of the group corresponding to "group"
 *
 * @param netif the network interface
 * @*group IP address of the Multicast group the message have to be forwarded by the eth module
 * @action action to be done (remove group from the forwarded packet or add group)
 * Implements design_id_IPv6_Activity
 */
err_t Tcpip_MldEthFilter (struct netif *netif,
                       const ip6_addr_t *group,
                       enum netif_mac_filter_action action)
{
    /* Generate MAC address based on IPv6 address */
    uint8_t group_MAC[6];
    group_MAC[0] = 0x33;
    group_MAC[1] = 0x33;
    group_MAC[2] = (uint8_t)((IP6_ADDR_BLOCK7(group)) >> 8);
    group_MAC[3] = (uint8_t)(IP6_ADDR_BLOCK7(group));
    group_MAC[4] = (uint8_t)((IP6_ADDR_BLOCK8(group)) >> 8);
    group_MAC[5] = (uint8_t)(IP6_ADDR_BLOCK8(group));

    /* Configure Eth driver setting to forward or stop forwarding multicast packet for MLD (IPv6) */
    if (action != NETIF_DEL_MAC_FILTER)
    {
        /* Forward multicast packet of the group corresponding to group_MAC */

        EthIf_UpdatePhysAddrFilter(netif_cfg[netif->num]->num, group_MAC, ETH_ADD_TO_FILTER);

    }
    else
    {
        /* Stop forwarding multicast packet of the group corresponding to group_MAC */

        EthIf_UpdatePhysAddrFilter(netif_cfg[netif->num]->num, group_MAC, ETH_REMOVE_FROM_FILTER);

    }

    return ERR_OK;
}
#endif /* (LWIP_IPV6 == 1) */

#if (LWIP_IGMP == 1) && (LWIP_IPV4 == 1)
/**
 * @ingroup netif_ip4
 * Modify/Configure eth driver setting  to forward (or stop forwarding) multicast packet for IGMP (IPv4)
 * if "action" = NETIF_ADD_MAC_FILTER , eth module will forward multicast packet of the group corresponding to "group"
 * if "action" = NETIF_DEL_MAC_FILTER , eth module will stop forwarding multicast packet of the group corresponding to "group"
 *
 * @param netif the network interface
 * @*group IP address of the Multicast group the message have to be forwarded by the eth module
 * @action action to be done (remove group from the forwarded packet or add group)
 * Implements design_id_IGMP_Activity
 */

err_t Tcpip_IgmpEthFilter (struct netif *netif,
                        const ip4_addr_t *group,
                        enum netif_mac_filter_action action)
{
    /* Generate MAC address based on IPv4 address */
    uint8_t group_MAC[6];
    group_MAC[0] = 0x01;
    group_MAC[1] = 0x00;
    group_MAC[2] = 0x5e;
    group_MAC[3] = (0x7f & ip4_addr2(group));
    group_MAC[4] = ip4_addr3(group);
    group_MAC[5] = ip4_addr4(group);

    /* Configure ETH driver setting to forward or stop forwarding multicast packet for IGMP (IPv4) */
    if (action != NETIF_DEL_MAC_FILTER)
    {
        /* Forward multicast packet of the group corresponding to group_MAC */

        EthIf_UpdatePhysAddrFilter(netif_cfg[netif->num]->num, group_MAC, ETH_ADD_TO_FILTER);

    }
    else
    {
        /* Stop forwarding multicast packet of the group corresponding to group_MAC */

        EthIf_UpdatePhysAddrFilter(netif_cfg[netif->num]->num, group_MAC, ETH_REMOVE_FROM_FILTER);

    }

    return ERR_OK;
}
#endif /* (LWIP_IGMP == 1) && (LWIP_IPV4 == 1) */

void send_tx_pbuffs_dummy_char(void)
{
    struct pbuf* dummy  = pbuf_alloc(PBUF_RAW, TCPIP_RXBUFF_SIZE, PBUF_RAM);
    sys_mbox_post((sys_mbox_t *)&tcpip_InFlightTxPbufs, dummy);
}

void Tcpip_RegisterRxBuffProcessConditionHandler(Tcpip_RxBuffProcessCondHandler_t handler)
{
    tcpip_RxBuffProcessHandler = handler;
}

#if (TCPIP_BACKLOG_MANAGEMENT_SUPPORT == STD_ON)
void Tcpip_FreeCustomCallback(struct pbuf *p)
{
	struct pbuf *q = (struct pbuf *)p;

	if(NULL != tcpip_BacklogMngmtCallback)
	{
		tcpip_BacklogMngmtCallback(q);
	}
	pbuf_free(q);
}
#endif

/**
* @brief          This function handles the received Ethernet frame.
* @details        Function should parse the received frame and pass the gathered
*                 information to the appropriate upper layer module.
* @note           The passed data buffer is no longer valid after the function
*                 is exited.
* @warning        This is only an empty stub function provided only to be able
*                 to compile and link the Eth module.
* @param[in]      CtrlIdx Index of the controller which received the frame.
* @param[in]      FrameType The received frame Ethertype (from the frame header)
* @param[in]      IsBroadcast The value TRUE indicates that the received frame
*                 was sent to broadcast address (ff-ff-ff-ff-ff-ff)
* @param[in]      PhysAddrPtr Pointer to received frame source MAC address
*                 (6 bytes).
* @param[in]      DataPtr Data buffer containing the received Ethernet frame 
*                 payload.
* @param[in]      LenByte Length of the data in the buffer DataPtr.
*
*/
void Tcpip_RxIndication(\
                        uint8 CtrlIdx,\
                        Eth_FrameType FrameType, \
                        boolean IsBroadcast, \
                        const uint8* PhysAddrPtr, \
                        const uint8_t* DataPtr,\
						uint16 DataLen,\
						TimeTupleType* IngressTimeTuplePtr,\
						Eth_BufIdxType RxHandleId)
{
    (void)FrameType;
    (void)IsBroadcast;
    (void)PhysAddrPtr;
    (void)IngressTimeTuplePtr;

    DataPtr -= TCPIP_FRAME_PAYLOAD_OFFSET;
    DataLen += TCPIP_FRAME_HEADER_LENGTH;

#if defined(D_CACHE_ENABLE) && (NETIF_CUSTOM_CACHE_MANAGEMENT == STD_ON) && defined(CPU_CORTEX_M7)
    DataCacheInvbyAddr((uint32)bd.Data, bd.Length);
#endif /*  defined(D_CACHE_ENABLE) && (NETIF_CUSTOM_CACHE_MANAGEMENT == STD_ON) && defined(CPU_CORTEX_M7) */

    (void)Tcpip_Input((struct netif *)tcpip_gNetif[0],(uint8_t *)DataPtr,(uint16_t)DataLen);
}

/*================================================================================================*/
/**
* @brief          This function confirms that transmission of an Ethernet frame
*                 was finished.
* @details        Function should notify the appropriate upper layer module that
*                 the data transmission was successfully finished.
* @warning        This is only an empty stub function provided only to be able
*                 to compile and link the Eth module.
* @param[in]      CtrlIdx Index of the controller which transmitted the frame.
* @param[in]      BufIdx Index of the transmitted data buffer.

*/
void Tcpip_TxConfirmation(\
                          uint8 CtrlIdx, \
                          Eth_BufIdxType BufIdx, \
                          Std_ReturnType Result)
{
	(void)CtrlIdx;
	(void)BufIdx;
	(void)Result;

    uint8_t i;
    for( i =0; i < NETIF_TXBD_NUM; i++)
    {
        if(tcpip_TxPbufs[i].tx_pbuf != NULL)
        {
#if (NO_SYS == 1)
            (void)pbuf_free(tcpip_TxPbufs[i].tx_pbuf);
#else /* (NO_SYS == 0) */
#if (TCPIP_BACKLOG_MANAGEMENT_SUPPORT == STD_ON)
            (void)tcpip_try_callback(Tcpip_FreeCustomCallback, tcpip_TxPbufs[i].tx_pbuf);
#else
            /* request to free the outstanding pbuf on tcpip thread */
            (void)pbuf_free_callback(tcpip_TxPbufs[i].tx_pbuf);
#endif
#endif /* (NO_SYS == 1) */
            tcpip_TxPbufs[i].tx_pbuf = NULL;
        }
    }
}
