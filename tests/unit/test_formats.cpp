/*
 * getLength(pData, nMaxLength) contract of every data item format (Fixed, Variable, Compound,
 * Repetitive, Explicit, BDS), checked on every item of every category in the definitions:
 *   - it never reads past nMaxLength (buffers are allocated with exactly that size: run under
 *     AddressSanitizer to catch over-reads, as the sanitizer CI job does)
 *   - the result is > nMaxLength when the item does not fit; 0 only marks an invalid item (an
 *     Explicit length byte of 0), which DataItem::parse rejects
 *   - the result is never negative
 *   - when it fits, the length depends only on the item's own bytes: the same call on just those
 *     bytes gives the same length
 */
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "minitest.h"
#include "test_support.h"
#include "Category.h"
#include "DataItemDescription.h"
#include "DataItemFormat.h"

namespace {

struct Item {
    int cat;
    std::string id;
    DataItemFormat *format;
};

std::vector<Item> allItems() {
    std::vector<Item> items;
    AsterixDefinition &def = definitions();
    for (int c = 0; c < MAX_CATEGORIES; c++) {
        if (!def.CategoryDefined(c))
            continue;
        Category *cat = def.getCategory(c);
        for (std::list<DataItemDescription *>::iterator it = cat->m_lDataItems.begin();
             it != cat->m_lDataItems.end(); ++it) {
            if ((*it)->m_pFormat) {
                Item item = {c, (*it)->m_strID, (*it)->m_pFormat};
                items.push_back(item);
            }
        }
    }
    return items;
}

long lengthOf(DataItemFormat *format, const unsigned char *data, long n) {
    // exact-size heap copy, so any read past n is an out-of-bounds read
    std::unique_ptr<unsigned char[]> buf(new unsigned char[n > 0 ? n : 1]);
    if (n > 0)
        memcpy(buf.get(), data, n);
    return format->getLength(n > 0 ? buf.get() : buf.get() + 1, n);
}

}  // namespace

TEST_CASE("definitions: every category in the ini file loads, with items") {
    std::vector<Item> items = allItems();
    CHECK(items.size() > 500);
    int categories = 0;
    for (int c = 0; c < MAX_CATEGORIES; c++)
        categories += definitions().CategoryDefined(c) ? 1 : 0;
    CHECK(categories >= 20);
}

TEST_CASE("getLength contract on every item: no over-read, > nMaxLength when it does not fit, prefix property") {
    std::vector<Item> items = allItems();
    REQUIRE(!items.empty());
    const long MAXN = 64;
    unsigned char fills[4][MAXN];
    memset(fills[0], 0x00, MAXN);   // every FX bit clear: items end as soon as possible
    memset(fills[1], 0xFF, MAXN);   // every FX bit set, repetition counts 255: items never fit
    unsigned long x = 1;
    for (long i = 0; i < MAXN; i++) {
        x = x * 6364136223846793005UL + 1442695040888963407UL;
        fills[2][i] = (unsigned char) (x >> 56);
        fills[3][i] = (unsigned char) (i % 2 ? 0x01 : 0x80);
    }
    int checks = 0;
    for (size_t k = 0; k < items.size(); k++) {
        for (int f = 0; f < 4; f++) {
            for (long n = 0; n <= MAXN; n++) {
                long len = lengthOf(items[k].format, fills[f], n);
                checks++;
                if (len < 0) {
                    CHECK_EQ(std::string("CAT") + std::to_string(items[k].cat) + " I" + items[k].id + " length < 0",
                             std::string(""));
                    break;
                }
                if (len == 0)
                    continue;  // invalid item (rejected by DataItem::parse)
                if (len <= n) {
                    // fits: the same bytes alone must give the same length
                    long again = lengthOf(items[k].format, fills[f], len);
                    if (again != len) {
                        CHECK_EQ(std::string("CAT") + std::to_string(items[k].cat) + " I" + items[k].id +
                                 " length " + std::to_string(len) + " with " + std::to_string(n) + " bytes, " +
                                 std::to_string(again) + " with its own bytes", std::string(""));
                    }
                }
            }
        }
    }
    CHECK(checks > 10000);
}

TEST_CASE("getLength: an empty buffer never fits") {
    std::vector<Item> items = allItems();
    unsigned char dummy = 0;
    for (size_t k = 0; k < items.size(); k++) {
        long len = items[k].format->getLength(&dummy + 1, 0);
        if (len <= 0) {
            CHECK_EQ(std::string("CAT") + std::to_string(items[k].cat) + " I" + items[k].id + " fits in 0 bytes",
                     std::string(""));
        }
    }
}
