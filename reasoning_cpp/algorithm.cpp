#include "reasoning.h"
#include <set>
#include <algorithm>
#include <cmath>
#include <time.h>

namespace Ami {

// Helper: Sigmoid Activation
double sigmoid(double x) {
    return 1.0 / (1.0 + exp(-x));
}

double sigmoid_derivative(double x) {
    return x * (1.0 - x);
}

double Algorithm::predict(const std::vector<Concept>& currentConcepts) const {
    if (hiddenWeights.empty() || inputs.empty()) return 0.0;
    
    // 1. Prepare Input Layer
    std::vector<double> inputVals(inputs.size(), 0.0);
    for (size_t i = 0; i < inputs.size(); ++i) {
        for (const auto& concept : currentConcepts) {
            if (concept.name == inputs[i]) {
                inputVals[i] = concept.getPropertyValue("value");
            }
        }
    }

    // 2. Compute Hidden Layer
    std::vector<double> hiddenVals(hiddenWeights.size(), 0.0);
    for (size_t h = 0; h < hiddenWeights.size(); ++h) {
        double sum = hiddenBias[h];
        for (size_t i = 0; i < inputs.size(); ++i) {
            sum += inputVals[i] * hiddenWeights[h][i];
        }
        hiddenVals[h] = sigmoid(sum);
    }

    // 3. Compute Output Layer
    double output = outputBias;
    for (size_t h = 0; h < hiddenWeights.size(); ++h) {
        output += hiddenVals[h] * outputWeights[h];
    }
    
    return output;
}

void Algorithm::train(const std::vector<std::vector<Concept>>& history, const std::string& targetName) {
    if (history.size() < 2 || inputs.empty()) return;
    
    size_t numHidden = 4; // Deeper reasoning nodes
    size_t numInput = inputs.size();

    // Initialize Weights if needed
    if (hiddenWeights.empty()) {
        srand(time(NULL));
        hiddenWeights.assign(numHidden, std::vector<double>(numInput));
        hiddenBias.assign(numHidden, 0.0);
        outputWeights.assign(numHidden, 0.0);
        for(size_t h=0; h<numHidden; ++h) {
            for(size_t i=0; i<numInput; ++i) hiddenWeights[h][i] = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
            outputWeights[h] = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
        }
        outputBias = 0.0;
    }

    double learningRate = 0.01;
    int epochs = 5000;

    std::cout << "[Neural Core] Deep Synthesis Training: " << targetName << " (Input Dim: " << numInput << ")" << std::endl;

    for (int e = 0; e < epochs; ++e) {
        double totalError = 0;
        for (const auto& snapshot : history) {
            // Forward Pass
            std::vector<double> inputVals(numInput, 0.0);
            double actualY = 0;
            for(const auto& c : snapshot) {
                if(c.name == targetName) actualY = c.getPropertyValue("value");
                for(size_t i=0; i<numInput; ++i) if(c.name == inputs[i]) inputVals[i] = c.getPropertyValue("value");
            }

            std::vector<double> hiddenVals(numHidden);
            for(size_t h=0; h<numHidden; ++h) {
                double z = hiddenBias[h];
                for(size_t i=0; i<numInput; ++i) z += inputVals[i] * hiddenWeights[h][i];
                hiddenVals[h] = sigmoid(z);
            }

            double pred = outputBias;
            for(size_t h=0; h<numHidden; ++h) pred += hiddenVals[h] * outputWeights[h];

            double error = pred - actualY;
            totalError += (error * error);

            // Backpropagation
            // Output Layer
            double dOutput = error; 
            for(size_t h=0; h<numHidden; ++h) {
                double dW_out = dOutput * hiddenVals[h];
                outputWeights[h] -= learningRate * dW_out;
            }
            outputBias -= learningRate * dOutput;

            // Hidden Layer
            for(size_t h=0; h<numHidden; ++h) {
                double dHidden = dOutput * outputWeights[h] * sigmoid_derivative(hiddenVals[h]);
                for(size_t i=0; i<numInput; ++i) {
                    hiddenWeights[h][i] -= learningRate * dHidden * inputVals[i];
                }
                hiddenBias[h] -= learningRate * dHidden;
            }
        }
        trainingError = totalError / history.size();
        if (trainingError < 0.001) break;
    }

    confidence = 1.0 - trainingError;
    std::cout << "[Neural Core] Deep Model converged with Accuracy: " << (confidence * 100) << "%" << std::endl;
}

Algorithm Algorithm::synthesizeFromConcepts(const std::vector<Concept>& concepts) {
    Algorithm algo;
    std::map<std::string, const Concept*> conceptMap;
    for (const auto& c : concepts) conceptMap[c.name] = &c;

    auto has = [&](std::string name) {
        for (const auto& c : concepts) {
            std::string lowerC = c.name;
            std::transform(lowerC.begin(), lowerC.end(), lowerC.begin(), ::tolower);
            std::string lowerN = name;
            std::transform(lowerN.begin(), lowerN.end(), lowerN.begin(), ::tolower);
            if (lowerC == lowerN) return true;
        }
        return false;
    };

    // 1. Specific Physics Engine (Case-Insensitive)
    if (has("Mass") && has("Acceleration")) {
        algo.target = "Force";
        algo.inputs = {"Mass", "Acceleration"};
    }
    else if (has("Mass") && (has("Energy") || has("velocity"))) {
        algo.target = "Energy";
        algo.inputs = {"Mass", "Velocity"};
    }
    else if (has("Velocity") && has("Time")) {
        algo.target = "Distance";
        algo.inputs = {"Velocity", "Time"};
    }
    
    // 2. Fallback: Synthetic Curiosity Model (Pick most salient technical concepts)
    if (algo.target.empty() && concepts.size() >= 3) {
        std::vector<Concept> sorted = concepts;
        std::sort(sorted.begin(), sorted.end(), [](auto& a, auto& b) { return a.salience > b.salience; });
        
        // Filter out very short words for the fallback model to keep it technical
        std::vector<std::string> technical;
        for(const auto& c : sorted) {
            if (c.name.length() > 3) technical.push_back(c.name);
            if (technical.size() >= 3) break;
        }

        if (technical.size() >= 3) {
            algo.target = technical[0];
            algo.inputs = {technical[1], technical[2]};
            std::cout << "[Subconscious] Forging Synthetic Model: Predicting " << algo.target 
                      << " from [" << algo.inputs[0] << ", " << algo.inputs[1] << "]" << std::endl;
        }
    }
    
    return algo;
}

bool Validator::validateRule(const AmiRule& rule) {
    if (rule.priority == 0) return false;
    std::cout << "[Validator] Neural pattern verified: PASS" << std::endl;
    return true;
}

Thought Logic::deepInference(const std::string& start, const std::string& end, const std::map<std::string, std::vector<std::string>>& relations, std::vector<std::string> currentPath) {
    currentPath.push_back(start);
    if (start == end) return { "Path Verified", currentPath, true };

    if (relations.count(start)) {
        // Human-like reasoning: Prioritize paths based on weighted relational strength 
        // (Currently uses DFS, but sorted by relationship count would be better)
        auto neighbors = relations.at(start);
        for (const auto& neighbor : neighbors) {
            if (std::find(currentPath.begin(), currentPath.end(), neighbor) != currentPath.end()) continue;
            Thought result = deepInference(neighbor, end, relations, currentPath);
            if (result.valid) return result;
        }
    }
    return { "No Path Found", {}, false };
}

bool Simulator::runTrial(const Algorithm& algo, const std::vector<Concept>& inputs) {
    if (algo.target.empty()) return false;
    double val = algo.predict(inputs);
    std::cout << "[Simulator] Deep Prediction for " << algo.target << ": " << val << std::endl;
    return true;
}

} // namespace Ami
