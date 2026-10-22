#include "MessageProtocol.hpp"
#include <cassert>
using namespace MessageProtocol;
int main() {
    const std::array<uint8_t, 6> states{1,2,3,4,254,25};
    auto state = decode(Command::State, "1", states);
    assert(state && state->operation == Operation::Override && state->current == 1 && state->target == 1);
    auto current = decode(Command::Current, "0", states);
    assert(current && current->operation == Operation::Current && current->target == 6);
    auto target = decode(Command::Target, "0", states);
    assert(target && target->operation == Operation::Target && target->current == 6);
    assert(decode(Command::Current, "3", states));
    assert(!decode(Command::Current, "4", states));
    assert(!decode(Command::State, "2", states));
    assert(!decode(Command::Target, "255", states));
    assert(decode(Command::Battery, "100", states));
    assert(!decode(Command::Battery, "101", states));
    for (auto bad : {"", "-1", "+1", "1x", "1.0", " 1", "1\n", "256", "9999999"})
        assert(!decode(Command::State, bad, states));
    for (size_t i = 0; i < states.size(); ++i) {
        auto request = decode(Command::Custom, std::to_string(states[i]), states);
        assert(request);
        if (i == 1 || i == 3) assert(request->operation == Operation::Target);
        else assert(request->operation == Operation::Override);
    }
    assert(!decode(Command::Custom, "99", states));
    assert(stateValue(1,1) == 1 && stateValue(0,0) == 0);
    assert(stateValue(1,0) == 4 && stateValue(0,1) == 5);
    std::string decoded;
    const std::string payload = "{\"uid\":\"A&B\",\"text\":\"{topic}+%/\"}";
    auto encoded = encode(payload);
    assert(decodeUrl(encoded, decoded) && decoded == payload);
    assert(decodeUrl("ESP_123%2Fhomekit%2Fset_state", decoded) && decoded == "ESP_123/homekit/set_state");
    for (auto bad : {"%", "%0", "%GG", "%00", "%0D%0A"}) assert(!decodeUrl(bad, decoded));
    assert(expand("http://host/?topic={topic}&payload={payload}", "a/b", "{topic}") ==
           "http://host/?topic=a%2Fb&payload=%7Btopic%7D");
}
