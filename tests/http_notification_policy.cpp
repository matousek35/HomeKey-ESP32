#include "HttpNotificationPolicy.hpp"
#include <cassert>
#include <string>
using namespace HttpNotificationPolicy;
int main() {
    assert(lockEvent(0, 1) == 1); // unlocked URL
    assert(lockEvent(1, 0) == 2); // locked URL
    assert(lockEvent(1, 1) == -1); // target-only update must not notify
    assert(lockEvent(0, 0) == -1);
    for (uint8_t state : {2, 3, 4, 5, 255}) assert(lockEvent(state, 0) == -1);
    assert(lockEvent(1, 5) == 2); // locking -> locked
    assert(!expired(5000001, 1));
    assert(expired(5000002, 1));
    assert(validUrl(""));
    assert(validUrl("http://192.168.1.2/dev/sps/io/Bell/Pulse"));
    assert(validUrl("https://server.local:443/path?q=1"));
    assert(validUrl("http://[::1]/test"));
    assert(!validUrl("ftp://host/path"));
    assert(!validUrl("http://"));
    assert(!validUrl("http:///path"));
    assert(!validUrl("http://?foo"));
    assert(!validUrl("http://user:password@host/path"));
    assert(!validUrl("http://host/with space"));
    assert(!validUrl("http://host/\r\nHeader: bad"));
    assert(!validUrl("http://host/#fragment"));
    assert(!validUrl("http://host/" + std::string(512, 'x')));
}
