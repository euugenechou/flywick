#include "newick.hpp"
#include <queue>
#include <unordered_map>

NewickTree::NewickTree(const char *label_, double length_,
                       std::vector<NewickTree *> *children_) {
    label = label_;
    length = length_;
    children = children_;
}

static void remove(NewickTree *n) {
    if (n) {
        delete n;
    }
}

NewickTree::~NewickTree() {
    for (size_t i = 0; i < children->size(); i += 1) {
        remove(children->at(i));
    }
    delete children;
}

Json NewickTree::to_json() {
    Json tree = {
        {"label", label},
        {"length", length},
        {"children", Json::array()},
    };

    for (size_t i = 0; i < children->size(); i += 1) {
        tree["children"].push_back(children->at(i)->to_json());
    }

    return tree;
}

void NewickTree::dump_json(FILE *outfile, int indent) {
    if (indent > 0) {
        fprintf(outfile, "%s\n", to_json().dump(indent).c_str());
    } else {
        fprintf(outfile, "%s\n", to_json().dump().c_str());
    }
}

void NewickTree::dump_newick(FILE *outfile) {
    static int depth = 0;

    if (children->size() > 0) {
        depth += 1;
        fprintf(outfile, "(");
        for (size_t i = 0; i < children->size(); i += 1) {
            children->at(i)->dump_newick(outfile);
            if (i + 1 != children->size()) {
                fprintf(outfile, ",");
            }
        }
        fprintf(outfile, ")");
        depth -= 1;
    }

    if (label.length() > 0) {
        fprintf(outfile, "%s", label.c_str());
    }
    if (length >= 0.0) {
        fprintf(outfile, ":%lf", length);
    }
    if (depth == 0) {
        fprintf(outfile, ";\n");
    }
}

NewickTree *NewickTree::from_json(const Json &tree) {
    std::vector<NewickTree *> *children = new std::vector<NewickTree *>();

    for (auto &child : tree["children"]) {
        children->push_back(NewickTree::from_json(child));
    }

    std::string label = tree["label"].get<std::string>();
    double length = tree["length"].get<double>();
    return new NewickTree(label.c_str(), length, children);
}

void NewickTree::annotate_json(Json &tree, const Json &annotations) {
    for (auto &child : tree["children"]) {
        annotate_json(child, annotations);
    }

    std::string label = tree["label"].get<std::string>();
    if (annotations.contains(label)) {
        tree["metadata"] = annotations[label];
    }
}
