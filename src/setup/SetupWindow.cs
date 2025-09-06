using System.Windows;
using AetherVisor.Frontend.ViewModels;

namespace AetherVisor.Frontend.Views
{
    public partial class SetupWindow : Window
    {
        public SetupWindow()
        {
            InitializeComponent();
            DataContext = new SetupViewModel();
        }
    }
}
