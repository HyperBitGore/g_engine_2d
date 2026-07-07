#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

// TODO JSON data access
// TODO Just use JSON instead of JSONFile?

namespace gore {
    enum class JSONTYPE { STRING, OBJECT, ARRAY, INTEGER, FLOAT };
    struct Element {
        JSONTYPE type;
        Element(JSONTYPE t) : type(t) {}
        virtual ~Element() = default;
    };

    // either a string or JSON struct
    template <class T>
    struct JSONElement : public Element {
        T value;
        JSONElement(JSONTYPE t, T v) : Element{t}, value(std::move(v)) {}
    };

    typedef std::vector<std::unique_ptr<Element>> JSONArray;

    // Maps T to its expected JSONTYPE; static_assert fires at compile time for unrecognised T.
    template <class T> struct json_type_for {
        static_assert(sizeof(T) == 0, "getElement<T>: T is not a recognised JSON scalar type");
    };
    template <> struct json_type_for<int>         { static constexpr JSONTYPE value = JSONTYPE::INTEGER; };
    template <> struct json_type_for<float>        { static constexpr JSONTYPE value = JSONTYPE::FLOAT;   };
    template <> struct json_type_for<std::string>  { static constexpr JSONTYPE value = JSONTYPE::STRING;  };

    class JSON {
        private:
        std::string elementArrayToString (std::vector<std::unique_ptr<Element>>* elements);
        static JSONArray copyJSONArray (JSONArray& array);
        public:
        JSON ();
        // copy
        JSON (const JSON& j);
        JSON& operator=(const JSON& j);
        // move
        JSON (JSON&& j) noexcept;
        JSON& operator=(JSON&& j) noexcept;
        std::string label;
        std::unordered_map<std::string, std::unique_ptr<Element>> children;
        template <class T>
        JSONElement<T>* getElement (std::string child) {
            auto it = children.find(child);
            if (it == children.end()) return nullptr;
            // For scalar types (int, float, string): validate stored type matches T at runtime.
            if constexpr (std::is_same_v<T, int> || std::is_same_v<T, float> || std::is_same_v<T, std::string>) {
                if (it->second->type != json_type_for<T>::value)
                    throw std::runtime_error("JSON type mismatch on key '" + child + "'");
            }
            return reinterpret_cast<JSONElement<T>*>(it->second.get());
        }
        template <class T>
        JSONElement<T>* operator[](std::string child) {
            return getElement<T>(child);
        }
        template <class T>
        T getElementValue (std::string child) {
            auto it = children.find(child);
            if (it == children.end()) return T{};
            if constexpr (std::is_same_v<T, int> || std::is_same_v<T, float> || std::is_same_v<T, std::string>) {
                if (it->second->type != json_type_for<T>::value)
                    throw std::runtime_error("JSON type mismatch on key '" + child + "'");
            }
            return reinterpret_cast<JSONElement<T>*>(it->second.get())->value;
        }
        JSONArray* getArray (std::string child) {
            auto* e = getElement<JSONArray>(child);
            return e ? &e->value : nullptr;
        }
        std::string toString ();
        template <class T>
        static T& readElementAs (std::unique_ptr<Element>& e) {
            if constexpr (std::is_same_v<T, int> || std::is_same_v<T, float> || std::is_same_v<T, std::string>) {
                if (e->type != json_type_for<T>::value)
                    throw std::runtime_error("JSON type mismatch in readElementAs");
            }
            return reinterpret_cast<JSONElement<T>*>(e.get())->value;
        }
        template <class T>
        static T& readElementAs (const std::unique_ptr<Element>& e) {
            if constexpr (std::is_same_v<T, int> || std::is_same_v<T, float> || std::is_same_v<T, std::string>) {
                if (e->type != json_type_for<T>::value)
                    throw std::runtime_error("JSON type mismatch in readElementAs");
            }
            return reinterpret_cast<JSONElement<T>*>(e.get())->value;
        }
    };

    class JSONLoader {
        private:
            static JSON parseJSONSection (std::string section);
            static std::vector<std::unique_ptr<Element>> readArray (std::string section, size_t* offset);
        public:
        JSONLoader () = delete;
        // wraps JSON data so you don't have to do annoying ass ptr casting manually
        class JSONFile {
            private:
                JSON json_data;
            public:
                JSONFile (JSON&& json) : json_data(std::move(json)) {}
                // copying values in children
                JSONFile (const JSONFile& f) {
                    this->json_data.label = f.json_data.label;
                    this->json_data = f.json_data;
                }
                JSONFile& operator=(const JSONFile& f) {
                    *this = JSONFile(f);
                    return *this;
                }
                // move
                JSONFile (JSONFile&& f) noexcept {
                    this->json_data.label = std::move(f.json_data.label);
                    this->json_data.children = std::move(f.json_data.children);
                }
                JSONFile& operator=(JSONFile&& f) noexcept {
                    this->json_data.label = std::move(f.json_data.label);
                    this->json_data.children = std::move(f.json_data.children);
                    return *this;
                }
                // label path format "<label>/<label>/<label>" 
                template <class T>
                T& getLabel (std::string label_path) {
                    std::vector<std::string> parts;
                    std::string part;
                    for (char c : label_path) {
                        if (c == '/') {
                            if (!part.empty()) { parts.push_back(part); part.clear(); }
                        } else {
                            part.push_back(c);
                        }
                    }
                    if (!part.empty()) parts.push_back(part);
                    if (parts.empty()) throw std::runtime_error("Empty JSON label path");

                    JSON* current = &json_data;
                    for (size_t i = 0; i + 1 < parts.size(); i++) {
                        auto it = current->children.find(parts[i]);
                        if (it == current->children.end())
                            throw std::runtime_error("JSON label not found: " + parts[i]);
                        current = &(reinterpret_cast<JSONElement<JSON>*>(it->second.get())->value);
                    }
                    auto it = current->children.find(parts.back());
                    if (it == current->children.end())
                        throw std::runtime_error("JSON label not found: " + parts.back());
                    return reinterpret_cast<JSONElement<T>*>(it->second.get())->value;
                }
                template <class T>
                T& operator[](std::string child) {
                    auto it = json_data.children.find(child);
                    if (it == json_data.children.end())
                        throw std::runtime_error("JSON key not found: " + child);
                    return (reinterpret_cast<JSONElement<T>*>(it->second.get()))->value;
                }
                JSONArray* getArray(std::string label) {
                    return json_data.getArray(label);
                }
        };
        static JSONFile loadJSONFile (std::string path);
        static JSONFile parseJSONFile (std::string data);
    };

}