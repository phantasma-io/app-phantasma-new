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

#ifdef HAVE_BAGL

#include <stdbool.h>  // bool
#include <string.h>   // memset

#include "os.h"
#include "ux.h"
#include "glyphs.h"
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

static action_validate_cb g_validate_callback;
static char g_amount[30];
static char g_bip32_path[60];
static char g_address[240];
static char g_token[TOKEN_LEN];
static char g_contract[TOKEN_LEN];
static char g_contract_method[CONTRACT_METHOD_LEN];
//static char g_contract_method_args[64];
static char g_nexus[NEXUS_LEN];
static char g_chain[CHAIN_LEN];
static char g_txlength[TXLENGTH_LEN];
static char g_scriptlength[SCRIPTLENGTH_LEN];

// Validate/Invalidate public key and go back to home
static void ui_action_validate_pubkey(bool choice) {
    validate_pubkey(choice);
    ui_menu_main();
}

// Validate/Invalidate transaction and go back to home
static void ui_action_validate_transaction(bool choice) {
    validate_transaction(choice);
    ui_menu_main();
}

// Step with icon and text
UX_STEP_NOCB(ux_display_confirm_addr_step, pn, {&C_icon_eye, "Confirm Address"});
// Step with title/text for BIP32 path
UX_STEP_NOCB(ux_display_path_step,
             bnnn_paging,
             {
                 .title = "Path",
                 .text = g_bip32_path,
             });
// Step with title/text for address
UX_STEP_NOCB(ux_display_token_step,
             bnnn_paging,
             {
                 .title = "Token",
                 .text = g_token,
             });
// Step with title/text for address
UX_STEP_NOCB(ux_display_txlength_step,
             bnnn_paging,
             {
                 .title = "Tx Length",
                 .text = g_txlength,
             });
// Step with title/text for address
UX_STEP_NOCB(ux_display_nexus_step,
             bnnn_paging,
             {
                 .title = "Nexus",
                 .text = g_nexus,
             });
// Step with title/text for address
UX_STEP_NOCB(ux_display_chain_step,
             bnnn_paging,
             {
                 .title = "Chain",
                 .text = g_chain,
             });
// Step with title/text for address
UX_STEP_NOCB(ux_display_scriptlength_step,
             bnnn_paging,
             {
                 .title = "Script Length",
                 .text = g_scriptlength,
             });
// Step with title/text for address
UX_STEP_NOCB(ux_display_address_step,
             bnnn_paging,
             {
                 .title = "Address",
                 .text = g_address,
             });
// Step with approve button
UX_STEP_CB(ux_display_approve_step,
           pb,
           (*g_validate_callback)(true),
           {
               &C_icon_validate_14,
               "Approve",
           });
// Step with reject button
UX_STEP_CB(ux_display_reject_step,
           pb,
           (*g_validate_callback)(false),
           {
               &C_icon_crossmark,
               "Reject",
           });

// FLOW to display address:
// #1 screen: eye icon + "Confirm Address"
// #2 screen: display address
// #3 screen: approve button
// #4 screen: reject button
UX_FLOW(ux_display_pubkey_flow,
        &ux_display_confirm_addr_step,
        &ux_display_address_step,
        &ux_display_approve_step,
        &ux_display_reject_step);

int ui_display_address() {
    if (G_context.req_type != CONFIRM_ADDRESS || G_context.state != STATE_NONE) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }
    memset(g_address, 0, sizeof(g_address));
    uint8_t address[ADDRESS_LEN] = {0};
    if (!address_from_pubkey(G_context.pk_info.raw_public_key, address, sizeof(address))) {
        return io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
    }

    if (format_hex(address, sizeof(address), g_address, sizeof(g_address)) == -1) {
        return io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
    }

    g_validate_callback = &ui_action_validate_pubkey;

    ux_flow_init(0, ux_display_pubkey_flow, NULL);
    return 0;
}

// Step with icon and text
UX_STEP_NOCB(ux_display_review_step,
             pnn,
             {
                 &C_icon_eye,
                 "Review",
                 "Transaction",
             });
// Step with title/text for amount
UX_STEP_NOCB(ux_display_amount_step,
             bnnn_paging,
             {
                 .title = "Amount",
                 .text = g_amount,
             });

// FLOW to display transaction information:
// #1 screen : eye icon + "Review Transaction"
// #2 screen : display amount
// #3 screen : display destination address
// #4 screen : approve button
// #5 screen : reject button
UX_FLOW(ux_display_transaction_flow,
        &ux_display_review_step,
        &ux_display_txlength_step,
        &ux_display_nexus_step,
        &ux_display_chain_step,
        &ux_display_scriptlength_step,
        &ux_display_address_step,
        &ux_display_token_step,
        &ux_display_amount_step,
        &ux_display_approve_step,
        &ux_display_reject_step);

int ui_display_transaction() {
    if (G_context.req_type != CONFIRM_TRANSACTION || G_context.state != STATE_PARSED) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    memset(g_amount, 0, sizeof(g_amount));
    char amount[30] = {0};
    if (!format_fpu64(amount, sizeof(amount), *G_context.tx_info.transaction.value, EXPONENT_SMALLEST_UNIT)) {
        return io_send_sw(SW_DISPLAY_AMOUNT_FAIL);
    }
    snprintf(g_amount, sizeof(g_amount), "BOL %.*s", sizeof(amount), amount);
    PRINTF("Amount: %s\n", g_amount);

    memset(g_address, 0, sizeof(g_address));

    if (format_hex(G_context.tx_info.transaction.to, ADDRESS_LEN, g_address, sizeof(g_address)) ==
        -1) {
        return io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
    }

    g_validate_callback = &ui_action_validate_transaction;

    ux_flow_init(0, ux_display_transaction_flow, NULL);

    return 0;
}

// Step with title/text for Contract name
UX_STEP_NOCB(ux_display_contract_step,
             bnnn_paging,
             {
                 .title = "Contract",
                 .text = g_contract,
             });

// Step with title/text for Method name
UX_STEP_NOCB(ux_display_method_step,
             bnnn_paging,
             {
                 .title = "Method",
                 .text = g_contract_method,
             });

// Step with title/text for Method name
UX_STEP_NOCB(ux_display_method_args_step,
             bnnn_paging,
             {
                 .title = "Method Args",
                 .text = g_address,
             });

// FLOW to display transaction information:
// #1 screen : eye icon + "Review Transaction"
// #2 screen : display contract name
// #3 screen : display method name
// #4 screen : display arguments
// #4 screen : approve button
// #5 screen : reject button
UX_FLOW(ux_display_custom_transaction_flow,
        &ux_display_review_step,
        &ux_display_nexus_step,
        &ux_display_chain_step,
        &ux_display_contract_step,
        &ux_display_method_step,
        &ux_display_method_args_step,
        &ux_display_approve_step,
        &ux_display_reject_step);

int ui_display_custom_transaction() {
    if (G_context.req_type != CONFIRM_TRANSACTION || G_context.state != STATE_PARSED) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    //memset(g_txlength, 0, sizeof(g_txlength));
    //format_u64(g_txlength, sizeof(g_txlength), G_context.tx_info.transaction.allow_gas.args_len);

    memset(g_nexus, 0, sizeof(g_nexus));
    memmove(g_nexus, G_context.tx_info.transaction.nexus, G_context.tx_info.transaction.nexus_len);

    memset(g_chain, 0, sizeof(g_chain));
    memmove(g_chain, G_context.tx_info.transaction.chain, G_context.tx_info.transaction.chain_len);

    memset(g_contract, 0, sizeof(g_contract));
    memmove(g_contract, (uint8_t *)  G_context.tx_info.transaction.name, G_context.tx_info.transaction.name_len);

    memset(g_contract_method, 0, sizeof(g_contract_method));
    memmove(g_contract_method, (uint8_t *)  G_context.tx_info.transaction.method, G_context.tx_info.transaction.method_len);

    memset(g_address, 0, sizeof(g_address));
    memmove(g_address, (uint8_t *)  G_context.tx_info.transaction.output_args, G_context.tx_info.transaction.output_args_len);

    //memset(g_scriptlength, 0, sizeof(g_scriptlength));
    //format_u64(g_scriptlength, sizeof(g_scriptlength), G_context.tx_info.transaction.script_len);

    //memset(g_address, 0, sizeof(g_address));
    //memmove(g_address, G_context.tx_info.transaction.to, G_context.tx_info.transaction.to_len);

    // TODO: Needs to show args from the contract call.

    g_validate_callback = &ui_action_validate_transaction;

    ux_flow_init(0, ux_display_custom_transaction_flow, NULL);

    return 0;
}

#endif
