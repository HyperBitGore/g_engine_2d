#pragma once

#include <memory>
#include <stdexcept>
#include <string>
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

    struct JSON {
        private:
        std::string elementArrayToString (std::vector<std::unique_ptr<Element>>* elements);
        public:
        std::string label;
        std::unordered_map<std::string, std::unique_ptr<Element>> children;
        template <class T>
        JSONElement<T>* getElement (std::string child) {
            auto it = children.find(child);
            if (it == children.end()) return nullptr;
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
            return reinterpret_cast<JSONElement<T>*>(it->second.get())->value;
        }
        JSONArray* getArray (std::string child) {
            auto* e = getElement<JSONArray>(child);
            return e ? &e->value : nullptr;
        }
        std::string toString ();
        template <class T>
        static T& readElementAs (std::unique_ptr<Element>& e) {
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
                // not copyable — JSON owns unique_ptr children
                JSONFile (const JSONFile&) = delete;
                JSONFile& operator=(const JSONFile&) = delete;
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
                    auto& it = json_data.children[child];
                    return (reinterpret_cast<JSONElement<T>*>(it.get()))->value;
                }
                JSONArray* getArray(std::string label) {
                    return json_data.getArray(label);
                }
        };
        static JSONFile loadJSONFile (std::string path);
        static JSONFile parseJSONFile (std::string data);
    };

}