#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <cmocka.h>

#include "transaction/serialize.h"
#include "transaction/deserialize.h"
#include "types.h"

#define RAW_TX_LEN (sizeof(hex_tx)-1)/2

#define RAW_SCRIPT_LEN (sizeof(hex_script)-1)/2

static void test_tx_serialization(void **state) {
    (void) state;

    transaction_t tx;
    // clang-format off
    uint8_t raw_tx[] = {
        // nonce (8)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
        // to (20)
        0x7a, 0xc3, 0x39, 0x97, 0x54, 0x4e, 0x31, 0x75,
        0xd2, 0x66, 0xbd, 0x02, 0x24, 0x39, 0xb2, 0x2c,
        0xdb, 0x16, 0x50, 0x8c,
        // value (8)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x08, 0x07,
        // memo length (varint: 1-9)
        0xf1,
        // memo (var: 241)
        0x54, 0x68, 0x65, 0x20, 0x54, 0x68, 0x65, 0x6f,
        0x72, 0x79, 0x20, 0x6f, 0x66, 0x20, 0x47, 0x72,
        0x6f, 0x75, 0x70, 0x73, 0x20, 0x69, 0x73, 0x20,
        0x61, 0x20, 0x62, 0x72, 0x61, 0x6e, 0x63, 0x68,
        0x20, 0x6f, 0x66, 0x20, 0x6d, 0x61, 0x74, 0x68,
        0x65, 0x6d, 0x61, 0x74, 0x69, 0x63, 0x73, 0x20,
        0x69, 0x6e, 0x20, 0x77, 0x68, 0x69, 0x63, 0x68,
        0x20, 0x6f, 0x6e, 0x65, 0x20, 0x64, 0x6f, 0x65,
        0x73, 0x20, 0x73, 0x6f, 0x6d, 0x65, 0x74, 0x68,
        0x69, 0x6e, 0x67, 0x20, 0x74, 0x6f, 0x20, 0x73,
        0x6f, 0x6d, 0x65, 0x74, 0x68, 0x69, 0x6e, 0x67,
        0x20, 0x61, 0x6e, 0x64, 0x20, 0x74, 0x68, 0x65,
        0x6e, 0x20, 0x63, 0x6f, 0x6d, 0x70, 0x61, 0x72,
        0x65, 0x73, 0x20, 0x74, 0x68, 0x65, 0x20, 0x72,
        0x65, 0x73, 0x75, 0x6c, 0x74, 0x20, 0x77, 0x69,
        0x74, 0x68, 0x20, 0x74, 0x68, 0x65, 0x20, 0x72,
        0x65, 0x73, 0x75, 0x6c, 0x74, 0x20, 0x6f, 0x62,
        0x74, 0x61, 0x69, 0x6e, 0x65, 0x64, 0x20, 0x66,
        0x72, 0x6f, 0x6d, 0x20, 0x64, 0x6f, 0x69, 0x6e,
        0x67, 0x20, 0x74, 0x68, 0x65, 0x20, 0x73, 0x61,
        0x6d, 0x65, 0x20, 0x74, 0x68, 0x69, 0x6e, 0x67,
        0x20, 0x74, 0x6f, 0x20, 0x73, 0x6f, 0x6d, 0x65,
        0x74, 0x68, 0x69, 0x6e, 0x67, 0x20, 0x65, 0x6c,
        0x73, 0x65, 0x2c, 0x20, 0x6f, 0x72, 0x20, 0x73,
        0x6f, 0x6d, 0x65, 0x74, 0x68, 0x69, 0x6e, 0x67,
        0x20, 0x65, 0x6c, 0x73, 0x65, 0x20, 0x74, 0x6f,
        0x20, 0x74, 0x68, 0x65, 0x20, 0x73, 0x61, 0x6d,
        0x65, 0x20, 0x74, 0x68, 0x69, 0x6e, 0x67, 0x2e,
        0x20, 0x4e, 0x65, 0x77, 0x6d, 0x61, 0x6e, 0x2c,
        0x20, 0x4a, 0x61, 0x6d, 0x65, 0x73, 0x20, 0x52,
        0x2e
    };

    buffer_t buf = {.ptr = raw_tx, .size = sizeof(raw_tx), .offset = 0};

    parser_status_e status = transaction_deserialize(&buf, &tx);

    assert_int_equal(status, PARSING_OK);

    uint8_t output[300];
    int length = transaction_serialize(&tx, output, sizeof(output));
    assert_int_equal(length, sizeof(raw_tx));
    assert_memory_equal(raw_tx, output, sizeof(raw_tx));
}

static int hex_str_to_bytes(uint8_t* out, const char* in, size_t out_len_max)
{
    const int in_len = strnlen(in, out_len_max * 2);
    if (in_len % 2 != 0) {
      return -1; // error, in str len should be even
    }

    // calc actual out len
    const int out_len = out_len_max < (in_len / 2) ? out_len_max : (in_len / 2);

    for (int i = 0; i < out_len; i++) {
        char ch0 = in[2 * i];
        char ch1 = in[2 * i + 1];
        uint8_t nib0 = ((ch0 & 0xF) + (ch0 >> 6)) | ((ch0 >> 3) & 0x8);
        uint8_t nib1 = ((ch1 & 0xF) + (ch1 >> 6)) | ((ch1 >> 3) & 0x8);
        uint8_t c = (nib0 << 4) | nib1;
        out[i] = c;
    }
    return out_len;
}

static void assert_stringn_equal(const uint8_t *a, const char *b, const size_t len) {
  uint8_t a0[len+1];
  memset(a0, 0, sizeof(a0));
  memmove(a0, a, len);
  assert_string_equal(a0,b);

}

static void test_tx_serialization_v2(void **state) {
    (void) state;

    transaction_t tx;

    char hex_script[] =
      "0D000402313003000D000402313003000D000423533131313131313131313131"
      "313131313131313131313131313131313131313131313103000D00042F50324B"
      "376E3170566679335A4D536578747258316E6A337859645251766A446E744547"
      "416631783262376836526B3403000D000408416C6C6F7747617303000D000403"
      "6761732D00012E010D000402313003000D0004074F50434F44455303000D0004"
      "2F50324B376E3170566679335A4D536578747258316E6A337859645251766A44"
      "6E744547416631783262376836526B3403000D00042F50324B376E3170566679"
      "335A4D536578747258316E6A337859645251766A446E74454741663178326237"
      "6836526B3403000D00041652756E74696D652E5472616E73666572546F6B656E"
      "7307000D00042F50324B376E3170566679335A4D536578747258316E6A337859"
      "645251766A446E744547416631783262376836526B3403000D0004085370656E"
      "6447617303000D0004036761732D00012E010B";

    char hex_tx[] = "076d61696e6e6574046d61696efd73010d000402313003000d00040231300300"
            "0d00042353313131313131313131313131313131313131313131313131313131"
            "3131313131313103000d00042f50324b376e3170566679335a4d536578747258"
            "316e6a337859645251766a446e744547416631783262376836526b3403000d00"
            "0408416c6c6f7747617303000d0004036761732d00012e010d00040231300300"
            "0d0004074f50434f44455303000d00042f50324b376e3170566679335a4d5365"
            "78747258316e6a337859645251766a446e744547416631783262376836526b34"
            "03000d00042f50324b376e3170566679335a4d536578747258316e6a33785964"
            "5251766a446e744547416631783262376836526b3403000d00041652756e7469"
            "6d652e5472616e73666572546f6b656e7307000d00042f50324b376e31705666"
            "79335a4d536578747258316e6a337859645251766a446e744547416631783262"
            "376836526b3403000d0004085370656e6447617303000d0004036761732d0001"
            "2e010b000000000c7068616E7461736D612D7473";
    assert_int_equal(sizeof(hex_tx), 0x0329);

    // clang-format off
    uint8_t raw_tx[RAW_TX_LEN] = {0};
    assert_int_equal(sizeof(raw_tx), 0x194);

    hex_str_to_bytes(raw_tx,hex_tx,sizeof(raw_tx));

    uint8_t raw_script[RAW_SCRIPT_LEN] = {0};
    assert_int_equal(sizeof(raw_script), 0x173);

    hex_str_to_bytes(raw_script,hex_script,sizeof(raw_script));

    buffer_t buf = {.ptr = raw_tx, .size = sizeof(raw_tx), .offset = 0};

    parser_status_e status = transaction_deserialize(&buf, &tx);

    // for(int i = 0; i < tx.script_len; i++) {
    //   uint8_t c = *(tx.script+i);
    //   uint8_t s = *(raw_script+i);
    //   uint8_t s2 = *(tx.script_buf.ptr+i);
    //   bool match = i < tx.script_buf.offset;
    //   printf("hex_t_bytes %d %d %02X %02X %02X %c\n", match, i, c, s, s2, s2);
    // }

    printf("buf.offset %lu\n", buf.offset);
    printf("buf.size %lu\n", buf.size);

    printf("tx.script_buf.offset %lu\n", tx.script_buf.offset);
    printf("tx.script_buf.size %lu\n", tx.script_buf.size);

    assert_int_equal(tx.allow_gas.args[0].load.opcode, 13);
    assert_int_equal(tx.allow_gas.args[0].load.reg, 0);
    assert_int_equal(tx.allow_gas.args[0].load.type, 4);
    assert_int_equal(tx.allow_gas.args[0].load.buf.size, 2);
    assert_stringn_equal(tx.allow_gas.args[0].load.buf.ptr,
       "10", tx.allow_gas.args[0].load.buf.size);

    assert_int_equal(tx.allow_gas.args[0].push.opcode, 3);
    assert_int_equal(tx.allow_gas.args[0].push.reg, 0);

    // value
    assert_stringn_equal(tx.transfer_tokens.args[0].load.buf.ptr,
       "10", tx.transfer_tokens.args[0].load.buf.size);

    assert_stringn_equal(tx.from,"P2K7n1pVfy3ZMSextrX1nj3xYdRQvjDntEGAf1x2b7h6Rk4", tx.from_len);
    assert_stringn_equal(tx.to,"P2K7n1pVfy3ZMSextrX1nj3xYdRQvjDntEGAf1x2b7h6Rk4", tx.to_len);

    // yes OPCODES is a fake token name.
    assert_stringn_equal(tx.token,"OPCODES", tx.token_len);
    assert_stringn_equal(tx.value,"10", tx.value_len);
    assert_stringn_equal(tx.gas_limit,"10", tx.gas_limit_len);
    assert_stringn_equal(tx.gas_price,"10", tx.gas_price_len);

    //assert_int_equal(tx.opcode_offsets_ix, 0);

    //assert_int_equal(tx.script_buf.offset, tx.script_buf.size);

    assert_int_equal(status, PARSING_OK);

    assert_stringn_equal(tx.nexus, "mainnet", tx.nexus_len);
    assert_stringn_equal(tx.chain, "main", tx.chain_len);
    assert_stringn_equal(tx.payload, "phantasma-ts", tx.payload_len);

    uint8_t output[RAW_TX_LEN];
    int length = transaction_serialize(&tx, output, sizeof(output));
    assert_int_equal(length, sizeof(raw_tx));
    assert_memory_equal(raw_tx, output, sizeof(raw_tx));
}


int main() {
    const struct CMUnitTest tests[] = {cmocka_unit_test(test_tx_serialization)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
