#ifndef TUTORIAL_TR_INTERNAL_H
#define TUTORIAL_TR_INTERNAL_H

#include <stdbool.h>
#include <stdio.h>

bool tr_parse_highlights(char *text, int *targets);

#ifdef TR_TUTORIAL_TESTING
bool tr_load_tutorial_stream_for_testing(FILE *file, const char *fixture_name);
#endif

#endif