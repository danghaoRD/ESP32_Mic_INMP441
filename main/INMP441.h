/**
 * INMP441.h
 * author: Hao Dang
 * date: 21 Dec 2025
 */

#ifndef INMP441_H
#define INMP441_H
#include "esp_err.h"


void inmp441_init(void);
void inmp441_deinit(void);
esp_err_t inmp441_read(int16_t *dest, size_t len, size_t *bytes_read, uint32_t timeout_ms);
#endif // INMP441_H