// This file represents the code-behind for MainWindow.xaml
// In a real WPF project, this would be MainWindow.xaml.cs

using System.Windows;
using AetherVisor.Frontend.ViewModels;

namespace AetherVisor.Frontend.Views
{
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            DataContext = new MainViewModel();
        }
    }
}
