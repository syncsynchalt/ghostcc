#pragma once

extern void die(const char *msg, ...) __attribute__((__noreturn__)) __attribute__((__format__(__printf__, 1, 2)));
