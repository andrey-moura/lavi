#include "lavi/lang/hash.hpp"

#include <lavi/lang/api.hpp>

lavi::lang::hash::hash(lavi::lang::interpreter* interpreter)
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

    auto ret = lavi::lang::api::call(interpreter, "==", a.key->shared_from_this(), { b.key->shared_from_this() });

    return lavi::lang::api::is_truthy(interpreter, ret);
}

void lavi::lang::hash::set(const std::shared_ptr<object>& key, const std::shared_ptr<object>& value)
{
    auto hash = lavi::lang::api::call(interpreter, "hash", key);

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

std::shared_ptr<lavi::lang::object> lavi::lang::hash::get(const std::shared_ptr<lavi::lang::object>& key) const
{
    auto hash = lavi::lang::api::call(interpreter, "hash", key);

    int hash_value = hash->as<int>();

    hash_key k{ (size_t)hash_value, key.get() };

    auto it = values.find(k);

    if(it != values.end()) {
        return it->second;
    } else {
        return lavi::lang::object::instantiate(interpreter, lavi::lang::null_class);
    }
}

bool lavi::lang::hash::empty() const
{
    return values.empty();
}