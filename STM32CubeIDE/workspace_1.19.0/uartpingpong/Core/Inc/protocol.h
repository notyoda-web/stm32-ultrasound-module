/*
 * protocol.h - Packet framing for the MCU <-> RPI UART link.
 *
 *   | HEAD 0xAA | CMD | LEN | PAYLOAD[LEN] | CHK | STOP 0x00 |
 *
 * CHK = CMD ^ LEN ^ PAYLOAD[0] ^ ... ^ PAYLOAD[LEN-1]
 * Total packet size = LEN + 5, LEN <= 0x20  ->  max 37 bytes.
 *
 * This file has NO hardware dependency, so it can be unit-tested on a PC.
 */
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stddef.h>

#define PROTO_HEAD          0xAAu
#define PROTO_STOP          0x00u
#define PROTO_MAX_PAYLOAD   32u
#define PROTO_OVERHEAD      5u
#define PROTO_MAX_PACKET    (PROTO_MAX_PAYLOAD + PROTO_OVERHEAD)

/* Command table (shared MCU/RPI interface) */
#define PROTO_CMD_PING        0x01u  /* host -> MCU : payload = anything (echoed) */
#define PROTO_CMD_PONG        0x02u  /* MCU -> host : same payload as the PING    */
#define PROTO_CMD_SET_WHEEL   0x21u  /* from the spec example (not used here)     */
#define PROTO_CMD_ERROR       0xFFu  /* error response, payload[0] = error code   */

/* Error codes carried in an ERROR packet */
#define PROTO_ERRCODE_UNKNOWN_CMD  0x01u

typedef struct {
    uint8_t cmd;
    uint8_t len;
    uint8_t payload[PROTO_MAX_PAYLOAD];
} proto_packet_t;

typedef enum {
    PROTO_S_HEAD = 0,
    PROTO_S_CMD,
    PROTO_S_LEN,
    PROTO_S_PAYLOAD,
    PROTO_S_CHK,
    PROTO_S_STOP
} proto_state_t;

typedef enum {
    PROTO_NONE = 0,      /* byte consumed, packet not finished yet   */
    PROTO_PACKET_OK,     /* a complete, valid packet is in p->packet */
    PROTO_ERR_LEN,       /* LEN > 0x20 -> rejected immediately       */
    PROTO_ERR_CHK,       /* checksum mismatch                        */
    PROTO_ERR_STOP       /* STOP byte != 0x00                        */
} proto_result_t;

typedef struct {
    proto_state_t  state;
    proto_packet_t packet;
    uint8_t        idx;
    uint8_t        chk;
} proto_parser_t;

uint8_t proto_checksum(uint8_t cmd, uint8_t len, const uint8_t *payload);

/* Build a frame into out[] (needs >= len+5 bytes). Returns frame size, 0 on error. */
size_t  proto_encode(uint8_t *out, uint8_t cmd, const uint8_t *payload, uint8_t len);

void           proto_parser_init(proto_parser_t *p);
proto_result_t proto_parser_feed(proto_parser_t *p, uint8_t byte);

#endif
