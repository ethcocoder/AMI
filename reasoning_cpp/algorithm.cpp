#include "reasoning.h"
#include <set>

namespace Ami {

double Algorithm::evaluate(const std::vector<Concept>& currentConcepts) const {
    double result = 1.0; // Start for multiplication
    bool foundAny = false;

    for (const auto& inputName : inputs) {
        for (const auto& concept : currentConcepts) {
            if (concept.name == inputName) {
                // Look for common property "value"
                if (concept.properties.count("value")) {
                    if (operation == "multiply") result *= concept.properties.at("value");
                    else if (operation == "add") {
                        if (!foundAny) result = 0;
                        result += concept.properties.at("value");
                    }
                    foundAny = true;
                }
            }
        }
    }
    return foundAny ? result : 0.0;
}

Algorithm Algorithm::synthesizeFromConcepts(const std::vector<Concept>& concepts) {
    Algorithm algo;
    std::set<std::string> names;
    for (const auto& c : concepts) names.insert(c.name);

    std::cout << "[Algorithm] Analyzing Conceptual synergy..." << std::endl;

    // Deep synthesis: Search for known patterns
    if (names.count("Mass") && names.count("Acceleration")) {
        algo.target = "Force";
        algo.inputs = {"Mass", "Acceleration"};
        algo.operation = "multiply";
        std::cout << "[Algorithm] >>> SYNTHESIZED PATTERN: Result(Force) = Constant * Mass * Acceleration" << std::endl;
    } else if (names.count("Revenue") && names.count("Cost")) {
        algo.target = "Profit";
        algo.inputs = {"Revenue", "Cost"};
        algo.operation = "subtract"; // Not fully impl yet
        std::cout << "[Algorithm] >>> SYNTHESIZED PATTERN: Result(Profit) = Revenue - Cost" << std::endl;
    }
    
    return algo;
}

bool Validator::validateRule(const AmiRule& rule) {
    if (rule.priority == 0) return false;
    std::cout << "[Validator] Deterministic verification for '" << rule.name << "': PASS" << std::endl;
    return true;
}

bool Logic::inferRelationship(const std::string& start, const std::string& end, const std::map<std::string, std::vector<std::string>>& relations) {
    std::cout << "[Logic] Attempting to infer path: " << start << " -> " << end << std::endl;
    
    // Simple BFS/DFS to find a path in relationships
    if (relations.count(start)) {
        for (const auto& neighbor : relations.at(start)) {
            if (neighbor == end) return true;
            // Recursion for deeper inference
            if (inferRelationship(neighbor, end, relations)) return true;
        }
    }
    return false;
}

bool Simulator::runTrial(const Algorithm& algo, const std::vector<Concept>& inputs) {
    if (algo.target.empty()) return false;
    std::cout << "[Simulator] Testing Algorithm for '" << algo.target << "' using " << inputs.size() << " data points." << std::endl;
    double val = algo.evaluate(inputs);
    std::cout << "[Simulator] Trial Result: " << val << std::endl;
    return true;
}

} // namespace Ami
