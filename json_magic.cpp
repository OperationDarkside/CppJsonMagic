#include <string>
#include <string_view>
#include <meta>
#include <type_traits>

class json_magic
{
public:
    template <typename T>
    static constexpr auto to_string(const T &t)
    {
        return serialize_value(t);
    }

    template <typename T>
    static T from_string(std::string_view json)
    {
        T obj{};
        deserialize_value(obj, json);
        return obj;
    }

    // Written by Gemini 3 Pro
    /*
    template <typename T>
    static T from_string(std::string_view json)
    {
        T obj{}; // Default construct the object

        // Reflection Setup
        constexpr static auto ctx = std::meta::access_context::current();
        constexpr static auto class_members = std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx));
        constexpr static auto class_members_indices = make_indices_array<class_members.size()>();

        // Compile-time iteration over members
        template for (constexpr auto i : class_members_indices)
        {
            constexpr auto member = class_members[i];
            constexpr auto member_name = std::meta::identifier_of(member);
            constexpr auto member_typeinfo = std::meta::type_of(member);
            constexpr auto true_type = std::meta::dealias(std::meta::remove_cvref(member_typeinfo));

            // Runtime: Find the raw string value for this key in the JSON
            std::string_view raw_value = extract_json_value(json, member_name);

            if (!raw_value.empty())
            {
                // Splicing [: info :] lets us use standard C++ traits
                using MemberT = [:true_type:];

                if constexpr (std::is_same_v<MemberT, std::string>)
                {
                    // Strip quotes for strings
                    if (raw_value.size() >= 2)
                    {
                        obj.[:member:] = std::string(raw_value.substr(1, raw_value.size() - 2));
                    }
                }
                else if constexpr (std::is_arithmetic_v<MemberT>)
                {
                    // Parse numbers
                    std::from_chars(raw_value.data(), raw_value.data() + raw_value.size(), obj.[:member:]);
                }
                else
                {
                    // TODO: Recursive objects, vectors, etc.
                }
            }
        }

        return obj;
    }
    */

private:
    template <std::size_t N>
    consteval static std::array<std::size_t, N> make_indices_array()
    {
        std::array<std::size_t, N> a{};
        for (std::size_t i = 0; i < N; i++)
        {
            a[i] = i;
        }
        return a;
    }

    // Partially written by Gemini 3 Flash thinking
    // 1. Logic for serializing an Object (Class/Struct)
    template <typename T>
    static std::string serialize_object(const T &t)
    {
        std::string res = "{";

        constexpr static auto ctx = std::meta::access_context::current();
        constexpr static auto class_members = std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx));

        bool first = true;
        constexpr static auto class_members_indices = make_indices_array<class_members.size()>();
        template for (constexpr auto i : class_members_indices)
        {
            if (!first)
                res += ",";

            constexpr auto member = class_members[i];
            constexpr auto name = std::meta::identifier_of(member);
            res += "\"";
            res += name;
            res += "\":";

            // Recursive call: handles strings, ints, vectors, or nested structs
            res += serialize_value(t.[:member:]);
            first = false;
        }

        res += "}";
        return res;
    }

    // Written by Gemini 3 Flash thinking
    // 2. Main Dispatcher (The "serialize_value" function)
    template <typename T>
    static std::string serialize_value(const T &v)
    {
        using RawT = std::remove_cvref_t<T>;

        // Case A: Strings (Must come before Range check!)
        if constexpr (std::convertible_to<RawT, std::string_view>)
        {
            return "\"" + std::string(v) + "\"";
        }
        // Case B: Arithmetic (ints, floats, bools)
        else if constexpr (std::is_arithmetic_v<RawT>)
        {
            if constexpr (std::is_same_v<RawT, bool>)
                return v ? "true" : "false";
            return std::to_string(v);
        }
        // Case C: Ranges (vector, array, list, etc.)
        else if constexpr (std::ranges::range<RawT>)
        {
            std::string res = "[";
            bool first = true;
            for (const auto &item : v)
            {
                if (!first)
                    res += ",";
                res += serialize_value(item); // Recurse for elements
                first = false;
            }
            res += "]";
            return res;
        }
        // Case D: User-Defined Types (Classes/Structs)
        else if constexpr (std::is_class_v<RawT>)
        {
            return serialize_object(v); // Recurse for the object
        }
        else
        {
            return "\"unknown type\"";
        }
    }

    // Written by Gemini 3 Pro
    // Simple helper to find "key": value in JSON string (Demonstration only, not robust)
    static std::string_view extract_json_value(std::string_view json, std::string_view key)
    {
        // 1. Find "key"
        std::string search_key = "\"" + std::string(key) + "\"";
        size_t key_pos = json.find(search_key);
        if (key_pos == std::string_view::npos)
            return {};

        // 2. Find colon
        size_t colon_pos = json.find(':', key_pos + search_key.size());
        if (colon_pos == std::string_view::npos)
            return {};

        // 3. Find value start (skip whitespace)
        size_t val_start = json.find_first_not_of(" \t\n\r", colon_pos + 1);
        if (val_start == std::string_view::npos)
            return {};

        // 4. Find value end
        size_t val_end = std::string_view::npos;

        if (json[val_start] == '"')
        {
            // It's a string, find closing quote (ignoring escaped quotes for simplicity)
            val_end = json.find('"', val_start + 1);
            if (val_end != std::string_view::npos)
                val_end += 1; // Include the closing quote
        }
        else
        {
            // It's a primitive, read until comma or closing brace
            val_end = json.find_first_of(",}", val_start);
        }

        if (val_end == std::string_view::npos)
            return {};

        return json.substr(val_start, val_end - val_start);
    }

    // --- Helper: Find closing bracket/brace taking nesting into account ---
    static size_t find_closing(std::string_view json, char open, char close)
    {
        int depth = 0;
        for (size_t i = 0; i < json.size(); ++i)
        {
            if (json[i] == open)
                depth++;
            else if (json[i] == close)
            {
                if (--depth == 0)
                    return i;
            }
        }
        return std::string_view::npos;
    }

    // --- 1. Object Deserialization (Recursive) ---
    template <typename T>
    static T deserialize_object(std::string_view json)
    {
        T obj{};
        constexpr static auto ctx = std::meta::access_context::current();
        constexpr static auto class_members = std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx));
        constexpr static auto class_members_indices = make_indices_array<class_members.size()>();

        template for (constexpr auto i : class_members_indices)
        {
            constexpr auto member = class_members[i];
            constexpr auto member_name = std::meta::identifier_of(member);

            // Use the helper you provided (or an improved version) to find the field
            std::string_view raw_value = extract_json_value(json, member_name);

            if (!raw_value.empty())
            {
                deserialize_value(obj.[:member:], raw_value);
            }
        }
        return obj;
    }

    // --- 2. Range/Array Deserialization ---
    template <typename T>
    static void deserialize_range(T &container, std::string_view json)
    {
        // Basic JSON array parsing: trim [ and ]
        if (json.front() != '[' || json.back() != ']')
            return;
        std::string_view content = json.substr(1, json.size() - 2);

        size_t start = 0;
        size_t element_index = 0;
        while (start < content.size())
        {
            // Skip whitespace/commas
            start = content.find_first_not_of(" \t\n\r,", start);
            if (start == std::string_view::npos)
                break;

            // Determine end of current element (handle nested objects/arrays)
            size_t end;
            if (content[start] == '{')
                end = start + find_closing(content.substr(start), '{', '}') + 1;
            else if (content[start] == '[')
                end = start + find_closing(content.substr(start), '[', ']') + 1;
            else
                end = content.find_first_of(", ", start);

            if (end == std::string_view::npos)
                end = content.size();

            std::string_view element_raw = content.substr(start, end - start);

            // Add element to container
            using ValueT = typename T::value_type;
            if constexpr (requires { container.push_back(std::declval<ValueT>()); })
            {
                ValueT val{};
                deserialize_value(val, element_raw);
                container.push_back(std::move(val));
            }
            else
            {
                // Fixed-size containers: std::array, raw arrays, etc.
                if (element_index < std::size(container))
                {
                    auto it = std::begin(container);
                    std::advance(it, element_index);
                    deserialize_value(*it, element_raw);
                    element_index++;
                }
            }

            start = end;
        }
    }

    // --- 3. Main Dispatcher ---
    template <typename T>
    static void deserialize_value(T &obj, std::string_view json)
    {
        using RawT = std::remove_cvref_t<T>;

        // Case A: Strings
        if constexpr (std::is_same_v<RawT, std::string>)
        {
            if (json.size() >= 2 && json.front() == '"')
            {
                obj = std::string(json.substr(1, json.size() - 2));
            }
        }
        // Case B: Arithmetic
        else if constexpr (std::is_arithmetic_v<RawT>)
        {
            if constexpr (std::is_same_v<RawT, bool>)
            {
                obj = (json == "true");
            }
            else
            {
                std::from_chars(json.data(), json.data() + json.size(), obj);
            }
        }
        // Case C: Ranges (vector, list, array)
        else if constexpr (std::ranges::range<RawT>)
        {
            deserialize_range(obj, json);
        }
        // Case D: Nested Structs/Classes
        else if constexpr (std::is_class_v<RawT>)
        {
            obj = deserialize_object<RawT>(json);
        }
    }
};