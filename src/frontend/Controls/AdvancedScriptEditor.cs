using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text.RegularExpressions;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Shapes;
using System.Threading.Tasks;
using AetherVisor.Frontend.Models;
using AetherVisor.Frontend.Services;

namespace AetherVisor.Frontend.Controls
{
    public partial class AdvancedScriptEditor : UserControl
    {
        #region Dependency Properties
        
        public static readonly DependencyProperty TextProperty = DependencyProperty.Register(
            nameof(Text), typeof(string), typeof(AdvancedScriptEditor), 
            new FrameworkPropertyMetadata(string.Empty, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault, OnTextChanged));

        public string Text
        {
            get => (string)GetValue(TextProperty);
            set => SetValue(TextProperty, value);
        }

        public static readonly DependencyProperty ThemeProperty = DependencyProperty.Register(
            nameof(Theme), typeof(EditorTheme), typeof(AdvancedScriptEditor), 
            new PropertyMetadata(EditorTheme.Dark, OnThemeChanged));

        public EditorTheme Theme
        {
            get => (EditorTheme)GetValue(ThemeProperty);
            set => SetValue(ThemeProperty, value);
        }

        public static readonly DependencyProperty ShowLineNumbersProperty = DependencyProperty.Register(
            nameof(ShowLineNumbers), typeof(bool), typeof(AdvancedScriptEditor), 
            new PropertyMetadata(true, OnShowLineNumbersChanged));

        public bool ShowLineNumbers
        {
            get => (bool)GetValue(ShowLineNumbersProperty);
            set => SetValue(ShowLineNumbersProperty, value);
        }

        public static readonly DependencyProperty EnableCodeFoldingProperty = DependencyProperty.Register(
            nameof(EnableCodeFolding), typeof(bool), typeof(AdvancedScriptEditor), 
            new PropertyMetadata(true));

        public bool EnableCodeFolding
        {
            get => (bool)GetValue(EnableCodeFoldingProperty);
            set => SetValue(EnableCodeFoldingProperty, value);
        }

        public static readonly DependencyProperty EnableMinimapProperty = DependencyProperty.Register(
            nameof(EnableMinimap), typeof(bool), typeof(AdvancedScriptEditor), 
            new PropertyMetadata(false, OnEnableMinimapChanged));

        public bool EnableMinimap
        {
            get => (bool)GetValue(EnableMinimapProperty);
            set => SetValue(EnableMinimapProperty, value);
        }

        #endregion

        #region Fields

        private readonly LuauSyntaxHighlighter _syntaxHighlighter;
        private readonly LuauIntelliSense _intelliSense;
        private readonly CodeFoldingManager _foldingManager;
        private readonly ErrorAnalyzer _errorAnalyzer;
        
        private readonly List<string> _luauKeywords = new()
        {
            "and", "break", "continue", "do", "else", "elseif", "end", "false", "for", 
            "function", "if", "in", "local", "nil", "not", "or", "repeat", "return", 
            "then", "true", "until", "while", "export", "type"
        };

        private readonly List<string> _robloxApi = new()
        {
            "game", "workspace", "script", "Players", "ReplicatedStorage", "ServerStorage",
            "Lighting", "SoundService", "TweenService", "UserInputService", "RunService",
            "ContextActionService", "GuiService", "MarketplaceService", "DataStoreService"
        };

        private int _currentLine = 1;
        private int _currentColumn = 1;
        private bool _isUpdatingText = false;

        #endregion

        #region Constructor

        public AdvancedScriptEditor()
        {
            InitializeComponent();
            
            _syntaxHighlighter = new LuauSyntaxHighlighter(this);
            _intelliSense = new LuauIntelliSense();
            _foldingManager = new CodeFoldingManager(this);
            _errorAnalyzer = new ErrorAnalyzer();

            InitializeEditor();
            LoadRobloxApiDefinitions();
        }

        #endregion

        #region Initialization

        private void InitializeEditor()
        {
            PART_TextBox.Focus();
            UpdateLineNumbers();
            ApplyTheme();
            
            // Setup syntax highlighting
            _syntaxHighlighter.Initialize();
            
            // Setup code folding
            if (EnableCodeFolding)
            {
                _foldingManager.Initialize();
            }
        }

        private async void LoadRobloxApiDefinitions()
        {
            try
            {
                await _intelliSense.LoadApiDefinitionsAsync();
            }
            catch (Exception ex)
            {
                // Log error but don't crash
                // Log error: Debug statement removed
            }
        }

        #endregion

        #region Event Handlers

        private static void OnTextChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var editor = (AdvancedScriptEditor)d;
            if (!editor._isUpdatingText)
            {
                editor._isUpdatingText = true;
                editor.PART_TextBox.Text = e.NewValue?.ToString() ?? string.Empty;
                editor._isUpdatingText = false;
                editor.UpdateLineNumbers();
                editor.UpdateSyntaxHighlighting();
                editor.UpdateErrorAnalysis();
            }
        }

        private static void OnThemeChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var editor = (AdvancedScriptEditor)d;
            editor.ApplyTheme();
        }

        private static void OnShowLineNumbersChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var editor = (AdvancedScriptEditor)d;
            editor.Grid.ColumnDefinitions[0].Width = (bool)e.NewValue ? new GridLength(60) : new GridLength(0);
        }

        private static void OnEnableMinimapChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var editor = (AdvancedScriptEditor)d;
            editor.PART_Minimap.Visibility = (bool)e.NewValue ? Visibility.Visible : Visibility.Collapsed;
        }

        private void PART_TextBox_TextChanged(object sender, System.Windows.Controls.TextChangedEventArgs e)
        {
            if (!_isUpdatingText)
            {
                _isUpdatingText = true;
                Text = PART_TextBox.Text;
                _isUpdatingText = false;
                
                UpdateLineNumbers();
                UpdateSyntaxHighlighting();
                UpdateCurrentLineHighlight();
                UpdateErrorAnalysis();
                
                // Update intellisense
                ShowIntelliSense();
            }
        }

        private void PART_TextBox_SelectionChanged(object sender, RoutedEventArgs e)
        {
            UpdateCurrentPosition();
            UpdateCurrentLineHighlight();
            UpdateBracketMatching();
        }

        private void PART_TextBox_PreviewKeyDown(object sender, KeyEventArgs e)
        {
            HandleSpecialKeys(e);
        }

        private void PART_TextBox_PreviewKeyUp(object sender, KeyEventArgs e)
        {
            UpdateIntelliSense(e);
        }

        private void PART_ScrollViewer_ScrollChanged(object sender, ScrollChangedEventArgs e)
        {
            // Sync line number scrolling
            PART_LineNumberScrollViewer.ScrollToVerticalOffset(e.VerticalOffset);
            PART_FoldingScrollViewer.ScrollToVerticalOffset(e.VerticalOffset);
            
            // Update minimap
            if (EnableMinimap)
            {
                UpdateMinimap();
            }
        }

        #endregion

        #region Text Processing

        private void UpdateLineNumbers()
        {
            var lines = PART_TextBox.LineCount;
            var lineNumbers = string.Join("\n", Enumerable.Range(1, Math.Max(lines, 1)));
            PART_LineNumbers.Text = lineNumbers;
        }

        private void UpdateSyntaxHighlighting()
        {
            Task.Run(() => _syntaxHighlighter.HighlightAsync(PART_TextBox.Text));
        }

        private void UpdateCurrentPosition()
        {
            var caretIndex = PART_TextBox.CaretIndex;
            var textBeforeCaret = PART_TextBox.Text.Substring(0, caretIndex);
            
            _currentLine = textBeforeCaret.Count(c => c == '\n') + 1;
            var lastNewLine = textBeforeCaret.LastIndexOf('\n');
            _currentColumn = lastNewLine == -1 ? caretIndex + 1 : caretIndex - lastNewLine;
        }

        private void UpdateCurrentLineHighlight()
        {
            PART_CurrentLineCanvas.Children.Clear();
            
            var lineHeight = PART_TextBox.FontSize * 1.3;
            var currentLineIndex = PART_TextBox.GetLineIndexFromCharacterIndex(PART_TextBox.CaretIndex);
            var y = currentLineIndex * lineHeight;
            
            var highlight = new Rectangle
            {
                Width = PART_TextBox.ActualWidth,
                Height = lineHeight,
                Fill = (SolidColorBrush)FindResource("CurrentLineBackground")
            };
            
            Canvas.SetTop(highlight, y);
            PART_CurrentLineCanvas.Children.Add(highlight);
        }

        private void UpdateBracketMatching()
        {
            PART_BracketCanvas.Children.Clear();
            
            var caretIndex = PART_TextBox.CaretIndex;
            if (caretIndex == 0 || caretIndex >= PART_TextBox.Text.Length) return;
            
            var currentChar = PART_TextBox.Text[caretIndex];
            var matchingBracket = FindMatchingBracket(caretIndex, currentChar);
            
            if (matchingBracket != -1)
            {
                HighlightBracket(caretIndex);
                HighlightBracket(matchingBracket);
            }
        }

        private int FindMatchingBracket(int position, char bracket)
        {
            var text = PART_TextBox.Text;
            var pairs = new Dictionary<char, char> 
            { 
                {'(', ')'}, {'[', ']'}, {'{', '}'}, 
                {')', '('}, {']', '['}, {'}', '{'}
            };
            
            if (!pairs.ContainsKey(bracket)) return -1;
            
            var target = pairs[bracket];
            var direction = "([{".Contains(bracket) ? 1 : -1;
            var count = 1;
            
            for (var i = position + direction; i >= 0 && i < text.Length; i += direction)
            {
                if (text[i] == bracket) count++;
                else if (text[i] == target) count--;
                
                if (count == 0) return i;
            }
            
            return -1;
        }

        private void HighlightBracket(int position)
        {
            var rect = PART_TextBox.GetRectFromCharacterIndex(position);
            var highlight = new Rectangle
            {
                Width = 8,
                Height = PART_TextBox.FontSize,
                Fill = (SolidColorBrush)FindResource("BracketMatchBackground")
            };
            
            Canvas.SetLeft(highlight, rect.Left);
            Canvas.SetTop(highlight, rect.Top);
            PART_BracketCanvas.Children.Add(highlight);
        }

        #endregion

        #region IntelliSense

        private async void ShowIntelliSense()
        {
            var caretIndex = PART_TextBox.CaretIndex;
            var currentWord = GetCurrentWord(caretIndex);
            
            if (string.IsNullOrEmpty(currentWord) || currentWord.Length < 2)
            {
                PART_AutocompletePopup.IsOpen = false;
                return;
            }
            
            var suggestions = await _intelliSense.GetSuggestionsAsync(currentWord, GetContext(caretIndex));
            
            if (suggestions.Any())
            {
                PART_AutocompleteListBox.ItemsSource = suggestions;
                PART_AutocompleteListBox.SelectedIndex = 0;
                
                var caretRect = PART_TextBox.GetRectFromCharacterIndex(caretIndex);
                PART_AutocompletePopup.HorizontalOffset = caretRect.Left;
                PART_AutocompletePopup.VerticalOffset = caretRect.Bottom;
                PART_AutocompletePopup.IsOpen = true;
                
                UpdateDocumentationPreview();
            }
            else
            {
                PART_AutocompletePopup.IsOpen = false;
            }
        }

        private string GetCurrentWord(int caretIndex)
        {
            var text = PART_TextBox.Text;
            var start = caretIndex;
            var end = caretIndex;
            
            // Find start of word
            while (start > 0 && (char.IsLetterOrDigit(text[start - 1]) || text[start - 1] == '_' || text[start - 1] == '.'))
                start--;
            
            // Find end of word
            while (end < text.Length && (char.IsLetterOrDigit(text[end]) || text[end] == '_'))
                end++;
            
            return start < end ? text.Substring(start, end - start) : string.Empty;
        }

        private string GetContext(int caretIndex)
        {
            var text = PART_TextBox.Text;
            var lineStart = text.LastIndexOf('\n', Math.Max(0, caretIndex - 1)) + 1;
            return text.Substring(lineStart, caretIndex - lineStart);
        }

        private void UpdateDocumentationPreview()
        {
            if (PART_AutocompleteListBox.SelectedItem is IntelliSenseItem item)
            {
                PART_DocumentationText.Text = item.Documentation ?? "No documentation available.";
            }
        }

        private void UpdateIntelliSense(KeyEventArgs e)
        {
            if (e.Key == Key.Escape)
            {
                PART_AutocompletePopup.IsOpen = false;
                return;
            }
            
            if (PART_AutocompletePopup.IsOpen)
            {
                switch (e.Key)
                {
                    case Key.Down:
                        PART_AutocompleteListBox.SelectedIndex = 
                            Math.Min(PART_AutocompleteListBox.SelectedIndex + 1, PART_AutocompleteListBox.Items.Count - 1);
                        UpdateDocumentationPreview();
                        e.Handled = true;
                        break;
                    case Key.Up:
                        PART_AutocompleteListBox.SelectedIndex = 
                            Math.Max(PART_AutocompleteListBox.SelectedIndex - 1, 0);
                        UpdateDocumentationPreview();
                        e.Handled = true;
                        break;
                    case Key.Tab:
                    case Key.Enter:
                        AcceptCompletion();
                        e.Handled = true;
                        break;
                }
            }
            else
            {
                if (char.IsLetterOrDigit((char)KeyInterop.VirtualKeyFromKey(e.Key)) || e.Key == Key.OemPeriod)
                {
                    ShowIntelliSense();
                }
            }
        }

        private void AcceptCompletion()
        {
            if (PART_AutocompleteListBox.SelectedItem is IntelliSenseItem item)
            {
                var caretIndex = PART_TextBox.CaretIndex;
                var currentWord = GetCurrentWord(caretIndex);
                var wordStart = caretIndex - currentWord.Length;
                
                PART_TextBox.Select(wordStart, currentWord.Length);
                PART_TextBox.SelectedText = item.CompletionText;
                PART_TextBox.CaretIndex = wordStart + item.CompletionText.Length;
                
                PART_AutocompletePopup.IsOpen = false;
            }
        }

        #endregion

        #region Special Key Handling

        private void HandleSpecialKeys(KeyEventArgs e)
        {
            if (e.Key == Key.Tab)
            {
                HandleTabKey(e);
            }
            else if (e.Key == Key.Enter)
            {
                HandleEnterKey(e);
            }
            else if (e.KeyboardDevice.Modifiers == ModifierKeys.Control)
            {
                HandleControlKeys(e);
            }
        }

        private void HandleTabKey(KeyEventArgs e)
        {
            if (PART_AutocompletePopup.IsOpen)
            {
                AcceptCompletion();
                e.Handled = true;
                return;
            }
            
            // Handle indentation
            var selectedText = PART_TextBox.SelectedText;
            if (string.IsNullOrEmpty(selectedText))
            {
                // Insert tab or spaces
                PART_TextBox.SelectedText = "    "; // 4 spaces
            }
            else
            {
                // Indent/unindent selected lines
                var lines = selectedText.Split('\n');
                var indentedLines = lines.Select(line => 
                    Keyboard.Modifiers.HasFlag(ModifierKeys.Shift) ? 
                    line.StartsWith("    ") ? line.Substring(4) : line :
                    "    " + line);
                
                PART_TextBox.SelectedText = string.Join("\n", indentedLines);
            }
            
            e.Handled = true;
        }

        private void HandleEnterKey(KeyEventArgs e)
        {
            if (PART_AutocompletePopup.IsOpen)
            {
                AcceptCompletion();
                e.Handled = true;
                return;
            }
            
            // Auto-indentation
            var caretIndex = PART_TextBox.CaretIndex;
            var lineStart = PART_TextBox.Text.LastIndexOf('\n', Math.Max(0, caretIndex - 1)) + 1;
            var currentLine = PART_TextBox.Text.Substring(lineStart, caretIndex - lineStart);
            var indent = new string(currentLine.TakeWhile(c => c == ' ' || c == '\t').ToArray());
            
            // Increase indent for certain keywords
            if (Regex.IsMatch(currentLine.Trim(), @"\b(if|for|while|function|do)\b.*then\s*$|.*do\s*$|.*then\s*$"))
            {
                indent += "    ";
            }
            
            PART_TextBox.SelectedText = "\n" + indent;
            e.Handled = true;
        }

        private void HandleControlKeys(KeyEventArgs e)
        {
            switch (e.Key)
            {
                case Key.F:
                    ShowFindPanel();
                    e.Handled = true;
                    break;
                case Key.H:
                    ShowFindPanel(true);
                    e.Handled = true;
                    break;
                case Key.G:
                    ShowGoToLineDialog();
                    e.Handled = true;
                    break;
                case Key.OemQuestion: // Ctrl+/
                    ToggleComment();
                    e.Handled = true;
                    break;
                case Key.Space:
                    ShowIntelliSense();
                    e.Handled = true;
                    break;
            }
        }

        #endregion

        #region Find/Replace

        private void ShowFindPanel(bool showReplace = false)
        {
            PART_FindPanel.Visibility = Visibility.Visible;
            
            if (showReplace)
            {
                PART_ReplaceTextBox.Visibility = Visibility.Visible;
            }
            
            // Pre-fill with selected text
            if (!string.IsNullOrEmpty(PART_TextBox.SelectedText))
            {
                PART_FindTextBox.Text = PART_TextBox.SelectedText;
            }
            
            PART_FindTextBox.Focus();
            PART_FindTextBox.SelectAll();
        }

        private void FindNext_Click(object sender, RoutedEventArgs e)
        {
            var findText = PART_FindTextBox.Text;
            if (string.IsNullOrEmpty(findText)) return;
            
            var text = PART_TextBox.Text;
            var startIndex = PART_TextBox.CaretIndex;
            var index = text.IndexOf(findText, startIndex, StringComparison.OrdinalIgnoreCase);
            
            if (index == -1)
            {
                // Search from beginning
                index = text.IndexOf(findText, 0, StringComparison.OrdinalIgnoreCase);
            }
            
            if (index != -1)
            {
                PART_TextBox.Select(index, findText.Length);
                PART_TextBox.ScrollToLine(PART_TextBox.GetLineIndexFromCharacterIndex(index));
            }
        }

        private void Replace_Click(object sender, RoutedEventArgs e)
        {
            var findText = PART_FindTextBox.Text;
            var replaceText = PART_ReplaceTextBox.Text;
            
            if (PART_TextBox.SelectedText.Equals(findText, StringComparison.OrdinalIgnoreCase))
            {
                PART_TextBox.SelectedText = replaceText;
            }
            
            FindNext_Click(sender, e);
        }

        private void ReplaceAll_Click(object sender, RoutedEventArgs e)
        {
            var findText = PART_FindTextBox.Text;
            var replaceText = PART_ReplaceTextBox.Text;
            
            if (string.IsNullOrEmpty(findText)) return;
            
            PART_TextBox.Text = PART_TextBox.Text.Replace(findText, replaceText);
        }

        private void CloseFindPanel_Click(object sender, RoutedEventArgs e)
        {
            PART_FindPanel.Visibility = Visibility.Collapsed;
            PART_TextBox.Focus();
        }

        #endregion

        #region Error Analysis

        private async void UpdateErrorAnalysis()
        {
            var errors = await _errorAnalyzer.AnalyzeAsync(PART_TextBox.Text);
            UpdateErrorUnderlines(errors);
        }

        private void UpdateErrorUnderlines(List<AnalysisResult> errors)
        {
            PART_ErrorCanvas.Children.Clear();
            
            foreach (var error in errors)
            {
                var brush = error.Severity == AnalysisSeverity.Error ? 
                    (SolidColorBrush)FindResource("ErrorUnderline") : 
                    (SolidColorBrush)FindResource("WarningUnderline");
                
                var line = new Line
                {
                    X1 = error.StartColumn * 7, // Approximate character width
                    X2 = error.EndColumn * 7,
                    Y1 = error.Line * PART_TextBox.FontSize * 1.3 + PART_TextBox.FontSize,
                    Y2 = error.Line * PART_TextBox.FontSize * 1.3 + PART_TextBox.FontSize,
                    Stroke = brush,
                    StrokeThickness = 2
                };
                
                PART_ErrorCanvas.Children.Add(line);
            }
        }

        #endregion

        #region Theme Management

        private void ApplyTheme()
        {
            var resources = Resources;
            
            switch (Theme)
            {
                case EditorTheme.Dark:
                    ApplyDarkTheme(resources);
                    break;
                case EditorTheme.Light:
                    ApplyLightTheme(resources);
                    break;
                case EditorTheme.HighContrast:
                    ApplyHighContrastTheme(resources);
                    break;
            }
        }

        private void ApplyDarkTheme(ResourceDictionary resources)
        {
            resources["EditorBackground"] = new SolidColorBrush(Color.FromRgb(30, 30, 30));
            resources["EditorForeground"] = new SolidColorBrush(Color.FromRgb(212, 212, 212));
            resources["LineNumberBackground"] = new SolidColorBrush(Color.FromRgb(37, 37, 38));
            resources["LineNumberForeground"] = new SolidColorBrush(Color.FromRgb(133, 133, 133));
        }

        private void ApplyLightTheme(ResourceDictionary resources)
        {
            resources["EditorBackground"] = new SolidColorBrush(Colors.White);
            resources["EditorForeground"] = new SolidColorBrush(Colors.Black);
            resources["LineNumberBackground"] = new SolidColorBrush(Color.FromRgb(245, 245, 245));
            resources["LineNumberForeground"] = new SolidColorBrush(Color.FromRgb(128, 128, 128));
        }

        private void ApplyHighContrastTheme(ResourceDictionary resources)
        {
            resources["EditorBackground"] = new SolidColorBrush(Colors.Black);
            resources["EditorForeground"] = new SolidColorBrush(Colors.White);
            resources["LineNumberBackground"] = new SolidColorBrush(Colors.Black);
            resources["LineNumberForeground"] = new SolidColorBrush(Colors.Yellow);
        }

        #endregion

        #region Utility Methods

        private void ToggleComment()
        {
            var selectedText = PART_TextBox.SelectedText;
            if (string.IsNullOrEmpty(selectedText))
            {
                // Comment current line
                var caretIndex = PART_TextBox.CaretIndex;
                var lineStart = PART_TextBox.Text.LastIndexOf('\n', Math.Max(0, caretIndex - 1)) + 1;
                var lineEnd = PART_TextBox.Text.IndexOf('\n', caretIndex);
                if (lineEnd == -1) lineEnd = PART_TextBox.Text.Length;
                
                var line = PART_TextBox.Text.Substring(lineStart, lineEnd - lineStart);
                var newLine = line.TrimStart().StartsWith("--") ? 
                    line.Replace("--", "", 1) : 
                    "--" + line;
                
                PART_TextBox.Select(lineStart, lineEnd - lineStart);
                PART_TextBox.SelectedText = newLine;
            }
            else
            {
                // Toggle comment for selected lines
                var lines = selectedText.Split('\n');
                var allCommented = lines.All(l => l.TrimStart().StartsWith("--"));
                
                var newLines = lines.Select(line => 
                    allCommented ? 
                    line.Replace("--", "", 1) : 
                    "--" + line);
                
                PART_TextBox.SelectedText = string.Join("\n", newLines);
            }
        }

        private void ShowGoToLineDialog()
        {
            // Implementation for Go to Line dialog
            // This would show a simple input dialog
        }

        private void UpdateMinimap()
        {
            // Implementation for minimap update
            // This would render a small overview of the entire document
        }

        #endregion

        #region Event Handlers for Autocomplete

        private void PART_AutocompleteListBox_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            AcceptCompletion();
        }

        #endregion
    }

    #region Supporting Classes

    public enum EditorTheme
    {
        Dark,
        Light,
        HighContrast
    }

    public enum AnalysisSeverity
    {
        Error,
        Warning,
        Information
    }

    public class AnalysisResult
    {
        public int Line { get; set; }
        public int StartColumn { get; set; }
        public int EndColumn { get; set; }
        public string Message { get; set; }
        public AnalysisSeverity Severity { get; set; }
    }

    #endregion
}