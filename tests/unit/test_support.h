/*
 * Helpers shared by the unit tests: definitions loaded once, hex conversion, bit-field rendering,
 * and capture of the decoder's error messages (Tracer).
 */
#ifndef TEST_SUPPORT_H_
#define TEST_SUPPORT_H_

#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>

#include "AsterixDefinition.h"
#include "DataItemBits.h"
#include "InputParser.h"
#include "Tracer.h"
#include "XMLParser.h"
#include "asterixformat.hxx"

extern const char *g_definitionsFile;

// Tracer messages are kept here instead of being printed
inline std::vector<std::string> &tracerMessages() {
    static std::vector<std::string> messages;
    return messages;
}

inline int captureTracer(const char *format, ...) {
    char buffer[2048];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    tracerMessages().push_back(buffer);
    return 0;
}

// Same loading as CAsterixFormat::CreateFormatDescriptor: one XML file per line of the ini file,
// relative to the current directory or to the ini file's directory.
inline bool loadDefinitions(const char *iniFile, AsterixDefinition *pDefinition) {
    FILE *ini = fopen(iniFile, "rt");
    if (!ini)
        return false;
    std::string dir(iniFile);
    size_t slash = dir.find_last_of('/');
    dir = slash == std::string::npos ? "" : dir.substr(0, slash + 1);
    char line[512];
    bool ok = true;
    while (fgets(line, sizeof(line), ini)) {
        std::string name(line);
        while (!name.empty() && (name[name.size() - 1] == '\n' || name[name.size() - 1] == '\r'))
            name.erase(name.size() - 1);
        if (name.empty())
            continue;
        FILE *fp = fopen(name.c_str(), "rt");
        if (!fp)
            fp = fopen((dir + name).c_str(), "rt");
        if (!fp) {
            ok = false;
            continue;
        }
        XMLParser parser;
        ok = parser.Parse(fp, pDefinition, name.c_str()) && ok;
        fclose(fp);
    }
    fclose(ini);
    return ok;
}

// Parser with every definition of g_definitionsFile, built once (InputParser owns the definitions)
inline InputParser &parser() {
    static InputParser *p = NULL;
    if (!p) {
        Tracer::Configure(captureTracer);
        AsterixDefinition *def = new AsterixDefinition();
        if (!loadDefinitions(g_definitionsFile, def))
            fprintf(stderr, "warning: some definitions in %s did not load\n", g_definitionsFile);
        p = new InputParser(def);
    }
    return *p;
}

// the definitions held by parser() (InputParser keeps them private: loaded a second time, also once)
inline AsterixDefinition &definitions() {
    static AsterixDefinition *def = NULL;
    if (!def) {
        parser();  // configures the Tracer
        def = new AsterixDefinition();
        loadDefinitions(g_definitionsFile, def);
    }
    return *def;
}

inline std::vector<unsigned char> fromHex(const std::string &hex) {
    std::vector<unsigned char> out;
    for (size_t i = 0; i + 1 < hex.size(); i += 2)
        out.push_back((unsigned char) strtoul(hex.substr(i, 2).c_str(), NULL, 16));
    return out;
}

// A bits field on its own, as the XML parser would build it
inline DataItemBits *makeBits(const char *name, int from, int to,
                              DataItemBits::_eEncoding encoding = DataItemBits::DATAITEM_ENCODING_UNSIGNED,
                              double scale = 0.0) {
    DataItemBits *b = new DataItemBits(0);
    b->m_strShortName = name;
    b->m_strName = name;
    b->m_nFrom = from;
    b->m_nTo = to;
    b->m_eEncoding = encoding;
    b->m_dScale = scale;
    return b;
}

// text the field writes for data (hex string) in a given output format; pos counted from the data start
inline std::string render(DataItemBits *b, const std::string &hexData, unsigned int formatType) {
    std::vector<unsigned char> data = fromHex(hexData);
    std::string out, header;
    DataItemBits::JsonPosition pos(formatType == CAsterixFormat::EJSONX ? &data[0] : NULL, 0);
    b->getText(out, header, formatType, &data[0], (long) data.size());
    return out;
}

#endif /* TEST_SUPPORT_H_ */
