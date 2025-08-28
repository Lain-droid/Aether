using System.Windows;
using AetherVisor.Frontend.ViewModels;

namespace AetherVisor.Frontend.Views
{
    public partial class SettingsWindow : Window
    {
        public SettingsWindow(AppSettingsViewModel vm)
        {
            InitializeComponent();
            DataContext = vm;
        }
    }
}
