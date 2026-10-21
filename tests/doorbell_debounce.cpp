#include "DoorbellDebouncer.hpp"
#include <cassert>
#include <cstdint>
int main() {
    DoorbellDebouncer d;
    assert(!d.update(true, 0, 60));
    assert(!d.update(true, 1000, 60)); // held during boot never rings
    assert(!d.update(false, 1010, 60));
    assert(!d.update(false, 1070, 60)); // stable release arms
    assert(!d.update(true, 1100, 60));
    assert(!d.update(false, 1110, 60)); // contact bounce
    assert(!d.update(true, 1120, 60));
    assert(!d.update(true, 1179, 60));
    assert(d.update(true, 1180, 60));
    assert(!d.update(true, 9000, 60)); // long hold only once
    assert(!d.update(false, 9010, 60));
    assert(!d.update(true, 9020, 60));
    assert(!d.update(true, 9100, 60)); // release bounce cannot rearm
    assert(!d.update(false, 9200, 60));
    assert(!d.update(false, 9260, 60));
    assert(!d.update(true, 9300, 60));
    assert(d.update(true, 9360, 60));
    DoorbellDebouncer wrap;
    assert(!wrap.update(false, UINT32_MAX - 100, 60));
    assert(!wrap.update(false, UINT32_MAX - 40, 60));
    assert(!wrap.update(true, UINT32_MAX - 20, 60));
    assert(wrap.update(true, 40, 60)); // millis rollover
}
