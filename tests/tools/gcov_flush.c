/*
 * Linked into coverage builds only (cmake -DASTERIX_COVERAGE=ON): the application has no signal
 * handlers, so a test that stops it with SIGTERM (the multicast input test) would lose its coverage
 * data. exit() runs the gcov handlers that write the .gcda files.
 */
#include <signal.h>
#include <stdlib.h>

static void on_term(int sig) {
    (void) sig;
    exit(0);
}

__attribute__((constructor)) static void install_term_handler(void) {
    signal(SIGTERM, on_term);
}
