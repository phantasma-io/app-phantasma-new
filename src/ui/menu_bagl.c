/*****************************************************************************
 *   Ledger App Phantasma.
 *   (c) 2023 Ledger SAS.
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

#include "os.h"
#include "ux.h"
#include "glyphs.h"

#include "../globals.h"
#include "../address.h"
#include "../handler/get_public_key.h"
#include "menu.h"

static char g_address[ADDRESS_LEN];

UX_STEP_NOCB(ux_menu_ready_step, pnn, {&C_phantasma_logo, "Phantasma", "is ready"});
UX_STEP_NOCB(ux_menu_version_step, bn, {"Version", APPVERSION});
UX_STEP_CB(ux_menu_about_step, pb, ui_menu_about(), {&C_icon_certificate, "About"});
UX_STEP_CB(ux_display_public_step, pb, ui_menu_pubkey(), {&C_icon_certificate, "Display Address"});
UX_STEP_VALID(ux_menu_exit_step, pb, os_sched_exit(-1), {&C_icon_dashboard_x, "Quit"});

// FLOW for the main menu:
// #1 screen: ready
// #2 screen: version of the app
// #3 screen: about submenu
// #4 screen: quit
UX_FLOW(ux_menu_main_flow,
        &ux_menu_ready_step,
        &ux_menu_version_step,
        &ux_menu_about_step,
        &ux_display_public_step,
        &ux_menu_exit_step,
        FLOW_LOOP);

void ui_menu_main() {
    if (G_ux.stack_count == 0) {
        ux_stack_push();
    }

    ux_flow_init(0, ux_menu_main_flow, NULL);
}

UX_STEP_NOCB(ux_menu_info_step, bn, {"Phantasma App", "(c) 2023 Phantasma Team"});
UX_STEP_CB(ux_menu_back_step, pb, ui_menu_main(), {&C_icon_back, "Back"});

// FLOW for the about submenu:
// #1 screen: app info
// #2 screen: back button to main menu
UX_FLOW(ux_menu_about_flow, &ux_menu_info_step, &ux_menu_back_step, FLOW_LOOP);

void ui_menu_about() {
    ux_flow_init(0, ux_menu_about_flow, NULL);
}

UX_STEP_NOCB(ux_menu_publickey_step,
             bnnn_paging,
             {
                 .title = "Address",
                 .text = g_address,
             });

UX_FLOW(ux_menu_publickey_flow, &ux_menu_publickey_step, &ux_menu_back_step, FLOW_LOOP);

void ui_menu_pubkey() {
    explicit_bzero(&G_context, sizeof(G_context));
    G_context.req_type = CONFIRM_ADDRESS;
    G_context.state = STATE_NONE;

    handler_get_public_key_menu();

    memset(g_address, 0, sizeof(g_address));
    uint8_t address[ADDRESS_LEN] = {0};
    if (!address_from_pubkey(G_context.pk_info.raw_public_key, address, sizeof(address))) {
        // Handle Errors here.
    }

    memmove(g_address, address, ADDRESS_LEN);

    ux_flow_init(0, ux_menu_publickey_flow, NULL);
}

#endif
