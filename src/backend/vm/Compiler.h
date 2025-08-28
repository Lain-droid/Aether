#pragma once

#include "VMOpcodes.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <stack>
#include <queue>
#include <cstdint>
#include <cstddef>

// Forward declarations to avoid circular dependencies
namespace AetherVisor {
    namespace VM {
        enum class VMDataType : uint8_t;
        struct VMValue;
        struct VMConstant;
        struct VMFunction;
        struct VMSecurityContext;
        enum class VMOpcode : uint8_t;
    }
}

namespace AetherVisor {
    namespace VM {

        // Token types for lexical analysis
        enum class TokenType {
            // Literals
            INTEGER, FLOAT, STRING, BOOLEAN, IDENTIFIER,
            
            // Operators
            PLUS, MINUS, MULTIPLY, DIVIDE, MODULO,
            ASSIGN, PLUS_ASSIGN, MINUS_ASSIGN,
            
            // Comparison
            EQUAL, NOT_EQUAL, LESS_THAN, GREATER_THAN,
            LESS_EQUAL, GREATER_EQUAL,
            
            // Logical
            AND, OR, NOT,
            
            // Bitwise
            BIT_AND, BIT_OR, BIT_XOR, BIT_NOT, SHL, SHR,
            
            // Punctuation
            SEMICOLON, COMMA, DOT,
            LPAREN, RPAREN, LBRACE, RBRACE, LBRACKET, RBRACKET,
            
            // Keywords
            IF, ELSE, WHILE, FOR, FUNCTION, RETURN, VAR, CONST_KW,
            TRY, CATCH, THROW, TRUE_LIT, FALSE_LIT, NULL_TOKEN,
            
            // Special
            NEWLINE, EOF_TOKEN, UNKNOWN
        };

        // Token structure
        struct Token {
            TokenType type;
            std::string value;
            size_t line;
            size_t column;
            
            Token(TokenType t, const std::string& v, size_t l = 0, size_t c = 0)
                : type(t), value(v), line(l), column(c) {}
        };

        // AST Node types
        enum class ASTNodeType {
            PROGRAM,
            FUNCTION_DECL,
            VAR_DECL,
            ASSIGNMENT,
            BINARY_OP,
            UNARY_OP,
            FUNCTION_CALL,
            IF_STMT,
            WHILE_STMT,
            FOR_STMT,
            RETURN_STMT,
            BLOCK_STMT,
            EXPRESSION_STMT,
            LITERAL,
            IDENTIFIER,
            ARRAY_ACCESS,
            MEMBER_ACCESS,
            TRY_CATCH,
            THROW_STMT
        };

        // AST Node base class
        struct ASTNode {
            ASTNodeType type;
            size_t line;
            size_t column;
            std::vector<std::unique_ptr<ASTNode>> children;
            std::string value; // For literals and identifiers
            
            ASTNode(ASTNodeType t, size_t l = 0, size_t c = 0)
                : type(t), line(l), column(c) {}
            virtual ~ASTNode() = default;
        };

        // Symbol table entry
        struct Symbol {
            std::string name;
            VMDataType type;
            uint32_t address;     // Stack offset or global address
            bool is_global;
            bool is_function;
            bool is_constant;
            VMValue default_value;
        };

        // Scope for symbol management
        class Scope {
        public:
            Scope(Scope* parent = nullptr) : m_parent(parent), m_next_address(0) {}
            
            void DefineSymbol(const std::string& name, const Symbol& symbol);
            Symbol* LookupSymbol(const std::string& name);
            uint32_t AllocateAddress() { return m_next_address++; }
            
        private:
            Scope* m_parent;
            std::unordered_map<std::string, Symbol> m_symbols;
            uint32_t m_next_address;
        };

        // Compilation context
        struct CompilationContext {
            std::vector<VMConstant> constant_pool;
            std::vector<VMFunction> functions;
            std::unique_ptr<Scope> global_scope;
            Scope* current_scope;
            std::vector<uint8_t> bytecode;
            std::vector<std::string> errors;
            std::vector<std::string> warnings;
            
            // Security settings
            VMSecurityContext security;
            bool enable_optimization;
            bool enable_obfuscation;
            bool enable_encryption;
            
            CompilationContext();
        };

        // Advanced compiler with full language support
        class Compiler {
        public:
            Compiler();
            ~Compiler() = default;

            // Main compilation interface
            bool Compile(const std::string& source_code, CompilationContext& context);
            std::vector<uint8_t> GetBytecode(const CompilationContext& context);
            
            // Individual compilation phases
            std::vector<Token> Tokenize(const std::string& source);
            std::unique_ptr<ASTNode> Parse(const std::vector<Token>& tokens);
            bool Analyze(ASTNode* ast, CompilationContext& context);
            bool Generate(ASTNode* ast, CompilationContext& context);
            
            // Optimization passes
            void OptimizeConstantFolding(ASTNode* ast);
            void OptimizeDeadCodeElimination(ASTNode* ast);
            void OptimizeInlining(ASTNode* ast, CompilationContext& context);
            
            // Security features
            void ApplyObfuscation(std::vector<uint8_t>& bytecode);
            void EncryptConstants(CompilationContext& context);
            void InsertAntiAnalysis(std::vector<uint8_t>& bytecode);
            
            // Error handling
            const std::vector<std::string>& GetErrors() const { return m_errors; }
            const std::vector<std::string>& GetWarnings() const { return m_warnings; }
            void ClearDiagnostics() { m_errors.clear(); m_warnings.clear(); }

        private:
            std::vector<std::string> m_errors;
            std::vector<std::string> m_warnings;
            
            // Lexical analysis
            bool IsAlpha(char c);
            bool IsDigit(char c);
            bool IsAlphaNumeric(char c);
            bool IsWhitespace(char c);
            TokenType GetKeywordType(const std::string& word);
            
            // Parsing helpers
            std::unique_ptr<ASTNode> ParseProgram(const std::vector<Token>& tokens, size_t& pos);
            std::unique_ptr<ASTNode> ParseStatement(const std::vector<Token>& tokens, size_t& pos);
            std::unique_ptr<ASTNode> ParseExpression(const std::vector<Token>& tokens, size_t& pos);
            std::unique_ptr<ASTNode> ParseBinaryExpression(const std::vector<Token>& tokens, size_t& pos, int min_precedence);
            std::unique_ptr<ASTNode> ParseUnaryExpression(const std::vector<Token>& tokens, size_t& pos);
            std::unique_ptr<ASTNode> ParsePrimaryExpression(const std::vector<Token>& tokens, size_t& pos);
            std::unique_ptr<ASTNode> ParseFunctionDecl(const std::vector<Token>& tokens, size_t& pos);
            std::unique_ptr<ASTNode> ParseVariableDecl(const std::vector<Token>& tokens, size_t& pos);
            std::unique_ptr<ASTNode> ParseIfStatement(const std::vector<Token>& tokens, size_t& pos);
            std::unique_ptr<ASTNode> ParseWhileStatement(const std::vector<Token>& tokens, size_t& pos);
            std::unique_ptr<ASTNode> ParseForStatement(const std::vector<Token>& tokens, size_t& pos);
            std::unique_ptr<ASTNode> ParseTryCatch(const std::vector<Token>& tokens, size_t& pos);
            
            int GetOperatorPrecedence(TokenType type);
            bool IsRightAssociative(TokenType type);
            
            // Semantic analysis
            bool AnalyzeNode(ASTNode* node, CompilationContext& context);
            bool CheckTypes(ASTNode* node, CompilationContext& context);
            bool ResolveSymbols(ASTNode* node, CompilationContext& context);
            
            // Code generation
            void GenerateNode(ASTNode* node, CompilationContext& context);
            void GenerateExpression(ASTNode* expr, CompilationContext& context);
            void GenerateStatement(ASTNode* stmt, CompilationContext& context);
            void GenerateFunctionCall(ASTNode* call, CompilationContext& context);
            
            // Bytecode emission
            void EmitOpcode(VMOpcode opcode, CompilationContext& context);
            void EmitOperand(uint32_t operand, CompilationContext& context);
            void EmitInstruction(VMOpcode opcode, uint32_t op1, uint32_t op2, uint32_t op3, CompilationContext& context);
            uint32_t AddConstant(const VMValue& value, CompilationContext& context);
            uint32_t GetCurrentAddress(const CompilationContext& context);
            void PatchAddress(uint32_t address, uint32_t value, CompilationContext& context);
            
            // Advanced features
            void GenerateJIT(ASTNode* node, CompilationContext& context);
            void InsertProfilingCode(CompilationContext& context);
            void ApplySecurityMeasures(CompilationContext& context);
            
            // Error reporting
            void ReportError(const std::string& message, size_t line = 0, size_t column = 0);
            void ReportWarning(const std::string& message, size_t line = 0, size_t column = 0);
        };

    } // namespace VM
} // namespace AetherVisor
