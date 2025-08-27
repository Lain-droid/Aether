using System.Collections.ObjectModel;

namespace AetherVisor.Frontend.Models
{
    public class FileNode
    {
        public string Name { get; set; }
        public string FullPath { get; set; }
        public bool IsDirectory { get; set; }
        public ObservableCollection<FileNode> Children { get; set; } = new ObservableCollection<FileNode>();
    }
}
