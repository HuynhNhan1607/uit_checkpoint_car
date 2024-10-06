#pragma once

#include "nvs_flash.h"

void nvs_init();
void save_pass(nvs_handle *, const char *, const char *);
void save_ip(nvs_handle *, const char *);
void delete_all(nvs_handle *);
void delete_part(nvs_handle *, const char *);
char *get_part(nvs_handle *, const char *);