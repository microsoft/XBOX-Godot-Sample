// Doctest implementation TU. Defines DOCTEST_CONFIG_IMPLEMENT (not _WITH_MAIN)
// so test source files can live in their own TUs without redefining the
// doctest implementation. See spec/testing-strategy.md ("doctest layout").
#define DOCTEST_CONFIG_IMPLEMENT
#include "third_party/doctest/doctest.h"

int main(int argc, char** argv) {
    doctest::Context ctx;
    ctx.applyCommandLine(argc, argv);
    int rc = ctx.run();
    if (ctx.shouldExit()) {
        return rc;
    }
    return rc;
}
