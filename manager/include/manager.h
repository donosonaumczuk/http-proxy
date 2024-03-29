#ifndef MANAGER_H
#define MANAGER_H
#include <stdlib.h>
#include <protocol.h>

enum resId_t { NO_ID, MIME_ID, CMD_ID, MTR_CN_ID, MTR_HS_ID, MTR_BT_ID, TF_ID };
typedef enum resId_t resId_t;

#define ON 0x01
#define OFF 0x00

#define DEFAULT_PORT 9090
#define DEFAULT_IP "127.0.0.1"

#define STREAM_QUANTITY 2
#define GET_STREAM 0
#define SET_STREAM 1
#define BYE_STREAM 1

#define ID_QUANTITY 9

#endif
