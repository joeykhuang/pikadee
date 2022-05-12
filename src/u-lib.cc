#include "printf.hh"
#include "u-lib.hh"

void assert_fail(const char* file, int line, const char* msg,
                 const char* description) {
    if (description) {
        printf("%s:%d: %s\n", file, line, description);
    }
    printf("%s:%d: user assertion '%s' failed\n", file, line, msg);
}