/*****************************************************************************
 *   Ledger App Boilerplate.
 *   (c) 2020 Ledger SAS.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool
#include <stddef.h>   // size_t
#include <string.h>   // memmove

#include "write.h"
#include "varint.h"

#include "serialize.h"

int transaction_serialize(const transaction_t *tx, uint8_t *out, size_t out_len) {
    size_t offset = 0;
    size_t max_len = tx->nexus_len + tx->chain_len + tx->script_len + 3;

    if (max_len > out_len) {
        return -1;
    }

    offset += varint_write(out, offset, tx->nexus_len);
    memmove(out + offset, tx->nexus, tx->nexus_len);
    offset += tx->nexus_len;

    offset += varint_write(out, offset, tx->chain_len);
    memmove(out + offset, tx->chain, tx->chain_len);
    offset += tx->chain_len;

    offset += varint_write(out, offset, tx->script_len);
    memmove(out + offset, tx->script, tx->script_len);
    offset += tx->script_len;

    write_u32_be(out, offset, tx->expiration);
    offset += 4;

    offset += varint_write(out, offset, tx->payload_len);
    memmove(out + offset, tx->payload, tx->payload_len);
    offset += tx->payload_len;

    return (int) offset;
}
