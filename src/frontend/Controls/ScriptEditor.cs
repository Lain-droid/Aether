// This file represents a custom control for Luau script editing.
// In a real project, this would likely inherit from a control like TextBox
// or use a third-party library like AvalonEdit.

using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Collections.Generic;
using System.Linq;
using ICSharpCode.AvalonEdit;

namespace AetherVisor.Frontend.Controls
{
    public partial class ScriptEditor : UserControl
    {
        public static readonly DependencyProperty TextProperty = DependencyProperty.Register(
            nameof(Text), typeof(string), typeof(ScriptEditor), new FrameworkPropertyMetadata(string.Empty, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault));

        public string Text
        {
            get { return (string)GetValue(TextProperty); }
            set { SetValue(TextProperty, value); }
        }

        private readonly List<string> _keywords = new List<string>
        {
            "local","function","end","if","then","elseif","else","for","in","pairs","ipairs","while","do","repeat","until","return","true","false","nil","and","or","not","break","continue","game","workspace","script"
        };

        public ScriptEditor()
        {
            InitializeComponent();
            PART_Avalon.TextChanged += (s, e) =>
            {
                Text = PART_Avalon.Text;
                UpdateLineNumbers();
            };
            PART_Avalon.PreviewKeyUp += OnKeyUpForAutocomplete;
            PART_ListBox.MouseDoubleClick += (s, e) => AcceptCompletion();
            UpdateLineNumbers();
        }

        public static readonly DependencyProperty EditorFontSizeProperty = DependencyProperty.Register(
            nameof(EditorFontSize), typeof(double), typeof(ScriptEditor), new PropertyMetadata(14.0, (o, e) =>
            {
                var c = (ScriptEditor)o;
                c.PART_Avalon.FontSize = (double)e.NewValue;
            }));
        public double EditorFontSize
        {
            get { return (double)GetValue(EditorFontSizeProperty); }
            set { SetValue(EditorFontSizeProperty, value); }
        }

        public static readonly DependencyProperty AutocompleteEnabledProperty = DependencyProperty.Register(
            nameof(AutocompleteEnabled), typeof(bool), typeof(ScriptEditor), new PropertyMetadata(true));
        public bool AutocompleteEnabled
        {
            get { return (bool)GetValue(AutocompleteEnabledProperty); }
            set { SetValue(AutocompleteEnabledProperty, value); }
        }

        private void UpdateLineNumbers()
        {
            var lines = PART_Avalon?.Document?.LineCount ?? 1;
            var nums = string.Join("\n", Enumerable.Range(1, lines == 0 ? 1 : lines));
            PART_LineNumbers.Text = nums;
        }

        private void OnKeyUpForAutocomplete(object sender, KeyEventArgs e)
        {
            // auto-indent
            if (e.Key == Key.Return)
            {
                var caret = PART_Avalon.CaretOffset;
                var line = PART_Avalon.Document.GetLineByOffset(caret);
                if (line.PreviousLine != null)
                {
                    var prevText = PART_Avalon.Document.GetText(line.PreviousLine);
                    var indent = new string(prevText.TakeWhile(ch => ch == ' ' || ch == '\t').ToArray());
                    PART_Avalon.Document.Insert(caret, indent);
                    PART_Avalon.CaretOffset = caret + indent.Length;
                }
            }

            // simple bracket match visual hint could be added here (not selecting for now)

            if (!AutocompleteEnabled) { PART_Popup.IsOpen = false; return; }
            if (char.IsLetterOrDigit((char)KeyInterop.VirtualKeyFromKey(e.Key)) || e.Key == Key.OemPeriod)
            {
                var caret = PART_Avalon.CaretOffset;
                var start = caret;
                var text = PART_Avalon.Text;
                while (start > 0 && (char.IsLetterOrDigit(text[start - 1]) || text[start - 1] == '_')) start--;
                var token = text.Substring(start, caret - start);
                if (token.Length >= 1)
                {
                    var suggestions = _keywords.Where(k => k.StartsWith(token)).Take(8).ToList();
                    if (suggestions.Count > 0)
                    {
                        PART_ListBox.ItemsSource = suggestions;
                        PART_Popup.IsOpen = true;
                        return;
                    }
                }
            }
            PART_Popup.IsOpen = false;
        }

        private void AcceptCompletion()
        {
            if (!PART_Popup.IsOpen || PART_ListBox.SelectedItem == null) return;
            var suggestion = PART_ListBox.SelectedItem.ToString();
            var caret = PART_Avalon.CaretOffset;
            var start = caret;
            var text = PART_Avalon.Text;
            while (start > 0 && (char.IsLetterOrDigit(text[start - 1]) || text[start - 1] == '_')) start--;
            PART_Avalon.Document.Replace(start, caret - start, suggestion);
            PART_Avalon.CaretOffset = start + suggestion.Length;
            PART_Popup.IsOpen = false;
        }

        private void PART_TextBox_PreviewKeyDown(object sender, KeyEventArgs e)
        {
            if (PART_Popup.IsOpen)
            {
                if (e.Key == Key.Down) { PART_ListBox.SelectedIndex = System.Math.Min(PART_ListBox.SelectedIndex + 1, PART_ListBox.Items.Count - 1); e.Handled = true; }
                else if (e.Key == Key.Up) { PART_ListBox.SelectedIndex = System.Math.Max(PART_ListBox.SelectedIndex - 1, 0); e.Handled = true; }
                else if (e.Key == Key.Tab || e.Key == Key.Enter) { AcceptCompletion(); e.Handled = true; }
                else if (e.Key == Key.Escape) { PART_Popup.IsOpen = false; e.Handled = true; }
            }
        }
    }
}
