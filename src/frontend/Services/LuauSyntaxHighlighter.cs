using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Media;
using AetherVisor.Frontend.Controls;

namespace AetherVisor.Frontend.Services
{
    public class LuauSyntaxHighlighter
    {
        private readonly AdvancedScriptEditor _editor;
        private readonly Dictionary<string, SolidColorBrush> _colorScheme;

        // Regex patterns for syntax highlighting
        private readonly List<HighlightRule> _highlightRules;

        public LuauSyntaxHighlighter(AdvancedScriptEditor editor)
        {
            _editor = editor;
            _colorScheme = InitializeColorScheme();
            _highlightRules = InitializeHighlightRules();
        }

        public void Initialize()
        {
            // Setup initial highlighting
        }

        public async Task HighlightAsync(string text)
        {
            await Task.Run(() =>
            {
                var runs = GenerateHighlightedRuns(text);
                
                Application.Current.Dispatcher.BeginInvoke(() =>
                {
                    ApplyHighlighting(runs);
                });
            });
        }

        private Dictionary<string, SolidColorBrush> InitializeColorScheme()
        {
            return new Dictionary<string, SolidColorBrush>
            {
                ["keyword"] = new SolidColorBrush(Color.FromRgb(86, 156, 214)),     // Blue
                ["string"] = new SolidColorBrush(Color.FromRgb(206, 145, 120)),     // Orange
                ["comment"] = new SolidColorBrush(Color.FromRgb(106, 153, 85)),     // Green
                ["number"] = new SolidColorBrush(Color.FromRgb(181, 206, 168)),     // Light green
                ["function"] = new SolidColorBrush(Color.FromRgb(220, 220, 170)),   // Yellow
                ["variable"] = new SolidColorBrush(Color.FromRgb(156, 220, 254)),   // Light blue
                ["operator"] = new SolidColorBrush(Color.FromRgb(212, 212, 212)),   // White
                ["punctuation"] = new SolidColorBrush(Color.FromRgb(212, 212, 212)), // White
                ["type"] = new SolidColorBrush(Color.FromRgb(78, 201, 176)),        // Teal
                ["builtin"] = new SolidColorBrush(Color.FromRgb(86, 156, 214)),     // Blue
                ["default"] = new SolidColorBrush(Color.FromRgb(212, 212, 212))     // White
            };
        }

        private List<HighlightRule> InitializeHighlightRules()
        {
            return new List<HighlightRule>
            {
                // Comments (highest priority)
                new HighlightRule
                {
                    Pattern = @"--.*$",
                    Type = "comment",
                    Options = RegexOptions.Multiline
                },
                
                // Multi-line comments
                new HighlightRule
                {
                    Pattern = @"--\[\[.*?\]\]",
                    Type = "comment",
                    Options = RegexOptions.Singleline
                },

                // Strings
                new HighlightRule
                {
                    Pattern = @"""([^""\\]|\\.)*""",
                    Type = "string"
                },
                new HighlightRule
                {
                    Pattern = @"'([^'\\]|\\.)*'",
                    Type = "string"
                },
                new HighlightRule
                {
                    Pattern = @"\[\[.*?\]\]",
                    Type = "string",
                    Options = RegexOptions.Singleline
                },

                // Numbers
                new HighlightRule
                {
                    Pattern = @"\b\d+\.?\d*([eE][+-]?\d+)?\b",
                    Type = "number"
                },
                new HighlightRule
                {
                    Pattern = @"\b0[xX][0-9a-fA-F]+\b",
                    Type = "number"
                },

                // Keywords
                new HighlightRule
                {
                    Pattern = @"\b(and|break|continue|do|else|elseif|end|false|for|function|if|in|local|nil|not|or|repeat|return|then|true|until|while|export|type)\b",
                    Type = "keyword"
                },

                // Types (Luau type annotations)
                new HighlightRule
                {
                    Pattern = @"\b(string|number|boolean|table|function|thread|userdata|any|never|unknown)\b",
                    Type = "type"
                },

                // Built-in functions and globals
                new HighlightRule
                {
                    Pattern = @"\b(print|warn|error|assert|type|typeof|getmetatable|setmetatable|rawget|rawset|rawequal|rawlen|next|pairs|ipairs|select|tonumber|tostring|pcall|xpcall|coroutine|string|table|math|os|debug|game|workspace|script|_G|_VERSION)\b",
                    Type = "builtin"
                },

                // Roblox-specific globals
                new HighlightRule
                {
                    Pattern = @"\b(Instance|Vector3|Vector2|CFrame|UDim2|UDim|Color3|BrickColor|Ray|Region3|Enum)\b",
                    Type = "builtin"
                },

                // Function definitions
                new HighlightRule
                {
                    Pattern = @"\bfunction\s+([a-zA-Z_][a-zA-Z0-9_]*)",
                    Type = "function",
                    CaptureGroup = 1
                },
                new HighlightRule
                {
                    Pattern = @"([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*function",
                    Type = "function",
                    CaptureGroup = 1
                },

                // Function calls
                new HighlightRule
                {
                    Pattern = @"([a-zA-Z_][a-zA-Z0-9_]*)\s*\(",
                    Type = "function",
                    CaptureGroup = 1
                },

                // Variables (identifiers)
                new HighlightRule
                {
                    Pattern = @"\b[a-zA-Z_][a-zA-Z0-9_]*\b",
                    Type = "variable"
                },

                // Operators
                new HighlightRule
                {
                    Pattern = @"[+\-*/%^#=<>~]|\.\.|\.\.\.|==|~=|<=|>=|//",
                    Type = "operator"
                },

                // Punctuation
                new HighlightRule
                {
                    Pattern = @"[(){}[\],.;:]",
                    Type = "punctuation"
                }
            };
        }

        private List<HighlightedRun> GenerateHighlightedRuns(string text)
        {
            var runs = new List<HighlightedRun>();
            var processedRanges = new List<(int Start, int End)>();

            foreach (var rule in _highlightRules)
            {
                var matches = Regex.Matches(text, rule.Pattern, rule.Options);
                
                foreach (Match match in matches)
                {
                    var start = match.Index;
                    var length = match.Length;
                    var captureGroup = rule.CaptureGroup ?? 0;
                    
                    if (captureGroup > 0 && match.Groups.Count > captureGroup)
                    {
                        var group = match.Groups[captureGroup];
                        start = group.Index;
                        length = group.Length;
                    }

                    var end = start + length;

                    // Check if this range overlaps with already processed ranges
                    if (processedRanges.Any(r => start < r.End && end > r.Start))
                        continue;

                    runs.Add(new HighlightedRun
                    {
                        Start = start,
                        Length = length,
                        Type = rule.Type,
                        Text = text.Substring(start, length)
                    });

                    processedRanges.Add((start, end));
                }
            }

            // Sort by position
            runs.Sort((a, b) => a.Start.CompareTo(b.Start));

            // Fill gaps with default styling
            var filledRuns = new List<HighlightedRun>();
            var currentPos = 0;

            foreach (var run in runs)
            {
                // Add default text before this run
                if (run.Start > currentPos)
                {
                    filledRuns.Add(new HighlightedRun
                    {
                        Start = currentPos,
                        Length = run.Start - currentPos,
                        Type = "default",
                        Text = text.Substring(currentPos, run.Start - currentPos)
                    });
                }

                filledRuns.Add(run);
                currentPos = run.Start + run.Length;
            }

            // Add remaining text
            if (currentPos < text.Length)
            {
                filledRuns.Add(new HighlightedRun
                {
                    Start = currentPos,
                    Length = text.Length - currentPos,
                    Type = "default",
                    Text = text.Substring(currentPos)
                });
            }

            return filledRuns;
        }

        private void ApplyHighlighting(List<HighlightedRun> runs)
        {
            // This is a simplified approach. In a real implementation,
            // you would need to use a more sophisticated text rendering system
            // like TextFormattingMode or a custom text rendering solution.
            
            // For now, we'll just update the colors via the TextBox's formatting
            // Note: WPF TextBox doesn't support rich text formatting natively
            // You would typically use RichTextBox or a custom text editor control
            
            // Here we would apply formatting to the actual text control
            // This is a placeholder for the actual implementation
        }

        public void UpdateColorScheme(Dictionary<string, Color> newColors)
        {
            foreach (var color in newColors)
            {
                if (_colorScheme.ContainsKey(color.Key))
                {
                    _colorScheme[color.Key] = new SolidColorBrush(color.Value);
                }
            }
        }

        public void SetTheme(EditorTheme theme)
        {
            switch (theme)
            {
                case EditorTheme.Dark:
                    ApplyDarkTheme();
                    break;
                case EditorTheme.Light:
                    ApplyLightTheme();
                    break;
                case EditorTheme.HighContrast:
                    ApplyHighContrastTheme();
                    break;
            }
        }

        private void ApplyDarkTheme()
        {
            _colorScheme["keyword"] = new SolidColorBrush(Color.FromRgb(86, 156, 214));
            _colorScheme["string"] = new SolidColorBrush(Color.FromRgb(206, 145, 120));
            _colorScheme["comment"] = new SolidColorBrush(Color.FromRgb(106, 153, 85));
            _colorScheme["number"] = new SolidColorBrush(Color.FromRgb(181, 206, 168));
            _colorScheme["function"] = new SolidColorBrush(Color.FromRgb(220, 220, 170));
            _colorScheme["variable"] = new SolidColorBrush(Color.FromRgb(156, 220, 254));
            _colorScheme["default"] = new SolidColorBrush(Color.FromRgb(212, 212, 212));
        }

        private void ApplyLightTheme()
        {
            _colorScheme["keyword"] = new SolidColorBrush(Color.FromRgb(0, 0, 255));
            _colorScheme["string"] = new SolidColorBrush(Color.FromRgb(163, 21, 21));
            _colorScheme["comment"] = new SolidColorBrush(Color.FromRgb(0, 128, 0));
            _colorScheme["number"] = new SolidColorBrush(Color.FromRgb(9, 134, 88));
            _colorScheme["function"] = new SolidColorBrush(Color.FromRgb(116, 83, 31));
            _colorScheme["variable"] = new SolidColorBrush(Color.FromRgb(0, 0, 0));
            _colorScheme["default"] = new SolidColorBrush(Color.FromRgb(0, 0, 0));
        }

        private void ApplyHighContrastTheme()
        {
            _colorScheme["keyword"] = new SolidColorBrush(Colors.Yellow);
            _colorScheme["string"] = new SolidColorBrush(Colors.Cyan);
            _colorScheme["comment"] = new SolidColorBrush(Colors.Green);
            _colorScheme["number"] = new SolidColorBrush(Colors.Magenta);
            _colorScheme["function"] = new SolidColorBrush(Colors.White);
            _colorScheme["variable"] = new SolidColorBrush(Colors.White);
            _colorScheme["default"] = new SolidColorBrush(Colors.White);
        }
    }

    public class HighlightRule
    {
        public string Pattern { get; set; }
        public string Type { get; set; }
        public RegexOptions Options { get; set; } = RegexOptions.None;
        public int? CaptureGroup { get; set; }
    }

    public class HighlightedRun
    {
        public int Start { get; set; }
        public int Length { get; set; }
        public string Type { get; set; }
        public string Text { get; set; }
    }
}