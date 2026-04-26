#include <stdio.h>
#include <string.h>
#include "unity.h"

static void print_banner(const char * text);

void app_main(void)
{
    print_banner("Starting unit tests");
    unity_run_all_tests();
}

static void print_banner(const char* text)
{
    printf("\n#### %s #####\n\n", text);
}