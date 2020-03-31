#ifndef SNTP_H
#define SNTP_H

#include "esp_sntp.h"


#ifdef __cplusplus
extern "C" {
#endif


void sntp_sync(sntp_sync_time_cb_t callback);

#ifdef __cplusplus
}
#endif

#endif  // SMBUS_H
