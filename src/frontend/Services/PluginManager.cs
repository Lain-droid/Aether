using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel.Composition;
using System.ComponentModel.Composition.Hosting;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using AetherVisor.Frontend.Models;

namespace AetherVisor.Frontend.Services
{
    public interface IPlugin
    {
        string Name { get; }
        string Version { get; }
        string Description { get; }
        string Author { get; }
        PluginInfo Info { get; }
        
        Task InitializeAsync(IPluginHost host);
        Task ActivateAsync();
        Task DeactivateAsync();
        void Dispose();
    }

    public interface IPluginHost
    {
        IEditorService EditorService { get; }
        IUIService UIService { get; }
        ISettingsService SettingsService { get; }
        IEventService EventService { get; }
        
        void RegisterCommand(string commandId, Action<object> action, string displayName = null);
        void RegisterMenuItem(string path, Action<object> action, string icon = null);
        void RegisterStatusBarItem(string id, FrameworkElement element);
        void RegisterSidePanel(string id, string title, UserControl panel, string icon = null);
    }

    public interface IEditorService
    {
        string GetCurrentText();
        void SetCurrentText(string text);
        void InsertText(string text);
        void InsertTextAtPosition(int position, string text);
        int GetCaretPosition();
        void SetCaretPosition(int position);
        string GetSelectedText();
        void SetSelection(int start, int length);
        event EventHandler<TextChangedEventArgs> TextChanged;
        event EventHandler<CaretPositionChangedEventArgs> CaretPositionChanged;
    }

    public interface IUIService
    {
        void ShowNotification(string message, NotificationType type = NotificationType.Info);
        bool ShowConfirmationDialog(string message, string title = "Confirm");
        string ShowInputDialog(string prompt, string title = "Input", string defaultValue = "");
        void ShowCustomDialog(Window dialog);
        void AddToToolbar(string id, FrameworkElement element);
        void AddToStatusBar(string id, FrameworkElement element);
    }

    public interface ISettingsService
    {
        T GetSetting<T>(string key, T defaultValue = default);
        void SetSetting<T>(string key, T value);
        void SaveSettings();
        event EventHandler<SettingChangedEventArgs> SettingChanged;
    }

    public interface IEventService
    {
        void Subscribe<T>(Action<T> handler) where T : class;
        void Unsubscribe<T>(Action<T> handler) where T : class;
        void Publish<T>(T eventData) where T : class;
    }

    public class PluginManager : IPluginHost
    {
        private CompositionContainer _container;
        private DirectoryCatalog _catalog;
        private readonly string _pluginsDirectory;
        private readonly ObservableCollection<PluginInfo> _loadedPlugins;
        private readonly Dictionary<string, IPlugin> _activePlugins;

        [ImportMany]
        public IEnumerable<Lazy<IPlugin, IPluginMetadata>> Plugins { get; set; }

        public IEditorService EditorService { get; private set; }
        public IUIService UIService { get; private set; }
        public ISettingsService SettingsService { get; private set; }
        public IEventService EventService { get; private set; }

        public ObservableCollection<PluginInfo> LoadedPlugins => _loadedPlugins;

        public event EventHandler<PluginEventArgs> PluginLoaded;
        public event EventHandler<PluginEventArgs> PluginUnloaded;
        public event EventHandler<PluginEventArgs> PluginActivated;
        public event EventHandler<PluginEventArgs> PluginDeactivated;

        public PluginManager()
        {
            _pluginsDirectory = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "Plugins");
            _loadedPlugins = new ObservableCollection<PluginInfo>();
            _activePlugins = new Dictionary<string, IPlugin>();

            // Ensure plugins directory exists
            Directory.CreateDirectory(_pluginsDirectory);
        }

        public async Task InitializeAsync(IEditorService editorService, IUIService uiService, 
            ISettingsService settingsService, IEventService eventService)
        {
            EditorService = editorService;
            UIService = uiService;
            SettingsService = settingsService;
            EventService = eventService;

            await LoadPluginsAsync();
        }

        public async Task LoadPluginsAsync()
        {
            try
            {
                _catalog = new DirectoryCatalog(_pluginsDirectory, "*.dll");
                _container = new CompositionContainer(_catalog);
                _container.ComposeParts(this);

                foreach (var plugin in Plugins)
                {
                    try
                    {
                        var pluginInstance = plugin.Value;
                        var metadata = plugin.Metadata;

                        var pluginInfo = new PluginInfo
                        {
                            Id = metadata.Id ?? pluginInstance.Name,
                            Name = pluginInstance.Name,
                            Version = pluginInstance.Version,
                            Description = pluginInstance.Description,
                            Author = pluginInstance.Author,
                            IsEnabled = SettingsService.GetSetting($"Plugin.{metadata.Id}.Enabled", true),
                            Plugin = pluginInstance
                        };

                        _loadedPlugins.Add(pluginInfo);

                        await pluginInstance.InitializeAsync(this);

                        if (pluginInfo.IsEnabled)
                        {
                            await ActivatePluginAsync(pluginInfo.Id);
                        }

                        PluginLoaded?.Invoke(this, new PluginEventArgs(pluginInfo));
                    }
                    catch (Exception ex)
                    {
                        UIService?.ShowNotification($"Failed to load plugin: {ex.Message}", NotificationType.Error);
                    }
                }
            }
            catch (Exception ex)
            {
                UIService?.ShowNotification($"Failed to initialize plugin system: {ex.Message}", NotificationType.Error);
            }
        }

        public async Task<bool> InstallPluginAsync(string pluginPath)
        {
            try
            {
                var fileName = Path.GetFileName(pluginPath);
                var targetPath = Path.Combine(_pluginsDirectory, fileName);

                if (File.Exists(targetPath))
                {
                    var overwrite = UIService.ShowConfirmationDialog(
                        $"Plugin {fileName} already exists. Overwrite?", "Plugin Installation");
                    if (!overwrite) return false;
                }

                File.Copy(pluginPath, targetPath, true);
                
                // Reload plugins
                await ReloadPluginsAsync();
                
                UIService.ShowNotification($"Plugin {fileName} installed successfully", NotificationType.Success);
                return true;
            }
            catch (Exception ex)
            {
                UIService.ShowNotification($"Failed to install plugin: {ex.Message}", NotificationType.Error);
                return false;
            }
        }

        public async Task<bool> UninstallPluginAsync(string pluginId)
        {
            try
            {
                var plugin = _loadedPlugins.FirstOrDefault(p => p.Id == pluginId);
                if (plugin == null) return false;

                // Deactivate if active
                if (_activePlugins.ContainsKey(pluginId))
                {
                    await DeactivatePluginAsync(pluginId);
                }

                // Remove from loaded plugins
                _loadedPlugins.Remove(plugin);

                // Delete plugin file (this might require restart)
                var pluginAssembly = plugin.Plugin.GetType().Assembly;
                var assemblyPath = pluginAssembly.Location;
                
                if (File.Exists(assemblyPath))
                {
                    try
                    {
                        File.Delete(assemblyPath);
                    }
                    catch
                    {
                        UIService.ShowNotification(
                            "Plugin file will be deleted on next restart", NotificationType.Warning);
                    }
                }

                PluginUnloaded?.Invoke(this, new PluginEventArgs(plugin));
                UIService.ShowNotification($"Plugin {plugin.Name} uninstalled", NotificationType.Success);
                return true;
            }
            catch (Exception ex)
            {
                UIService.ShowNotification($"Failed to uninstall plugin: {ex.Message}", NotificationType.Error);
                return false;
            }
        }

        public async Task<bool> ActivatePluginAsync(string pluginId)
        {
            try
            {
                var pluginInfo = _loadedPlugins.FirstOrDefault(p => p.Id == pluginId);
                if (pluginInfo == null || _activePlugins.ContainsKey(pluginId)) return false;

                await pluginInfo.Plugin.ActivateAsync();
                _activePlugins[pluginId] = pluginInfo.Plugin;
                pluginInfo.IsActive = true;

                SettingsService.SetSetting($"Plugin.{pluginId}.Enabled", true);
                PluginActivated?.Invoke(this, new PluginEventArgs(pluginInfo));
                
                return true;
            }
            catch (Exception ex)
            {
                UIService.ShowNotification($"Failed to activate plugin: {ex.Message}", NotificationType.Error);
                return false;
            }
        }

        public async Task<bool> DeactivatePluginAsync(string pluginId)
        {
            try
            {
                if (!_activePlugins.TryGetValue(pluginId, out var plugin)) return false;

                await plugin.DeactivateAsync();
                _activePlugins.Remove(pluginId);

                var pluginInfo = _loadedPlugins.FirstOrDefault(p => p.Id == pluginId);
                if (pluginInfo != null)
                {
                    pluginInfo.IsActive = false;
                }

                SettingsService.SetSetting($"Plugin.{pluginId}.Enabled", false);
                PluginDeactivated?.Invoke(this, new PluginEventArgs(pluginInfo));

                return true;
            }
            catch (Exception ex)
            {
                UIService.ShowNotification($"Failed to deactivate plugin: {ex.Message}", NotificationType.Error);
                return false;
            }
        }

        public async Task ReloadPluginsAsync()
        {
            // Deactivate all plugins
            var activePluginIds = _activePlugins.Keys.ToList();
            foreach (var pluginId in activePluginIds)
            {
                await DeactivatePluginAsync(pluginId);
            }

            // Dispose container
            _container?.Dispose();
            _catalog?.Dispose();

            // Clear loaded plugins
            _loadedPlugins.Clear();
            _activePlugins.Clear();

            // Reload
            await LoadPluginsAsync();
        }

        public PluginInfo GetPluginInfo(string pluginId)
        {
            return _loadedPlugins.FirstOrDefault(p => p.Id == pluginId);
        }

        public IEnumerable<PluginInfo> GetAllPlugins()
        {
            return _loadedPlugins.ToList();
        }

        public IEnumerable<PluginInfo> GetActivePlugins()
        {
            return _loadedPlugins.Where(p => p.IsActive).ToList();
        }

        // IPluginHost implementation
        public void RegisterCommand(string commandId, Action<object> action, string displayName = null)
        {
            // Implementation for registering commands
            EventService.Publish(new CommandRegisteredEvent
            {
                CommandId = commandId,
                Action = action,
                DisplayName = displayName
            });
        }

        public void RegisterMenuItem(string path, Action<object> action, string icon = null)
        {
            // Implementation for registering menu items
            EventService.Publish(new MenuItemRegisteredEvent
            {
                Path = path,
                Action = action,
                Icon = icon
            });
        }

        public void RegisterStatusBarItem(string id, FrameworkElement element)
        {
            UIService.AddToStatusBar(id, element);
        }

        public void RegisterSidePanel(string id, string title, UserControl panel, string icon = null)
        {
            EventService.Publish(new SidePanelRegisteredEvent
            {
                Id = id,
                Title = title,
                Panel = panel,
                Icon = icon
            });
        }

        public void Dispose()
        {
            // Deactivate all plugins
            foreach (var plugin in _activePlugins.Values)
            {
                try
                {
                    plugin.DeactivateAsync().Wait(TimeSpan.FromSeconds(5));
                    plugin.Dispose();
                }
                catch (Exception ex)
                {
                    // Log error: Debug statement removed
                }
            }

            _activePlugins.Clear();
            _loadedPlugins.Clear();
            
            _container?.Dispose();
            _catalog?.Dispose();
        }
    }

    // Plugin metadata interface for MEF
    public interface IPluginMetadata
    {
        string Id { get; }
        string Name { get; }
        string Version { get; }
    }

    // Event classes
    public class PluginEventArgs : EventArgs
    {
        public PluginInfo Plugin { get; }

        public PluginEventArgs(PluginInfo plugin)
        {
            Plugin = plugin;
        }
    }

    public class CommandRegisteredEvent
    {
        public string CommandId { get; set; }
        public Action<object> Action { get; set; }
        public string DisplayName { get; set; }
    }

    public class MenuItemRegisteredEvent
    {
        public string Path { get; set; }
        public Action<object> Action { get; set; }
        public string Icon { get; set; }
    }

    public class SidePanelRegisteredEvent
    {
        public string Id { get; set; }
        public string Title { get; set; }
        public UserControl Panel { get; set; }
        public string Icon { get; set; }
    }

    public class TextChangedEventArgs : EventArgs
    {
        public string NewText { get; }
        public string OldText { get; }

        public TextChangedEventArgs(string newText, string oldText)
        {
            NewText = newText;
            OldText = oldText;
        }
    }

    public class CaretPositionChangedEventArgs : EventArgs
    {
        public int Position { get; }
        public int Line { get; }
        public int Column { get; }

        public CaretPositionChangedEventArgs(int position, int line, int column)
        {
            Position = position;
            Line = line;
            Column = column;
        }
    }

    public class SettingChangedEventArgs : EventArgs
    {
        public string Key { get; }
        public object OldValue { get; }
        public object NewValue { get; }

        public SettingChangedEventArgs(string key, object oldValue, object newValue)
        {
            Key = key;
            OldValue = oldValue;
            NewValue = newValue;
        }
    }

    public enum NotificationType
    {
        Info,
        Success,
        Warning,
        Error
    }
}