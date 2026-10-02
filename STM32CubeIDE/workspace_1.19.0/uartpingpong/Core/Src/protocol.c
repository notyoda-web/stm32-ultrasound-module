#include "protocol.h"

uint8_t proto_checksum(uint8_t cmd, uint8_t len, const uint8_t *payload)
{
    uint8_t chk = cmd ^ len;                 /* HEAD and STOP are NOT included */
    for (uint8_t i = 0; i < len; i++) chk ^= payload[i];
    return chk;
}

size_t proto_encode(uint8_t *out, uint8_t cmd, const uint8_t *payload, uint8_t len)
{
    if (len > PROTO_MAX_PAYLOAD) return 0;
    size_t n = 0;
    out[n++] = PROTO_HEAD;
    out[n++] = cmd;
    out[n++] = len;
    for (uint8_t i = 0; i < len; i++) out[n++] = payload[i];
    out[n++] = proto_checksum(cmd, len, payload);
    out[n++] = PROTO_STOP;
    return n;                                /* == len + 5 */
}

void proto_parser_init(proto_parser_t *p)
{
    p->state = PROTO_S_HEAD;
    p->idx = 0;
    p->chk = 0;
}

/*
 * Byte-by-byte state machine. Framing relies on HEAD + LEN (0x00 may appear
 * inside the payload, so STOP alone can never be used to find the end).
 * STOP is only an extra format check. Any failure -> back to searching for HEAD.
 */
proto_result_t proto_parser_feed(proto_parser_t *p, uint8_t b)
{
    switch (p->state) {
    case PROTO_S_HEAD:
        if (b == PROTO_HEAD) p->state = PROTO_S_CMD;
        break;

    case PROTO_S_CMD:
        p->packet.cmd = b;
        p->chk = b;
        p->state = PROTO_S_LEN;
        break;

    case PROTO_S_LEN:
        if (b > PROTO_MAX_PAYLOAD) {         /* protect the buffer */
            p->state = PROTO_S_HEAD;
            return PROTO_ERR_LEN;
        }
        p->packet.len = b;
        p->chk ^= b;
        p->idx = 0;
        p->state = (b == 0) ? PROTO_S_CHK : PROTO_S_PAYLOAD;
        break;

    case PROTO_S_PAYLOAD:
        p->packet.payload[p->idx++] = b;
        p->chk ^= b;
        if (p->idx == p->packet.len) p->state = PROTO_S_CHK;
        break;

    case PROTO_S_CHK:
        if (b != p->chk) {
            p->state = PROTO_S_HEAD;
            return PROTO_ERR_CHK;
        }
        p->state = PROTO_S_STOP;
        break;

    case PROTO_S_STOP:
        p->state = PROTO_S_HEAD;
        return (b == PROTO_STOP) ? PROTO_PACKET_OK : PROTO_ERR_STOP;
    }
    return PROTO_NONE;
}
