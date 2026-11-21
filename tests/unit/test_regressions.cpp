/*
 * Regression tests of fixed bugs (see HISTORY). Several of them need the sanitizer build to fail
 * loudly; in a normal build they still check the results.
 */
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <unistd.h>

#include "minitest.h"
#include "test_support.h"

TEST_CASE("Tracer formats once: a '%' in the message arguments is printed, not interpreted") {
    parser();
    tracerMessages().clear();
    Tracer::Error("file %s", "bad%s%s%n.xml");
    REQUIRE(tracerMessages().size() == 1);
    CHECK_EQ(tracerMessages()[0], std::string("file bad%s%s%n.xml"));
}

TEST_CASE("XMLParser: a definitions file with a syntax error and % in its name fails cleanly") {
    parser();
    char dir[] = "/tmp/asterix_unit_XXXXXX";
    REQUIRE(mkdtemp(dir) != NULL);
    std::string path = std::string(dir) + "/bad%s%s%n.xml";
    FILE *f = fopen(path.c_str(), "w");
    REQUIRE(f != NULL);
    fputs("<?xml version=\"1.0\"?>\n<Category id=\"1\">\n<Bad%s%s%n>\n</Category>\n", f);
    fclose(f);
    tracerMessages().clear();
    AsterixDefinition def;
    FILE *fp = fopen(path.c_str(), "rt");
    XMLParser p;
    bool ok = p.Parse(fp, &def, path.c_str());
    fclose(fp);
    CHECK(!ok);
    REQUIRE(!tracerMessages().empty());
    CHECK(tracerMessages()[0].find("bad%s%s%n.xml") != std::string::npos);
    unlink(path.c_str());
    rmdir(dir);
}

TEST_CASE("XMLParser: the malformed definitions of the Python tests are rejected") {
    parser();
    const char *files[] = {"asterix/test/parsing_error_1.xml", "asterix/test/parsing_error_2.xml"};
    for (size_t i = 0; i < 2; i++) {
        FILE *fp = fopen(files[i], "rt");
        REQUIRE(fp != NULL);
        AsterixDefinition def;
        XMLParser p;
        CHECK(!p.Parse(fp, &def, files[i]));
        fclose(fp);
    }
}

TEST_CASE("Repetitive BDS item cut in the middle: rejected without reading past the data") {
    // CAT048 record: FSPEC 01 20 = only FRN 10 (I250, repetitive BDS), which says 3 entries of 8 bytes
    // but has one
    std::vector<unsigned char> b = fromHex("30000E" "0120" "03" "C0780031BC000040");
    AsterixData *d = parser().parsePacket(&b[0], (unsigned int) b.size());
    REQUIRE(d != NULL);
    std::string out;
    d->getText(out, CAsterixFormat::EJSONX);
    CHECK_EQ(out, std::string(""));
    delete d;
    // the same record with the count fixed to 1 decodes
    b[5] = 0x01;
    b[2] = 0x0E;
    d = parser().parsePacket(&b[0], (unsigned int) b.size());
    out.clear();
    d->getText(out, CAsterixFormat::EJSONX);
    CHECK(out.find("\"I250\"") != std::string::npos);
    delete d;
}
