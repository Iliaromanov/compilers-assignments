#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <algorithm>
#include <sstream>
#include <memory>
#include "wlp4data.h"

using namespace std;

// actual code starts here
struct Node {
    string production_rule = "";
    string kind = "";
    string lexeme = "";
    vector<Node> children;
};

// overload << operator for Node
ostream& operator<<(ostream& os, const Node& node) {
    if (node.production_rule != "") {
        os << node.production_rule << endl;
    } else {
        os << node.kind << " " << node.lexeme << endl;
    }
    return os;
}

void preorder_print(Node node) {
    cout << node;
    for (auto child : node.children) {
        preorder_print(child);
    }
}


int main() {
    stringstream input{WLP4_COMBINED};

    string line;
    string lhs;
    string symbol;
    map<int, pair<string, vector<string>>> cfg_rules;
    map<int, map<string, int>> transitions;
    map<int, map<int, vector<string>>> reductions;
    int terminal_count = 0;
    vector<Node> reduct_seq;
    vector<Node> input_seq;
    vector<int> state_stack = {0};

    getline(input, line); // skip first line .CFG

    int i = 0;
    while (getline(input, line)) {
        if (line[0] == '.') { // reached .TRANSITIONS
            break;
        }
        stringstream ss{line};
        ss >> lhs;
        cfg_rules[i] = {lhs, {}};
        while (ss >> symbol) {
            if (symbol != ".EMPTY") {
                cfg_rules[i].second.push_back(symbol);
            }
        }
        i++;
    }
    

    // -------------------------------------------------------------- THIS WORKS
    // cout << "PLEASE (cfg_rules" << endl;
    // for (auto &p : cfg_rules) {
    //     cout << p.first << ":" << endl;
    //     cout << p.second.first << " -> ";
    //     for (auto &s : p.second.second) {
    //         cout << s << " ";
    //     }
    //     cout << endl;
    // }
    // cout << endl << endl;
    // ---------------------------------------------------------------


    // .INPUT
    input_seq.push_back(Node{"", "BOF", "BOF", {}});
    while (getline(cin, line)) {
        if (line[0] == '.') { // reached .TRANSITIONS
            break;
        }
        stringstream ss{line};
        string kind;
        string lexeme;
        ss >> kind;
        ss >> lexeme;
        input_seq.push_back(Node{"", kind, lexeme, {}});
    }
    input_seq.push_back(Node{"", "EOF", "EOF", {}});

    reverse(input_seq.begin(), input_seq.end()); // reverse input seq to access from end
    // cout << "INPUT SEQ PLEASE:" << endl;
    // for (auto &s : input_seq) {
    //     cout << s << " ";
    // }
    // cout << endl << endl;
    // ---------------------------------------------------------------



    // .TRANSITIONS
    int start_state;
    string transition_symbol;
    int end_state;
    while (getline(input, line)) {
        if (line[0] == '.') { // reached .REDUCTIONS
            break;
        }
        stringstream ss{line};
        ss >> start_state;
        ss >> transition_symbol;
        ss >> end_state;
        transitions[start_state][transition_symbol] = end_state;
    }

    // .REDUCTIONS
    // init empty reduction for every state
    for (int i = 0; i < cfg_rules.size(); i++) {
        reductions[i] = {};
    }
    int state_number;
    int rule_number;
    string tag;
    while (getline(input, line)) {
        if (line[0] == '.') { // reached .END
            break;
        }
        stringstream ss{line};
        ss >> state_number;
        ss >> rule_number;
        ss >> tag;
        reductions[state_number][rule_number].push_back(tag); // <-------------------- might fail cus tag vector is empty
    }

    // ONE MORE WHILE FOR ACTUALLY TRAVERSING THE SLR(1) DFA
    // Traversing the input using transitions and reductions
    bool end = false;
    while (true) {
        // print
        // for (auto &s : reduct_seq) {
        //     cout << "<" << s << ">";
        // }
        // cout << ". ";
        // for (int i = input_seq.size() - 1; i >= 0; i--) {
        //     cout << "<" << input_seq[i] << ">";
        // }
        // cout << "-------------------" << endl;

        if (end) {
            preorder_print(reduct_seq[0]);
            return 0;
        }

        // check if reductions are empty or input_seq.back() is not in Follow (LHS) for any possible reductions
        bool in_follow_set = false;
        int reduction_rule = -1;
        if (!reductions[state_stack.back()].empty()) {
            // check if input_seq.back is in Follow (LHS) for any possible reductions
            for (auto &p : reductions[state_stack.back()]) {
                if (input_seq.empty() and count(p.second.begin(), p.second.end(), ".ACCEPT")) {
                    // do one more reduction then accept
                    in_follow_set = true;
                    end = true;
                    reduction_rule = p.first;
                    break;
                }
                if (count(p.second.begin(), p.second.end(), input_seq.back().kind) > 0) {
                    in_follow_set = true;
                    reduction_rule = p.first;
                    break;
                }
            }
        }

        // reductions are empty or input_seq.back() is not in Follow (LHS) for any possible reductions
        if (!in_follow_set) { // shift
            if (transitions[state_stack.back()].find(input_seq.back().kind) == transitions[state_stack.back()].end()) { // no transition exists
                cerr << "ERROR at " << terminal_count + 1 << endl;
                return 0;
            }
            // otherwise shift and transition to next state
            state_stack.push_back(transitions[state_stack.back()][input_seq.back().kind]);
            reduct_seq.push_back(input_seq.back());
            if (input_seq.back().kind != "BOF" and input_seq.back().kind != "EOF") {
                terminal_count++;
            }
            input_seq.pop_back();
        } else { // reduce
            int num_states_to_pop = cfg_rules[reduction_rule].second.size(); // <----------------------not sure if need +-1
            for (int i = 0; i < num_states_to_pop; i++) state_stack.pop_back();
            string rule_lhs = cfg_rules[reduction_rule].first;
            string production_rule_str = rule_lhs;
            if (cfg_rules[reduction_rule].second.empty()) {
                production_rule_str += " .EMPTY";
            } else {
                for (auto &s : cfg_rules[reduction_rule].second) {
                    production_rule_str += " " + s;
                }
            }
            if (transitions[state_stack.back()].find(rule_lhs) == transitions[state_stack.back()].end() and !end) { // no transition exists
                cerr << "ERROR at " << terminal_count + 1 << endl;
                return 0;
            }
            // transition to next state
            state_stack.push_back(transitions[state_stack.back()][rule_lhs]);
            Node new_node{production_rule_str, "", "", {}};
            for (int i = 0; i < num_states_to_pop; i++) {
                new_node.children.insert(new_node.children.begin(), reduct_seq.back());
                reduct_seq.pop_back();
            }
            reduct_seq.push_back(new_node);
        }        
    }
}
