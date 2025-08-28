using System.ComponentModel;

namespace AetherVisor.Frontend.Models
{
    public class IntelliSenseItem : INotifyPropertyChanged
    {
        private string _name;
        private string _completionText;
        private string _description;
        private string _documentation;
        private IntelliSenseItemType _itemType;
        private string _icon;

        public string Name
        {
            get => _name;
            set { _name = value; OnPropertyChanged(nameof(Name)); }
        }

        public string CompletionText
        {
            get => _completionText ?? _name;
            set { _completionText = value; OnPropertyChanged(nameof(CompletionText)); }
        }

        public string Description
        {
            get => _description;
            set { _description = value; OnPropertyChanged(nameof(Description)); }
        }

        public string Documentation
        {
            get => _documentation;
            set { _documentation = value; OnPropertyChanged(nameof(Documentation)); }
        }

        public IntelliSenseItemType ItemType
        {
            get => _itemType;
            set { _itemType = value; OnPropertyChanged(nameof(ItemType)); UpdateIcon(); }
        }

        public string Icon
        {
            get => _icon;
            private set { _icon = value; OnPropertyChanged(nameof(Icon)); }
        }

        public string ReturnType { get; set; }
        public string[] Parameters { get; set; }
        public bool IsDeprecated { get; set; }
        public string Category { get; set; }
        public int Priority { get; set; } = 0;

        private void UpdateIcon()
        {
            Icon = ItemType switch
            {
                IntelliSenseItemType.Keyword => "\uE8C8",      // Symbol
                IntelliSenseItemType.Function => "\uE8E4",     // Method
                IntelliSenseItemType.Variable => "\uE8CD",     // Variable
                IntelliSenseItemType.Property => "\uE8B7",     // Property
                IntelliSenseItemType.Class => "\uE8B0",        // Class
                IntelliSenseItemType.Interface => "\uE8B1",    // Interface
                IntelliSenseItemType.Enum => "\uE8B2",         // Enum
                IntelliSenseItemType.Event => "\uE8D5",        // Event
                IntelliSenseItemType.Snippet => "\uE8C6",      // Code
                IntelliSenseItemType.Service => "\uE8D2",      // Service
                _ => "\uE8C8"
            };
        }

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        public override string ToString()
        {
            return Name;
        }
    }

    public enum IntelliSenseItemType
    {
        Keyword,
        Function,
        Variable,
        Property,
        Class,
        Interface,
        Enum,
        Event,
        Snippet,
        Service
    }
}