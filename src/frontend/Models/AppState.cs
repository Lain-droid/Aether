using System.Collections.Generic;

namespace AetherVisor.Frontend.Models
{
    public class AppState
    {
        public List<string> OpenFiles { get; set; } = new List<string>();
        public string ActiveFile { get; set; }
        public double EditorFontSize { get; set; } = 14;
        public bool AutocompleteEnabled { get; set; } = true;
    }
}
