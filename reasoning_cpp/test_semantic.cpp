#include "semantic_engine.h"
#include <iostream>
#include <vector>

int main() {
    SemanticEngine engine;

    std::vector<std::pair<std::string, std::string>> data = {
        {"King", "A king is a male monarch and ruler of a kingdom."},
        {"Queen", "A queen is a female monarch and ruler of a kingdom."},
        {"Apple", "An apple is a sweet, edible fruit produced by an apple tree."},
        {"Mars", "Mars is the fourth planet from the Sun and the second-smallest planet in the Solar System."},
        {"SpaceX", "SpaceX is an American aerospace manufacturer and space transportation services company."}
    };

    std::cout << "Learning..." << std::endl;
    engine.learn(data);

    std::string query1 = "Monarch ruler";
    std::cout << "\nQuery: " << query1 << std::endl;
    auto results1 = engine.findSimilar(query1, 3);
    for (auto& p : results1) {
        std::cout << "  > " << p.first << ": " << p.second << std::endl;
    }

    std::string query2 = "Space planet";
    std::cout << "\nQuery: " << query2 << std::endl;
    auto results2 = engine.findSimilar(query2, 3);
    for (auto& p : results2) {
        std::cout << "  > " << p.first << ": " << p.second << std::endl;
    }

    return 0;
}
