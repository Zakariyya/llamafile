// -*- mode:c++;indent-tabs-mode:nil;c-basic-offset:4;coding:utf-8 -*-
// vi: set et ft=c++ ts=4 sts=4 sw=4 fenc=utf-8 :vi
//
// Copyright 2024 Mozilla Foundation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "sgemm.h"
#include "llamafile.h"
#include <cosmo.h>

bool llamafile_sgemm(int m, int n, int k, const void *A, int lda, const void *B, int ldb, void *C,
                     int ldc, int ith, int nth, int task, int Atype, int Btype, int Ctype) {

    if (!X86_HAVE(AVX))
        return false;
    if (Ctype != GGML_TYPE_F32)
        return false;

    switch (Atype) {

    case GGML_TYPE_F32:
        if (Btype != GGML_TYPE_F32)
            return false;
        if (X86_HAVE(AVX512F) && !(k % 16))
            return llamafile_sgemm_sss_avx512f(m, n, k, (const float *)A, lda, (const float *)B,
                                               ldb, (float *)C, ldc, ith, nth, task);
        if (X86_HAVE(FMA) && !(k % 8))
            return llamafile_sgemm_sss_fma(m, n, k, (const float *)A, lda, (const float *)B, ldb,
                                           (float *)C, ldc, ith, nth, task);
        if (!(k % 8))
            return llamafile_sgemm_sss_avx(m, n, k, (const float *)A, lda, (const float *)B, ldb,
                                           (float *)C, ldc, ith, nth, task);
        return false;

    case GGML_TYPE_F16:
        if (Btype != GGML_TYPE_F32)
            return false;
        if (X86_HAVE(AVX512F) && !(k % 16))
            return llamafile_sgemm_hss_avx512f(m, n, k, (const unsigned short *)A, lda,
                                               (const float *)B, ldb, (float *)C, ldc, ith, nth,
                                               task);
        if (X86_HAVE(FMA) && X86_HAVE(F16C) && !(k % 8))
            return llamafile_sgemm_hss_f16c(m, n, k, (const unsigned short *)A, lda,
                                            (const float *)B, ldb, (float *)C, ldc, ith, nth, task);
        return false;

    case GGML_TYPE_Q8_0:
        if (Btype != GGML_TYPE_Q8_0)
            return false;
        if (X86_HAVE(AVX512VL) && X86_HAVE(AVX512_VNNI) && !(k % 32))
            return llamafile_sgemm_q0q0s_avx512vnni(m, n, k, (const block_q8_0 *)A, lda,
                                                    (const block_q8_0 *)B, ldb, (float *)C, ldc,
                                                    ith, nth, task);
        if (X86_HAVE(FMA) && X86_HAVE(AVXVNNI) && !(k % 32))
            return llamafile_sgemm_q0q0s_avxvnni(m, n, k, (const block_q8_0 *)A, lda,
                                                 (const block_q8_0 *)B, ldb, (float *)C, ldc, ith,
                                                 nth, task);
        if (X86_HAVE(FMA) && X86_HAVE(AVX2) && !(k % 32))
            return llamafile_sgemm_q0q0s_fma(m, n, k, (const block_q8_0 *)A, lda,
                                             (const block_q8_0 *)B, ldb, (float *)C, ldc, ith, nth,
                                             task);
        return false;

    case GGML_TYPE_Q4_0:
        if (Btype != GGML_TYPE_Q8_0)
            return false;
        if (X86_HAVE(AVX512VL) && X86_HAVE(AVX512_VNNI) && !(k % 32))
            return llamafile_sgemm_e0q0s_avx512vnni(m, n, k, (const block_q4_0 *)A, lda,
                                                    (const block_q8_0 *)B, ldb, (float *)C, ldc,
                                                    ith, nth, task);
        if (X86_HAVE(FMA) && X86_HAVE(AVXVNNI) && !(k % 32))
            return llamafile_sgemm_e0q0s_avxvnni(m, n, k, (const block_q4_0 *)A, lda,
                                                 (const block_q8_0 *)B, ldb, (float *)C, ldc, ith,
                                                 nth, task);
        if (X86_HAVE(FMA) && X86_HAVE(AVX2) && !(k % 32))
            return llamafile_sgemm_e0q0s_fma(m, n, k, (const block_q4_0 *)A, lda,
                                             (const block_q8_0 *)B, ldb, (float *)C, ldc, ith, nth,
                                             task);
        return false;

    case GGML_TYPE_Q4_1:
        if (Btype != GGML_TYPE_Q8_1)
            return false;
        if (X86_HAVE(AVX512VL) && X86_HAVE(AVX512_VNNI) && !(k % 32))
            return llamafile_sgemm_e1q1s_avx512vnni(m, n, k, (const block_q4_1 *)A, lda,
                                                    (const block_q8_1 *)B, ldb, (float *)C, ldc,
                                                    ith, nth, task);
        if (X86_HAVE(FMA) && X86_HAVE(AVXVNNI) && !(k % 32))
            return llamafile_sgemm_e1q1s_avxvnni(m, n, k, (const block_q4_1 *)A, lda,
                                                 (const block_q8_1 *)B, ldb, (float *)C, ldc, ith,
                                                 nth, task);
        if (X86_HAVE(FMA) && X86_HAVE(AVX2) && !(k % 32))
            return llamafile_sgemm_e1q1s_fma(m, n, k, (const block_q4_1 *)A, lda,
                                             (const block_q8_1 *)B, ldb, (float *)C, ldc, ith, nth,
                                             task);
        return false;

    default:
        return false;
    }
}
