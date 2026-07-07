#include "json.hpp"
#include <cctype>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

gore::JSON::JSON () {
    this->label = "NULL";
}

gore::JSONArray gore::JSON::copyJSONArray (gore::JSONArray& array) {
    gore::JSONArray result;
    result.reserve(array.size());
    for (const auto& elem : array) {
        switch (elem->type) {
            case gore::JSONTYPE::STRING: {
                std::string value = gore::JSON::readElementAs<std::string>(elem);
                result.push_back(std::make_unique<gore::JSONElement<std::string>>(gore::JSONTYPE::STRING, value));
                break;
            }
            case gore::JSONTYPE::OBJECT: {
                gore::JSON value = gore::JSON::readElementAs<gore::JSON>(elem);
                result.push_back(std::make_unique<gore::JSONElement<gore::JSON>>(gore::JSONTYPE::OBJECT, std::move(value)));
                break;
            }
            case gore::JSONTYPE::ARRAY: {
                gore::JSONArray& nested = gore::JSON::readElementAs<gore::JSONArray>(elem);
                gore::JSONArray copied = copyJSONArray(nested);
                result.push_back(std::make_unique<gore::JSONElement<gore::JSONArray>>(gore::JSONTYPE::ARRAY, std::move(copied)));
                break;
            }
            case gore::JSONTYPE::INTEGER: {
                int value = gore::JSON::readElementAs<int>(elem);
                result.push_back(std::make_unique<gore::JSONElement<int>>(gore::JSONTYPE::INTEGER, value));
                break;
            }
            case gore::JSONTYPE::FLOAT: {
                float value = gore::JSON::readElementAs<float>(elem);
                result.push_back(std::make_unique<gore::JSONElement<float>>(gore::JSONTYPE::FLOAT, value));
                break;
            }
        }
    }
    return result;
}
// copy
gore::JSON::JSON (const JSON& j) {
    this->label = j.label;
    for (auto& i : j.children) {
        switch (i.second->type) {
        case JSONTYPE::STRING:
        {
            std::string value = JSON::readElementAs<std::string>(i.second);
            std::unique_ptr<JSONElement<std::string>> elem = std::make_unique<JSONElement<std::string>>(JSONTYPE::STRING, value);
            this->children.emplace(i.first, std::move(elem));
        }
            break;
        case JSONTYPE::OBJECT:
        {
            JSON value = JSON::readElementAs<JSON>(i.second);
            std::unique_ptr<JSONElement<JSON>> elem = std::make_unique<JSONElement<JSON>>(JSONTYPE::OBJECT, value);
            this->children.emplace(i.first, std::move(elem));
        }
            break;
        case JSONTYPE::ARRAY:
        {
            JSONArray& value = JSON::readElementAs<JSONArray>(i.second);
            JSONArray v2 = copyJSONArray(value);
            std::unique_ptr<JSONElement<JSONArray>> elem = std::make_unique<JSONElement<JSONArray>>(JSONTYPE::ARRAY, std::move(v2));
            this->children.emplace(i.first, std::move(elem));
        }
            break;
        case JSONTYPE::INTEGER:
        {
            int& value = JSON::readElementAs<int>(i.second);
            std::unique_ptr<JSONElement<int>> elem = std::make_unique<JSONElement<int>>(JSONTYPE::INTEGER, value);
            this->children.emplace(i.first, std::move(elem));
        }
        break;
        case JSONTYPE::FLOAT:
        {
            float& value = JSON::readElementAs<float>(i.second);
            std::unique_ptr<JSONElement<float>> elem = std::make_unique<JSONElement<float>>(JSONTYPE::FLOAT, value);
            this->children.emplace(i.first, std::move(elem));
        }
            break;
        }
    }
}
gore::JSON& gore::JSON::operator=(const JSON& j) {
    *this = JSON(j);
    return *this;
}
// move
gore::JSON::JSON (JSON&& j) noexcept {
    this->label = std::move(j.label);
    this->children = std::move(j.children);
}
gore::JSON& gore::JSON::operator=(JSON&& j) noexcept {
    this->label = std::move(j.label);
    this->children = std::move(j.children);
    return *this;
}


std::string gore::JSON::elementArrayToString (std::vector<std::unique_ptr<Element>>* elements) {
    std::string out;
    out.append(": [\n");
    for (const auto& arr_elem : *elements) {
        switch (arr_elem->type) {
            case JSONTYPE::STRING:
                {
                    auto str_elem = dynamic_cast<JSONElement<std::string>*>(arr_elem.get());
                    if (str_elem) out.append("  \"" + str_elem->value + "\",\n");
                }
                break;
            case JSONTYPE::OBJECT:
                {
                    auto obj_elem = dynamic_cast<JSONElement<JSON>*>(arr_elem.get());
                    if (obj_elem) out.append("  " + obj_elem->value.toString() + "\n");
                }
                break;
            case JSONTYPE::ARRAY:
            {
                auto array_elem = dynamic_cast<std::vector<std::unique_ptr<Element>>*>(arr_elem.get());
                out.append(elementArrayToString(array_elem));
            }
                break;
            case JSONTYPE::INTEGER:
            {
                auto str_elem = dynamic_cast<JSONElement<int>*>(arr_elem.get());
                if (str_elem) out.append("  \"" + std::to_string(str_elem->value) + "\",\n");
            }
                break;
            case JSONTYPE::FLOAT:
            {
                auto str_elem = dynamic_cast<JSONElement<float>*>(arr_elem.get());
                if (str_elem) out.append("  \"" + std::to_string(str_elem->value) + "\",\n");
            }
              break;
            }
    }
    out.append("]");
    
    return out;
}


std::string gore::JSON::toString () {
    std::string out = label + ": {\n";
    for (auto& i : children) {
        if (i.first == "NULL") {
            out.push_back(' ');
        } else {
            out.append(i.first);
        }
        switch (i.second->type) {
            case JSONTYPE::STRING:
                {
                    auto elem = getElement<std::string>(i.first);
                    if (elem) out.append(": " + elem->value + "\n");
                }
            break;
            case JSONTYPE::OBJECT:
                {
                    auto elem = getElement<JSON>(i.first);
                    if (elem) out.append(": " + elem->value.toString() + "\n");
                }
                break;
            case JSONTYPE::ARRAY:
                {
                    auto elem = getElement<std::vector<std::unique_ptr<Element>>>(i.first);
                    if (elem) {
                        out.append(elementArrayToString(&(elem->value)) + "\n");
                    }
                }
              break;
            case JSONTYPE::INTEGER:
            {
                auto elem = getElement<int>(i.first);
                if (elem) out.append(": " + std::to_string(elem->value) + "\n");
            }
                break;
            case JSONTYPE::FLOAT:
            {
                auto elem = getElement<float>(i.first);
                if (elem) out.append(": " + std::to_string(elem->value) + "\n");
            }
                  break;
            }
    }

    out.push_back('}');
    return out;
}


std::string trimQuotesAndWhitespace(const std::string& str) {
    size_t start = 0, end = str.size();
    
    // Trim leading whitespace
    while (start < end && std::isspace(str[start])) start++;
    // Trim trailing whitespace
    while (end > start && std::isspace(str[end - 1])) end--;
    
    // Trim quotes
    if (start < end && str[start] == '"') start++;
    if (end > start && str[end - 1] == '"') end--;
    
    return str.substr(start, end - start);
}

std::string readBrackets (std::string section, size_t* offset) {
    size_t i = *offset + 1; // assuming we starting on the bracket
    std::string new_section;
    int depth = 1;
    for (; i < section.size() && depth > 0; i++) {
        switch (section[i]) {
            case '{':
            depth++;
            break;
            case '}':
            depth--;
            break;
        }
        if (depth > 0) {
            new_section.push_back(section[i]);
        }
    }
    i++;
    *offset = i;
    return new_section;
}

std::vector<std::unique_ptr<gore::Element>> gore::JSONLoader::readArray (std::string section, size_t* offset) {
    size_t i = *offset + 1;
    int depth = 1;
    std::string thing;
    std::vector<std::unique_ptr<Element>> items;
    for (; i < section.size() && depth > 0; ) {
        switch (section[i]) {
            case '[':
                // should probably parse this in recursive way
                depth++;
                i++;
            break;
            case ']':
                depth--;
                i++;
            break;
            case '{':
                {
                    // process the JSON object
                    std::string new_section = readBrackets(section, &i);
                    JSON child = parseJSONSection(new_section);
                    std::unique_ptr<JSONElement<JSON>> element = std::make_unique<JSONElement<JSON>>(JSONTYPE::OBJECT, std::move(child)); 
                    items.push_back(std::move(element));
                }
            break;
            case ',':
            case '\n':
                {
                    if (!thing.empty()) {
                        std::string trimmed = trimQuotesAndWhitespace(thing);
                        if (!trimmed.empty()) {
                            std::unique_ptr<JSONElement<std::string>> element = std::make_unique<JSONElement<std::string>>(JSONTYPE::STRING, trimmed); 
                            items.push_back(std::move(element));
                        }
                    }
                    thing.clear();
                    i++;
                }
            break;
            default:
                {
                    // combine until newline or ,
                    if (!std::isspace(section[i])) {
                        thing.push_back(section[i]);
                    }
                    i++;
                }
            break;
        }
    }                    
    *offset = i;
    return items;
}

gore::JSON gore::JSONLoader::parseJSONSection (std::string section) {
    size_t offset = 0;
    JSON output;
    size_t free = 0;
    for (; offset < section.size();) {
        switch (section[offset]) {
            case '"':
                {
                    size_t i = offset + 1;
                    std::string label;
                    for (; i < section.size() && section[i] != '"'; i++) {
                        label.push_back(section[i]);
                    }
                    i++;
                    // now read the data ahead of it
                    // read until we hit a non white or :
                    for (; i < section.size() && (std::isspace(section[i]) || section[i] == ':'); i++);
                    if (section[i] == '{') {
                        // recursive json parse
                        std::string new_section = readBrackets(section, &i);
                        JSON child = parseJSONSection(new_section);
                        child.label = label;
                        std::unique_ptr<JSONElement<JSON>> element = std::make_unique<JSONElement<JSON>>(JSONTYPE::OBJECT, std::move(child));
                        output.children.emplace(label, std::move(element));
                    } else if (section[i] == '[') {
                        std::vector<std::unique_ptr<Element>> elements = readArray(section, &i);
                        std::unique_ptr<JSONElement<std::vector<std::unique_ptr<Element>>>> element = std::make_unique<JSONElement<std::vector<std::unique_ptr<Element>>>>(JSONTYPE::ARRAY, std::move(elements));
                        output.children.emplace(label, std::move(element));
                    } else {
                        // read until newline or ,
                        std::string value;
                        for (; i < section.size() && (section[i] != ',') && section[i] != '\n'; i++) {
                            value.push_back(section[i]);
                        }
                        i++;
                        std::string trimmed = trimQuotesAndWhitespace(value);
                        if (!trimmed.empty()) {
                            // loop string and check if digits
                            bool isInt = true;
                            bool isFloat = false;
                            for (auto& i : trimmed) {
                                if (i == '.') {
                                    isFloat = true;
                                    isInt = false;
                                } else if (!std::isdigit(i) && i != '-') {
                                    isInt = false;
                                    isFloat = false;
                                    break;
                                }
                            }
                            if (isInt) {
                                int value = -1;
                                try {
                                    value = std::stoi(trimmed);
                                } catch (const std::invalid_argument& e) {
                                    std::cerr << "Invalid format! No conversion could be done.\n";
                                } catch (const std::out_of_range& e) {
                                    std::cerr << "Value is out of range for an int!\n";
                                }
                                std::unique_ptr<JSONElement<int>> element = std::make_unique<JSONElement<int>>(JSONTYPE::INTEGER, value);
                                output.children.emplace(label, std::move(element));
                            } else if (isFloat) {
                                float value = -1.0;
                                try {
                                    value = std::stof(trimmed);
                                } catch (const std::invalid_argument& e) {
                                    std::cerr << "Invalid format! No conversion could be done.\n";
                                } catch (const std::out_of_range& e) {
                                    std::cerr << "Value is out of range for an int!\n";
                                }
                                std::unique_ptr<JSONElement<float>> element = std::make_unique<JSONElement<float>>(JSONTYPE::FLOAT, value);
                                output.children.emplace(label, std::move(element));
                            } else {
                                std::unique_ptr<JSONElement<std::string>> element = std::make_unique<JSONElement<std::string>>(JSONTYPE::STRING, trimmed);
                                output.children.emplace(label, std::move(element));
                            }
                        }
                    }
                    offset = i;

                }
            break;
            case '[':
                {
                    std::vector<std::unique_ptr<Element>> elements = readArray(section, &offset);
                }
            break;
            case '{':
                {
                    std::string new_section = readBrackets(section, &offset);
                    JSON child = parseJSONSection(new_section);
                    return child;
                }
            default:
                offset++;
        }
    }
    return output;
}

// process JSON label

gore::JSONLoader::JSONFile gore::JSONLoader::parseJSONFile (std::string data) {
    JSON json = parseJSONSection(data);
    return JSONFile(std::move(json));
}

gore::JSONLoader::JSONFile gore::JSONLoader::loadJSONFile (std::string path) {
    std::stringstream ss;
    std::ifstream f;
    f.open(path);
    ss << f.rdbuf();
    f.close();
    std::string str = ss.str();
    return parseJSONFile(str);
}