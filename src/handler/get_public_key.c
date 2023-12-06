/*****************************************************************************
 *   Ledger App Phantasma.
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
#include <string.h>   // memset, explicit_bzero

#include "os.h"
#include "cx.h"
#include "io.h"
#include "buffer.h"
#include "crypto_helpers.h"

#include "get_public_key.h"
#include "../globals.h"
#include "../types.h"
#include "../sw.h"
#include "../ui/display.h"
#include "../helper/send_response.h"

int handler_get_public_key(buffer_t *cdata, bool display) {
    explicit_bzero(&G_context, sizeof(G_context));
    G_context.req_type = CONFIRM_ADDRESS;
    G_context.state = STATE_NONE;
// Read BIP32 path from incoming data and handle errors
    if (!buffer_read_u8(cdata, &G_context.bip32_path_len) ||
        !buffer_read_bip32_path(cdata, G_context.bip32_path, (size_t) G_context.bip32_path_len)) {
        return io_send_sw(SW_WRONG_DATA_LENGTH);
    }

    cx_err_t error = bip32_derive_get_pubkey_256(CX_CURVE_256K1,
                                                 G_context.bip32_path,
                                                 G_context.bip32_path_len,
                                                 G_context.pk_info.raw_public_key,
                                                 G_context.pk_info.chain_code,
                                                 CX_SHA512);

    if (error != CX_OK) {
        return io_send_sw(error);
    }

    if (display) {
        return ui_display_address();
    }

    return helper_send_response_pubkey();
}


int handler_get_public_key_menu() {
    // PATH = "44'/60'/0'/0/0"
    // Size = 21 // 0x15 
    // first byte is the length of the path (0x05)
    const uint8_t myCMD[] = {
        0x05, 0x80, 0x00, 0x00, 0x2C, 0x80, 0x00, 0x00, 0x3C,
        0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };

    // Initialize the buffer
    buffer_t cdata = {0};
    cdata.ptr = (const uint8_t*) myCMD; 
    cdata.size = 0x15;
    cdata.offset = 0;

    cx_ecfp_private_key_t private_key = {0};
    cx_ecfp_public_key_t public_key = {0};

    // Read BIP32 path from incoming data and handle errors
    if (!buffer_read_u8(&cdata, &G_context.bip32_path_len) || !buffer_read_bip32_path(&cdata, G_context.bip32_path, (size_t) G_context.bip32_path_len)) {
        return -3;
    }

    // Derive private key according to BIP32 path
    if (crypto_derive_private_key(&private_key, G_context.bip32_path, G_context.bip32_path_len) != 0) {
        explicit_bzero(&private_key, sizeof(private_key));
        return -2; // or appropriate error code
    }

    // Generate corresponding public key
    if (crypto_init_public_key(&private_key, &public_key, G_context.pk_info.raw_public_key) != 0) {
        explicit_bzero(&private_key, sizeof(private_key));
        return -1; // or appropriate error code
    }

    // Reset private key after use for security
    explicit_bzero(&private_key, sizeof(private_key));
    return 0;
}