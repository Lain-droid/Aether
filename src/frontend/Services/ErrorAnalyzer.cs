using System;
using System.Collections.Generic;
using System.Text.RegularExpressions;

namespace AetherVisor.Frontend.Services
{
    public class ErrorAnalyzer
    {
        private readonly List<Diagnostic> _diagnostics;
        private readonly Regex _syntaxErrorRegex;
        private readonly Regex _runtimeErrorRegex;

        public ErrorAnalyzer()
        {
            _diagnostics = new List<Diagnostic>();
            _syntaxErrorRegex = new Regex(@"syntax error|unexpected token|missing", RegexOptions.IgnoreCase);
            _runtimeErrorRegex = new Regex(@"attempt to|index a|nil value", RegexOptions.IgnoreCase);
        }

        public List<Diagnostic> AnalyzeCode(string code)
        {
            _diagnostics.Clear();
            
            var lines = code.Split('\n');
            
            for (int i = 0; i < lines.Length; i++)
            {
                var line = lines[i];
                AnalyzeLine(line, i + 1);
            }
            
            return _diagnostics;
        }

        private void AnalyzeLine(string line, int lineNumber)
        {
            // Check for common syntax errors
            if (HasUnclosedString(line))
            {
                _diagnostics.Add(new Diagnostic
                {
                    Line = lineNumber,
                    Column = line.Length,
                    Severity = DiagnosticSeverity.Error,
                    Message = "Unclosed string literal",
                    Code = "CS001"
                });
            }
            
            if (HasUnclosedParentheses(line))
            {
                _diagnostics.Add(new Diagnostic
                {
                    Line = lineNumber,
                    Column = line.Length,
                    Severity = DiagnosticSeverity.Error,
                    Message = "Unclosed parentheses",
                    Code = "CS002"
                });
            }
            
            if (HasInvalidFunctionCall(line))
            {
                _diagnostics.Add(new Diagnostic
                {
                    Line = lineNumber,
                    Column = line.IndexOf('(') + 1,
                    Severity = DiagnosticSeverity.Warning,
                    Message = "Invalid function call syntax",
                    Code = "CS003"
                });
            }
            
            if (HasUndefinedVariable(line))
            {
                _diagnostics.Add(new Diagnostic
                {
                    Line = lineNumber,
                    Column = 1,
                    Severity = DiagnosticSeverity.Warning,
                    Message = "Variable might be undefined",
                    Code = "CS004"
                });
            }
        }

        private bool HasUnclosedString(string line)
        {
            bool inString = false;
            char stringChar = '\0';
            
            for (int i = 0; i < line.Length; i++)
            {
                char c = line[i];
                
                if (c == '"' || c == '\'')
                {
                    if (!inString)
                    {
                        inString = true;
                        stringChar = c;
                    }
                    else if (c == stringChar)
                    {
                        inString = false;
                    }
                }
            }
            
            return inString;
        }

        private bool HasUnclosedParentheses(string line)
        {
            int openCount = 0;
            
            foreach (char c in line)
            {
                if (c == '(') openCount++;
                else if (c == ')') openCount--;
            }
            
            return openCount > 0;
        }

        private bool HasInvalidFunctionCall(string line)
        {
            // Check for function calls without proper syntax
            var match = Regex.Match(line, @"\w+\s*\(\s*[^)]*$");
            return match.Success && !line.Trim().EndsWith("(");
        }

        private bool HasUndefinedVariable(string line)
        {
            // Simple check for variables that might be undefined
            var words = line.Split(new[] { ' ', '\t', '(', ')', '[', ']', ',', ';' }, StringSplitOptions.RemoveEmptyEntries);
            
            foreach (var word in words)
            {
                if (word.StartsWith("local ") || word.StartsWith("function "))
                    continue;
                    
                if (Regex.IsMatch(word, @"^[a-zA-Z_][a-zA-Z0-9_]*$") && 
                    !IsKeyword(word) && !IsBuiltInFunction(word))
                {
                    // This is a potential undefined variable
                    return true;
                }
            }
            
            return false;
        }

        private bool IsKeyword(string word)
        {
            var keywords = new[] { "and", "break", "do", "else", "elseif", "end", "false", "for", "function", 
                                 "if", "in", "local", "nil", "not", "or", "repeat", "return", "then", "true", 
                                 "until", "while" };
            return Array.Exists(keywords, k => k == word);
        }

        private bool IsBuiltInFunction(string word)
        {
            var builtins = new[] { "print", "warn", "error", "assert", "type", "tonumber", "tostring", 
                                 "pairs", "ipairs", "next", "select", "pcall", "xpcall" };
            return Array.Exists(builtins, b => b == word);
        }

        public List<Diagnostic> GetDiagnostics()
        {
            return _diagnostics;
        }
    }

    public class Diagnostic
    {
        public int Line { get; set; }
        public int Column { get; set; }
        public DiagnosticSeverity Severity { get; set; }
        public string Message { get; set; }
        public string Code { get; set; }
        public string? Source { get; set; }
    }

    public enum DiagnosticSeverity
    {
        Error,
        Warning,
        Information,
        Hint
    }
}