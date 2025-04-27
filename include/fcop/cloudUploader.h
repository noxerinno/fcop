#ifndef CLOUD_UPLOADER_H
#define CLOUD_UPLOADER_H

#include <HTTPClient.h>

#include "fcop/credentials.h"
#include "fcop/logger.h"

String getAccessToken();
void sendLogFileToDrive();

#endif  // CLOUD_UPLOADER_H