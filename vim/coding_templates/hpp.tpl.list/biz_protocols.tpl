/* SPDX-License-Identifier: Apache-2.0 */

/*
 * Biz protocol definitions.
 *
 * Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
 * All rights reserved.
 */

#ifndef __${HEADER_GUARD}_HPP__
#define __${HEADER_GUARD}_HPP__

#include "communication_protocol.h"

#pragma pack(1)

#if !defined(INT64_SESSION_ID) && !defined(SESSION_ID_LENGTH)
#define SESSION_ID_LENGTH                   16
#define SESSION_ID_ARRAY_SIZE               (SESSION_ID_LENGTH + (sizeof(uint32_t) - (SESSION_ID_LENGTH % sizeof(uint32_t))))
#endif

#ifdef INT64_SESSION_ID
#define SESSION_ID_META_VAR                 COMMPROTO_INT64
#else
#define SESSION_ID_META_VAR                 COMMPROTO_INT8_FIXED_ARRAY, COMMPROTO_ARRAY_LEN_IS(SESSION_ID_ARRAY_SIZE)
#endif

// Comment out this macro if session_id is in packet body.
#define SESSION_ID_IN_HEADER

// Comment out this macro if the type of session_id is string.
#define INT64_SESSION_ID

typedef struct packet_head
{
    uint16_t length;
    uint16_t command_code;
    uint16_t packet_seq;
    uint16_t total_packets;
    uint32_t sizeof_all_bodies;
    uint32_t body_offset;
#ifdef SESSION_ID_IN_HEADER
#ifdef INT64_SESSION_ID
    uint64_t session_id;
#else
    char session_id[SESSION_ID_ARRAY_SIZE];
#endif
#endif

    COMMPROTO_META_VAR_IN_STRUCT = {
        COMMPROTO_INT16, // length
        COMMPROTO_INT16, // command_code
        COMMPROTO_INT16, // packet_seq
        COMMPROTO_INT16, // total_packets
        COMMPROTO_INT32, // sizeof_all_bodies
        COMMPROTO_INT32, // body_offset
#ifdef SESSION_ID_IN_HEADER
        SESSION_ID_META_VAR // session_id
#endif
    };

    COMMPROTO_DEFINE_META_FUNCTIONS_IN_STRUCT();

    inline void fill(uint16_t command_code, uint16_t total_packets,
        uint32_t sizeof_all_bodies, uint16_t body_size, uint32_t body_offset,
#ifdef SESSION_ID_IN_HEADER
#ifdef INT64_SESSION_ID
        uint64_t session_id
#else
        const char session_id[]
#endif
#endif
    )
    {
        this->length = sizeof(packet_head) - sizeof(packet_head::length) + body_size;
        this->command_code = command_code;
        ++this->packet_seq;
        this->total_packets = total_packets;
        this->sizeof_all_bodies = sizeof_all_bodies;
        this->body_offset = body_offset;
#ifdef SESSION_ID_IN_HEADER
#ifdef INT64_SESSION_ID
        this->session_id = session_id;
#else
        memcpy(this->session_id, session_id, SESSION_ID_LENGTH);
#endif
#endif
    }
} packet_head_t;

// NOTE: This enum type can be defined in somewhere else,
//      but its name after the enum keyword should not change
//      since the doc generation of packet_body_prefix depends on it!
enum proto_err_e //! Protocol Error Codes
{
    PROTO_ERR_OK = 0, //!> Success
    PROTO_ERR_UNSUPPORTED = 1, //!> Operation unsupported yet
    PROTO_ERR_UNIMPLEMENTED = 2, //!> Operation unimplemented yet
};

typedef struct packet_body_prefix //! Prefix that all bodies should contain one.
{
#ifndef SESSION_ID_IN_HEADER
#ifdef INT64_SESSION_ID
    uint64_t session_id; //!> | session id |
#else
    char session_id[SESSION_ID_ARRAY_SIZE]; //!> | session id |
#endif
#endif
    uint32_t version; //!> | version |
    uint16_t return_code; //!> | return code | See \ref{proto_err_e}

    inline void fill(
#ifndef SESSION_ID_IN_HEADER
#ifdef INT64_SESSION_ID
        uint64_t session_id,
#else
        const char session_id[],
#endif
#endif
        uint32_t version, uint16_t return_code)
    {
#ifndef SESSION_ID_IN_HEADER
#ifdef INT64_SESSION_ID
        this->session_id = session_id;
#else
        memcpy(this->session_id, session_id, SESSION_ID_LENGTH);
#endif
#endif
        this->version = version;
        this->return_code = return_code;
    }
} packet_body_prefix_t;

#ifdef SESSION_ID_IN_HEADER
#define PKT_BODY_PREFIX_META_VARS           \
                                            COMMPROTO_INT32, \
                                            COMMPROTO_INT16
#else
#define PKT_BODY_PREFIX_META_VARS           SESSION_ID_META_VAR, \
                                            COMMPROTO_INT32, \
                                            COMMPROTO_INT16
#endif

#if 0

/******************************** Demo structs begin ********************************/

/*
 * Rules of defining an enum list:
 *
 * 01. Start a definition with:
 *      enum <name>_e //! <mandatory description in one single line>
 *
 * 02. Each item should be defined in a format of:
 *      <name> = <value>, //!> <mandatory description in one single line>
 */

/*
 * Rules of defining a packet body struct:
 *
 * 01. Start a definition with:
 *      typedef struct {req,reply}_<command code hex>_<name> //! <mandatory description in one single line>
 *
 * 02. Name of a sub-struct does not need   {req,reply}_<command code hex>_   prefix.
 *
 * 03. Only single variables and arrays of BASIC TYPEs are allowed within a sub-structure.
 *
 * 04. Each field of a struct should be defined in a format of:
 *      <type> <name>; //!> | <meaning> | [optional remark in one single line]
 *
 * 05. LaTeX syntax is allowed when needed, for example: \ref{}, \makecell[l]{}, etc.
 */

typedef struct req_0000_demo //! Demo Request
{
    // NOTE: The "struct" keyword here is mandatory!
    struct packet_body_prefix prefix; //!> | body prefix | See \ref{packet_body_prefix}

    COMMPROTO_META_VAR_IN_STRUCT = {
        PKT_BODY_PREFIX_META_VARS, // prefix
    };

    // NOTE: You should define COMMPROTO_META_VAR_OUT_OF_STRUCT(req_0000_demo) in another source file.
    COMMPROTO_DEFINE_META_FUNCTIONS_IN_STRUCT();
} req_0000_demo_t;

enum demo_status_e //! Options for Demo Status
{
    DEMO_STATUS_UNKNOWN = 0, //!> Unknown
    DEMO_STATUS_RUNNING = 1, //!> Demo is running
    DEMO_STATUS_PAUSE = 2, //!> Demo has paused
    DEMO_STATUS_STOPPED = 3, //!> Demo has stopped
};

typedef struct demo_result //! Demo Result Info
{
    char name[32]; //!> | name |
    int8_t status; //!> | status | See \ref{demo_status_e}
} demo_result_t;

typedef struct reply_0001_demo //! Demo Reply
{
    // NOTE: The "struct" keyword here is mandatory!
    struct packet_body_prefix prefix; //!> | body prefix | See \ref{packet_body_prefix}
#if 0
    // NOTE: The "struct" keyword here is mandatory!
    struct demo_result result_array[4]; //!> | result items list | See \ref{demo_result}
#else
    arraylen8_t result_count; //!> | number of result items |
    // NOTE: The "struct" keyword here is mandatory!
    struct demo_result *result_array; //!> | result items list | See \ref{demo_result}
#endif
    arraylen16_t extra_value_count; //!> | number of extra values |
    int32_t *extra_value_array; //!> | extra values list |

    // Only needed by structs containing fields of dynamic array type.
    reply_0001_demo() = delete; // Avoid uninitialized declaration.
    COMMPROTO_DEFINE_DESTRUCTOR(reply_0001_demo);

    COMMPROTO_META_VAR_IN_STRUCT = {
        PKT_BODY_PREFIX_META_VARS, // prefix
#if 0
        COMMPROTO_STRUCT_FIXED_ARRAY, COMMPROTO_STRUCT_FIELD_COUNT(2), COMMPROTO_ARRAY_LEN_IS(4), // result_array
#else
        COMMPROTO_ARRAY_LEN8, // result_count
        COMMPROTO_STRUCT_DYNAMIC_ARRAY, COMMPROTO_STRUCT_FIELD_COUNT(2), // result_array
#endif
            COMMPROTO_INT8_FIXED_ARRAY, COMMPROTO_ARRAY_LEN_IS(32), // name
            COMMPROTO_INT8, // status
        COMMPROTO_ARRAY_LEN16, // extra_value_count
        COMMPROTO_INT32_DYNAMIC_ARRAY, // extra_value_array
    };

    // NOTE: You should define COMMPROTO_META_VAR_OUT_OF_STRUCT(reply_0001_demo) in another source file.
    COMMPROTO_DEFINE_META_FUNCTIONS_IN_STRUCT();
} reply_0001_demo_t;

typedef req_0000_demo_t                     req_0002_another_demo_t; //! Another Demo Request which reuses body \ref{req_0000_demo}

typedef reply_0001_demo_t                   reply_0003_another_demo_t; //! Another Demo Reply which reuses body \ref{reply_0001_demo}

/******************************** Demo structs end ********************************/

#endif

#pragma pack()

#endif /* #ifndef __${HEADER_GUARD}_HPP__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> ${DATE}, ${LCS_USER} <${LCS_EMAIL}>:
 *  01. Initial commit.
 */
