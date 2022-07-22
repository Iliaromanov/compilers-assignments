#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <sstream>
#include <memory>


using namespace std;

// int stack_count = 0; // keep track of how many times we need to decrement $30 
//     need to have local stack_count for every procedure now
int if_count = 0;
int while_count = 0;
int delete_count = 0;

struct Node {
    string production_rule_str;
    pair<string, vector<string>> production_rule_tokens;
    string kind = "";
    string lexeme = "";
    string type = "";
    vector<shared_ptr<Node>> children;
    void get_rule_tokens(string rule_str); // sets production_rule_tokens
};

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

void Node::get_rule_tokens(string rule_str) {
    stringstream ss(rule_str);
    string token;
    ss >> token;
    production_rule_tokens.first = token;
    while (ss >> token) {
        production_rule_tokens.second.push_back(token);
    }
}


void preorder_print(shared_ptr<Node> node) {
    cout << *node;

    for (auto &child : node->children) {
        preorder_print(child);
    }
}


shared_ptr<Node> build_tree() {
    string line;
    getline(cin, line);
    shared_ptr<Node> root = make_shared<Node>();
    root->production_rule_str = line;
    root->get_rule_tokens(line);
    stringstream ss{line};
    string lhs;
    ss >> lhs;

    // cout << "ROOT IS " << lhs << endl;
    if (root->production_rule_tokens.second[0] == ".EMPTY") {
        root->production_rule_tokens.second.clear();
        return root;
    } else if (!isupper(lhs[0])) { // internal node
        string child_str;
        while (ss >> child_str) {
            if (child_str == ":") { // <---------------------------------------------------- save the type
                // string type;
                // ss >> type;
                // root.type = type;
                return root;
            }
            // shared_ptr<Node> child = build_tree();
            // cout << child << endl;
            // if (child->production_rule_str != "" or child->kind != "" or child->lexeme != "") {
            root->children.emplace_back(build_tree());
            // } // this is because we don't want to add empty nodes to the tree
        }
        // cout << "DONNE" << endl << endl;
    } else { // leaf node
        string lexeme;
        string type;
        ss >> lexeme;  
        shared_ptr<Node> node = make_shared<Node>();
        node->kind = lhs;
        node->lexeme = lexeme;
        if (ss >> type) { // this would read ":" if there is a type
            ss >> type;
            node->type = type;
        }
        return node;
    }

    return root;
}


string get_expr_lexeme(shared_ptr<Node> expr_node) {
    while (!expr_node->children.empty()) {
        if (expr_node->children.size() == 1) { // expr → term, term → factor, factor → NUM, factor → ID
            expr_node = expr_node->children[0];
        } else { // factor → LP expr RP
            expr_node = expr_node->children[1];
        }
    }
    return expr_node->lexeme;
}

string get_lvalue_lexeme(shared_ptr<Node> lvalue_node) {
    while (!lvalue_node->children.empty()) {
        if (lvalue_node->children.size() == 1) { // lvalue → ID
            lvalue_node = lvalue_node->children[0];
        } else { // lvalue → LP lvalue RP
            if (lvalue_node->production_rule_tokens.second[0] == "STAR") {
                cout << endl << endl << endl << "unhandled" << endl << endl << endl;
            }
            lvalue_node = lvalue_node->children[1];
        }
    }
    return lvalue_node->lexeme;
}


// ------------------------------------ 
// MIPS code generation funcs
// ------------------------------------
// add $30, $30, $4
void increment_stack_ptr() { cout << "add $30, $30, $4" << endl; }

// sub $30, $30, $4
void decrement_stack_ptr() { cout << "sub $30, $30, $4" << endl; }

void push(int reg, int &local_stack_count, string what_was_pushed) {
    cout << "sw $" << reg << ", -4($30)";
    cout << " ; push " << what_was_pushed << " to stack" << endl;
    decrement_stack_ptr();
    local_stack_count++;
}

void pop(int reg, int &local_stack_count) {
    increment_stack_ptr();
    cout << "lw $" << reg << ", -4($30)" << endl;
    local_stack_count--;
}

void lis(int reg, string val) {
    cout << "lis $" << reg << endl;
    cout << ".word " << val << endl;
}

// works for both constants and variables
void code(map<string, pair<string, int>> symbol_table, string id) {
    if (isdigit(id[0])) {
        lis(3, id);
    } else if (id == "NULL") {
        cout << "add $3, $0, $11 ; setting to NULL" << endl;
    } else {
        cout << "lw $3, " << symbol_table[id].second << "($29)";
        cout << " ; $3 <- " << id << endl;
    }
}

void code_term(map<string, pair<string, int>> symbol_table, int &local_stack_count, Node *term);

void code_expr(map<string, pair<string, int>> symbol_table, int &local_stack_count, Node *expr) {
    if (expr->children.size() == 1) { // expr → term 
        code_term(symbol_table, local_stack_count, expr->children[0].get());
    } else if (expr->production_rule_tokens.second[1] == "PLUS") { // expr → expr PLUS term


        // cout << "~~~~~~~~~~~~~~IN EXPR ADD: " << endl;
        // cout << expr->production_rule_str << endl;
        // cout << expr->children[0]->production_rule_str << endl;
        // cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~ children[0]" << endl;
        // // cout << *expr->children[0] << endl;
        // for (auto &tok : expr->children[0]->production_rule_tokens.second) {
        //     cout << tok << " | ";
        // }
        // cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~ " << endl;
        // cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~ children[2]" << endl;
        // // cout << *expr->children[0] << endl;
        // for (auto &tok : expr->children[2]->production_rule_tokens.second) {
        //     cout << tok << " | ";
        // }
        // cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~ " << endl;

        // cout << expr->children[2]->production_rule_str << endl;
        // cout << "expr->children[0]->type: " << expr->children[0]->type;
        // cout << "expr->children[2]->type: " << expr->children[2]->type << endl;


        if (expr->children[0]->production_rule_tokens.second.back() == "int*" and 
            expr->children[2]->production_rule_tokens.second.back() == "int") { // type(expr) = int* and type(term) = int
            code_expr(symbol_table, local_stack_count, expr->children[0].get());
            push(3, local_stack_count, "expr from expr+term where type(expr) = int* and type(term) = int");
            code_term(symbol_table, local_stack_count, expr->children[2].get());
            cout << "mult $3, $4" << endl;
            cout << "mflo $3" << endl;
            pop(5, local_stack_count); // $5 <- expr 
            cout << "add $3, $5, $3 ; pointer arithmetic (int* + int)" << endl;
        } else if (expr->children[0]->production_rule_tokens.second.back() == "int" and 
                   expr->children[2]->production_rule_tokens.second.back() == "int*") { // type(expr) = int and type(term) = int*
            code_expr(symbol_table, local_stack_count, expr->children[0].get());
            cout << "mult $3, $4" << endl;
            cout << "mflo $3" << endl;
            push(3, local_stack_count, "expr * 4 from expr+term where type(expr) = int and type(term) = int*");
            code_term(symbol_table, local_stack_count, expr->children[2].get());
            pop(5, local_stack_count); // $5 <- expr * 4
            cout << "add $3, $5, $3 ; pointer arithmetic (int + int*)" << endl;
        } else { // type(expr) = int and type(term) = int
            code_expr(symbol_table, local_stack_count, expr->children[0].get());
            push(3, local_stack_count, "expr from expr PLUS term");
            code_term(symbol_table, local_stack_count, expr->children[2].get());
            pop(5, local_stack_count);
            cout << "add $3, $5, $3" << endl;
        }
    } else if (expr->production_rule_tokens.second[1] == "MINUS") { // expr → expr MINUS term
        if (expr->children[0]->production_rule_tokens.second.back() == "int*" and
            expr->children[2]->production_rule_tokens.second.back() == "int") { // type(expr) = int* and type(term) = int
            code_expr(symbol_table, local_stack_count, expr->children[0].get());
            push(3, local_stack_count, "expr from expr - term where type(expr) = int* and type(term) = int");
            code_term(symbol_table, local_stack_count, expr->children[2].get());
            cout << "mult $3, $4" << endl;
            cout << "mflo $3" << endl;
            pop(5, local_stack_count); // $5 <- expr 
            cout << "sub $3, $5, $3 ; pointer arithmetic (int* - int)" << endl;
        } else if (expr->children[0]->production_rule_tokens.second.back() == "int*" and
                   expr->children[2]->production_rule_tokens.second.back() == "int*") { // type(expr) = int* and type(term) = int*
            code_expr(symbol_table, local_stack_count, expr->children[0].get());
            push(3, local_stack_count, "expr from expr - term where type(expr) = int* and type(term) = int*");
            code_term(symbol_table, local_stack_count, expr->children[2].get());
            pop(5, local_stack_count); // $5 <- expr 
            cout << "sub $3, $5, $3 ; pointer arithmetic (int* - int*)" << endl;
            cout << "div $3, $4 ; (int* - int*) / 4" << endl;
            cout << "mflo $3" << endl;
        } else { // type(expr) = int and type(term) = int
            code_expr(symbol_table, local_stack_count, expr->children[0].get());
            push(3, local_stack_count, "expr from expr MINUS term");
            code_term(symbol_table, local_stack_count, expr->children[2].get());
            pop(5, local_stack_count);
            cout << "sub $3, $5, $3" << endl;
        }
    }
}

int code_arglist(map<string, pair<string, int>> symbol_table, int &local_stack_count, Node *arglist, int arg_count) {
    
    
    // cout << "in argslist: " << arglist->production_rule_str << endl;
    
    
    
    if (arglist->children.size() == 1) { // arglist → expr
        code_expr(symbol_table, local_stack_count, arglist->children[0].get());
        push(3, local_stack_count, "pushed arg " + to_string(arg_count));
        return arg_count;
    } else { // arglist → expr COMMA arglist
        code_expr(symbol_table, local_stack_count, arglist->children[0].get());
        push(3, local_stack_count, "pushed arg " + to_string(arg_count));
        arg_count++;
        return code_arglist(symbol_table, local_stack_count, arglist->children[2].get(), arg_count);
    }
}

void code_new(map<string, pair<string, int>> symbol_table, int &local_stack_count, Node *factor_new_node);

void code_factor(map<string, pair<string, int>> symbol_table, int &local_stack_count, Node *factor) {
    if (factor->children.size() == 1) {// factor → NUM  or factor → ID or factor → NULL
        code(symbol_table, factor->children[0]->lexeme);
    } else if (factor->production_rule_tokens.second[0] == "STAR") { // factor → STAR factor
        code_factor(symbol_table, local_stack_count, factor->children[1].get());
        cout << "lw $3, 0($3) ; load value at address (from factor -> STAR factor)" << endl;
    } else if (factor->production_rule_tokens.second[0] == "AMP") { // factor → AMP lvalue
        if (factor->children[1]->production_rule_tokens.second[0] == "STAR") { // lvalue → STAR factor
            code_factor(symbol_table, local_stack_count, factor->children[1]->children[1].get());
        } else { // lvalue → ID or lvalue → LPAREN lvalue RPAREN  
            string lvalue_id = get_lvalue_lexeme(factor->children[1]);
            int offset = symbol_table[lvalue_id].second;
            lis(3, to_string(offset));
            cout << "add $3, $3, $29 ; $3 <- &" << lvalue_id << endl;
        }
    } else if (factor->production_rule_tokens.second[0] == "LPAREN") { // factor → LPAREN expr RPAREN
        code_expr(symbol_table, local_stack_count, factor->children[1].get());
    } else if (factor->production_rule_tokens.second[0] == "NEW") { // factor → NEW INT LBRACK expr RBRACK
        code_new(symbol_table, local_stack_count, factor);
    } else if (factor->production_rule_tokens.second[0] == "ID" and
               factor->production_rule_tokens.second[1] == "LPAREN"
        ) { // factor → ID LPAREN RPAREN or factor → ID LPAREN arglist RPAREN
        int arglist_size = 0;
        push(29, local_stack_count, "saving $29 before call");
        push(31, local_stack_count, "saving $31 before call");
        if (factor->production_rule_tokens.second[2] == "arglist") { // factor → ID LPAREN arglist RPAREN
            arglist_size = code_arglist(symbol_table, local_stack_count, factor->children[2].get(), 1);
        }
        string id = factor->children[0]->lexeme;
        lis(5, "F" + id);
        cout << "jalr $5" << endl;
        for (int i = 0; i < arglist_size; i++) {
            pop(31, local_stack_count);
        }
        pop(31, local_stack_count);
        pop(29, local_stack_count);
    }
}

void code_term(map<string, pair<string, int>> symbol_table, int &local_stack_count, Node *term) {
    if (term->children.size() == 1) { // term → factor


        // cout << "in code_factor" << endl;



        code_factor(symbol_table, local_stack_count, term->children[0].get());
    } else if (term->production_rule_tokens.second[1] == "STAR") { // term → term STAR factor
        code_term(symbol_table, local_stack_count, term->children[0].get());
        push(3, local_stack_count, "term from term * factor");
        code_factor(symbol_table, local_stack_count, term->children[2].get());
        pop(5, local_stack_count);
        cout << "mult $5, $3" << endl;
        cout << "mflo $3" << endl;
    } else if (term->production_rule_tokens.second[1] == "SLASH") { // term → term SLASH factor
        code_term(symbol_table, local_stack_count, term->children[0].get());
        push(3, local_stack_count, "term from term / factor");
        code_factor(symbol_table, local_stack_count, term->children[2].get());
        pop(5, local_stack_count);
        cout << "div $5, $3" << endl;
        cout << "mflo $3" << endl;
    } else if (term->production_rule_tokens.second[1] == "PCT") { // term → term PCT factor
        code_term(symbol_table, local_stack_count, term->children[0].get());
        push(3, local_stack_count, "term from term % factor");
        code_factor(symbol_table, local_stack_count, term->children[2].get());
        pop(5, local_stack_count);
        cout << "div $5, $3" << endl;
        cout << "mfhi $3" << endl;
    }
}

void code_test(map<string, pair<string, int>> symbol_table, int &local_stack_count, Node *test) {
    string op = test->production_rule_tokens.second[1];
    if (op == "LT") {
        code_expr(symbol_table, local_stack_count, test->children[0].get());
        push(3, local_stack_count, "expr1 from test LT");
        code_expr(symbol_table, local_stack_count, test->children[2].get());
        pop(5, local_stack_count);
        if (test->children[0]->production_rule_tokens.second.back() == "int") {
            cout << "slt $3, $5, $3" << endl;
        } else {
            cout << "sltu $3, $5, $3" << endl;
        }
    } else if (op == "GT") {
        code_expr(symbol_table, local_stack_count, test->children[0].get());
        push(3, local_stack_count, "expr1 from test GT");
        code_expr(symbol_table, local_stack_count, test->children[2].get());
        pop(5, local_stack_count);
        if (test->children[0]->production_rule_tokens.second.back() == "int") {
            cout << "slt $3, $3, $5" << endl;
        } else {
            cout << "sltu $3, $3, $5" << endl;
        }
    } else if (op == "NE") {
        code_expr(symbol_table, local_stack_count, test->children[0].get());
        push(3, local_stack_count, "expr1 from test NE");
        code_expr(symbol_table, local_stack_count, test->children[2].get());
        pop(5, local_stack_count);
        if (test->children[0]->production_rule_tokens.second.back() == "int") {
            cout << "slt $6, $3, $5 ; $6 = $3 < $5" << endl;
            cout << "slt $7, $5, $3 ; $7 = $5 < $3" << endl;
            cout << "add $3, $6, $7" << endl;
        } else {
            cout << "sltu $6, $3, $5 ; $6 = $3 < $5" << endl;
            cout << "sltu $7, $5, $3 ; $7 = $5 < $3" << endl;
            cout << "add $3, $6, $7" << endl;
        }

    } else if (op == "EQ") {
        code_expr(symbol_table, local_stack_count, test->children[0].get());
        push(3, local_stack_count, "expr1 from test EQ");
        code_expr(symbol_table, local_stack_count, test->children[2].get());
        pop(5, local_stack_count);
        if (test->children[0]->production_rule_tokens.second.back() == "int") {
            cout << "slt $6, $3, $5 ; $6 = $3 < $5" << endl;
            cout << "slt $7, $5, $3 ; $7 = $5 < $3" << endl;
            cout << "add $3, $6, $7" << endl;
            cout << "sub $3, $11, $3" << endl;
        } else {
            cout << "sltu $6, $3, $5 ; $6 = $3 < $5" << endl;
            cout << "sltu $7, $5, $3 ; $7 = $5 < $3" << endl;
            cout << "add $3, $6, $7" << endl;
            cout << "sub $3, $11, $3" << endl;
        }
    } else if (op == "LE") {
        code_expr(symbol_table, local_stack_count, test->children[0].get());
        push(3, local_stack_count, "expr1 from test LE");
        code_expr(symbol_table, local_stack_count, test->children[2].get());
        pop(5, local_stack_count);

        if (test->children[0]->production_rule_tokens.second.back() == "int") {
            cout << "slt $3, $3, $5" << endl;
            cout << "sub $3, $11, $3" << endl;
        } else {
            cout << "sltu $3, $3, $5" << endl;
            cout << "sub $3, $11, $3" << endl;
        }
    } else if (op == "GE") {
        code_expr(symbol_table, local_stack_count, test->children[0].get());
        push(3, local_stack_count, "expr1 from test LT");
        code_expr(symbol_table, local_stack_count, test->children[2].get());
        pop(5, local_stack_count);

        if (test->children[0]->production_rule_tokens.second.back() == "int") {
            cout << "slt $3, $5, $3" << endl;
            cout << "sub $3, $11, $3" << endl;
        } else {
            cout << "sltu $3, $5, $3" << endl;
            cout << "sub $3, $11, $3" << endl;
        }
    }
}

void parse_statements(map<string, pair<string, int>> &symbol_table, int &local_stack_count, Node *statements_node);

void code_if(map<string, pair<string, int>> symbol_table, int &local_stack_count, Node *if_node) {
    if_count++;
    int cur_if_cout = if_count;
    code_test(symbol_table, local_stack_count, if_node->children[2].get());
    cout << "beq $3, $0, else" << cur_if_cout << endl;
    parse_statements(symbol_table, local_stack_count, if_node->children[5].get());
    cout << "beq $0, $0, endif" << cur_if_cout << endl;
    cout << "else" << cur_if_cout << ":" << endl;
    parse_statements(symbol_table, local_stack_count, if_node->children[9].get());
    cout << "endif" << cur_if_cout << ":" << endl;
}

void code_while(map<string, pair<string, int>> symbol_table, int &local_stack_count, Node *while_node) {
    while_count++;
    int cur_while_cout = while_count;
    cout << "loop" << cur_while_cout << ":" << endl;
    code_test(symbol_table, local_stack_count, while_node->children[2].get());
    cout << "beq $3, $0, endWhile" << cur_while_cout << endl;
    parse_statements(symbol_table, local_stack_count, while_node->children[5].get());
    cout << "beq $0, $0, loop" << cur_while_cout << endl;
    cout << "endWhile" << cur_while_cout << ":" << endl;
}

void code_new(map<string, pair<string, int>> symbol_table, int &local_stack_count, Node *factor_new_node) {
    // factor → NEW INT LBRACK expr RBRACK
    code_expr(symbol_table, local_stack_count, factor_new_node->children[3].get());
    cout << "add $1, $3, $0 ; new procedure expects value in $1" << endl;
    push(31, local_stack_count, "saving 31 before new");
    lis(5, "new");
    cout << "jalr $5" << endl;
    pop(31, local_stack_count);
    cout << "bne $3, $0, 1 ; if call succeeded skip next instruction" << endl;
    cout << "add $3, $11, $0 ; set $3 to NULL if allocation fails" << endl;
}

void code_delete(map<string, pair<string, int>> symbol_table, int &local_stack_count, Node *delete_node) {
    // statement → DELETE LBRACK RBRACK expr SEMI
    delete_count++;
    int cur_delete_cout = delete_count;
    code_expr(symbol_table, local_stack_count, delete_node->children[3].get());
    cout << "beq $3, $11, skipDelete" << cur_delete_cout << " ; do not call delete on NULL" << endl;
    cout << "add $1, $3, $0 ; delete expects the address in $1" << endl;
    push(31, local_stack_count, "save 31 before delete");
    lis(5, "delete");
    cout << "jalr $5" << endl;
    pop(31, local_stack_count);
    cout << "skipDelete" << cur_delete_cout << ":" << endl;
}

void code_print(map<string, pair<string, int>> symbol_table, int &local_stack_count, Node *print_node) {
    push(1, local_stack_count, "storing $1 before print");
    code_expr(symbol_table, local_stack_count, print_node->children[2].get());
    cout << "add $1, $3, $0" << endl; 
    push(31, local_stack_count, "storing $31 before print");
    cout << "jalr $10" << endl;
    pop(31, local_stack_count);
    pop(1, local_stack_count);
}

void code_dcls(
    map<string, pair<vector<string>, map<string, pair<string, int>>>> tables,
    string proc_name,
    int &local_stack_count,
    Node *dcls_node
) {
    if (dcls_node->children.empty()) { // dcls → 
        return;
    }

    if (dcls_node->production_rule_tokens.second[3] == "NUM") { // dcls → dcls dcl BECOMES NUM SEMI
        string id = dcls_node->children[1]->children[1]->lexeme;
        string rhs = dcls_node->children[3]->lexeme;
        lis(5, rhs);
        cout << "sw $5, " << tables[proc_name].second[id].second << "($29)" << endl;
        decrement_stack_ptr();
        local_stack_count++;
        code_dcls(tables, proc_name, local_stack_count, dcls_node->children[0].get());
    } else { // dcls → dcls dcl BECOMES NULL SEMI
        string id = dcls_node->children[1]->children[1]->lexeme;
        cout << "add $5, $0, $11 ; NULL ptr" << endl;
        cout << "sw $5, " << tables[proc_name].second[id].second << "($29)" << endl;
        decrement_stack_ptr();
        local_stack_count++;
        code_dcls(tables, proc_name, local_stack_count, dcls_node->children[0].get());
    }
}

void lw_from_frame_ptr(int reg, int offset) {
    cout << "lw $" << reg << ", " << offset << "($29)" << endl;
}

void parse_statements(map<string, pair<string, int>> &symbol_table, int &local_stack_count, Node *statements_node) {

    // statements → statements statement

    if (statements_node->children.empty()) {
        return;
    }
    parse_statements(symbol_table, local_stack_count, statements_node->children[0].get());

    // statement → lvalue BECOMES expr SEMI
    if (statements_node->children[1]->production_rule_tokens.second[0] == "lvalue") {
        if (statements_node->children[1]->children[0]->production_rule_tokens.second[0] == "STAR") { // lvalue → STAR factor
            code_expr(symbol_table, local_stack_count, statements_node->children[1]->children[2].get());
            push(3, local_stack_count, "expr from lvalue BECOMES expr SEMI");
            code_factor(symbol_table, local_stack_count, statements_node->children[1]->children[0]->children[1].get());
            pop(5, local_stack_count);
            cout << "sw $5, 0($3) ; dereferencing lhs and storing new value at that addr" << endl;
        } else { // lvalue → ID   or lvalue → LPAREN lvalue RPAREN  
            string lhs = get_lvalue_lexeme(statements_node->children[1]->children[0]);
            code_expr(symbol_table, local_stack_count, statements_node->children[1]->children[2].get()); // $3 <- expr
            cout << "sw $3, " << symbol_table[lhs].second << "($29)";
            cout << " ; update " << lhs << endl;
        }
        
    } else if (statements_node->children[1]->production_rule_tokens.second[0] == "IF") {
        code_if(symbol_table, local_stack_count, statements_node->children[1].get());
    } else if (statements_node->children[1]->production_rule_tokens.second[0] == "WHILE") {
        code_while(symbol_table, local_stack_count, statements_node->children[1].get());
    } else if (statements_node->children[1]->production_rule_tokens.second[0] == "PRINTLN") {
        code_print(symbol_table, local_stack_count, statements_node->children[1].get());
    } else if (statements_node->children[1]->production_rule_tokens.second[0] == "DELETE") { // statement → DELETE LBRACK RBRACK expr SEMI
        code_delete(symbol_table, local_stack_count, statements_node->children[1].get());
    }
}

void jr() { cout << "jr $31" << endl; }

void process_procedures(
    Node *procedures_subtree, 
    map<string, pair<vector<string>, map<string, pair<string, int>>>> tables) {
// procedures_subtree is either
//  procedures → main
// or
//  procedures → procedure procedures
    if (procedures_subtree->children.size() != 1) { // get to wain first to do postorder traversal
        process_procedures(procedures_subtree->children[1].get(), tables);
    }

    if (procedures_subtree->children.size() == 1) { //  procedures → main
        int local_stack_count = 0;
        cout << "; begin prologue" << endl;
        cout << ".import print" << endl;
        cout << ".import init" << endl;
        cout << ".import new" << endl; 
        cout << ".import delete" << endl; 
        lis(4, "4"); // lis $4 \n .word 4
        lis(10, "print"); // lis $10 \n .word print
        lis(11, "1"); // lis $11 \n .word 1
        cout << "sub $29, $30, $4" << endl;
        push(1, local_stack_count, "storing arg_1 of main");
        push(2, local_stack_count, "storing arg_2 of main");
        if (procedures_subtree->children[0]->children[3]->children[0]->production_rule_tokens.second.size() == 1) { // if arg1 is int (for init)
            cout << "add $2, $0, $0" << endl;
        }
        push(31, local_stack_count, "storing return address of main");
        lis(5, "init");
        cout << "jalr $5" << endl;
        pop(31, local_stack_count);
        code_dcls(tables, "wain", local_stack_count, procedures_subtree->children[0]->children[8].get());
        cout << "; end prologue" << endl;
        parse_statements(tables["wain"].second, local_stack_count, procedures_subtree->children[0]->children[9].get());
        code_expr(tables["wain"].second, local_stack_count, procedures_subtree->children[0]->children[11].get()); // $3 <- expr

        cout << "; begin epilogue" << endl;      
        for (int i = 0; i < local_stack_count; i++) { // <--------------------NOT SURE IF SHOULD STILL BE USING STACK_COUNT????
            increment_stack_ptr();
        }
        jr();

    } else { // procedures → procedure procedures
        int local_stack_count = 0;
        Node *procedure_node = procedures_subtree->children[0].get();
        string proc_name = procedure_node->children[1]->lexeme;
        cout << "F" << proc_name << ":" << endl;
        cout << "sub $29, $30, $4" << endl;
        cout << "; begin prologue for " << proc_name << "-----------------------" << endl;
        code_dcls(tables, proc_name, local_stack_count, procedure_node->children[6].get());
        // SAVE REGISTERS $
        push(5, local_stack_count, "saving $5");
        push(6, local_stack_count, "saving $6");
        push(7, local_stack_count, "saving $7");
        cout << "; end prologue" << endl;

        parse_statements(tables[proc_name].second, local_stack_count, procedure_node->children[7].get());
        code_expr(tables[proc_name].second, local_stack_count, procedure_node->children[9].get()); // $3 <- expr

        cout << "; begin epilogue" << endl;
        pop(7, local_stack_count);
        pop(6, local_stack_count);
        pop(5, local_stack_count);

        // cout << "LOCAL STACK COUNT before dcls popped: " << local_stack_count << endl;

        for (int i = 0; i < local_stack_count; i++) { // <--------------------NOT SURE IF SHOULD STILL BE USING STACK_COUNT????
            // pop(1, local_stack_count); // POP THE DCLS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 
            cout << "; pop dcl (just add to $30)" << endl;
            increment_stack_ptr();
        }
        jr();
        cout << "; end epilogue for " << proc_name << "-----------------------" << endl;
    }

}

void build_symbol_table(
    map<string, pair<vector<string>, map<string, pair<string, int>>>> &tables,
    Node *procedures_subtree
);

int main() {
    shared_ptr<Node> root = build_tree();
    
    map<string, pair<vector<string>, map<string, pair<string, int>>>> tables;
    build_symbol_table(tables, root->children[1].get());

    // ------- TESTING SYMBOL TABLE
    // for (auto &proc : tables) {
    //     cout << "TABLE FOR " << proc.first << endl;
    //     for (auto &table : proc.second.second) {
    //         cout << "  " << table.first << ": " << table.second.first << " " << table.second.second << endl;
    //     }
    // }
    // -------

    process_procedures(root->children[1].get(), tables);
    return 0;
}


void get_paramlist(
    Node *params_node,
    string proc_name,
    int param_count,
    int cur_param_i,
    map<string, pair<vector<string>, map<string, pair<string, int>>>> &tables
) {
    if (params_node->production_rule_tokens.second.empty()) {
        return;
    }

    if (params_node->production_rule_tokens.second[0] == "paramlist") { // params -> paramlist
        get_paramlist(params_node->children[0].get(), proc_name, param_count, cur_param_i, tables);
    } else if (params_node->production_rule_tokens.second.size() == 3) { // paramlist -> dcl COMMA paramlist
        string type = params_node->children[0]->children[0]->production_rule_tokens.second.size() == 1 ? "int" : "int*";
        string name = params_node->children[0]->children[1]->lexeme; // get the id lexeme of the param
        int offset = 4 * (param_count - cur_param_i + 1); // get the offset of the param
        tables[proc_name].first.push_back(type);
        tables[proc_name].second[name] = make_pair(type, offset);
        cur_param_i++;

        // cout << "stored param " << name << endl;

        get_paramlist(params_node->children[2].get(), proc_name, param_count, cur_param_i, tables);
    } else { // paramlist -> dcl
        string type = params_node->children[0]->children[0]->production_rule_tokens.second.size() == 1 ? "int" : "int*";
        string name = params_node->children[0]->children[1]->lexeme; // get the id lexeme of the param
        int offset = 4 * (param_count - cur_param_i + 1); // get the offset of the param
        tables[proc_name].first.push_back(type);
        tables[proc_name].second[name] = make_pair(type, offset);

        // cout << "stored param " << name << endl;
    }
}

int count_params(Node *params_node) { // params -> paramlist or params -> .EMPTY
    int param_count = 0;
    // count in a while looop
    if (params_node->production_rule_tokens.second.empty()) { // params -> .EMPTY
        return param_count;
    }
    // params -> paramlist
    Node *paramlist_node = params_node->children[0].get();
    
    while (paramlist_node->production_rule_tokens.second.size() == 3) {

        // cout << "COUNTING PARAMS " << param_count << endl;
        // cout << paramlist_node->production_rule_str << endl;
        // cout << paramlist_node->children.size() << endl;
        // cout << paramlist_node->children[0]->production_rule_str << endl;
        // cout << paramlist_node->children[1]->production_rule_str << endl;
        // cout << paramlist_node->children[2]->production_rule_str << endl;

        param_count++;
        paramlist_node = paramlist_node->children[2].get();
     }
    param_count++; // +1 for paramlist -> dcl
    return param_count;
}

void get_dcls(
    Node *dcls_node,
    string name,
    int starting_frame_ptr_offset,
    map<string, pair<vector<string>, map<string, pair<string, int>>>> &tables
) {
    int frame_ptr_offset = starting_frame_ptr_offset;
    while (!dcls_node->production_rule_tokens.second.empty()) {
        string type = dcls_node->children[1]->children[0]->production_rule_tokens.second.size() == 1 ? "int" : "int*";
        string id = dcls_node->children[1]->children[1]->lexeme;
        tables[name].second[id] = make_pair(type, frame_ptr_offset);
        frame_ptr_offset -= 4;
        dcls_node = dcls_node->children[0].get();
    }
}


void build_symbol_table(
    map<string, pair<vector<string>, map<string, pair<string, int>>>> &tables,
    Node *procedures_subtree) { // chaged root to procedure_root
// procedure root is either 
//  procedure → INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE
// or
//  main → INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE
    if (procedures_subtree->children.size() == 2) { // procedures → procedure procedures
        string name = procedures_subtree->children[0]->children[1]->lexeme;


        // cout << "GOT TO PROCEDURE " << name << endl;


        vector<string> paramlist;
        tables[name] = make_pair<>(paramlist, map<string, pair<string, int>>());
        int param_count = count_params(procedures_subtree->children[0]->children[3].get());
        // update tables[name] with params

        // cout << "param_count: " << param_count << endl;

        get_paramlist(procedures_subtree->children[0]->children[3].get(), name, param_count, 1, tables);


        // cout << "GOT PARAMLIST" << endl;

        // update tables[name] with dcls
        get_dcls(procedures_subtree->children[0]->children[6].get(), name, 0, tables);
        build_symbol_table(tables, procedures_subtree->children[1].get());
    } else { // procedures → main
        Node *wain_subtree_root = procedures_subtree->children[0].get();
        string wain_arg_1_id = wain_subtree_root->children[3]->children[1]->lexeme;
        string wain_arg_2_id = wain_subtree_root->children[5]->children[1]->lexeme;
        string wain_arg_1_type = wain_subtree_root->children[3]->children[0]->production_rule_tokens.second.size() == 1 ? "int" : "int*";
        string wain_arg_2_type = wain_subtree_root->children[5]->children[0]->production_rule_tokens.second.size() == 1 ? "int" : "int*";
        tables["wain"].second[wain_arg_1_id] = make_pair(wain_arg_1_type, 0);
        tables["wain"].second[wain_arg_2_id] = make_pair(wain_arg_2_type, -4);
        get_dcls(wain_subtree_root->children[8].get(), "wain", -8, tables);
    }
}
