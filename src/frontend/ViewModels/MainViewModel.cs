using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Windows.Input;
using System;
using System.Windows;
using AetherVisor.Frontend.Services;
using AetherVisor.Frontend.Models;
using System.IO;
using System.Text.Json;
using System.Linq;

namespace AetherVisor.Frontend.ViewModels
{
    // A base class for observable properties, essential for MVVM
    public abstract class ObservableObject : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class RelayCommand : ICommand
    {
        private readonly Predicate<object> _canExecute;
        private readonly Action<object> _execute;

        public RelayCommand(Predicate<object> canExecute, Action<object> execute)
        {
            _canExecute = canExecute ?? (_ => true);
            _execute = execute ?? (_ => { });
        }

        public bool CanExecute(object parameter) => _canExecute(parameter);
        public void Execute(object parameter) => _execute(parameter);
        public event EventHandler CanExecuteChanged
        {
            add { CommandManager.RequerySuggested += value; }
            remove { CommandManager.RequerySuggested -= value; }
        }
    }

    public class MainViewModel : ObservableObject
    {
        private double _editorFontSize = 14;
        public double EditorFontSize { get => _editorFontSize; set { _editorFontSize = value; OnPropertyChanged(nameof(EditorFontSize)); SaveState(); } }

        private bool _autocompleteEnabled = true;
        public bool AutocompleteEnabled { get => _autocompleteEnabled; set { _autocompleteEnabled = value; OnPropertyChanged(nameof(AutocompleteEnabled)); SaveState(); } }

        private string _scriptText = "-- Aether script here\nprint('ready')";
        public string ScriptText
        {
            get => _scriptText;
            set { _scriptText = value; OnPropertyChanged(nameof(ScriptText)); }
        }

        public ObservableCollection<string> ConsoleOutput { get; } = new ObservableCollection<string>();
        public ObservableCollection<FileNode> Files { get; } = new ObservableCollection<FileNode>();
        public ObservableCollection<OpenDocument> Tabs { get; } = new ObservableCollection<OpenDocument>();
        private OpenDocument _activeTab;
        public OpenDocument ActiveTab { get => _activeTab; set { _activeTab = value; if (value != null) ScriptText = value.Content; OnPropertyChanged(nameof(ActiveTab)); } }
        private string _currentFile;
        public string CurrentFile { get => _currentFile; set { _currentFile = value; OnPropertyChanged(nameof(CurrentFile)); } }

        public ICommand ExecuteScriptCommand { get; }
        public ICommand InjectCommand { get; }
        private readonly IIpcClientService _ipc = new IpcClientService();

        public MainViewModel()
        {
            ExecuteScriptCommand = new RelayCommand(_ => CanExecuteScript(), _ => ExecuteScript());
            InjectCommand = new RelayCommand(_ => true, _ => Inject());

            ConsoleOutput.Add("AetherVisor initialized");
            ConsoleOutput.Add("Idle");

            LoadFiles(Environment.CurrentDirectory);
            RestoreState();
        }

        private bool CanExecuteScript()
        {
            return !string.IsNullOrWhiteSpace(ScriptText);
        }

        private async void ExecuteScript()
        {
            ConsoleOutput.Add($"Execute requested at {DateTime.Now:HH:mm:ss}");
            try {
                await _ipc.ConnectAsync();
                await _ipc.SendScriptAsync(ScriptText);
                ConsoleOutput.Add("Script sent to backend");
            } catch (Exception ex) {
                ConsoleOutput.Add($"IPC error: {ex.Message}");
            }
        }

        private void Inject()
        {
            ConsoleOutput.Add("Inject requested");
        }

        private void LoadFiles(string root)
        {
            Files.Clear();
            Files.Add(BuildNode(root));
        }

        private FileNode BuildNode(string path)
        {
            var node = new FileNode { Name = System.IO.Path.GetFileName(path), FullPath = path, IsDirectory = Directory.Exists(path) };
            if (node.IsDirectory)
            {
                foreach (var dir in Directory.GetDirectories(path)) node.Children.Add(BuildNode(dir));
                foreach (var file in Directory.GetFiles(path)) node.Children.Add(new FileNode { Name = System.IO.Path.GetFileName(file), FullPath = file, IsDirectory = false });
            }
            return node;
        }

        public void OpenFile(FileNode node)
        {
            if (node == null || node.IsDirectory) return;
            try {
                var content = File.ReadAllText(node.FullPath);
                ScriptText = content;
                CurrentFile = node.FullPath;
                var tab = new OpenDocument { FilePath = node.FullPath, FileName = System.IO.Path.GetFileName(node.FullPath), Content = content };
                Tabs.Add(tab);
                ActiveTab = tab;
                SaveState();
            } catch (Exception ex) {
                ConsoleOutput.Add($"Open error: {ex.Message}");
            }
        }

        private string StatePath => System.IO.Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "Aether", "state.json");

        private void RestoreState()
        {
            try {
                if (File.Exists(StatePath))
                {
                    var json = File.ReadAllText(StatePath);
                    var state = JsonSerializer.Deserialize<AppState>(json);
                    if (state != null)
                    {
                        foreach (var f in state.OpenFiles)
                        {
                            if (File.Exists(f))
                            {
                                var node = new FileNode { FullPath = f, Name = System.IO.Path.GetFileName(f), IsDirectory = false };
                                OpenFile(node);
                            }
                        }
                        if (!string.IsNullOrEmpty(state.ActiveFile))
                        {
                            ActiveTab = Tabs.FirstOrDefault(t => t.FilePath == state.ActiveFile);
                        }
                        if (state.EditorFontSize > 0) EditorFontSize = state.EditorFontSize;
                        AutocompleteEnabled = state.AutocompleteEnabled;
                    }
                }
            } catch { }
        }

        private void SaveState()
        {
            try {
                var state = new AppState
                {
                    OpenFiles = new System.Collections.Generic.List<string>(Tabs.Select(t => t.FilePath)),
                    ActiveFile = ActiveTab?.FilePath,
                    EditorFontSize = EditorFontSize,
                    AutocompleteEnabled = AutocompleteEnabled
                };
                var dir = System.IO.Path.GetDirectoryName(StatePath);
                if (!Directory.Exists(dir)) Directory.CreateDirectory(dir);
                File.WriteAllText(StatePath, JsonSerializer.Serialize(state));
            } catch { }
        }
    }
}
