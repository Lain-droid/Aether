// This file represents the code-behind for MainWindow.xaml
// In a real WPF project, this would be MainWindow.xaml.cs

using System.Windows;
using AetherVisor.Frontend.ViewModels;
using AetherVisor.Frontend.Views;
using System.Windows.Controls;
using AetherVisor.Frontend.Models;
using System.Linq;

namespace AetherVisor.Frontend.Views
{
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            DataContext = new MainViewModel();
        }

        private void OpenSettings_Click(object sender, RoutedEventArgs e)
        {
            var vm = new AppSettingsViewModel();
            var dlg = new SettingsWindow(vm) { Owner = this };
            dlg.ShowDialog();
        }

        private void TreeView_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
        {
            var node = e.NewValue as FileNode;
            if (node == null) return;
            if (DataContext is MainViewModel vm)
            {
                vm.OpenFile(node);
            }
        }

        private void Minimize_Click(object sender, RoutedEventArgs e)
        {
            var mini = new MiniWindow { Owner = this };
            mini.Left = this.Left + this.Width - 120;
            mini.Top = this.Top + 40;
            mini.Show();
            this.Hide();
            mini.Closed += (s, _) => this.Show();
        }

        private void Find_Click(object sender, RoutedEventArgs e)
        {
            var find = FindName("FindBox") as TextBox;
            if (find == null) return;
            if (DataContext is ViewModels.MainViewModel vm)
            {
                var text = vm.ScriptText ?? string.Empty;
                var idx = text.IndexOf(find.Text ?? string.Empty, System.StringComparison.OrdinalIgnoreCase);
                if (idx >= 0)
                {
                    // naive select not implemented (TextBox is inside ScriptEditor); could extend ScriptEditor API
                    vm.ConsoleOutput.Add($"Found at {idx}");
                }
            }
        }

        private void Replace_Click(object sender, RoutedEventArgs e)
        {
            var find = FindName("FindBox") as TextBox;
            var repl = FindName("ReplaceBox") as TextBox;
            if (find == null || repl == null) return;
            if (DataContext is ViewModels.MainViewModel vm)
            {
                vm.ScriptText = (vm.ScriptText ?? string.Empty).Replace(find.Text ?? string.Empty, repl.Text ?? string.Empty);
            }
        }
    }
}
