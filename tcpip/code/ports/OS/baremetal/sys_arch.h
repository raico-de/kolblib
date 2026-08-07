/*
 * Copyright 2025 NXP
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
 */

#ifndef SYS_ARCH_H
#define SYS_ARCH_H

#ifdef USING_RTD
#include "OsIf.h"
#endif

sys_prot_t sys_arch_protect(void);
void sys_arch_unprotect(sys_prot_t pval);

#endif /* _ARCH_H_*/
