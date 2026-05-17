#pragma once

#include <vector>
#include <unordered_map>
#include <memory>

namespace andy
{
    namespace lang
    {
        class object;
        class interpreter;
    }
};
struct hash_key
{
    size_t hash;
    andy::lang::object* key;
};
struct hasher
{
    andy::lang::interpreter* interpreter;
    size_t operator()(const hash_key& k) const;
};
struct equal_hash_key
{
    andy::lang::interpreter* interpreter;
    bool operator()(const hash_key& a, const hash_key& b) const;
};

namespace andy
{
    namespace lang
    {
        class hash
        {
        public:
            /// @brief Creates a hash object.
            /// @param interpreter The interpreter.
            hash(andy::lang::interpreter* interpreter);
        private:
            andy::lang::interpreter* interpreter;
        private:
            std::unordered_map<
                hash_key,
                std::shared_ptr<andy::lang::object>,
                hasher,
                equal_hash_key
            > values;
        public:
            std::vector<std::shared_ptr<andy::lang::object>> keys;

            void set(const std::shared_ptr<object>& key, const std::shared_ptr<andy::lang::object>& value);
            std::shared_ptr<andy::lang::object> get(const std::shared_ptr<andy::lang::object>& key) const;

            bool empty() const;
        };
    }
};