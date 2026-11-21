/*
 * InputParser / DataBlock / DataRecord: data block framing, FSPEC, UAP selection and the record
 * header of each output format.
 */
#include <algorithm>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "minitest.h"
#include "test_support.h"
#include "AsterixData.h"
#include "DataBlock.h"
#include "DataRecord.h"

namespace {

// a real CAT001 track record (from install/sample_data/cat_001_002.pcap): FSPEC F7C6, I020 TYP=1
const char *CAT001_RECORD = "F7C619C9A00EB2767F189408AA42D8033405C8800D400E";

std::vector<unsigned char> block(int cat, const std::string &recordsHex) {
    std::vector<unsigned char> rec = fromHex(recordsHex);
    std::vector<unsigned char> b;
    b.push_back((unsigned char) cat);
    b.push_back((unsigned char) ((rec.size() + 3) >> 8));
    b.push_back((unsigned char) ((rec.size() + 3) & 0xFF));
    b.insert(b.end(), rec.begin(), rec.end());
    return b;
}

std::unique_ptr<AsterixData> parse(const std::vector<unsigned char> &data) {
    return std::unique_ptr<AsterixData>(parser().parsePacket(data.empty() ? NULL : &data[0],
                                                             (unsigned int) data.size(), 12.5));
}

std::string text(AsterixData *d, unsigned int formatType) {
    std::string out;
    d->getText(out, formatType);
    return out;
}

// records decoded without error (blocks also keep the records that failed)
size_t records(AsterixData *d) {
    size_t n = 0;
    for (std::list<DataBlock *>::iterator it = d->m_lDataBlocks.begin(); it != d->m_lDataBlocks.end(); ++it)
        for (std::list<DataRecord *>::iterator r = (*it)->m_lDataRecords.begin(); r != (*it)->m_lDataRecords.end(); ++r)
            n += (*r)->m_bFormatOK ? 1 : 0;
    return n;
}

}  // namespace

TEST_CASE("parser: one block, one record") {
    std::unique_ptr<AsterixData> d = parse(block(1, CAT001_RECORD));
    REQUIRE(d.get() != NULL);
    CHECK_EQ(d->m_lDataBlocks.size(), (size_t) 1);
    CHECK_EQ(records(d.get()), (size_t) 1);
    std::string j = text(d.get(), CAsterixFormat::EJSONX);
    CHECK(j.find("\"fspec\":\"F7C6\",\"hexdata\":\"19C9A00EB2767F189408AA42D8033405C8800D400E\"") != std::string::npos);
    CHECK(j.find("\"timestamp\":12.500000") != std::string::npos);
}

TEST_CASE("parser: several records in a block, several blocks in a packet") {
    std::string two = std::string(CAT001_RECORD) + CAT001_RECORD;
    std::vector<unsigned char> data = block(1, two);
    std::vector<unsigned char> second = block(1, CAT001_RECORD);
    data.insert(data.end(), second.begin(), second.end());
    std::unique_ptr<AsterixData> d = parse(data);
    CHECK_EQ(d->m_lDataBlocks.size(), (size_t) 2);
    CHECK_EQ(records(d.get()), (size_t) 3);
}

TEST_CASE("parser: buffers too short or with impossible lengths give no records and no crash") {
    const char *cases[] = {"", "01", "0100", "010003", "010002F7", "010000", "01FFFF" /* length past the end */};
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        std::unique_ptr<AsterixData> d = parse(fromHex(cases[i]));
        REQUIRE(d.get() != NULL);
        CHECK_EQ(records(d.get()), (size_t) 0);
    }
}

TEST_CASE("parser: a block length larger than the data parses what is there") {
    std::vector<unsigned char> data = block(1, CAT001_RECORD);
    data[1] = 0x01;  // length 256+, data is 26 bytes
    std::unique_ptr<AsterixData> d = parse(data);
    CHECK_EQ(records(d.get()), (size_t) 1);
}

TEST_CASE("parser: a truncated record is rejected, not read past its end") {
    // Note: after a bad record, DataBlock goes on from where that record stopped, so leftover bytes may
    // decode as another record (ASTERIX records carry no length to resynchronise). Only the truncated
    // record itself is checked here.
    std::string rec(CAT001_RECORD);
    for (size_t cut = 2; cut < rec.size(); cut += 2) {
        std::unique_ptr<AsterixData> d = parse(block(1, rec.substr(0, cut)));
        REQUIRE(d.get() != NULL);
        REQUIRE(!d->m_lDataBlocks.empty());
        DataBlock *b = d->m_lDataBlocks.front();
        REQUIRE(!b->m_lDataRecords.empty());
        CHECK(!b->m_lDataRecords.front()->m_bFormatOK);  // every prefix misses at least one item byte
    }
}

TEST_CASE("parser: FSPEC that never ends (every FX bit set) gives no record") {
    std::unique_ptr<AsterixData> d = parse(block(1, "FFFFFFFFFFFFFFFF"));
    CHECK_EQ(records(d.get()), (size_t) 0);
}

TEST_CASE("UAP selection: CAT001 I020 TYP chooses the track or the plot UAP") {
    // TYP=1 (track): FRN 4 is I161 (track number)
    std::string track = text(parse(block(1, CAT001_RECORD)).get(), CAsterixFormat::EJSON);
    CHECK(track.find("\"I161\"") != std::string::npos);
    // TYP=0 (plot): the same FSPEC bit is I040 in the plot UAP, so the items are read differently
    std::string rec(CAT001_RECORD);
    // the UAP is chosen by bit 17 after the FSPEC (use_if_bit_set="17"): MSB of the third byte, I020 0xA0
    rec.replace(8, 2, "20");  // I020 0xA0 -> 0x20: TYP bit cleared
    std::string plot = text(parse(block(1, rec)).get(), CAsterixFormat::EJSON);
    CHECK(plot.find("\"I161\"") == std::string::npos);
    CHECK(plot != track);
}

TEST_CASE("record header: fspec apart and hexdata without it in text and every JSON format; XML whole record") {
    std::unique_ptr<AsterixData> d = parse(block(1, CAT001_RECORD));
    const std::string items = "19C9A00EB2767F189408AA42D8033405C8800D400E";
    CHECK(text(d.get(), CAsterixFormat::ETxt).find("\nFspec: F7C6\nHexData: " + items + "\n") != std::string::npos);
    CHECK(text(d.get(), CAsterixFormat::EJSON).find("\"fspec\":\"F7C6\",\"hexdata\":\"" + items + "\"") != std::string::npos);
    CHECK(text(d.get(), CAsterixFormat::EJSONH).find("\"fspec\":\"F7C6\",\n\"hexdata\":\"" + items + "\"") != std::string::npos);
    CHECK(text(d.get(), CAsterixFormat::EJSONE).find("\"fspec\":\"F7C6\",\n\"hexdata\":\"" + items + "\"") != std::string::npos);
    CHECK(text(d.get(), CAsterixFormat::EXML).find("hexdata=\"F7C6" + items + "\"") != std::string::npos);
}

TEST_CASE("-jx keys: repeated names get a number, FX bits are numbered by extent") {
    std::string jx = text(parse(block(1, CAT001_RECORD)).get(), CAsterixFormat::EJSONX);
    // I070 and I170 have one spare each, I020/I170 one FX each: no duplicates inside an object
    CHECK(jx.find("\"FX0\"") != std::string::npos);
    size_t i020 = jx.find("\"I020\"");
    REQUIRE(i020 != std::string::npos);
    std::string obj = jx.substr(i020, jx.find('}', jx.find("\"FX0\"", i020)) - i020);
    CHECK_EQ(obj.find("\"FX1\""), std::string::npos);
}

TEST_CASE("a block with a malformed record reports a warning with the whole block, output unchanged") {
    std::string rec(CAT001_RECORD);
    std::string body = rec + rec.substr(0, 20) + rec;  // a cut record between two good ones
    std::vector<unsigned char> data = block(1, body);
    tracerMessages().clear();
    std::unique_ptr<AsterixData> d = parse(data);
    std::string blockHex;
    for (size_t i = 0; i < data.size(); i++) {
        char h[3];
        snprintf(h, sizeof(h), "%02X", data[i]);
        blockHex += h;
    }
    bool found = false;
    for (size_t i = 0; i < tracerMessages().size(); i++) {
        const std::string &m = tracerMessages()[i];
        if (m.compare(0, 9, "Warning: ") == 0) {
            found = true;
            CHECK(m.find("malformed record") != std::string::npos);
            CHECK(m.find("Block: " + blockHex) != std::string::npos);
        }
    }
    CHECK(found);
    // a good block gives no warning
    tracerMessages().clear();
    parse(block(1, rec + rec));
    for (size_t i = 0; i < tracerMessages().size(); i++)
        CHECK(tracerMessages()[i].compare(0, 9, "Warning: ") != 0);
}

TEST_CASE("warning messages longer than 1024 characters are not cut") {
    std::string rec(CAT001_RECORD);
    std::string body;
    for (int i = 0; i < 40; i++)
        body += rec;              // 920 bytes of good records
    body += rec.substr(0, 10);    // and a cut one at the end
    std::vector<unsigned char> data = block(1, body);
    tracerMessages().clear();
    parse(data);
    size_t longest = 0;
    for (size_t i = 0; i < tracerMessages().size(); i++)
        longest = std::max(longest, tracerMessages()[i].size());
    CHECK(longest > 2 * data.size());
}

TEST_CASE("-LF: items with no field in the filter are stepped over, the rest prints as without filter") {
    extern bool gFiltering;
    // a parser of its own: filtering marks the definitions it is applied to
    static AsterixDefinition *def = NULL;
    if (!def) {
        def = new AsterixDefinition();
        loadDefinitions(g_definitionsFile, def);
    }
    InputParser filtered(def);
    REQUIRE(filtered.filterOutItem(1, "010", "SIC"));
    REQUIRE(filtered.filterOutItem(1, "040", "THETA"));
    REQUIRE(filtered.filterOutItem(1, "170", "GHO"));

    std::vector<unsigned char> data = block(1, std::string(CAT001_RECORD) + CAT001_RECORD);
    std::string all = text(parse(data).get(), CAsterixFormat::EJSONX);

    gFiltering = true;
    std::unique_ptr<AsterixData> d(filtered.parsePacket(&data[0], (unsigned int) data.size(), 12.5));
    std::string j = text(d.get(), CAsterixFormat::EJSON);
    std::string jx = text(d.get(), CAsterixFormat::EJSONX);
    std::string txt = text(d.get(), CAsterixFormat::ETxt);
    gFiltering = false;

    // only the 3 items with a field in the filter are made, in both records
    REQUIRE(d->m_lDataBlocks.size() == 1);
    DataBlock *b = d->m_lDataBlocks.front();
    REQUIRE(b->m_lDataRecords.size() == 2);
    for (std::list<DataRecord *>::iterator r = b->m_lDataRecords.begin(); r != b->m_lDataRecords.end(); ++r) {
        CHECK((*r)->m_bFormatOK);
        CHECK_EQ((*r)->m_lDataItems.size(), (size_t) 3);
        CHECK_EQ((*r)->m_nLength, (unsigned long) 23);
    }
    CHECK(j.find("\"CAT001\":{\"I010\":{\"SIC\":201},\"I040\":{\"THETA\":34.5629883},\"I170\":{\"GHO\":0}}}\n")
          != std::string::npos);
    CHECK(j.find("\"id\":2,") != std::string::npos);  // the second record is found after the stepped over items
    CHECK(txt.find("Item 040") != std::string::npos);
    CHECK(txt.find("Item 020") == std::string::npos);
    // -jx: same header, same field text (val, hex, mask, pos) as without filter
    const char *fields[] = {"\"SIC\":{", "\"THETA\":{", "\"GHO\":{"};
    for (size_t i = 0; i < 3; i++) {
        size_t a = all.find(fields[i]);
        REQUIRE(a != std::string::npos);
        std::string field = all.substr(a, all.find('}', a) - a + 1);
        CHECK(jx.find(field) != std::string::npos);
    }
    CHECK_EQ(jx.substr(0, jx.find("\"CAT001\"")), all.substr(0, all.find("\"CAT001\"")));
}
