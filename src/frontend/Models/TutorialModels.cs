using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows;

namespace AetherVisor.Frontend.Models
{
    public class Tutorial : INotifyPropertyChanged
    {
        private string _name;
        private string _description;
        private bool _isCompleted;
        private bool _isFavorite;

        public string Id { get; set; }
        
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

        public string Category { get; set; }
        public TutorialDifficulty Difficulty { get; set; }
        public TimeSpan EstimatedDuration { get; set; }
        public DateTime CreatedAt { get; set; }
        public DateTime? UpdatedAt { get; set; }
        public string Author { get; set; }
        public string Version { get; set; }
        public bool IsCustom { get; set; }
        public List<string> Tags { get; set; } = new();
        public List<string> Prerequisites { get; set; } = new();
        public List<TutorialStep> Steps { get; set; } = new();
        public string ThumbnailUrl { get; set; }
        public float Rating { get; set; }
        public int RatingCount { get; set; }

        public bool IsCompleted
        {
            get => _isCompleted;
            set { _isCompleted = value; OnPropertyChanged(nameof(IsCompleted)); }
        }

        public bool IsFavorite
        {
            get => _isFavorite;
            set { _isFavorite = value; OnPropertyChanged(nameof(IsFavorite)); }
        }

        public string DifficultyText => Difficulty.ToString();
        public string DifficultyColor
        {
            get
            {
                return Difficulty switch
                {
                    TutorialDifficulty.Beginner => "#6A9955",
                    TutorialDifficulty.Intermediate => "#FFCC02",
                    TutorialDifficulty.Advanced => "#FF8C00",
                    TutorialDifficulty.Expert => "#D13438",
                    _ => "#007ACC"
                };
            }
        }

        public string EstimatedDurationText
        {
            get
            {
                if (EstimatedDuration.TotalHours >= 1)
                    return $"{EstimatedDuration.TotalHours:F1}h";
                else
                    return $"{EstimatedDuration.TotalMinutes:F0}min";
            }
        }

        public string StepCountText => $"{Steps?.Count ?? 0} steps";
        public string FormattedRating => Rating > 0 ? $"{Rating:F1} ⭐ ({RatingCount})" : "No ratings";
        public string TagsText => Tags != null ? string.Join(", ", Tags) : "";

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public enum TutorialDifficulty
    {
        Beginner = 1,
        Intermediate = 2,
        Advanced = 3,
        Expert = 4
    }

    public class TutorialStep : INotifyPropertyChanged
    {
        private string _title;
        private string _content;
        private bool _isCompleted;

        public string Id { get; set; }
        public int StepNumber { get; set; }
        
        public string Title
        {
            get => _title;
            set { _title = value; OnPropertyChanged(nameof(Title)); }
        }

        public string Content
        {
            get => _content;
            set { _content = value; OnPropertyChanged(nameof(Content)); }
        }

        public TutorialStepType Type { get; set; }
        public string Code { get; set; }
        public string ExpectedAction { get; set; }
        public string TargetElement { get; set; }
        public Rect? HighlightArea { get; set; }
        public TimeSpan ExpectedDuration { get; set; }
        public List<string> Images { get; set; } = new();
        public List<TutorialHint> Hints { get; set; } = new();
        public string VideoUrl { get; set; }
        public Dictionary<string, object> Metadata { get; set; } = new();

        public bool IsCompleted
        {
            get => _isCompleted;
            set { _isCompleted = value; OnPropertyChanged(nameof(IsCompleted)); }
        }

        public string TypeIcon
        {
            get
            {
                return Type switch
                {
                    TutorialStepType.Information => "ℹ️",
                    TutorialStepType.Interactive => "🎯",
                    TutorialStepType.Guided => "👆",
                    TutorialStepType.Quiz => "❓",
                    TutorialStepType.Exercise => "💪",
                    _ => "📝"
                };
            }
        }

        public string TypeColor
        {
            get
            {
                return Type switch
                {
                    TutorialStepType.Information => "#007ACC",
                    TutorialStepType.Interactive => "#6A9955",
                    TutorialStepType.Guided => "#FFCC02",
                    TutorialStepType.Quiz => "#9B59B6",
                    TutorialStepType.Exercise => "#E67E22",
                    _ => "#95A5A6"
                };
            }
        }

        public bool HasCode => !string.IsNullOrEmpty(Code);
        public bool HasHints => Hints != null && Hints.Count > 0;
        public bool HasVideo => !string.IsNullOrEmpty(VideoUrl);
        public bool HasImages => Images != null && Images.Count > 0;

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public enum TutorialStepType
    {
        Information,
        Interactive,
        Guided,
        Quiz,
        Exercise,
        Video,
        Reading
    }

    public class TutorialHint
    {
        public string Id { get; set; }
        public string Title { get; set; }
        public string Content { get; set; }
        public int OrderIndex { get; set; }
        public bool IsRevealed { get; set; }
    }

    public class TutorialProgress : INotifyPropertyChanged
    {
        private bool _isStarted;
        private bool _isCompleted;
        private int _currentStepIndex;
        private double _completionPercentage;

        public string TutorialId { get; set; }
        
        public bool IsStarted
        {
            get => _isStarted;
            set { _isStarted = value; OnPropertyChanged(nameof(IsStarted)); }
        }

        public bool IsCompleted
        {
            get => _isCompleted;
            set { _isCompleted = value; OnPropertyChanged(nameof(IsCompleted)); }
        }

        public int CurrentStepIndex
        {
            get => _currentStepIndex;
            set { _currentStepIndex = value; OnPropertyChanged(nameof(CurrentStepIndex)); }
        }

        public double CompletionPercentage
        {
            get => _completionPercentage;
            set { _completionPercentage = value; OnPropertyChanged(nameof(CompletionPercentage)); }
        }

        public DateTime? StartedAt { get; set; }
        public DateTime? CompletedAt { get; set; }
        public DateTime? LastAccessedAt { get; set; }
        public TimeSpan? TimeSpent { get; set; }
        public int HintsUsed { get; set; }
        public Dictionary<string, object> StepData { get; set; } = new();

        public string StatusText
        {
            get
            {
                if (IsCompleted) return "Completed";
                if (IsStarted) return $"Step {CurrentStepIndex + 1}";
                return "Not started";
            }
        }

        public string StatusColor
        {
            get
            {
                if (IsCompleted) return "#6A9955";
                if (IsStarted) return "#FFCC02";
                return "#858585";
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class TutorialStats
    {
        public int TotalTutorials { get; set; }
        public int CompletedTutorials { get; set; }
        public int InProgressTutorials { get; set; }
        public TimeSpan TotalTimeSpent { get; set; }
        public double CompletionRate { get; set; }
        public string FavoriteCategory { get; set; }
        public TutorialDifficulty AverageDifficulty { get; set; }

        public string CompletionRateText => $"{CompletionRate:F1}%";
        public string TotalTimeText
        {
            get
            {
                if (TotalTimeSpent.TotalHours >= 1)
                    return $"{TotalTimeSpent.TotalHours:F1} hours";
                else
                    return $"{TotalTimeSpent.TotalMinutes:F0} minutes";
            }
        }
    }

    public class CodeExample : INotifyPropertyChanged
    {
        private string _title;
        private string _description;
        private bool _isFavorite;

        public string Id { get; set; }
        
        public string Title
        {
            get => _title;
            set { _title = value; OnPropertyChanged(nameof(Title)); }
        }

        public string Description
        {
            get => _description;
            set { _description = value; OnPropertyChanged(nameof(Description)); }
        }

        public string Code { get; set; }
        public string Language { get; set; } = "luau";
        public string Category { get; set; }
        public List<string> Tags { get; set; } = new();
        public TutorialDifficulty Difficulty { get; set; }
        public string Author { get; set; }
        public DateTime CreatedAt { get; set; }
        public DateTime? UpdatedAt { get; set; }
        public float Rating { get; set; }
        public int RatingCount { get; set; }
        public int Views { get; set; }
        public int Downloads { get; set; }
        public string Output { get; set; }
        public List<string> Requirements { get; set; } = new();

        public bool IsFavorite
        {
            get => _isFavorite;
            set { _isFavorite = value; OnPropertyChanged(nameof(IsFavorite)); }
        }

        public string DifficultyColor
        {
            get
            {
                return Difficulty switch
                {
                    TutorialDifficulty.Beginner => "#6A9955",
                    TutorialDifficulty.Intermediate => "#FFCC02",
                    TutorialDifficulty.Advanced => "#FF8C00",
                    TutorialDifficulty.Expert => "#D13438",
                    _ => "#007ACC"
                };
            }
        }

        public string FormattedRating => Rating > 0 ? $"{Rating:F1} ⭐" : "No ratings";
        public string FormattedStats => $"{Views} views • {Downloads} downloads";
        public string TagsText => string.Join(", ", Tags);
        public int LineCount => string.IsNullOrEmpty(Code) ? 0 : Code.Split('\n').Length;

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class LearningPath : INotifyPropertyChanged
    {
        private string _name;
        private string _description;
        private bool _isStarted;
        private bool _isCompleted;

        public string Id { get; set; }
        
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

        public List<string> TutorialIds { get; set; } = new();
        public TutorialDifficulty Difficulty { get; set; }
        public TimeSpan EstimatedDuration { get; set; }
        public string Category { get; set; }
        public string Author { get; set; }
        public DateTime CreatedAt { get; set; }
        public List<string> Tags { get; set; } = new();
        public string ThumbnailUrl { get; set; }

        public bool IsStarted
        {
            get => _isStarted;
            set { _isStarted = value; OnPropertyChanged(nameof(IsStarted)); }
        }

        public bool IsCompleted
        {
            get => _isCompleted;
            set { _isCompleted = value; OnPropertyChanged(nameof(IsCompleted)); }
        }

        public string StatusText
        {
            get
            {
                if (IsCompleted) return "Completed";
                if (IsStarted) return "In Progress";
                return "Not Started";
            }
        }

        public string TutorialCountText => $"{TutorialIds?.Count ?? 0} tutorials";
        public string EstimatedDurationText => $"~{EstimatedDuration.TotalHours:F1}h";

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}