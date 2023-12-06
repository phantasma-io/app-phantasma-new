#pragma once

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t

#include "buffer.h"

#define ADDRESS_LEN      128
#define NEXUS_LEN        16
#define CHAIN_LEN        16
#define SCRIPT_LEN       MAX_TRANSACTION_LEN - (CHAIN_LEN + NEXUS_LEN + PAYLOAD_LEN)
#define TOKEN_LEN        10
#define CONTRACT_METHOD_LEN        16
#define CONTRACT_METHOD_ARGS_LEN   MAX_ARGS_LEN * ADDRESS_LEN
#define TXLENGTH_LEN     16
#define SCRIPTLENGTH_LEN 16
#define PAYLOAD_LEN      64

#define MAX_ARGS_LEN             4
#define ALLOW_GAS_ARGS_LEN       4
#define STAKING_TOKENS_ARGS_LEN 2
#define UNSTAKING_TOKENS_ARGS_LEN 2
#define TRANSFER_TOKENS_ARGS_LEN 4
#define INTEROP_CALL_ARGS_LEN 8
#define SPEND_GAS_ARGS_LEN       1

#define MAX_TX_LEN   1020
#define MAX_MEMO_LEN 465  // 510 - ADDRESS_LEN - 2*SIZE(U64) - SIZE(MAX_VARINT)

typedef enum {
    PARSING_OK = 1,
    NONCE_PARSING_ERROR = -1,
    TO_PARSING_ERROR = -2,
    VALUE_PARSING_ERROR = -3,
    MEMO_LENGTH_ERROR = -4,
    MEMO_PARSING_ERROR = -5,
    MEMO_ENCODING_ERROR = -6,
    WRONG_LENGTH_ERROR = -7,

    NEXUS_ZERO_ERROR = 0xF0,
    NEXUS_OVERFLOW_ERROR = 0xF01,
    NEXUS_PARSING_ERROR = 0xF2,

    CHAIN_ZERO_ERROR = 0xE0,
    CHAIN_OVERFLOW_ERROR = 0xE1,
    CHAIN_PARSING_ERROR = 0xE2,

    SCRIPT_ZERO_ERROR = 0xD0,
    SCRIPT_OVERFLOW_ERROR = 0xD1,
    SCRIPT_PARSING_ERROR = 0xD2,
    SCRIPT_UNDERFLOW_ERROR = 0xD3,
    SCRIPT_PARSING_ERROR_OPCODE_EXTCALL7 = 0xD4,
    SCRIPT_PARSING_ERROR_OPCODE_CTX45 = 0xD5,
    SCRIPT_PARSING_ERROR_OPCODE_LOAD13 = 0xD6,
    SCRIPT_PARSING_ERROR_OPCODE_SWITCH46 = 0xD7,
    SCRIPT_PARSING_ERROR_OPCODE_PUSH2 = 0xD8,
    SCRIPT_PARSING_ERROR_OPCODE_RET11 = 0xD9,

    PAYLOAD_ZERO_ERROR = 0xC0,
    PAYLOAD_OVERFLOW_ERROR = 0xC1,
    PAYLOAD_PARSING_ERROR = 0xC2,

    TOKEN_PARSING_ERROR = 0xB0,
    //TO_PARSING_ERROR = 0xB1,
    // VALUE_PARSING_ERROR = 0xB2,
    // GAS_PRICE_PARSING_ERROR = 0xB3,
    // GAS_LIMIT_PARSING_ERROR = 0xB4,

    LENGTH_UNDERFLOW_ERROR = 0xA0,
    LENGTH_OVERFLOW_ERROR = 0xA1,
    LENGTH_ZERO_ERROR = 0xA2,

    PARSING_DEBUG = 0xA3,
} parser_status_e;

typedef enum {
    TRANSACTION_TYPE_TRANSFER = 0x01,
    TRANSACTION_TYPE_STAKE = 0x02,
    TRANSACTION_TYPE_UNSTAKE = 0x03,
    TRANSACTION_TYPE_CLAIM = 0x04,
    TRANSACTION_TYPE_CUSTOM = 0x05,
} transaction_type_e;

typedef struct {
    buffer_t buf;
    uint8_t opcode;
    uint8_t reg;
    uint8_t type;
} load_t;

typedef struct {
    uint8_t opcode;
    uint8_t reg;
} push_t;

typedef struct {
    load_t load;
    push_t push;
} load_push_t;

typedef struct {
    uint8_t opcode;
} end_t;

typedef struct {
    uint8_t opcode;
    uint8_t src_reg;
    uint8_t dest_reg;
} ctx_t;

typedef struct {
    uint8_t opcode;
    uint8_t dest_reg;
} extcall_t;

typedef struct {
    uint8_t opcode;
    uint8_t dest_reg;
} switch_t;

typedef struct {
    load_push_t args[MAX_ARGS_LEN];
    uint8_t args_len;
    load_push_t method;
    load_t name;
    ctx_t ctx;
    switch_t swwitch;
} contract_t;

typedef struct {
    load_push_t args[MAX_ARGS_LEN];
    uint8_t args_len;
    load_t method;
    extcall_t extcall;
} interop_t;

typedef struct {
    uint64_t nonce;     /// nonce (8 bytes)
    uint64_t value;     /// amount value (8 bytes)
    uint8_t *to;        /// pointer to address (20 bytes)
    uint8_t *memo;      /// memo (variable length)
    uint64_t memo_len;  /// length of memo (8 bytes)
} transaction_t;
