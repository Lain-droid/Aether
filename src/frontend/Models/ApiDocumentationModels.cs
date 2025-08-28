using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;

namespace AetherVisor.Frontend.Models
{
    public class ApiDocItem : INotifyPropertyChanged
    {
        private string _name;
        private string _description;
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

        public ApiItemType Type { get; set; }
        public string Category { get; set; }
        public string Syntax { get; set; }
        public string ReturnType { get; set; }
        public List<ApiParameter> Parameters { get; set; } = new();
        public List<string> Tags { get; set; } = new();
        public List<ApiExample> Examples { get; set; } = new();
        public List<ApiDocItem> Children { get; set; }
        public string Documentation { get; set; }
        public string Url { get; set; }
        public DateTime LastUpdated { get; set; }
        public string Version { get; set; }
        public bool IsDeprecated { get; set; }
        public string DeprecationMessage { get; set; }

        public bool IsFavorite
        {
            get => _isFavorite;
            set { _isFavorite = value; OnPropertyChanged(nameof(IsFavorite)); }
        }

        public string TypeIcon
        {
            get
            {
                return Type switch
                {
                    ApiItemType.Class => "📦",
                    ApiItemType.Function => "⚡",
                    ApiItemType.Property => "🔧",
                    ApiItemType.Event => "📡",
                    ApiItemType.Enum => "📋",
                    ApiItemType.EnumItem => "📄",
                    ApiItemType.Service => "🔄",
                    ApiItemType.Callback => "🔗",
                    _ => "📝"
                };
            }
        }

        public string FormattedSyntax
        {
            get
            {
                if (string.IsNullOrEmpty(Syntax))
                    return Name;

                return Syntax;
            }
        }

        public string ParametersText
        {
            get
            {
                if (Parameters == null || !Parameters.Any())
                    return "No parameters";

                return string.Join(", ", Parameters.Select(p => 
                    p.IsOptional ? $"{p.Name}?: {p.Type}" : $"{p.Name}: {p.Type}"));
            }
        }

        public string TagsText => Tags != null ? string.Join(", ", Tags) : "";

        public bool HasExamples => Examples != null && Examples.Any();
        public bool HasChildren => Children != null && Children.Any();
        public bool HasParameters => Parameters != null && Parameters.Any();

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public enum ApiItemType
    {
        Class,
        Function,
        Property,
        Event,
        Enum,
        EnumItem,
        Service,
        Callback,
        Module,
        Interface,
        Type
    }

    public class ApiParameter
    {
        public string Name { get; set; }
        public string Type { get; set; }
        public string Description { get; set; }
        public bool IsOptional { get; set; }
        public string DefaultValue { get; set; }

        public string FormattedParameter
        {
            get
            {
                var result = $"{Name}: {Type}";
                if (IsOptional)
                    result = $"{Name}?: {Type}";
                if (!string.IsNullOrEmpty(DefaultValue))
                    result += $" = {DefaultValue}";
                return result;
            }
        }
    }

    public class ApiExample
    {
        public string Title { get; set; }
        public string Description { get; set; }
        public string Code { get; set; }
        public string Output { get; set; }
        public List<string> Tags { get; set; } = new();
    }

    public class ApiCategory : INotifyPropertyChanged
    {
        private string _name;
        private string _description;
        private bool _isExpanded;

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

        public string Icon { get; set; }
        public ObservableCollection<ApiDocItem> Items { get; set; } = new();

        public bool IsExpanded
        {
            get => _isExpanded;
            set { _isExpanded = value; OnPropertyChanged(nameof(IsExpanded)); }
        }

        public int ItemCount => Items?.Count ?? 0;

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class ScriptTemplate : INotifyPropertyChanged
    {
        private string _name;
        private string _description;
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

        public string Author { get; set; }
        public string Version { get; set; }
        public string Code { get; set; }
        public string Category { get; set; }
        public List<string> Tags { get; set; } = new();
        public DateTime CreatedAt { get; set; }
        public DateTime UpdatedAt { get; set; }
        public int Downloads { get; set; }
        public float Rating { get; set; }
        public int RatingCount { get; set; }
        public string ThumbnailUrl { get; set; }
        public List<string> Screenshots { get; set; } = new();
        public string LicenseType { get; set; }
        public string SourceUrl { get; set; }
        public List<ScriptDependency> Dependencies { get; set; } = new();

        public bool IsFavorite
        {
            get => _isFavorite;
            set { _isFavorite = value; OnPropertyChanged(nameof(IsFavorite)); }
        }

        public string FormattedRating => $"{Rating:F1} ({RatingCount} reviews)";
        public string FormattedDownloads => Downloads >= 1000 ? $"{Downloads / 1000:F1}k" : Downloads.ToString();
        public string TagsText => string.Join(", ", Tags);

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class ScriptDependency
    {
        public string Name { get; set; }
        public string Version { get; set; }
        public string Source { get; set; }
        public bool IsRequired { get; set; }
    }

    public class MarketplaceItem : INotifyPropertyChanged
    {
        private string _name;
        private string _description;
        private bool _isFavorite;
        private bool _isInstalled;

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

        public MarketplaceItemType Type { get; set; }
        public string Author { get; set; }
        public string AuthorUrl { get; set; }
        public string Version { get; set; }
        public decimal Price { get; set; }
        public bool IsFree => Price == 0;
        public string Currency { get; set; } = "USD";
        public List<string> Tags { get; set; } = new();
        public DateTime CreatedAt { get; set; }
        public DateTime UpdatedAt { get; set; }
        public int Downloads { get; set; }
        public float Rating { get; set; }
        public int RatingCount { get; set; }
        public string ThumbnailUrl { get; set; }
        public List<string> Screenshots { get; set; } = new();
        public string LicenseType { get; set; }
        public string SourceUrl { get; set; }
        public string DownloadUrl { get; set; }
        public long FileSize { get; set; }
        public List<string> SupportedVersions { get; set; } = new();
        public List<MarketplaceDependency> Dependencies { get; set; } = new();

        public bool IsFavorite
        {
            get => _isFavorite;
            set { _isFavorite = value; OnPropertyChanged(nameof(IsFavorite)); }
        }

        public bool IsInstalled
        {
            get => _isInstalled;
            set { _isInstalled = value; OnPropertyChanged(nameof(IsInstalled)); OnPropertyChanged(nameof(InstallButtonText)); }
        }

        public string InstallButtonText => IsInstalled ? "Installed" : (IsFree ? "Install" : $"Buy ${Price:F2}");
        public string FormattedPrice => IsFree ? "Free" : $"${Price:F2}";
        public string FormattedRating => $"{Rating:F1} ⭐ ({RatingCount})";
        public string FormattedDownloads => Downloads >= 1000 ? $"{Downloads / 1000:F1}k downloads" : $"{Downloads} downloads";
        public string FormattedFileSize => FormatFileSize(FileSize);
        public string TagsText => string.Join(", ", Tags);

        public string TypeIcon
        {
            get
            {
                return Type switch
                {
                    MarketplaceItemType.Plugin => "🔌",
                    MarketplaceItemType.Theme => "🎨",
                    MarketplaceItemType.ScriptTemplate => "📝",
                    MarketplaceItemType.Tool => "🔧",
                    MarketplaceItemType.Library => "📚",
                    _ => "📦"
                };
            }
        }

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

    public enum MarketplaceItemType
    {
        Plugin,
        Theme,
        ScriptTemplate,
        Tool,
        Library,
        Extension
    }

    public class MarketplaceDependency
    {
        public string Name { get; set; }
        public string Version { get; set; }
        public string Source { get; set; }
        public bool IsRequired { get; set; }
        public bool IsInstalled { get; set; }
    }

    public class MarketplaceCategory
    {
        public string Name { get; set; }
        public string Description { get; set; }
        public string Icon { get; set; }
        public int ItemCount { get; set; }
        public bool IsPopular { get; set; }
    }

    public class UserReview
    {
        public string Id { get; set; }
        public string UserId { get; set; }
        public string UserName { get; set; }
        public string UserAvatarUrl { get; set; }
        public float Rating { get; set; }
        public string ReviewText { get; set; }
        public DateTime CreatedAt { get; set; }
        public DateTime UpdatedAt { get; set; }
        public bool IsVerifiedPurchase { get; set; }
        public int HelpfulVotes { get; set; }
        public int TotalVotes { get; set; }

        public string FormattedRating => $"{Rating:F1} ⭐";
        public string FormattedDate => CreatedAt.ToString("MMM dd, yyyy");
        public string HelpfulPercentage => TotalVotes > 0 ? $"{(float)HelpfulVotes / TotalVotes * 100:F0}% helpful" : "";
    }

    public class MarketplaceSearchFilter
    {
        public List<MarketplaceItemType> Types { get; set; } = new();
        public List<string> Categories { get; set; } = new();
        public List<string> Tags { get; set; } = new();
        public decimal? MinPrice { get; set; }
        public decimal? MaxPrice { get; set; }
        public float? MinRating { get; set; }
        public bool? IsFree { get; set; }
        public bool? IsInstalled { get; set; }
        public DateTime? CreatedAfter { get; set; }
        public DateTime? UpdatedAfter { get; set; }
        public MarketplaceSortBy SortBy { get; set; } = MarketplaceSortBy.Relevance;
        public bool SortDescending { get; set; } = true;
    }

    public enum MarketplaceSortBy
    {
        Relevance,
        Name,
        Price,
        Rating,
        Downloads,
        CreatedDate,
        UpdatedDate
    }
}