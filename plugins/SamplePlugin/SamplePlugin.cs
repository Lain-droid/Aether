using System;
using System.ComponentModel.Composition;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using AetherVisor.Frontend.Models;
using AetherVisor.Frontend.Services;

namespace AetherVisor.Plugins.Sample
{
    [Export(typeof(IPlugin))]
    [ExportMetadata("Id", "AetherVisor.Plugins.Sample")]
    [ExportMetadata("Name", "Sample Plugin")]
    [ExportMetadata("Version", "1.0.0")]
    public class SamplePlugin : IPlugin
    {
        private IPluginHost _host;
        private UserControl _sidePanel;

        public string Name => "Sample Plugin";
        public string Version => "1.0.0";
        public string Description => "A sample plugin demonstrating the Aether plugin system";
        public string Author => "Aether Team";

        public PluginInfo Info { get; private set; }

        public async Task InitializeAsync(IPluginHost host)
        {
            _host = host;
            
            Info = new PluginInfo
            {
                Id = "AetherVisor.Plugins.Sample",
                Name = Name,
                Version = Version,
                Description = Description,
                Author = Author,
                Icon = "🔌"
            };

            // Subscribe to editor events
            _host.EditorService.TextChanged += OnTextChanged;
            _host.EditorService.CaretPositionChanged += OnCaretPositionChanged;

            await Task.CompletedTask;
        }

        public async Task ActivateAsync()
        {
            // Register commands
            _host.RegisterCommand("sample.insertHello", InsertHelloWorld, "Insert Hello World");
            _host.RegisterCommand("sample.showInfo", ShowPluginInfo, "Show Plugin Info");

            // Register menu items
            _host.RegisterMenuItem("Tools/Sample Plugin/Insert Hello", param => InsertHelloWorld(param), "💬");
            _host.RegisterMenuItem("Tools/Sample Plugin/Plugin Info", param => ShowPluginInfo(param), "ℹ️");

            // Register side panel
            CreateSidePanel();
            _host.RegisterSidePanel("sample", "Sample", _sidePanel, "🔌");

            // Register status bar item
            var statusItem = new TextBlock 
            { 
                Text = "Sample Plugin Active", 
                Foreground = System.Windows.Media.Brushes.LightGreen,
                Margin = new Thickness(8, 0, 8, 0)
            };
            _host.RegisterStatusBarItem("sample", statusItem);

            _host.UIService.ShowNotification("Sample Plugin activated!", NotificationType.Success);

            await Task.CompletedTask;
        }

        public async Task DeactivateAsync()
        {
            _host.UIService.ShowNotification("Sample Plugin deactivated", NotificationType.Info);
            await Task.CompletedTask;
        }

        private void CreateSidePanel()
        {
            _sidePanel = new UserControl();
            
            var stackPanel = new StackPanel { Margin = new Thickness(8) };
            
            // Title
            var title = new TextBlock 
            { 
                Text = "Sample Plugin", 
                FontSize = 16, 
                FontWeight = FontWeights.Bold,
                Margin = new Thickness(0, 0, 0, 16)
            };
            stackPanel.Children.Add(title);

            // Info
            var info = new TextBlock 
            { 
                Text = "This is a sample plugin that demonstrates various plugin capabilities.",
                TextWrapping = TextWrapping.Wrap,
                Margin = new Thickness(0, 0, 0, 16)
            };
            stackPanel.Children.Add(info);

            // Buttons
            var helloButton = new Button 
            { 
                Content = "Insert Hello World", 
                Margin = new Thickness(0, 0, 0, 8),
                Padding = new Thickness(8, 4)
            };
            helloButton.Click += (s, e) => InsertHelloWorld(null);
            stackPanel.Children.Add(helloButton);

            var infoButton = new Button 
            { 
                Content = "Show Editor Info", 
                Margin = new Thickness(0, 0, 0, 8),
                Padding = new Thickness(8, 4)
            };
            infoButton.Click += (s, e) => ShowEditorInfo();
            stackPanel.Children.Add(infoButton);

            // Statistics
            var statsGroup = new GroupBox 
            { 
                Header = "Statistics", 
                Margin = new Thickness(0, 16, 0, 0) 
            };
            var statsPanel = new StackPanel();
            
            var textLengthLabel = new TextBlock { Text = "Text Length: 0" };
            var caretPositionLabel = new TextBlock { Text = "Caret Position: 0" };
            var lineCountLabel = new TextBlock { Text = "Line Count: 0" };
            
            statsPanel.Children.Add(textLengthLabel);
            statsPanel.Children.Add(caretPositionLabel);
            statsPanel.Children.Add(lineCountLabel);
            
            statsGroup.Content = statsPanel;
            stackPanel.Children.Add(statsGroup);

            _sidePanel.Content = stackPanel;
        }

        private void InsertHelloWorld(object parameter)
        {
            var helloText = "print('Hello World from Sample Plugin!')";
            _host.EditorService.InsertText(helloText);
        }

        private void ShowPluginInfo(object parameter)
        {
            var message = $"Plugin: {Name}\nVersion: {Version}\nAuthor: {Author}\n\nDescription: {Description}";
            _host.UIService.ShowNotification(message, NotificationType.Info);
        }

        private void ShowEditorInfo()
        {
            var currentText = _host.EditorService.GetCurrentText();
            var caretPosition = _host.EditorService.GetCaretPosition();
            var selectedText = _host.EditorService.GetSelectedText();
            
            var message = $"Current text length: {currentText.Length}\n" +
                         $"Caret position: {caretPosition}\n" +
                         $"Selected text: '{selectedText}'\n" +
                         $"Line count: {currentText.Split('\n').Length}";
            
            _host.UIService.ShowNotification(message, NotificationType.Info);
        }

        private void OnTextChanged(object sender, TextChangedEventArgs e)
        {
            // React to text changes
            if (_sidePanel?.Content is StackPanel panel)
            {
                var statsGroup = panel.Children[4] as GroupBox;
                if (statsGroup?.Content is StackPanel statsPanel)
                {
                    var textLengthLabel = statsPanel.Children[0] as TextBlock;
                    if (textLengthLabel != null)
                    {
                        textLengthLabel.Text = $"Text Length: {e.NewText?.Length ?? 0}";
                    }

                    var lineCountLabel = statsPanel.Children[2] as TextBlock;
                    if (lineCountLabel != null)
                    {
                        var lineCount = e.NewText?.Split('\n').Length ?? 0;
                        lineCountLabel.Text = $"Line Count: {lineCount}";
                    }
                }
            }
        }

        private void OnCaretPositionChanged(object sender, CaretPositionChangedEventArgs e)
        {
            // React to caret position changes
            if (_sidePanel?.Content is StackPanel panel)
            {
                var statsGroup = panel.Children[4] as GroupBox;
                if (statsGroup?.Content is StackPanel statsPanel)
                {
                    var caretPositionLabel = statsPanel.Children[1] as TextBlock;
                    if (caretPositionLabel != null)
                    {
                        caretPositionLabel.Text = $"Caret Position: {e.Position} (L{e.Line}:C{e.Column})";
                    }
                }
            }
        }

        public void Dispose()
        {
            // Cleanup
            if (_host != null)
            {
                _host.EditorService.TextChanged -= OnTextChanged;
                _host.EditorService.CaretPositionChanged -= OnCaretPositionChanged;
            }
        }
    }
}