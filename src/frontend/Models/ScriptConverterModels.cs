using System;
using System.Collections.Generic;
using System.ComponentModel;

namespace AetherVisor.Frontend.Models
{
    public class ConversionResult : INotifyPropertyChanged
    {
        private bool _success;
        private string _convertedCode;
        private string _errorMessage;

        public bool Success
        {
            get => _success;
            set { _success = value; OnPropertyChanged(nameof(Success)); }
        }

        public string ConvertedCode
        {
            get => _convertedCode;
            set { _convertedCode = value; OnPropertyChanged(nameof(ConvertedCode)); }
        }

        public string ErrorMessage
        {
            get => _errorMessage;
            set { _errorMessage = value; OnPropertyChanged(nameof(ErrorMessage)); }
        }

        public List<string> Warnings { get; set; } = new();
        public List<ConversionSuggestion> Suggestions { get; set; } = new();
        public ConversionStatistics Statistics { get; set; } = new();
        public DateTime ConversionTime { get; set; } = DateTime.Now;

        public bool HasWarnings => Warnings.Count > 0;
        public bool HasSuggestions => Suggestions.Count > 0;
        public string WarningsText => string.Join("\n", Warnings);
        public string StatusText => Success ? "Conversion successful" : "Conversion failed";

        public event PropertyChangedEventHandler? PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class ConversionOptions
    {
        public bool ApplyOptimizations { get; set; } = true;
        public bool AddComments { get; set; } = true;
        public bool PreserveFormatting { get; set; } = false;
        public bool ValidateOutput { get; set; } = true;
        public bool GenerateSuggestions { get; set; } = true;
        public ConversionStyle Style { get; set; } = ConversionStyle.Conservative;
        public Dictionary<string, object> CustomOptions { get; set; } = new();
    }

    public enum ConversionStyle
    {
        Conservative,  // Minimal changes, preserve original structure
        Modernized,    // Apply modern Luau conventions
        Optimized      // Focus on performance optimizations
    }

    public class ConversionSuggestion : INotifyPropertyChanged
    {
        private bool _isApplied;

        public string Id { get; set; } = Guid.NewGuid().ToString();
        public SuggestionType Type { get; set; }
        public string Description { get; set; }
        public string Original { get; set; }
        public string Suggested { get; set; }
        public int Line { get; set; }
        public int Column { get; set; }
        public SuggestionPriority Priority { get; set; } = SuggestionPriority.Medium;
        public bool IsAutomaticApplicable { get; set; } = true;

        public bool IsApplied
        {
            get => _isApplied;
            set { _isApplied = value; OnPropertyChanged(nameof(IsApplied)); }
        }

        public string TypeIcon
        {
            get
            {
                return Type switch
                {
                    SuggestionType.SyntaxChange => "🔧",
                    SuggestionType.Optimization => "⚡",
                    SuggestionType.Modernization => "🆕",
                    SuggestionType.StyleImprovement => "✨",
                    SuggestionType.FunctionMapping => "🔄",
                    SuggestionType.WarningFix => "⚠️",
                    _ => "💡"
                };
            }
        }

        public string PriorityColor
        {
            get
            {
                return Priority switch
                {
                    SuggestionPriority.High => "#D13438",
                    SuggestionPriority.Medium => "#FFCC02",
                    SuggestionPriority.Low => "#6A9955",
                    _ => "#007ACC"
                };
            }
        }

        public event PropertyChangedEventHandler? PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public enum SuggestionType
    {
        SyntaxChange,
        Optimization,
        Modernization,
        StyleImprovement,
        FunctionMapping,
        WarningFix,
        BestPractice
    }

    public enum SuggestionPriority
    {
        Low,
        Medium,
        High,
        Critical
    }

    public class ConversionStatistics
    {
        public int SourceLines { get; set; }
        public int ConvertedLines { get; set; }
        public int CharactersChanged { get; set; }
        public int RulesApplied { get; set; }
        public TimeSpan ConversionTime { get; set; }
        public Dictionary<string, int> ChangesByType { get; set; } = new();

        public double ChangePercentage => SourceLines > 0 ? 
            (double)CharactersChanged / (SourceLines * 50) * 100 : 0; // Rough estimate

        public string FormattedStats => 
            $"{SourceLines} → {ConvertedLines} lines, {RulesApplied} rules, {ConversionTime.TotalMilliseconds:F0}ms";
    }

    public class ConversionProject : INotifyPropertyChanged
    {
        private string _name;
        private string _description;

        public string Id { get; set; } = Guid.NewGuid().ToString();
        
        public string Name
        {
            get => _name;
            set { _name = value; OnPropertyChanged(nameof(Name)); }
        }

        public string Description
        {
            get => _description;
            set { _description = value; OnPropertyChanged(nameof(Description)); }
        }

        public List<ConversionFile> Files { get; set; } = new();
        public ConversionOptions DefaultOptions { get; set; } = new();
        public DateTime CreatedAt { get; set; } = DateTime.Now;
        public DateTime? LastModifiedAt { get; set; }
        public string SourceFormat { get; set; }
        public string TargetFormat { get; set; }

        public int TotalFiles => Files.Count;
        public int CompletedFiles => Files.Count(f => f.IsConverted);
        public double ProgressPercentage => TotalFiles > 0 ? (double)CompletedFiles / TotalFiles * 100 : 0;
        public string ProgressText => $"{CompletedFiles}/{TotalFiles} files converted";

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class ConversionFile : INotifyPropertyChanged
    {
        private string _fileName;
        private bool _isConverted;
        private ConversionResult _result;

        public string Id { get; set; } = Guid.NewGuid().ToString();
        
        public string FileName
        {
            get => _fileName;
            set { _fileName = value; OnPropertyChanged(nameof(FileName)); }
        }

        public string FilePath { get; set; }
        public string SourceCode { get; set; }
        
        public bool IsConverted
        {
            get => _isConverted;
            set { _isConverted = value; OnPropertyChanged(nameof(IsConverted)); OnPropertyChanged(nameof(StatusText)); }
        }

        public ConversionResult Result
        {
            get => _result;
            set { _result = value; OnPropertyChanged(nameof(Result)); }
        }

        public DateTime? ConvertedAt { get; set; }
        public long FileSizeBytes { get; set; }

        public string StatusText => IsConverted ? "Converted" : "Pending";
        public string FileSizeText => FormatFileSize(FileSizeBytes);

        private string FormatFileSize(long bytes)
        {
            string[] sizes = { "B", "KB", "MB", "GB" };
            double len = bytes;
            int order = 0;
            while (len >= 1024 && order < sizes.Length - 1)
            {
                order++;
                len = len / 1024;
            }
            return $"{len:F2} {sizes[order]}";
        }

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class ConversionTemplate
    {
        public string Id { get; set; } = Guid.NewGuid().ToString();
        public string Name { get; set; }
        public string Description { get; set; }
        public string SourceFormat { get; set; }
        public string TargetFormat { get; set; }
        public ConversionOptions Options { get; set; } = new();
        public List<ConversionRule> CustomRules { get; set; } = new();
        public bool IsBuiltIn { get; set; }
        public string Author { get; set; }
        public DateTime CreatedAt { get; set; } = DateTime.Now;

        public string FormattedInfo => $"{SourceFormat} → {TargetFormat}";
    }

    public class ConversionHistory : INotifyPropertyChanged
    {
        public string Id { get; set; } = Guid.NewGuid().ToString();
        public string FileName { get; set; }
        public string SourceFormat { get; set; }
        public string TargetFormat { get; set; }
        public DateTime ConvertedAt { get; set; }
        public bool WasSuccessful { get; set; }
        public int WarningCount { get; set; }
        public TimeSpan ConversionTime { get; set; }

        public string StatusIcon => WasSuccessful ? "✅" : "❌";
        public string FormattedTime => ConvertedAt.ToString("MMM dd, yyyy HH:mm");
        public string ConversionInfo => $"{SourceFormat} → {TargetFormat}";

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class ConversionRule
    {
        public string Name { get; set; } = string.Empty;
        public string Description { get; set; } = string.Empty;
        public string Pattern { get; set; } = string.Empty;
        public string Replacement { get; set; } = string.Empty;
        public bool IsEnabled { get; set; } = true;
        public int Priority { get; set; } = 0;
        public ConversionRuleType Type { get; set; } = ConversionRuleType.Syntax;
    }

    public enum ConversionRuleType
    {
        Syntax,
        API,
        Performance,
        Style,
        Custom
    }
}