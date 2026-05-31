#ifndef ESC_PARSER_H
#define ESC_PARSER_H

#include <stdint.h>

void esc_parser_reset();
bool esc_parser_feed(char c);

#endif
