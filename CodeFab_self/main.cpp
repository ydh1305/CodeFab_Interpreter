#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <stdexcept>
#include <cctype>

// 1. 토큰 및 AST 노드
enum class TokenType {
    VAR, PRINT, IDENTIFIER, NUMBER, EQUAL,
    PLUS, MINUS, STAR, SLASH, LESS, GREATER, // LESS, GREATER 추가
    LPAREN, RPAREN, LBRACE, RBRACE, SEMICOLON, EOF_TOKEN
};

struct Token { TokenType type; std::string origin; double value = 0.0; };

struct Expr { virtual ~Expr() = default; };
struct LiteralExpr : Expr { double value; LiteralExpr(double v) : value(v) {} };
struct VariableExpr : Expr { Token name; VariableExpr(Token t) : name(t) {} };
struct BinaryExpr : Expr {
    std::shared_ptr<Expr> left; Token op; std::shared_ptr<Expr> right;
    BinaryExpr(std::shared_ptr<Expr> l, Token o, std::shared_ptr<Expr> r) : left(l), op(o), right(r) {}
};
struct UnaryExpr : Expr {
    Token op; std::shared_ptr<Expr> right;
    UnaryExpr(Token o, std::shared_ptr<Expr> r) : op(o), right(r) {}
};
struct GroupingExpr : Expr { std::shared_ptr<Expr> expression; GroupingExpr(std::shared_ptr<Expr> e) : expression(e) {} };

struct Stmt { virtual ~Stmt() = default; };
struct VarStmt : Stmt { Token name; std::shared_ptr<Expr> initializer; VarStmt(Token n, std::shared_ptr<Expr> i) : name(n), initializer(i) {} };
struct PrintStmt : Stmt { std::shared_ptr<Expr> expression; PrintStmt(std::shared_ptr<Expr> e) : expression(e) {} };

// 2. Tokenizer (LESS, GREATER 추가)
class Tokenizer {
    std::string source; size_t current = 0;
public:
    Tokenizer(std::string src) : source(src) {}
    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (current < source.length()) {
            char c = source[current++];
            if (isspace(c)) continue;
            if (isdigit(c)) {
                size_t start = current - 1;
                while (current < source.length() && isdigit(source[current])) current++;
                tokens.push_back({ TokenType::NUMBER, source.substr(start, current - start), std::stod(source.substr(start, current - start)) });
            }
            else if (isalpha(c)) {
                size_t start = current - 1;
                while (current < source.length() && isalnum(source[current])) current++;
                std::string s = source.substr(start, current - start);
                if (s == "var") tokens.push_back({ TokenType::VAR, s });
                else if (s == "print") tokens.push_back({ TokenType::PRINT, s });
                else tokens.push_back({ TokenType::IDENTIFIER, s });
            }
            else if (c == '<') tokens.push_back({ TokenType::LESS, "<" });
            else if (c == '>') tokens.push_back({ TokenType::GREATER, ">" });
            else if (c == '+') tokens.push_back({ TokenType::PLUS, "+" });
            else if (c == '-') tokens.push_back({ TokenType::MINUS, "-" });
            else if (c == '*') tokens.push_back({ TokenType::STAR, "*" });
            else if (c == '/') tokens.push_back({ TokenType::SLASH, "/" });
            else if (c == '=') tokens.push_back({ TokenType::EQUAL, "=" });
            else if (c == ';') tokens.push_back({ TokenType::SEMICOLON, ";" });
            else if (c == '(') tokens.push_back({ TokenType::LPAREN, "(" });
            else if (c == ')') tokens.push_back({ TokenType::RPAREN, ")" });
            else throw std::runtime_error("알 수 없는 문자: " + std::string(1, c));
        }
        tokens.push_back({ TokenType::EOF_TOKEN, "" });
        return tokens;
    }
};

// 3. Parser (연산자 우선순위 체계 확립)
class Parser {
    std::vector<Token> tokens; size_t pos = 0;
public:
    Parser(std::vector<Token> t) : tokens(t) {}

    std::shared_ptr<Expr> primary() {
        if (tokens[pos].type == TokenType::NUMBER) return std::make_shared<LiteralExpr>(tokens[pos++].value);
        if (tokens[pos].type == TokenType::IDENTIFIER) return std::make_shared<VariableExpr>(tokens[pos++]);
        if (tokens[pos].type == TokenType::LPAREN) {
            pos++; auto expr = comparison(); // 괄호 내부에서는 최상위 레벨 호출
            if (tokens[pos].type != TokenType::RPAREN) throw std::runtime_error("')' 필요");
            pos++; return std::make_shared<GroupingExpr>(expr);
        }
        throw std::runtime_error("표현식 오류");
    }

    std::shared_ptr<Expr> unary() {
        if (tokens[pos].type == TokenType::MINUS) return std::make_shared<UnaryExpr>(tokens[pos++], unary());
        return primary();
    }

    std::shared_ptr<Expr> factor() {
        auto expr = unary();
        while (tokens[pos].type == TokenType::STAR || tokens[pos].type == TokenType::SLASH) {
            Token op = tokens[pos++];
            expr = std::make_shared<BinaryExpr>(expr, op, unary());
        }
        return expr;
    }

    std::shared_ptr<Expr> expression() {
        auto expr = factor();
        while (tokens[pos].type == TokenType::PLUS || tokens[pos].type == TokenType::MINUS) {
            Token op = tokens[pos++];
            expr = std::make_shared<BinaryExpr>(expr, op, factor());
        }
        return expr;
    }

    std::shared_ptr<Expr> comparison() { // [추가] 비교 연산 레벨
        auto expr = expression();
        while (tokens[pos].type == TokenType::LESS || tokens[pos].type == TokenType::GREATER) {
            Token op = tokens[pos++];
            expr = std::make_shared<BinaryExpr>(expr, op, expression());
        }
        return expr;
    }

    std::shared_ptr<Stmt> statement() {
        if (tokens[pos].type == TokenType::VAR) {
            pos++; Token name = tokens[pos++]; pos++;
            auto init = comparison(); pos++;
            return std::make_shared<VarStmt>(name, init);
        }
        if (tokens[pos].type == TokenType::PRINT) {
            pos++; auto expr = comparison(); pos++;
            return std::make_shared<PrintStmt>(expr);
        }
        throw std::runtime_error("문장 파싱 오류");
    }

    std::vector<std::shared_ptr<Stmt>> parse() {
        std::vector<std::shared_ptr<Stmt>> stmts;
        while (tokens[pos].type != TokenType::EOF_TOKEN) stmts.push_back(statement());
        return stmts;
    }
};

// 4. Executor
class Environment {
public:
    std::map<std::string, double> values;
    std::shared_ptr<Environment> enclosing;
    Environment(std::shared_ptr<Environment> enc = nullptr) : enclosing(enc) {}
    void define(std::string n, double v) { values[n] = v; }
    double get(std::string n) {
        if (values.count(n)) return values[n];
        if (enclosing) return enclosing->get(n);
        throw std::runtime_error("미정의 변수: " + n);
    }
};

class Executor {
    std::shared_ptr<Environment> env;
public:
    Executor(std::shared_ptr<Environment> e) : env(e) {}

    double evaluate(std::shared_ptr<Expr> e) {
        if (auto lit = std::dynamic_pointer_cast<LiteralExpr>(e)) return lit->value;
        if (auto var = std::dynamic_pointer_cast<VariableExpr>(e)) return env->get(var->name.origin);
        if (auto grp = std::dynamic_pointer_cast<GroupingExpr>(e)) return evaluate(grp->expression);
        if (auto un = std::dynamic_pointer_cast<UnaryExpr>(e)) {
            double right = evaluate(un->right);
            if (un->op.type == TokenType::MINUS) return -right;
        }
        if (auto bin = std::dynamic_pointer_cast<BinaryExpr>(e)) {
            double l = evaluate(bin->left);
            double r = evaluate(bin->right);
            if (bin->op.type == TokenType::PLUS) return l + r;
            if (bin->op.type == TokenType::MINUS) return l - r;
            if (bin->op.type == TokenType::STAR) return l * r;
            if (bin->op.type == TokenType::SLASH) {
                if (r == 0) throw std::runtime_error("0으로 나눈 오류");
                return l / r;
            }
            if (bin->op.type == TokenType::LESS) return l < r ? 1.0 : 0.0;
            if (bin->op.type == TokenType::GREATER) return l > r ? 1.0 : 0.0;
        }
        return 0;
    }

    void execute(std::shared_ptr<Stmt> s) {
        if (auto var = std::dynamic_pointer_cast<VarStmt>(s)) env->define(var->name.origin, evaluate(var->initializer));
        if (auto prt = std::dynamic_pointer_cast<PrintStmt>(s)) {
            double val = evaluate(prt->expression);
            // 비교 결과(1.0/0.0)를 true/false로 출력하는 로직 추가
            // (주의: 단순 비교문일 경우만 적용하거나, 전체 숫자에 적용할 수 있습니다)
            std::cout << (val == 1.0 ? "true" : (val == 0.0 ? "false" : std::to_string(val))) << std::endl;
        }
    }
};

int main() {
    auto global = std::make_shared<Environment>();
    std::string line;
    while (std::cout << ">>> ", std::getline(std::cin, line)) {
        if (line == "exit") break;
        try {
            auto tokens = Tokenizer(line).tokenize();
            auto stmts = Parser(tokens).parse();
            for (auto& s : stmts) Executor(global).execute(s);
        }
        catch (const std::exception& e) { std::cout << e.what() << std::endl; }
    }
    return 0;
}