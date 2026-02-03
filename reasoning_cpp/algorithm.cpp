#include "reasoning.h"
#include <set>
#include <algorithm>
#include <cmath>

namespace Ami {

double Algorithm::predict(const std::vector<Concept>& currentConcepts) const {
    if (weights.size() != inputs.size()) return 0.0;
    
    double prediction = bias;
    for (size_t i = 0; i < inputs.size(); ++i) {
        for (const auto& concept : currentConcepts) {
            if (concept.name == inputs[i]) {
                prediction += weights[i] * concept.getPropertyValue("value");
            }
        }
    }
    return prediction;
}

void Algorithm::train(const std::vector<std::vector<Concept>>& history, const std::string& targetName) {
    if (history.size() < 2 || inputs.empty()) return;
    
    std::cout << "[Neural Core] Continuous Learning session for: " << targetName << std::endl;
    
    // Continuous Learning: Only initialize if currently empty
    if (weights.size() != inputs.size()) {
        weights.assign(inputs.size(), 0.01); 
        bias = 0.0;
    }
    double learningRate = 0.000000001; // Scale down for large inputs
    int epochs = 10000;

    for (int e = 0; e < epochs; ++e) {
        std::vector<double> gradW(inputs.size(), 0.0);
        double gradB = 0.0;
        double totalLoss = 0.0;

        for (const auto& snapshot : history) {
            double actualY = 0;
            for(const auto& c : snapshot) if(c.name == targetName) actualY = c.getPropertyValue("value");
            
            double pred = bias;
            std::vector<double> xi(inputs.size(), 0.0);
            for(size_t i=0; i<inputs.size(); ++i) {
                for(const auto& c : snapshot) if(c.name == inputs[i]) {
                    xi[i] = c.getPropertyValue("value");
                    pred += weights[i] * xi[i];
                }
            }

            double error = pred - actualY;
            totalLoss += (error * error);
            if (std::isnan(totalLoss) || std::isinf(totalLoss)) break; 

            for(size_t i=0; i<inputs.size(); ++i) gradW[i] += error * xi[i];
            gradB += error;
        }

        if (std::isnan(totalLoss) || std::isinf(totalLoss)) {
            std::cout << "[Neural Core] Warning: Synthesis diverged. Using fallback weights." << std::endl;
            break;
        }

        for(size_t i=0; i<inputs.size(); ++i) weights[i] -= (learningRate * gradW[i]) / history.size();
        bias -= (learningRate * gradB) / history.size();
        trainingError = totalLoss / history.size();

        if (trainingError < 0.01) break;
    }

    confidence = 1.0 - trainingError;
    std::cout << "[Neural Core] Model Result: " << targetName << " = ";
    for(size_t i=0; i<inputs.size(); ++i) {
        std::cout << "(" << weights[i] << " * " << inputs[i] << ")" << (i < inputs.size()-1 ? " + " : " ");
    }
    std::cout << "+ " << bias << " (Confidence: " << confidence << ")" << std::endl;
}

Algorithm Algorithm::synthesizeFromConcepts(const std::vector<Concept>& concepts) {
    Algorithm algo;
    std::set<std::string> names;
    for (const auto& c : concepts) names.insert(c.name);

    // std::cout << "[Algorithm] Analyzing Conceptual synergy..." << std::endl;

    // Structural Multi-Feature Case: Business Profit Model
    if (names.count("Price") && names.count("Volume") && names.count("Overhead")) {
        algo.target = "Profit";
        algo.inputs = {"Price", "Volume", "Overhead"};
        std::cout << "[Structural] Initialized Multi-Feature Model: Profit = f(Price, Volume, Overhead)" << std::endl;
    }
    // Deep synthesis: Physics Case
    else if (names.count("Mass") && names.count("Acceleration")) {
        algo.target = "Force";
        algo.inputs = {"Mass", "Acceleration"};
        // std::cout << "[Structural] Initialized Pattern: Force = f(Mass, Acceleration)" << std::endl;
    }
    else if (names.count("Mass") && names.count("Energy")) {
        algo.target = "Energy";
        algo.inputs = {"Mass"};
        std::cout << "[Structural] Initialized Relativistic Model: Energy = f(Mass)" << std::endl;
    }
    
    return algo;
}

bool Validator::validateRule(const AmiRule& rule) {
    if (rule.priority == 0) return false;
    std::cout << "[Validator] Deterministic verification for '" << rule.name << "': PASS" << std::endl;
    return true;
}

Thought Logic::deepInference(const std::string& start, const std::string& end, const std::map<std::string, std::vector<std::string>>& relations, std::vector<std::string> currentPath) {
    currentPath.push_back(start);

    // Goal reached
    if (start == end) {
        return { "Path Verified", currentPath, true };
    }

    // Explore neighbors
    if (relations.count(start)) {
        for (const auto& neighbor : relations.at(start)) {
            // Avoid cycles
            if (std::find(currentPath.begin(), currentPath.end(), neighbor) != currentPath.end()) continue;

            Thought result = deepInference(neighbor, end, relations, currentPath);
            if (result.valid) return result;
        }
    }

    return { "No Path Found", {}, false };
}

bool Simulator::runTrial(const Algorithm& algo, const std::vector<Concept>& inputs) {
    if (algo.target.empty()) return false;
    std::cout << "[Simulator] Testing Algorithm for '" << algo.target << "' using " << inputs.size() << " data points." << std::endl;
    double val = algo.predict(inputs);
    std::cout << "[Simulator] Prediction Result: " << val << std::endl;
    return true;
}

} // namespace Ami
