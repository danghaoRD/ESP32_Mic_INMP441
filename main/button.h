/**
 * button.h
 * author: Hao Dang
 * date: 03 Jan 2026
 */

#ifndef BUTTON_H
#define BUTTON_H
#include <stdint.h>

void button_init(void);
void led_init(void);
void led_set_brigh_percen(uint8_t percent);

#endif // BUTTON_H