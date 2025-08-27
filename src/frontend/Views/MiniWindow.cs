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
                // Basit bir arama yerine örnek bir siyah-beyaz Aether tarzı svg/png URL'si kullanın
                var url = "https://upload.wikimedia.org/wikipedia/commons/3/3f/OOjs_UI_icon_puzzle.svg";
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
