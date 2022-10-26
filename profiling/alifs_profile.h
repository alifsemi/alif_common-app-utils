/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/*
 * Profile system to count CYCCNT (Cycle count) from Data Watchpoint and Trace register.
 * Also included is convenience function to translate the cycle count into nanoseconds.
 *
 * As CYCCNT register is uint32_t in length, this system only supports profiling of
 * 0xFFFF FFFF (4,294,967,295) cycles which translates to approximately 10 seconds on HP core
 * and 26 seconds on HE.
 */

#ifndef ALIFS_PROFILE_H_
#define ALIFS_PROFILE_H_

#include <inttypes.h>
#include "RTE_Components.h"
#include CMSIS_device_header

/*
 * Start Cycle counter (if it's not started yet) and return the initial cycle count.
 * @return the initial cycle count from tracing register.
 */
__STATIC_FORCEINLINE uint32_t alifs_profile_start()
{
    DCB->DEMCR |= DCB_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    return DWT->CYCCNT;
}

/*
 * Return the elapsed cycles.
 *
 * @param counter_start_value The value returned by alifs_profile_start -function.
 * @return the elapsed cycles.
 */
__STATIC_FORCEINLINE uint32_t alifs_profile_end(const uint32_t counter_start_value)
{
    return (DWT->CYCCNT - counter_start_value);
}

/*
 * Return the approximate number of nanoseconds the given cycle count corresponds to.
 * (calculation is done with integer arithmetic which always rounds towards floor)
 *
 * @param counter_value The number of cycles used.
 * @return The amount of time in nanoseconds the given cycle count corresponds to with the running core.
 */
__STATIC_INLINE uint32_t alifs_profile_cycles_to_ns(const uint32_t counter_value)
{
    uint64_t temp = (uint64_t)counter_value * 1000000000;
    return (uint32_t)(temp / GetSystemCoreClock());
}

#endif // #ifndef ALIFS_PROFILE_H_
