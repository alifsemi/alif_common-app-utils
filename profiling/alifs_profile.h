/* Copyright (c) 2022 ALIF SEMICONDUCTOR

   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   - Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   - Neither the name of ALIF SEMICONDUCTOR nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.
   *
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
   ---------------------------------------------------------------------------*/

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

