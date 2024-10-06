#pragma once

void led_on();
void led_off();
void gpios_setup( );
void blink_led_task(void *);
void blink_led();
void stop_blink_led();
