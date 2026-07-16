#ifndef ASSERTS_H
#define ASSERTS_H

#define FUS_TEST_ASSERT(expr, msg) \
    do { \
        if (!(expr)) { \
            printf("❌ FALHA: %s (%s:%d)\n", msg, __FILE__, __LINE__); \
            return false; \
        } \
    } while (0)

#endif