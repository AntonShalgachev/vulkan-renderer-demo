#include "platform/uuid.h"

#include <Rpc.h>

static_assert(sizeof(UUID) == 16, "Unexpected size of the UUID structure");

bool platform::uuid_generate(nstl::span<uint8_t> bytes)
{
    assert(bytes.size() == 16);

    UUID uuid{};

    RPC_STATUS status = UuidCreate(&uuid);
    if (status != RPC_S_OK)
        return false;

    assert(bytes.size() == sizeof(uuid));
    memcpy(bytes.data(), &uuid, sizeof(uuid));

    return true;
}

// bool platform::uuid_from_string(nstl::string_view str, nstl::span<uint8_t> bytes)
// {
//     return false;
// }

nstl::string platform::uuid_to_string(nstl::span<uint8_t const> bytes)
{
    assert(bytes.size() == 16);

    UUID uuid{};
    assert(bytes.size() == sizeof(uuid));
    memcpy(&uuid, bytes.data(), bytes.size());

    RPC_CSTR str;
    RPC_STATUS status = UuidToString(&uuid, &str);
    nstl::string result = reinterpret_cast<char*>(str);
    RpcStringFree(&str);

    if (status != RPC_S_OK)
        return {};

    return result;
}
