/*
 * Copyright 2017-2025 NXP
 * All rights reserved.
 *
 * THIS SOFTWARE IS PROVIDED BY NXP "AS IS" AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL NXP OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is derived from lwIP contribution example with the following copyright:
 *
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 */


/* lwIP includes. */

/**
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 1.3, Taking address of near auto variable.
 * The code is not dynamically linked. An absolute stack address is obtained
 * when taking the address of the near auto variable. A source of error in
 * writing dynamic code is that the stack segment may be different from the data
 * segment.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 2.1, A project shall not contain unreachable code.
 * These are safety checks to avoid dereferencing NULL pointers.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Directive 4.6, typedefs that indicate size and
 * signedness should be used in place of the basic numerical types.
 * These are function parameters whose size is platform dependent.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 4.7, Ignoring return value of function
 * The return value of the function can be ignored, as checks are done on parameters
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 8.3, All declarations or an object or function shall use the
 * same names and type qualifiers.
 * Function should take NULL as parameter. This is a fake finding.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 8.4, external symbol defined without a prior
 * declaration.
 * The symbol is declared in the hardware access file as external; it is needed
 * for accessing the installed callback, but is not a part of the public API.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.13, Pointer parameter 'dummyptr' could be declared as pointing to const
 * Memory pointed by 'dummyptr' is used for receive.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 10.1, Unpermitted operand to operator '||'
 * Variable is of essential boolean type
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 10.3, The value of an expression shall not be assigned to an
 * object with a narrower essential type or a different essential type category.
 * These are exit codes returned by functions when an error occurs.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.2, Conversions shall not be performed between a pointer
 * to an incomplete type and any other type
 * This cast is to required write 8bit or 16bit data into corresponding address.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.3, Cast performed between a pointer
 * to object type and a pointer to a different object type.
 * This cast is to required write 8bit or 16bit data into corresponding address.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 11.4, Conversion between a pointer and integer type.
 * The cast is required to initialize a pointer with an unsigned int define, representing a memory-mapped address.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 11.5, Conversion from pointer to void to pointer to other type .
 * The conversion is needed to wrap a thread function call.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 14.4, Conditional expression should have essentially Boolean type.
 * This is required for macro constructs in form do {...} while(0).
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 15.5, Return statement before end of function
 * The return statement before end of function is used for simpler code structure
 * and better readability.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 17.7, Ignoring return value of function
 * The return value of the function can be ignored, as checks are done on parameters
 *
 */

#include "string.h"

#ifdef USING_RTD
#include "OsIf.h"
#include "OsIf_rtd_port.h"
#else
#include "osif.h"
#endif /* USING_RTD */

#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/stats.h"
#include "lwip/debug.h"
#include "lwip/sys.h"
#include "lwip/api.h"

/* Initialize sys arch layer
*/
void sysarch_init(void)
{
#ifdef USING_RTD
    /* Initialize timer in baremetal environment before using OsIf_GetElapsed */
    OsIf_Init(NULL_PTR);
#else
    /* Initialize timer in baremetal environment before using OSIF_GetMilliseconds */
    OSIF_TimeDelay(0);
#endif /* USING_RTD */
}

/*
   Ticks/jiffies since power up
*/
u32_t sys_jiffies(void)
{
#ifdef USING_RTD
    return (u32_t)OsIf_GetMilliseconds();
#else
    return (u32_t)OSIF_GetMilliseconds();
#endif /* USING_RTD */
}

/*
  Returns the current time in milliseconds,
  may be the same as sys_jiffies or at least based on it.
*/
u32_t sys_now(void)
{
    return sys_jiffies();
}

sys_prot_t sys_arch_protect(void)
{
	OsIf_SuspendAllInterrupts();

	return 0;
}

void sys_arch_unprotect(sys_prot_t pval)
{
  (void) pval;
  OsIf_ResumeAllInterrupts();
}

