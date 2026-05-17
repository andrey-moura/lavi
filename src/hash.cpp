#include "andy/lang/hash.hpp"

#include <andy/lang/api.hpp>

andy::lang::hash::hash(andy::lang::interpreter* interpreter)
    : interpreter(interpreter),
      values(0, hasher{interpreter}, equal_hash_key{interpreter})
{
    int i = 0;
    i++;
}

size_t hasher::operator()(const hash_key& k) const
{
    return k.hash;
}

auto equal_hash_key::operator()(const hash_key& a, const hash_key& b) const -> bool
{
    if(a.hash != b.hash) {
        return false;
    }

    auto ret = andy::lang::api::call(interpreter, "==", a.key->shared_from_this(), { b.key->shared_from_this() });

    return andy::lang::api::is_truthy(interpreter, ret);
}

void andy::lang::hash::set(const std::shared_ptr<object>& key, const std::shared_ptr<object>& value)
{
    auto hash = andy::lang::api::call(interpreter, "hash", key);

    int hash_value = hash->as<int>();

    hash_key k{ (size_t)hash_value, key.get() };

    auto it = values.find(k);

    if(it != values.end()) {
        it->second = value;
    } else {
        keys.push_back(key);
        values[k] = value;
    }
}

std::shared_ptr<andy::lang::object> andy::lang::hash::get(const std::shared_ptr<andy::lang::object>& key) const
{
    auto hash = andy::lang::api::call(interpreter, "hash", key);

    int hash_value = hash->as<int>();

    hash_key k{ (size_t)hash_value, key.get() };

    auto it = values.find(k);

    if(it != values.end()) {
        return it->second;
    } else {
        return andy::lang::object::instantiate(interpreter, interpreter->NullClass);
    }
}

bool andy::lang::hash::empty() const
{
    return values.empty();
}