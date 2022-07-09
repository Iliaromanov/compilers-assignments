#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>
#include <sstream>
#include <cctype>

using namespace std;

class SemanticError {
    std::string message;

  public:
    SemanticError(std::string message);

    // Returns the message associated with the exception.
    const std::string &what() const;
};

SemanticError::SemanticError(std::string message):
  message(std::move(message)) {}

const std::string &SemanticError::what() const { return message; }

struct Node {
    string production_rule_str;
    pair<string, vector<string>> production_rule_tokens;
    string kind = "";
    string lexeme = "";
    string type = "";
    vector<Node> children;
    void get_rule_tokens(string rule_str); // sets production_rule_tokens
    void build_procedure_tables(map<string, pair<vector<string>, map<string, string>>> &procedure_tables); // builds procedure tables
    void assign_types(map<string, pair<vector<string>, map<string, string>>> &tables, string cur_procedure); // sets type
};

void Node::get_rule_tokens(string rule_str) {
    stringstream ss(rule_str);
    string token;
    ss >> token;
    production_rule_tokens.first = token;
    while (ss >> token) {
        production_rule_tokens.second.push_back(token);
    }
}

void get_paramlist(Node node, vector<string> &paramlist) {
    if (node.production_rule_tokens.second.empty()) {
        return;
    }
    if (node.production_rule_tokens.second[0] == "paramlist") {
        get_paramlist(node.children[0], paramlist);
    } else if (node.production_rule_tokens.second.size() == 3) {
        string type = node.children[0].children[0].production_rule_tokens.second.size() == 1 ? "int" : "int*";
        paramlist.push_back(type);
        get_paramlist(node.children[2], paramlist);
    } else { // paramlist -> dcl
        string type = node.children[0].children[0].production_rule_tokens.second.size() == 1 ? "int" : "int*";
        paramlist.push_back(type);
    }
}

void get_arglist(Node node, vector<string> &arglist) {
    if (node.production_rule_tokens.second.size() == 3) { // arglist -> expr COMMA arglist
        string type = node.children[0].type;
        arglist.push_back(type);
        get_arglist(node.children[2], arglist);
    } else { // arglist -> expr
        string type = node.children[0].type;
        arglist.push_back(type);
    }
}

// overload << operator for Node
ostream& operator<<(ostream& os, const Node& node) {
    if (node.production_rule_str != "") {
        os << node.production_rule_str;
    } else if (node.kind != "" and node.lexeme != "") {
        os << node.kind << " " << node.lexeme;
    }
    if (node.type != "") {
        os << " : " << node.type;
    }
    cout << endl;
    return os;
}

// void preorder_print(shared_ptr<Node> node) {
//     cout << *node;
//     for (auto child : node->children) {
//         preorder_print(child);
//     }
// }
void preorder_print(Node node) {
    cout << node;

    // cout << "children: " << endl;
    // for (auto child : node.children) {
    //     cout << child << endl;
    // }

    for (auto child : node.children) {
        preorder_print(child);
    }
}



Node build_tree() {
    string line;
    getline(cin, line);
    auto root = Node{};
    root.production_rule_str = line;
    root.get_rule_tokens(line);
    stringstream ss{line};
    string lhs;
    ss >> lhs;

    // cout << "ROOT IS " << lhs << endl;
    if (root.production_rule_tokens.second[0] == ".EMPTY") {
        root.production_rule_tokens.second.clear();
        return root;
    } else if (!isupper(lhs[0])) { // internal node
        string child_str;
        while (ss >> child_str) {
            Node child = build_tree();
            // cout << child << endl;
            if (child.production_rule_str != "" or child.kind != "" or child.lexeme != "") {
                root.children.push_back(child);
            } // this is because we don't want to add empty nodes to the tree
        }
        // cout << "DONNE" << endl << endl;
    } else {
        string lexeme;
        ss >> lexeme;
        Node node = Node();
        node.kind = lhs;
        node.lexeme = lexeme;
        return node;
    }

    return root;
}

void Node::assign_types(map<string, pair<vector<string>, map<string, string>>> &tables, string cur_procedure) {
    // cout << "~~~~ assigning types to " << node << endl;
    for (int i = 0; i < children.size(); i++) {
        children[i].assign_types(tables, cur_procedure);
    }

    // Check 2nd argument of wain is int*
    if (production_rule_tokens.first == "main") {
        // cout << "~~~~~~" << endl << "checking wain second arg" << endl << children[5].children[1]  << "~~~~~~~~" << endl;
        if (children[5].children[1].type != "int") {
            throw SemanticError("wain's second argument must be int");
        }

        // cout << "~~~~~~" << endl << "checking wain return arg" << endl << children[11]  << "~~~~~~~~" << endl;


        // for (auto child : children) {
        //     cout << child << endl;
        // }

        // cout << "~~~~~~" << endl << "ceturn arg" << endl << "~~~~~~~~" << endl;

        if (children[11].type != "int") {
            throw SemanticError("wain's return type must be int");
        }
    }

    // check procedure
    if (production_rule_tokens.first == "procedure") {
        if (children[9].type != "int") {
            throw SemanticError("procedure's return type must be int");
        }
    }

    // check statement
    if (production_rule_tokens.first == "statement") {
        // When statement derives lvalue BECOMES expr SEMI, the derived lvalue and the derived expr must have the same type.
        if (production_rule_tokens.second[0] == "lvalue") { // statement → lvalue BECOMES expr SEMI
            if (children[0].type != children[2].type) {
                throw SemanticError("lvalue and expr must have the same type in 'lvalue BECOMES expr SEMI'");
            }
        // When statement derives PRINTLN LPAREN expr RPAREN SEMI, the derived expr must have type int.
        } else if (production_rule_tokens.second[0] == "PRINTLN") { // statement → PRINTLN LPAREN expr RPAREN SEMI
            if (children[2].type != "int") {
                throw SemanticError("expr must be int in 'PRINTLN LPAREN expr RPAREN SEMI'");
            }
        // When statement derives DELETE LBRACK RBRACK expr SEMI, the derived expr must have type int*.
        } else if (production_rule_tokens.second[0] == "DELETE") { // statement → DELETE LBRACK RBRACK expr SEMI
            if (children[3].type != "int*") {
                throw SemanticError("expr must be int* in 'DELETE LBRACK RBRACK expr SEMI'");
            }
        }
    }

    // check test
    if (production_rule_tokens.first == "test") {
        // Whenever test directly derives a sequence containing two exprs, they must both have the same type.
        if (production_rule_tokens.second.size() == 3) { // test → expr _ expr
            if (children[0].type != children[2].type) {
                throw SemanticError("exprs must have the same type in 'test -> expr _ expr'");
            }
        }
        // test → expr EQ expr
        // test → expr NE expr
        // test → expr LT expr
        // test → expr LE expr
        // test → expr GE expr
        // test → expr GT expr
    }

    // check declaration types match
    if (production_rule_tokens.first == "dcls") {
        if (!children.empty() and children[1].children[1].type != children[3].type) {
            throw SemanticError("assignment types must match");
        }
    }


    if (children.size() == 0) { // leaf
        if (kind == "NULL") {
            type = "int*";
        } else if (kind == "NUM") {
            type = "int";
        } else if (kind == "ID" and tables[cur_procedure].second.find(lexeme) != tables[cur_procedure].second.end()) { // <----------not sure if should make it type "int" if its a procedure name
            type = tables[cur_procedure].second[lexeme];
        }
        return;
    }

    if (production_rule_tokens.first == "dcl") {
        // cout << "FOUND DCL" << endl;
        Node type_node = children[0]; 
        // theres two possiblities for type: type → INT, type → INT STAR
        string type = type_node.production_rule_tokens.second.size() == 1 ? "int" : "int*";
        children[1].type = type;
        // cout << "~~~~~~~~~~~~~~~~~~" << endl << "ASSIGNED DCL TYPE: " << endl << children[1] << "~~~~~~~~~~~~~~~~~~" << endl;
        
        // update symbol_table
        if (tables[cur_procedure].second.find(children[1].lexeme) == tables[cur_procedure].second.end()) {
            tables[cur_procedure].second[children[1].lexeme] = type;
        } else {
            throw SemanticError("Variable " + children[1].lexeme + " already declared");
        }
    }

    if (production_rule_tokens.first == "expr") {
        if (production_rule_tokens.second.size() == 1) { // expr -> term 
            type = children[0].type;
        } else if (production_rule_tokens.second[1] == "PLUS") { // expr -> expr PLUS term 
            if (children[0].type == "int" and children[2].type == "int") {
                type = "int";
            } else if (children[0].type == "int*" and children[2].type == "int") {
                type = "int*";
            } else if (children[0].type == "int" and children[2].type == "int*") {
                type = "int*";
            } else if (children[0].type == "int*" and children[2].type == "int*") {
                type = "int*";
            } else {
                throw SemanticError("Plus operator cannot be between int* and int*");
            }
        } else if (production_rule_tokens.second[1] == "MINUS") { // expr -> expr MINUS term
            if (children[0].type == "int" and children[2].type == "int") {
                type = "int";
            } else if (children[0].type == "int*" and children[2].type == "int") {
                type = "int*";
            } else if (children[0].type == "int*" and children[2].type == "int*") {
                type = "int";
            } else {
                throw SemanticError("Minus operator cannot be between int and int*");
            }
        }
    }
    
    if (production_rule_tokens.first == "term") {
        if (production_rule_tokens.second.size() == 1) {// term → factor
            type = children[0].type;
        } else { // term → term STAR factor or term → term SLASH factor or term → term PCT factor 
            if (children[0].type == "int" and children[2].type == "int") {
                type = "int";
            } else {
                throw SemanticError("* / mod operators for term must be between int and int");
            }
        }        
    }

    if (production_rule_tokens.first == "factor") {
        if (production_rule_tokens.second.size() == 1) { // factor → NUM or factor → NULL or -> ID
            if (production_rule_tokens.second[0] == "NUM") {
                type = "int";
                children[0].type = "int";
            } else if (production_rule_tokens.second[0] == "NULL") {
                type = "int*";
                children[0].type = "int*";
            } else if (production_rule_tokens.second[0] == "ID") { //
                if (tables[cur_procedure].second.find(children[0].lexeme) == tables[cur_procedure].second.end()) {
                    throw SemanticError("Variable " + children[0].lexeme + " not declared");
                } else {
                    type = tables[cur_procedure].second[children[0].lexeme];
                    children[0].type = type;
                }
            }
        } else if (production_rule_tokens.second[0] == "LPAREN") { // factor → LPAREN expr RPAREN 
            type = children[1].type;
        } else if (production_rule_tokens.second[0] == "AMP") { // factor → AMP lvalue
            if (children[1].type != "int") {
                throw SemanticError("& must be applied to int");
            } else {
                type = "int*";
            }
        } else if (production_rule_tokens.second[0] == "STAR") { // factor → STAR factor
            if (children[1].type != "int*") {
                throw SemanticError("* must be applied to int*");
            } else {
                type = "int";
            }
        } else if (production_rule_tokens.second[0] == "NEW") { // factor → NEW INT LBRACK expr RBRACK
            if (children[3].type != "int") {
                throw SemanticError("factor → NEW INT LBRACK expr RBRACK must be applied to int");
            } else {
                type = "int*";
            }
        } else if (production_rule_tokens.second[0] == "ID") {// factor → ID LPAREN RPAREN or factor → ID LPAREN arglist RPAREN
            // ID must be the name of a procedure and NOT a variable
            string ID_lexeme = children[0].lexeme;


            // cout << "~~~~~~~~~~~~~ checking factor -> ID(...) in " << cur_procedure << " calling " << ID_lexeme << endl;


            if (tables.find(ID_lexeme) == tables.end() or tables[cur_procedure].second.find(ID_lexeme) != tables[cur_procedure].second.end()) {
                throw SemanticError("Procedure " + ID_lexeme + " not declared");
            } else {
                type = "int";
            }
            if (production_rule_tokens.second.size() == 3) { // factor → ID LPAREN RPAREN
                if (!tables[ID_lexeme].first.empty()) {
                    throw SemanticError("Procedure " + ID_lexeme + " expects " + to_string(tables[ID_lexeme].first.size()) + " arguments but was given 0");
                }
            } else { // factor → ID LPAREN arglist RPAREN
                vector<string> arglist;
                get_arglist(children[2], arglist);
                if (arglist.size() != tables[ID_lexeme].first.size()) {
                    throw SemanticError("Procedure " + ID_lexeme + " expects " + to_string(tables[ID_lexeme].first.size()) + " arguments but was given " + to_string(arglist.size()));
                } else {
                    for (int i = 0; i < arglist.size(); i++) {
                        if (tables[ID_lexeme].first[i] != arglist[i]) {
                            throw SemanticError("Procedure " + ID_lexeme + " expects argument " + to_string(i + 1) + " to be " + tables[ID_lexeme].first[i] + " but was given " + arglist[i]);
                        }
                    }
                }
            }
        }
    }

    if (production_rule_tokens.first == "lvalue") {
        if (production_rule_tokens.second[0] == "ID") { //
            if (tables[cur_procedure].second.find(children[0].lexeme) == tables[cur_procedure].second.end()) {
                throw SemanticError("Variable " + children[0].lexeme + " not declared");
            } else {
                type = tables[cur_procedure].second[children[0].lexeme];
                children[0].type = type;
            }
        } else if (production_rule_tokens.second[0] == "LPAREN") { // lvalue → LPAREN lvalue RPAREN 
            type = children[1].type;
        } else if (production_rule_tokens.second[0] == "STAR") { // lvalue → STAR factor
            if (children[1].type != "int*") {
                throw SemanticError("* must be applied to int*");
            } else {
                type = "int";
            }
        }        
    }
}


void Node::build_procedure_tables(map<string, pair<vector<string>, map<string, string>>> &tables) {
    if (production_rule_tokens.first == "procedures" and production_rule_tokens.second[0] == "procedure") {
        string name = children[0].children[1].lexeme;

        if (tables.find(name) != tables.end()) {
            throw SemanticError("Procedure " + name + " already declared");
        }
        vector<string> paramlist;
        get_paramlist(children[0].children[3], paramlist);
        tables[name] = make_pair<>(paramlist, map<string, string>());
        children[0].assign_types(tables, name);
        children[1].build_procedure_tables(tables);
    } else { // reached procedures -> main
        string name = "wain";
        if (tables.find(name) != tables.end()) {
            throw SemanticError("Procedure " + name + " already declared");
        }
        tables[name] = make_pair<>(vector<string>(), map<string, string>()); // <-----------------------------------not sure if its ok to make sig empty (wain cant call itself)
        children[0].assign_types(tables, name);
    }
}


int main() {
    // Build tree
    map<string, pair<vector<string>, map<string, string>>> tables;
    Node root = build_tree();
    try {
        root.children[1].build_procedure_tables(tables);
    } catch (SemanticError &e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 0;
    }
    preorder_print(root);
}
