#include "reasoning.h"

namespace Ami {

Concept::Concept(std::string name) : name(name) {}

void Concept::addAttribute(std::string key, std::string value) {
    attributes[key] = value;
}

void Concept::setProperty(std::string key, double value) {
    properties[key] = value;
}

double Concept::getPropertyValue(std::string key) const {
    if (properties.count(key)) return properties.at(key);
    return 0.0;
}

void Concept::addRelationship(std::string targetConceptName) {
    relatedConcepts.push_back(targetConceptName);
}

void Concept::display() const {
    std::cout << "Concept: " << name << std::endl;
    for (const auto& [key, val] : attributes) {
        std::cout << "  - [Attr] " << key << ": " << val << std::endl;
    }
    for (const auto& [key, val] : properties) {
        std::cout << "  - [Prop] " << key << ": " << val << std::endl;
    }
    for (const auto& rel : relatedConcepts) {
        std::cout << "  - [Relates to] " << rel << std::endl;
    }
}

} // namespace Ami
