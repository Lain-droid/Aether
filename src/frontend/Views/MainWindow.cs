// This file represents the code-behind for MainWindow.xaml
// In a real WPF project, this would be MainWindow.xaml.cs

using System.Windows;
using AetherVisor.Frontend.ViewModels;
using AetherVisor.Frontend.Views;
using System.Windows.Controls;
using AetherVisor.Frontend.Models;

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
    }
}
