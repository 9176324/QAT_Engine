/* ====================================================================
 *
 *
 *   BSD LICENSE
 *
 *   Copyright(c) 2016-2017 Intel Corporation.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * ====================================================================
 */

/*****************************************************************************
 * @file e_qat.c
 *
 * This file provides an implementaion for fork in QAT engine
 *
 *****************************************************************************/

/* macros defined to allow use of the cpu get and set affinity functions */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#ifndef __USE_GNU
# define __USE_GNU
#endif

/* Standard Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>

/* Local Includes */
#include "qat_fork.h"
#include "qat_utils.h"
#include "e_qat_err.h"

/* OpenSSL Includes */
#include <openssl/err.h>

/* QAT includes */
#ifdef USE_QAT_CONTIG_MEM
# include "qae_mem_utils.h"
#endif
#ifdef USE_QAE_MEM
# include "cmn_mem_drv_inf.h"
#endif
#include "cpa.h"
#include "cpa_types.h"

void engine_init_child_at_fork_handler(void)
{
    /* Reinitialise the engine */
    ENGINE* e = ENGINE_by_id(engine_qat_id);
    if (e == NULL) {
        WARN("Engine pointer is NULL\n");
        QATerr(QAT_F_ENGINE_INIT_CHILD_AT_FORK_HANDLER, QAT_R_ENGINE_NULL);
        return;
    }

    if (qat_engine_init(e) != 1) {
        WARN("Failure in qat_engine_init function\n");
        QATerr(QAT_F_ENGINE_INIT_CHILD_AT_FORK_HANDLER, QAT_R_ENGINE_INIT_FAILURE);
    }
    ENGINE_free(e);
}

void engine_finish_before_fork_handler(void)
{
    /* Reset the engine preserving the value of global variables */
    ENGINE* e = ENGINE_by_id(engine_qat_id);
    if (e == NULL) {
        WARN("Engine pointer is NULL\n");
        QATerr(QAT_F_ENGINE_FINISH_BEFORE_FORK_HANDLER, QAT_R_ENGINE_NULL);
        return;
    }

    qat_engine_finish_int(e, QAT_RETAIN_GLOBALS);
    ENGINE_free(e);

    keep_polling = 1;
}

void qat_set_instance_for_thread(long instanceNum)
{
    int rc;

    if ((rc =
         pthread_setspecific(qatInstanceForThread,
                             qat_instance_handles[instanceNum %
                                                qat_num_instances])) != 0) {
        WARN("pthread_setspecific: %s\n", strerror(rc));
        QATerr(QAT_F_QAT_SET_INSTANCE_FOR_THREAD, QAT_R_PTHREAD_SETSPECIFIC_FAILURE);
        return;
    }
    enable_instance_for_thread = 1;
}
