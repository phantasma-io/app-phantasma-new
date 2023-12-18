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

#ifdef HAVE_NBGL

#include <stdbool.h>  // bool
#include <string.h>   // memset

#include "os.h"
#include "glyphs.h"
#include "os_io_seproxyhal.h"
#include "nbgl_use_case.h"
#include "io.h"
#include "bip32.h"
#include "format.h"

#include "display.h"
#include "constants.h"
#include "../globals.h"
#include "../sw.h"
#include "../address.h"
#include "action/validate.h"
#include "../transaction/types.h"
#include "../menu.h"

// Buffer where the transaction amount string is written
static char g_amount[30];
// Buffer where the transaction address string is written
// static char g_address[43];
static char g_address[64];
static char g_token[TOKEN_LEN];
static char g_contract[TOKEN_LEN];
static char g_contract_method[CONTRACT_METHOD_LEN];
static char g_contract_method_args[240];
static char g_nexus[NEXUS_LEN];
static char g_chain[CHAIN_LEN];
static char g_txlength[TXLENGTH_LEN];
static char g_scriptlength[SCRIPTLENGTH_LEN];

static nbgl_layoutTagValue_t pairs[7];
static nbgl_layoutTagValueList_t pairList;
static nbgl_pageInfoLongPress_t infoLongPress;

static void confirm_transaction_rejection(void) {
    // display a status page and go back to main
    validate_transaction(false);
    nbgl_useCaseStatus("Transaction rejected", false, ui_menu_main);
}

static void ask_transaction_rejection_confirmation(void) {
    // display a choice to confirm/cancel rejection
    nbgl_useCaseConfirm("Reject transaction?",
                        NULL,
                        "Yes, Reject",
                        "Go back to transaction",
                        confirm_transaction_rejection);
}

// called when long press button on 3rd page is long-touched or when reject footer is touched
static void review_choice(bool confirm) {
    if (confirm) {
        // display a status page and go back to main
        validate_transaction(true);
        nbgl_useCaseStatus("TRANSACTION\nSIGNED", true, ui_menu_main);
    } else {
        ask_transaction_rejection_confirmation();
    }
}

static void review_continue(void) {
    // Setup data to display
    pairs[0].item = "Token";
    pairs[0].value = g_token;
    pairs[1].item = "Amount";
    pairs[1].value = g_amount;
    pairs[2].item = "Address";
    pairs[2].value = g_address;
    pairs[3].item = "Transaction length";
    pairs[3].value = g_txlength;
    pairs[4].item = "Nexus";
    pairs[4].value = g_nexus;
    pairs[5].item = "Chain";
    pairs[5].value = g_chain;
    pairs[6].item = "Script length";
    pairs[6].value = g_scriptlength;

    // Setup list
    pairList.nbMaxLinesForValue = 0;
    pairList.nbPairs = 7;
    pairList.pairs = pairs;

    // Info long press
    infoLongPress.icon = &C_app_phantasma_64px;
    infoLongPress.text = "Sign transaction\nto send Phantasma";
    infoLongPress.longPressText = "Hold to sign";

    nbgl_useCaseStaticReview(&pairList, &infoLongPress, "Reject transaction", review_choice);
}

// Public function to start the transaction review
// - Check if the app is in the right state for transaction review
// - Format the amount and address strings in g_amount and g_address buffers
// - Display the first screen of the transaction review
int ui_display_transaction() {
    if (G_context.req_type != CONFIRM_TRANSACTION || G_context.state != STATE_PARSED) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    memset(g_txlength, 0, sizeof(g_txlength));
    format_u64(g_txlength, sizeof(g_txlength), G_context.tx_info.raw_tx_len);

    memset(g_nexus, 0, sizeof(g_nexus));
    memmove(g_nexus, G_context.tx_info.transaction.nexus, G_context.tx_info.transaction.nexus_len);

    memset(g_chain, 0, sizeof(g_chain));
    memmove(g_chain, G_context.tx_info.transaction.chain, G_context.tx_info.transaction.chain_len);

    memset(g_scriptlength, 0, sizeof(g_scriptlength));
    format_u64(g_scriptlength, sizeof(g_scriptlength), G_context.tx_info.transaction.script_len);

    memset(g_token, 0, sizeof(g_token));
    memmove(g_token, G_context.tx_info.transaction.token, G_context.tx_info.transaction.token_len);

    // Format amount and address to g_amount and g_address buffers
    memset(g_amount, 0, sizeof(g_amount));
    memmove(g_amount, G_context.tx_info.transaction.value, G_context.tx_info.transaction.value_len);

    memset(g_address, 0, sizeof(g_address));
    memmove(g_address, G_context.tx_info.transaction.to, G_context.tx_info.transaction.to_len);

    // Start review
    nbgl_useCaseReviewStart(&C_app_phantasma_64px,
                            "Review transaction\nto send Phantasma",
                            NULL,
                            "Reject transaction",
                            review_continue,
                            ask_transaction_rejection_confirmation);
    return 0;
}

static void review_continue_custom(void) {
    // Setup data to display
    pairs[0].item = "Nexus";
    pairs[0].value = g_nexus;
    pairs[1].item = "Chain";
    pairs[1].value = g_chain;
    pairs[2].item = "Contract";
    pairs[2].value = g_contract;
    pairs[3].item = "Contract Method";
    pairs[3].value = g_contract_method;
    pairs[4].item = "Address";
    pairs[4].value = g_address;
    pairs[5].item = "Contract Method Args";
    pairs[5].value = g_contract_method_args;

    // Setup list
    pairList.nbMaxLinesForValue = 1;
    pairList.nbPairs = 6;
    pairList.pairs = pairs;

    // Info long press
    infoLongPress.icon = &C_app_phantasma_64px;
    infoLongPress.text = "Sign transaction\nto send Phantasma";
    infoLongPress.longPressText = "Hold to sign";

    nbgl_useCaseStaticReview(&pairList, &infoLongPress, "Reject transaction", review_choice);
}

int ui_display_custom_transaction() {
    if (G_context.req_type != CONFIRM_TRANSACTION || G_context.state != STATE_PARSED) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    memset(g_nexus, 0, sizeof(g_nexus));
    memmove(g_nexus, G_context.tx_info.transaction.nexus, G_context.tx_info.transaction.nexus_len);

    memset(g_chain, 0, sizeof(g_chain));
    memmove(g_chain, G_context.tx_info.transaction.chain, G_context.tx_info.transaction.chain_len);

    memset(g_contract, 0, sizeof(g_contract));
    memmove(g_contract,
            (uint8_t *) G_context.tx_info.transaction.name,
            G_context.tx_info.transaction.name_len);

    memset(g_contract_method, 0, sizeof(g_contract_method));
    memmove(g_contract_method,
            (uint8_t *) G_context.tx_info.transaction.method,
            G_context.tx_info.transaction.method_len);

    memset(g_address, 0, sizeof(g_address));
    memmove(g_address, G_context.tx_info.transaction.from, G_context.tx_info.transaction.from_len);

    memset(g_contract_method_args, 0, sizeof(g_contract_method_args));
    memmove(g_contract_method_args,
            (uint8_t *) G_context.tx_info.transaction.output_args,
            G_context.tx_info.transaction.output_args_len);

    // Start review
    nbgl_useCaseReviewStart(&C_app_phantasma_64px,
                            "Review transaction\nto send Phantasma",
                            NULL,
                            "Reject transaction",
                            review_continue_custom,
                            ask_transaction_rejection_confirmation);
    return 0;
}

#endif
