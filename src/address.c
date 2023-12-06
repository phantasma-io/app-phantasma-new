/*****************************************************************************
 *   Ledger App Phantasma.
 *   (c) 2023 Phantasma Team.
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
#include <stddef.h>   // size_t
#include <stdbool.h>  // bool
#include <string.h>   // memmove

#include "os.h"
#include "cx.h"

#include "address.h"

#include "common/base58.h"
#include "common/format.h"

#include "transaction/types.h"

bool address_from_pubkey(const uint8_t public_key[static 64], uint8_t *out, size_t out_len) {
    /*uint8_t address[32] = {0};
    cx_sha3_t keccak256;

    if (out_len < ADDRESS_LEN) {
        return false;
    }

    if (cx_keccak_init_no_throw(&keccak256, 256) != CX_OK) {
        return false;
    }

    if (cx_hash_no_throw((cx_hash_t *) &keccak256,
                         CX_LAST,
                         public_key + 1,
                         64,
                         address,
                         sizeof(address)) != CX_OK) {
        return false;
    }

    memmove(out, address + sizeof(address) - ADDRESS_LEN, ADDRESS_LEN);*/
    
    uint8_t address_hex[34] = {0};
    address_hex[0] = 0x01;
    address_hex[1] = 0x00;
    memmove(address_hex + 2, public_key, 32);

    char address[128] = {0};
    memset(address, 0, sizeof(address));
    address[0] = 'P';
    size_t encodedLength = base58_encode(address_hex, sizeof(address_hex), address + 1, sizeof(address) - 1);
    
    if (out_len < encodedLength) {
        return false;
    }

    memmove(out, address, encodedLength + 1);
    return true;
}
