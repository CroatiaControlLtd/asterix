/*
 * Fuzz target of the decoder: parse arbitrary bytes as an ASTERIX packet and print it in every
 * output format, with and without a filter. Built by CMake with -DASTERIX_FUZZ=ON (clang, libFuzzer
 * and AddressSanitizer/UBSan). Without libFuzzer (-DFUZZ_STANDALONE) it is a plain program that runs
 * the files given on the command line, to reproduce a crash with any compiler.
 *
 * Definitions: $ASTERIX_DEFS, default install/config/asterix.ini (run from the repository root).
 */
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "AsterixData.h"
#include "AsterixDefinition.h"
#include "InputParser.h"
#include "Tracer.h"
#include "XMLParser.h"
#include "asterixformat.hxx"

bool gVerbose = false;
bool gSynchronous = false;
bool gForceRouting = false;
int gHeartbeat = 0;
const char *gAsterixDefinitionsFile = NULL;
bool gFiltering = false;

static int quiet(const char *, ...) { return 0; }

static InputParser *g_parser = NULL;

static void init() {
    if (g_parser)
        return;
    Tracer::Configure(quiet);
    const char *ini = getenv("ASTERIX_DEFS");
    if (!ini)
        ini = "install/config/asterix.ini";
    AsterixDefinition *def = new AsterixDefinition();
    FILE *f = fopen(ini, "rt");
    if (!f) {
        fprintf(stderr, "definitions file %s not found (set ASTERIX_DEFS)\n", ini);
        exit(1);
    }
    std::string dir(ini);
    size_t slash = dir.find_last_of('/');
    dir = slash == std::string::npos ? "" : dir.substr(0, slash + 1);
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        std::string name(line);
        while (!name.empty() && (name[name.size() - 1] == '\n' || name[name.size() - 1] == '\r'))
            name.erase(name.size() - 1);
        if (name.empty())
            continue;
        FILE *x = fopen(name.c_str(), "rt");
        if (!x)
            x = fopen((dir + name).c_str(), "rt");
        if (!x)
            continue;
        XMLParser p;
        p.Parse(x, def, name.c_str());
        fclose(x);
    }
    fclose(f);
    g_parser = new InputParser(def);
    // filter some items, so that the filtered paths run too (they only print when gFiltering is set)
    g_parser->filterOutItem(48, "010", "SAC");
    g_parser->filterOutItem(48, "250", "MCP_ALT");
    g_parser->filterOutItem(62, "105", "Lat");
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    init();
    AsterixData *d = g_parser->parsePacket(data, (unsigned int) size, 1.0, 0x7F000001);
    if (d) {
        static const unsigned int formats[] = {CAsterixFormat::ETxt, CAsterixFormat::EOut, CAsterixFormat::EJSON,
                                               CAsterixFormat::EJSONH, CAsterixFormat::EJSONE, CAsterixFormat::EJSONX,
                                               CAsterixFormat::EXML, CAsterixFormat::EXMLH};
        for (int filtered = 0; filtered < 2; filtered++) {
            gFiltering = filtered != 0;
            for (size_t i = 0; i < sizeof(formats) / sizeof(formats[0]); i++) {
                std::string out;
                d->getText(out, formats[i]);
            }
        }
        gFiltering = false;
        delete d;
    }
    return 0;
}

#ifdef FUZZ_STANDALONE
int main(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        FILE *f = fopen(argv[i], "rb");
        if (!f) {
            perror(argv[i]);
            return 1;
        }
        std::vector<uint8_t> buf;
        int c;
        while ((c = fgetc(f)) != EOF)
            buf.push_back((uint8_t) c);
        fclose(f);
        LLVMFuzzerTestOneInput(buf.empty() ? NULL : &buf[0], buf.size());
    }
    printf("%d inputs\n", argc - 1);
    return 0;
}
#endif
