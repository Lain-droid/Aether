using System;
using System.Collections.Generic;
using System.Text.RegularExpressions;

namespace AetherVisor.Frontend.Services
{
    public class CodeFoldingManager
    {
        private readonly List<FoldingRegion> _foldingRegions;
        private readonly Regex _functionRegex;
        private readonly Regex _ifRegex;
        private readonly Regex _forRegex;
        private readonly Regex _whileRegex;

        public CodeFoldingManager()
        {
            _foldingRegions = new List<FoldingRegion>();
            _functionRegex = new Regex(@"^\s*(local\s+)?function\s+\w+", RegexOptions.Compiled);
            _ifRegex = new Regex(@"^\s*if\s+", RegexOptions.Compiled);
            _forRegex = new Regex(@"^\s*for\s+", RegexOptions.Compiled);
            _whileRegex = new Regex(@"^\s*while\s+", RegexOptions.Compiled);
        }

        public List<FoldingRegion> GetFoldingRegions(string code)
        {
            _foldingRegions.Clear();
            var lines = code.Split('\n');
            
            for (int i = 0; i < lines.Length; i++)
            {
                var line = lines[i];
                
                if (_functionRegex.IsMatch(line))
                {
                    var endLine = FindBlockEnd(lines, i, "end");
                    if (endLine > i)
                    {
                        _foldingRegions.Add(new FoldingRegion
                        {
                            StartLine = i,
                            EndLine = endLine,
                            Type = FoldingType.Function,
                            Name = ExtractFunctionName(line)
                        });
                    }
                }
                else if (_ifRegex.IsMatch(line))
                {
                    var endLine = FindBlockEnd(lines, i, "end");
                    if (endLine > i)
                    {
                        _foldingRegions.Add(new FoldingRegion
                        {
                            StartLine = i,
                            EndLine = endLine,
                            Type = FoldingType.Conditional,
                            Name = "if statement"
                        });
                    }
                }
                else if (_forRegex.IsMatch(line))
                {
                    var endLine = FindBlockEnd(lines, i, "end");
                    if (endLine > i)
                    {
                        _foldingRegions.Add(new FoldingRegion
                        {
                            StartLine = i,
                            EndLine = endLine,
                            Type = FoldingType.Loop,
                            Name = "for loop"
                        });
                    }
                }
                else if (_whileRegex.IsMatch(line))
                {
                    var endLine = FindBlockEnd(lines, i, "end");
                    if (endLine > i)
                    {
                        _foldingRegions.Add(new FoldingRegion
                        {
                            StartLine = i,
                            EndLine = endLine,
                            Type = FoldingType.Loop,
                            Name = "while loop"
                        });
                    }
                }
            }
            
            return _foldingRegions;
        }

        private int FindBlockEnd(string[] lines, int startLine, string endKeyword)
        {
            int depth = 0;
            
            for (int i = startLine; i < lines.Length; i++)
            {
                var line = lines[i].Trim();
                
                if (line.StartsWith("if ") || line.StartsWith("for ") || line.StartsWith("while ") || 
                    line.StartsWith("function ") || line.StartsWith("local function "))
                {
                    depth++;
                }
                else if (line == "end")
                {
                    depth--;
                    if (depth == 0)
                    {
                        return i;
                    }
                }
            }
            
            return startLine;
        }

        private string ExtractFunctionName(string line)
        {
            var match = Regex.Match(line, @"function\s+(\w+)");
            return match.Success ? match.Groups[1].Value : "function";
        }
    }

    public class FoldingRegion
    {
        public int StartLine { get; set; }
        public int EndLine { get; set; }
        public FoldingType Type { get; set; }
        public string Name { get; set; }
        public bool IsCollapsed { get; set; }
    }

    public enum FoldingType
    {
        Function,
        Conditional,
        Loop,
        Comment,
        Region
    }
}