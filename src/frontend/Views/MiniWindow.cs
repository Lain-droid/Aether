using System.Net.Http;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media.Imaging;

namespace AetherVisor.Frontend.Views
{
    public partial class MiniWindow : Window
    {
        public MiniWindow()
        {
            InitializeComponent();
            _ = LoadLogoAsync();
        }

        private async Task LoadLogoAsync()
        {
            try {
                using var http = new HttpClient();
                var url = "https://img.icons8.com/?size=100&id=N3ft0cEVp9ui&format=png&color=000000";
                var bytes = await http.GetByteArrayAsync(url);
                using var ms = new System.IO.MemoryStream(bytes);
                var bmp = new BitmapImage();
                bmp.BeginInit();
                bmp.StreamSource = ms;
                bmp.CacheOption = BitmapCacheOption.OnLoad;
                bmp.EndInit();
                bmp.Freeze();
                Logo.Source = bmp;
            } catch { }
        }
    }
}
