using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using AetherVisor.Frontend.Models;

namespace AetherVisor.Frontend.Services
{
    public class ScriptConverterService
    {
        private readonly Dictionary<string, Func<string, ConversionResult>> _converters;
        private readonly List<ConversionRule> _luaToLuauRules;
        private readonly List<ConversionRule> _optimizationRules;
        
        public List<string> SupportedSourceFormats { get; }
        public List<string> SupportedTargetFormats { get; }
        
        public ScriptConverterService()
        {
            SupportedSourceFormats = new List<string> { "lua", "luau", "javascript", "python", "pseudo" };
            SupportedTargetFormats = new List<string> { "luau", "lua" };
            
            _converters = new Dictionary<string, Func<string, ConversionResult>>
            {
                ["lua_to_luau"] = ConvertLuaToLuau,
                ["luau_to_lua"] = ConvertLuauToLua,
                ["javascript_to_luau"] = ConvertJavaScriptToLuau,
                ["python_to_luau"] = ConvertPythonToLuau,
                ["pseudo_to_luau"] = ConvertPseudoToLuau
            };
            
            InitializeLuaToLuauRules();
            InitializeOptimizationRules();
        }
        
        #region Public Methods
        
        public async Task<ConversionResult> ConvertAsync(string sourceCode, string sourceFormat, string targetFormat, ConversionOptions options = null)
        {
            try
            {
                options ??= new ConversionOptions();
                
                var conversionKey = $"{sourceFormat.ToLower()}_to_{targetFormat.ToLower()}";
                
                if (!_converters.ContainsKey(conversionKey))
                {
                    return new ConversionResult
                    {
                        Success = false,
                        ErrorMessage = $"Conversion from {sourceFormat} to {targetFormat} is not supported"
                    };
                }
                
                var result = await Task.Run(() => _converters[conversionKey](sourceCode));
                
                if (result.Success && options.ApplyOptimizations)
                {
                    result = ApplyOptimizations(result);
                }
                
                if (result.Success && options.AddComments)
                {
                    result = AddConversionComments(result, sourceFormat, targetFormat);
                }
                
                return result;
            }
            catch (Exception ex)
            {
                return new ConversionResult
                {
                    Success = false,
                    ErrorMessage = $"Conversion failed: {ex.Message}"
                };
            }
        }
        
        public ConversionResult ValidateSourceCode(string sourceCode, string format)
        {
            try
            {
                var result = new ConversionResult { Success = true };
                
                switch (format.ToLower())
                {
                    case "lua":
                    case "luau":
                        result = ValidateLuauSyntax(sourceCode);
                        break;
                    case "javascript":
                        result = ValidateJavaScriptSyntax(sourceCode);
                        break;
                    case "python":
                        result = ValidatePythonSyntax(sourceCode);
                        break;
                    default:
                        result.Warnings.Add("Syntax validation not available for this format");
                        break;
                }
                
                return result;
            }
            catch (Exception ex)
            {
                return new ConversionResult
                {
                    Success = false,
                    ErrorMessage = $"Validation failed: {ex.Message}"
                };
            }
        }
        
        public List<ConversionSuggestion> GetConversionSuggestions(string sourceCode, string sourceFormat)
        {
            var suggestions = new List<ConversionSuggestion>();
            
            try
            {
                switch (sourceFormat.ToLower())
                {
                    case "lua":
                        suggestions.AddRange(GetLuaToLuauSuggestions(sourceCode));
                        break;
                    case "javascript":
                        suggestions.AddRange(GetJavaScriptToLuauSuggestions(sourceCode));
                        break;
                    case "python":
                        suggestions.AddRange(GetPythonToLuauSuggestions(sourceCode));
                        break;
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Failed to generate suggestions: {ex.Message}");
            }
            
            return suggestions;
        }
        
        public ConversionResult OptimizeCode(string luauCode)
        {
            try
            {
                var result = new ConversionResult
                {
                    Success = true,
                    ConvertedCode = luauCode
                };
                
                return ApplyOptimizations(result);
            }
            catch (Exception ex)
            {
                return new ConversionResult
                {
                    Success = false,
                    ErrorMessage = $"Optimization failed: {ex.Message}"
                };
            }
        }
        
        public ConversionResult MinifyCode(string luauCode)
        {
            try
            {
                var minified = luauCode;
                
                // Remove comments
                minified = Regex.Replace(minified, @"--.*$", "", RegexOptions.Multiline);
                
                // Remove extra whitespace
                minified = Regex.Replace(minified, @"\s+", " ");
                
                // Remove unnecessary spaces around operators
                minified = Regex.Replace(minified, @"\s*([=+\-*/%<>~,;(){}[\]])\s*", "$1");
                
                // Remove leading/trailing whitespace from lines
                var lines = minified.Split('\n')
                    .Select(line => line.Trim())
                    .Where(line => !string.IsNullOrEmpty(line));
                
                return new ConversionResult
                {
                    Success = true,
                    ConvertedCode = string.Join("\n", lines)
                };
            }
            catch (Exception ex)
            {
                return new ConversionResult
                {
                    Success = false,
                    ErrorMessage = $"Minification failed: {ex.Message}"
                };
            }
        }
        
        public ConversionResult FormatCode(string luauCode, CodeFormattingOptions options = null)
        {
            try
            {
                options ??= new CodeFormattingOptions();
                
                var formatted = FormatLuauCode(luauCode, options);
                
                return new ConversionResult
                {
                    Success = true,
                    ConvertedCode = formatted
                };
            }
            catch (Exception ex)
            {
                return new ConversionResult
                {
                    Success = false,
                    ErrorMessage = $"Formatting failed: {ex.Message}"
                };
            }
        }
        
        #endregion
        
        #region Conversion Methods
        
        private ConversionResult ConvertLuaToLuau(string luaCode)
        {
            var result = new ConversionResult
            {
                Success = true,
                ConvertedCode = luaCode
            };
            
            foreach (var rule in _luaToLuauRules)
            {
                try
                {
                    if (rule.IsRegex)
                    {
                        result.ConvertedCode = Regex.Replace(result.ConvertedCode, rule.Pattern, rule.Replacement);
                    }
                    else
                    {
                        result.ConvertedCode = result.ConvertedCode.Replace(rule.Pattern, rule.Replacement);
                    }
                    
                    if (!string.IsNullOrEmpty(rule.Description))
                    {
                        result.Warnings.Add(rule.Description);
                    }
                }
                catch (Exception ex)
                {
                    result.Warnings.Add($"Rule application failed: {ex.Message}");
                }
            }
            
            return result;
        }
        
        private ConversionResult ConvertLuauToLua(string luauCode)
        {
            var result = new ConversionResult
            {
                Success = true,
                ConvertedCode = luauCode
            };
            
            // Remove Luau-specific type annotations
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @":\s*\w+(\s*\?)?", "");
            
            // Convert continue to goto statements (basic conversion)
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"\bcontinue\b", "goto continue");
            
            // Add warnings about potential compatibility issues
            if (Regex.IsMatch(luauCode, @"\bcontinue\b"))
            {
                result.Warnings.Add("'continue' statements converted to 'goto' - manual adjustment may be needed");
            }
            
            if (Regex.IsMatch(luauCode, @":\s*\w+"))
            {
                result.Warnings.Add("Type annotations removed - Lua doesn't support type hints");
            }
            
            return result;
        }
        
        private ConversionResult ConvertJavaScriptToLuau(string jsCode)
        {
            var result = new ConversionResult
            {
                Success = true,
                ConvertedCode = jsCode
            };
            
            // Convert var/let/const to local
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"\b(var|let|const)\s+", "local ");
            
            // Convert function declarations
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"function\s+(\w+)\s*\(", "local function $1(");
            
            // Convert arrow functions (basic)
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"(\w+)\s*=>\s*", "function($1) return ");
            
            // Convert console.log to print
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"console\.log\s*\(", "print(");
            
            // Convert null to nil
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"\bnull\b", "nil");
            
            // Convert undefined to nil
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"\bundefined\b", "nil");
            
            // Convert == to == (no change needed for equality)
            // Convert === to == (Luau doesn't have strict equality)
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"===", "==");
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"!==", "~=");
            
            // Convert array notation
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"\[(\d+)\]", "[$1]");
            
            result.Warnings.Add("JavaScript to Luau conversion is approximate - manual review recommended");
            result.Warnings.Add("Arrow functions, classes, and complex expressions may need manual adjustment");
            
            return result;
        }
        
        private ConversionResult ConvertPythonToLuau(string pythonCode)
        {
            var result = new ConversionResult
            {
                Success = true,
                ConvertedCode = pythonCode
            };
            
            // Convert print statements
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"print\s*\((.*?)\)", "print($1)");
            
            // Convert def to function
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"def\s+(\w+)\s*\(", "local function $1(");
            
            // Convert None to nil
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"\bNone\b", "nil");
            
            // Convert True/False to true/false
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"\bTrue\b", "true");
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"\bFalse\b", "false");
            
            // Convert and/or operators
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"\band\b", "and");
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"\bor\b", "or");
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"\bnot\b", "not");
            
            // Handle indentation - convert to explicit end statements
            var lines = result.ConvertedCode.Split('\n');
            var converted = new List<string>();
            var indentStack = new Stack<int>();
            
            for (int i = 0; i < lines.Length; i++)
            {
                var line = lines[i];
                var trimmed = line.TrimStart();
                var currentIndent = line.Length - trimmed.Length;
                
                // Handle dedentation
                while (indentStack.Count > 0 && currentIndent < indentStack.Peek())
                {
                    indentStack.Pop();
                    converted.Add(new string(' ', currentIndent) + "end");
                }
                
                if (trimmed.EndsWith(":"))
                {
                    // Remove colon and track indentation
                    converted.Add(line.Substring(0, line.Length - 1) + " then");
                    indentStack.Push(currentIndent);
                }
                else
                {
                    converted.Add(line);
                }
            }
            
            // Close remaining blocks
            while (indentStack.Count > 0)
            {
                indentStack.Pop();
                converted.Add("end");
            }
            
            result.ConvertedCode = string.Join("\n", converted);
            
            result.Warnings.Add("Python to Luau conversion is approximate - manual review required");
            result.Warnings.Add("List comprehensions, classes, and imports need manual conversion");
            result.Warnings.Add("Indentation converted to explicit 'end' statements");
            
            return result;
        }
        
        private ConversionResult ConvertPseudoToLuau(string pseudoCode)
        {
            var result = new ConversionResult
            {
                Success = true,
                ConvertedCode = pseudoCode
            };
            
            // Convert common pseudocode patterns
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"\bBEGIN\b", "", RegexOptions.IgnoreCase);
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"\bEND\b", "end", RegexOptions.IgnoreCase);
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"\bIF\b", "if", RegexOptions.IgnoreCase);
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"\bTHEN\b", "then", RegexOptions.IgnoreCase);
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"\bELSE\b", "else", RegexOptions.IgnoreCase);
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"\bWHILE\b", "while", RegexOptions.IgnoreCase);
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"\bFOR\b", "for", RegexOptions.IgnoreCase);
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"\bDO\b", "do", RegexOptions.IgnoreCase);
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"\bREPEAT\b", "repeat", RegexOptions.IgnoreCase);
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"\bUNTIL\b", "until", RegexOptions.IgnoreCase);
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"\bPRINT\b", "print", RegexOptions.IgnoreCase);
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"\bINPUT\b", "io.read", RegexOptions.IgnoreCase);
            
            // Convert assignment operator
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"←", "=");
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @":=", "=");
            
            // Convert comparison operators
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"≠", "~=");
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"≤", "<=");
            result.ConvertedCode = Regex.Replace(result.ConvertedCode, @"≥", ">=");
            
            result.Warnings.Add("Pseudocode conversion is basic - significant manual editing likely required");
            
            return result;
        }
        
        #endregion
        
        #region Helper Methods
        
        private void InitializeLuaToLuauRules()
        {
            _luaToLuauRules = new List<ConversionRule>
            {
                new ConversionRule
                {
                    Pattern = @"table\.unpack",
                    Replacement = "unpack",
                    IsRegex = true,
                    Description = "Converted table.unpack to unpack for Luau compatibility"
                },
                new ConversionRule
                {
                    Pattern = @"math\.huge",
                    Replacement = "math.huge",
                    IsRegex = false,
                    Description = null // No change needed, but keeping for future rules
                }
            };
        }
        
        private void InitializeOptimizationRules()
        {
            _optimizationRules = new List<ConversionRule>
            {
                new ConversionRule
                {
                    Pattern = @"if\s+(.+)\s+==\s+true\s+then",
                    Replacement = "if $1 then",
                    IsRegex = true,
                    Description = "Optimized boolean comparison"
                },
                new ConversionRule
                {
                    Pattern = @"if\s+(.+)\s+==\s+false\s+then",
                    Replacement = "if not $1 then",
                    IsRegex = true,
                    Description = "Optimized negative boolean comparison"
                },
                new ConversionRule
                {
                    Pattern = @"string\.len\(([^)]+)\)",
                    Replacement = "#$1",
                    IsRegex = true,
                    Description = "Optimized string length operator"
                }
            };
        }
        
        private ConversionResult ApplyOptimizations(ConversionResult result)
        {
            foreach (var rule in _optimizationRules)
            {
                try
                {
                    var oldCode = result.ConvertedCode;
                    
                    if (rule.IsRegex)
                    {
                        result.ConvertedCode = Regex.Replace(result.ConvertedCode, rule.Pattern, rule.Replacement);
                    }
                    else
                    {
                        result.ConvertedCode = result.ConvertedCode.Replace(rule.Pattern, rule.Replacement);
                    }
                    
                    if (oldCode != result.ConvertedCode && !string.IsNullOrEmpty(rule.Description))
                    {
                        result.Warnings.Add(rule.Description);
                    }
                }
                catch (Exception ex)
                {
                    result.Warnings.Add($"Optimization rule failed: {ex.Message}");
                }
            }
            
            return result;
        }
        
        private ConversionResult AddConversionComments(ConversionResult result, string sourceFormat, string targetFormat)
        {
            var header = $"-- Converted from {sourceFormat} to {targetFormat} using Aether Script Converter\n" +
                        $"-- Conversion date: {DateTime.Now:yyyy-MM-dd HH:mm:ss}\n" +
                        $"-- Please review the converted code for accuracy\n\n";
            
            result.ConvertedCode = header + result.ConvertedCode;
            
            if (result.Warnings.Any())
            {
                var warningsComment = "\n-- Conversion warnings:\n" +
                    string.Join("\n", result.Warnings.Select(w => $"-- {w}"));
                result.ConvertedCode += warningsComment;
            }
            
            return result;
        }
        
        private ConversionResult ValidateLuauSyntax(string code)
        {
            var result = new ConversionResult { Success = true };
            
            // Basic syntax validation
            if (HasUnmatchedParentheses(code))
            {
                result.Warnings.Add("Unmatched parentheses detected");
            }
            
            if (HasUnmatchedBrackets(code))
            {
                result.Warnings.Add("Unmatched brackets detected");
            }
            
            if (HasUnmatchedQuotes(code))
            {
                result.Warnings.Add("Unmatched quotes detected");
            }
            
            // Check for common Luau patterns
            var lines = code.Split('\n');
            for (int i = 0; i < lines.Length; i++)
            {
                var line = lines[i].Trim();
                
                if (line.StartsWith("function") && !line.Contains("end") && i == lines.Length - 1)
                {
                    result.Warnings.Add($"Line {i + 1}: Function without 'end' statement");
                }
                
                if (line.StartsWith("if") && !line.Contains("then"))
                {
                    result.Warnings.Add($"Line {i + 1}: 'if' statement without 'then'");
                }
            }
            
            return result;
        }
        
        private ConversionResult ValidateJavaScriptSyntax(string code)
        {
            var result = new ConversionResult { Success = true };
            
            if (HasUnmatchedParentheses(code))
            {
                result.Warnings.Add("Unmatched parentheses detected");
            }
            
            if (HasUnmatchedBrackets(code))
            {
                result.Warnings.Add("Unmatched brackets detected");
            }
            
            return result;
        }
        
        private ConversionResult ValidatePythonSyntax(string code)
        {
            var result = new ConversionResult { Success = true };
            
            // Check indentation consistency
            var lines = code.Split('\n');
            var indentPattern = "";
            
            foreach (var line in lines)
            {
                if (string.IsNullOrWhiteSpace(line)) continue;
                
                var leadingWhitespace = line.Substring(0, line.Length - line.TrimStart().Length);
                if (string.IsNullOrEmpty(indentPattern) && !string.IsNullOrEmpty(leadingWhitespace))
                {
                    indentPattern = leadingWhitespace.Substring(0, 1); // First character (space or tab)
                }
                
                if (!string.IsNullOrEmpty(leadingWhitespace) && 
                    leadingWhitespace.Contains(indentPattern == " " ? "\t" : " "))
                {
                    result.Warnings.Add("Mixed indentation (spaces and tabs) detected");
                    break;
                }
            }
            
            return result;
        }
        
        private List<ConversionSuggestion> GetLuaToLuauSuggestions(string code)
        {
            var suggestions = new List<ConversionSuggestion>();
            
            if (code.Contains("table.unpack"))
            {
                suggestions.Add(new ConversionSuggestion
                {
                    Type = SuggestionType.Modernization,
                    Description = "Consider using 'unpack' instead of 'table.unpack' in Luau",
                    Original = "table.unpack",
                    Suggested = "unpack"
                });
            }
            
            return suggestions;
        }
        
        private List<ConversionSuggestion> GetJavaScriptToLuauSuggestions(string code)
        {
            var suggestions = new List<ConversionSuggestion>();
            
            if (code.Contains("console.log"))
            {
                suggestions.Add(new ConversionSuggestion
                {
                    Type = SuggestionType.FunctionMapping,
                    Description = "Replace console.log with print",
                    Original = "console.log",
                    Suggested = "print"
                });
            }
            
            return suggestions;
        }
        
        private List<ConversionSuggestion> GetPythonToLuauSuggestions(string code)
        {
            var suggestions = new List<ConversionSuggestion>();
            
            if (Regex.IsMatch(code, @"def\s+\w+"))
            {
                suggestions.Add(new ConversionSuggestion
                {
                    Type = SuggestionType.SyntaxChange,
                    Description = "Python functions need to be converted to Luau function syntax",
                    Original = "def function_name():",
                    Suggested = "local function function_name()"
                });
            }
            
            return suggestions;
        }
        
        private string FormatLuauCode(string code, CodeFormattingOptions options)
        {
            var lines = code.Split('\n');
            var formatted = new List<string>();
            var indentLevel = 0;
            
            foreach (var line in lines)
            {
                var trimmed = line.Trim();
                if (string.IsNullOrEmpty(trimmed))
                {
                    formatted.Add("");
                    continue;
                }
                
                // Decrease indent for end statements
                if (trimmed == "end" || trimmed == "else" || trimmed == "elseif" || trimmed.StartsWith("elseif"))
                {
                    indentLevel = Math.Max(0, indentLevel - 1);
                }
                
                // Add indentation
                var indent = new string(options.UseSpaces ? ' ' : '\t', 
                    options.UseSpaces ? indentLevel * options.IndentSize : indentLevel);
                formatted.Add(indent + trimmed);
                
                // Increase indent for block statements
                if (trimmed.EndsWith("then") || trimmed.EndsWith("do") || 
                    trimmed.StartsWith("function") || trimmed == "else")
                {
                    indentLevel++;
                }
            }
            
            return string.Join("\n", formatted);
        }
        
        private bool HasUnmatchedParentheses(string code)
        {
            var count = 0;
            var inString = false;
            var stringChar = '\0';
            
            for (int i = 0; i < code.Length; i++)
            {
                var c = code[i];
                
                if (!inString && (c == '"' || c == '\''))
                {
                    inString = true;
                    stringChar = c;
                }
                else if (inString && c == stringChar && (i == 0 || code[i - 1] != '\\'))
                {
                    inString = false;
                }
                else if (!inString)
                {
                    if (c == '(') count++;
                    else if (c == ')') count--;
                }
            }
            
            return count != 0;
        }
        
        private bool HasUnmatchedBrackets(string code)
        {
            var count = 0;
            var inString = false;
            var stringChar = '\0';
            
            for (int i = 0; i < code.Length; i++)
            {
                var c = code[i];
                
                if (!inString && (c == '"' || c == '\''))
                {
                    inString = true;
                    stringChar = c;
                }
                else if (inString && c == stringChar && (i == 0 || code[i - 1] != '\\'))
                {
                    inString = false;
                }
                else if (!inString)
                {
                    if (c == '[') count++;
                    else if (c == ']') count--;
                }
            }
            
            return count != 0;
        }
        
        private bool HasUnmatchedQuotes(string code)
        {
            var singleQuoteCount = 0;
            var doubleQuoteCount = 0;
            
            for (int i = 0; i < code.Length; i++)
            {
                var c = code[i];
                
                if (c == '\'' && (i == 0 || code[i - 1] != '\\'))
                {
                    singleQuoteCount++;
                }
                else if (c == '"' && (i == 0 || code[i - 1] != '\\'))
                {
                    doubleQuoteCount++;
                }
            }
            
            return singleQuoteCount % 2 != 0 || doubleQuoteCount % 2 != 0;
        }
        
        #endregion
    }
    
    #region Supporting Classes
    
    public class ConversionRule
    {
        public string Pattern { get; set; }
        public string Replacement { get; set; }
        public bool IsRegex { get; set; }
        public string Description { get; set; }
    }
    
    public class CodeFormattingOptions
    {
        public bool UseSpaces { get; set; } = true;
        public int IndentSize { get; set; } = 4;
        public bool PreserveComments { get; set; } = true;
        public bool AddSpacesAroundOperators { get; set; } = true;
    }
    
    #endregion
}