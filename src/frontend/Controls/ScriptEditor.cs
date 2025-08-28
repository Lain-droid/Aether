// This file represents a custom control for Luau script editing.
// In a real project, this would likely inherit from a control like TextBox
// or use a third-party library like AvalonEdit.

using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Collections.Generic;
using System.Linq;

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
            PART_TextBox.TextChanged += (s, e) =>
            {
                Text = PART_TextBox.Text;
                UpdateLineNumbers();
            };
            PART_TextBox.PreviewKeyUp += OnKeyUpForAutocomplete;
            PART_ListBox.MouseDoubleClick += (s, e) => AcceptCompletion();
            UpdateLineNumbers();
        }

        public static readonly DependencyProperty EditorFontSizeProperty = DependencyProperty.Register(
            nameof(EditorFontSize), typeof(double), typeof(ScriptEditor), new PropertyMetadata(14.0, (o, e) =>
            {
                var c = (ScriptEditor)o;
                c.PART_TextBox.FontSize = (double)e.NewValue;
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
            var lines = PART_TextBox.LineCount;
            var nums = string.Join("\n", Enumerable.Range(1, lines == 0 ? 1 : lines));
            PART_LineNumbers.Text = nums;
        }

        private void OnKeyUpForAutocomplete(object sender, KeyEventArgs e)
        {
            // auto-indent
            if (e.Key == Key.Return)
            {
                int line = PART_TextBox.GetLineIndexFromCharacterIndex(PART_TextBox.CaretIndex);
                if (line > 0)
                {
                    var prev = PART_TextBox.GetLineText(line - 1);
                    var indent = new string(prev.TakeWhile(ch => ch == ' ' || ch == '\t').ToArray());
                    PART_TextBox.SelectedText = indent;
                    PART_TextBox.CaretIndex += indent.Length;
                }
            }

            // simple bracket match visual hint could be added here (not selecting for now)

            if (!AutocompleteEnabled) { PART_Popup.IsOpen = false; return; }
            if (char.IsLetterOrDigit((char)KeyInterop.VirtualKeyFromKey(e.Key)) || e.Key == Key.OemPeriod)
            {
                var caret = PART_TextBox.CaretIndex;
                var start = caret;
                while (start > 0 && (char.IsLetterOrDigit(PART_TextBox.Text[start - 1]) || PART_TextBox.Text[start - 1] == '_')) start--;
                var token = PART_TextBox.Text.Substring(start, caret - start);
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
            var caret = PART_TextBox.CaretIndex;
            var start = caret;
            while (start > 0 && (char.IsLetterOrDigit(PART_TextBox.Text[start - 1]) || PART_TextBox.Text[start - 1] == '_')) start--;
            PART_TextBox.Select(start, caret - start);
            PART_TextBox.SelectedText = suggestion;
            PART_TextBox.CaretIndex = start + suggestion.Length;
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
