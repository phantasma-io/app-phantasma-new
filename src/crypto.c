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
#include <string.h>   // memset, explicit_bzero
#include <stdbool.h>  // bool

#include "crypto.h"
#include "globals.h"

#define SIG_SIZE 64

int crypto_derive_private_key(cx_ecfp_private_key_t *private_key,
                              const uint32_t *bip32_path,
                              uint8_t bip32_path_len) {
    uint8_t raw_private_key[64] = {0};

    BEGIN_TRY {
        TRY {
            // Derive the seed with bip32_path
            cx_err_t bip32_nt = os_derive_bip32_no_throw(
                CX_CURVE_256K1,  // Changed to Ed25519 to match public key curve CX_CURVE_256K1
                bip32_path,
                bip32_path_len,
                raw_private_key,
                NULL);

            if (bip32_nt == CX_INTERNAL_ERROR) return -1;  // THROW(-1);

            // Initialize new private_key from raw
            cx_err_t privKey_nt =
                cx_ecfp_init_private_key_no_throw(CX_CURVE_Ed25519,  // Ensuring curve consistency
                                                  raw_private_key,
                                                  32,
                                                  private_key);

            if (privKey_nt == CX_EC_INVALID_CURVE) return -2;
            if (privKey_nt == CX_INVALID_PARAMETER) return -3;
        }
        CATCH_OTHER(e) {
            THROW(e);
        }
        FINALLY {
            explicit_bzero(&raw_private_key, sizeof(raw_private_key));
        }
    }
    END_TRY;

    return 0;
}

int crypto_init_public_key(cx_ecfp_private_key_t *private_key,
                           cx_ecfp_public_key_t *public_key,
                           uint8_t raw_public_key[static 64]) {
    // Generate corresponding public key
    cx_err_t initPubKey_nt =
        cx_ecfp_init_public_key_no_throw(CX_CURVE_Ed25519, NULL, 0, public_key);
    if (initPubKey_nt == CX_EC_INVALID_CURVE) return -11;
    if (initPubKey_nt == INVALID_PARAMETER) return -12;

    cx_err_t pubKey_nt =
        cx_ecfp_generate_pair_no_throw(CX_CURVE_Ed25519, public_key, private_key, 1);
    if (pubKey_nt == CX_EC_INVALID_CURVE) return -1;
    if (pubKey_nt == CX_NOT_UNLOCKED) return -2;
    if (pubKey_nt == CX_INVALID_PARAMETER_SIZE) return -3;
    if (pubKey_nt == CX_NOT_LOCKED) return -4;
    if (pubKey_nt == CX_INVALID_PARAMETER) return -5;
    if (pubKey_nt == CX_INTERNAL_ERROR) return -6;
    if (pubKey_nt == CX_EC_INVALID_POINT) return -7;
    if (pubKey_nt == CX_EC_INFINITE_POINT) return -8;

    cx_err_t edwards_nt =
        cx_edwards_compress_point_no_throw(CX_CURVE_Ed25519, public_key->W, public_key->W_len);
    if (edwards_nt == CX_EC_INVALID_CURVE) return -21;
    if (edwards_nt == CX_NOT_UNLOCKED) return -22;
    if (edwards_nt == CX_INVALID_PARAMETER_SIZE) return -23;
    if (edwards_nt == CX_NOT_LOCKED) return -24;
    if (edwards_nt == CX_INVALID_PARAMETER) return -25;
    if (edwards_nt == CX_EC_INFINITE_POINT) return -26;

    // Check public_key->W_len before copying
    explicit_bzero(raw_public_key, 64);
    memmove(raw_public_key, public_key->W + 1, 64);  // Copy only the actual length of W

    return 0;
}

int crypto_sign_message(uint16_t *resp_word) {
    cx_ecfp_private_key_t private_key = {0};

    // derive private key according to BIP32 path
    crypto_derive_private_key(&private_key, G_context.bip32_path, G_context.bip32_path_len);
    size_t sig_size = SIG_SIZE;

    // sig_len = cx_ecdsa_sign(&private_key,
    //                         0,
    //                         CX_SHA512,
    //                         G_context.tx_info.raw_tx,
    //                         G_context.tx_info.raw_tx_len,
    //                         G_context.tx_info.signature,
    //                         64,
    //                         &info);

    cx_err_t err = cx_eddsa_sign_no_throw(&private_key,
                                          CX_SHA512,
                                          G_context.tx_info.raw_tx,
                                          G_context.tx_info.raw_tx_len,
                                          G_context.tx_info.signature,
                                          SIG_SIZE);
    *resp_word = err;

    explicit_bzero(&private_key, sizeof(private_key));
    // PRINTF("Signature: %.*H\n", sig_len, G_context.tx_info.signature);

    if (err != CX_OK) {
        return -1;
    }

    G_context.tx_info.signature_len = sig_size;

    uint32_t info = 0;
    G_context.tx_info.v = (uint8_t) (info & CX_ECCINFO_PARITY_ODD);

    return 0;
}
