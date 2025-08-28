using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Text.Json;
using System.Threading.Tasks;
using AetherVisor.Frontend.Models;

namespace AetherVisor.Frontend.Services
{
    public class ApiDocumentationService
    {
        private readonly HttpClient _httpClient;
        private readonly string _cacheDirectory;
        private readonly Dictionary<string, List<ApiDocItem>> _documentationCache;
        
        public ObservableCollection<ApiCategory> Categories { get; }
        public ObservableCollection<ApiDocItem> SearchResults { get; }
        public ObservableCollection<ApiDocItem> Favorites { get; }
        public ObservableCollection<ApiDocItem> RecentlyViewed { get; }
        
        private const string ROBLOX_API_URL = "https://raw.githubusercontent.com/CloneTrooper1019/Roblox-Client-Tracker/roblox/API-Dump.json";
        private const string ROBLOX_DOCS_URL = "https://developer.roblox.com/en-us/api-reference";
        
        public ApiDocumentationService()
        {
            _httpClient = new HttpClient();
            _cacheDirectory = Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
                "Aether", "ApiDocumentation");
            
            Directory.CreateDirectory(_cacheDirectory);
            
            _documentationCache = new Dictionary<string, List<ApiDocItem>>();
            
            Categories = new ObservableCollection<ApiCategory>();
            SearchResults = new ObservableCollection<ApiDocItem>();
            Favorites = new ObservableCollection<ApiDocItem>();
            RecentlyViewed = new ObservableCollection<ApiDocItem>();
            
            LoadCachedData();
        }
        
        #region Public Methods
        
        public async Task InitializeAsync()
        {
            try
            {
                await LoadRobloxApiDocumentationAsync();
                await LoadCustomDocumentationAsync();
                LoadFavorites();
                LoadRecentlyViewed();
            }
            catch (Exception ex)
            {
                // Log error: Debug statement removed
            }
        }
        
        public async Task<List<ApiDocItem>> SearchAsync(string query, ApiSearchFilter filter = null)
        {
            if (string.IsNullOrWhiteSpace(query))
                return new List<ApiDocItem>();
            
            var results = new List<ApiDocItem>();
            
            foreach (var categoryItems in _documentationCache.Values)
            {
                var filteredItems = categoryItems.Where(item =>
                    MatchesSearchQuery(item, query) &&
                    MatchesFilter(item, filter)
                ).ToList();
                
                results.AddRange(filteredItems);
            }
            
            // Sort by relevance
            results = results.OrderByDescending(item => CalculateRelevanceScore(item, query)).ToList();
            
            // Update search results collection
            SearchResults.Clear();
            foreach (var result in results.Take(50)) // Limit to 50 results
            {
                SearchResults.Add(result);
            }
            
            return results;
        }
        
        public async Task<ApiDocItem> GetDocumentationAsync(string itemId)
        {
            foreach (var categoryItems in _documentationCache.Values)
            {
                var item = categoryItems.FirstOrDefault(i => i.Id == itemId);
                if (item != null)
                {
                    AddToRecentlyViewed(item);
                    return item;
                }
            }
            
            return null;
        }
        
        public void AddToFavorites(ApiDocItem item)
        {
            if (!Favorites.Any(f => f.Id == item.Id))
            {
                Favorites.Add(item);
                SaveFavorites();
            }
        }
        
        public void RemoveFromFavorites(ApiDocItem item)
        {
            var favorite = Favorites.FirstOrDefault(f => f.Id == item.Id);
            if (favorite != null)
            {
                Favorites.Remove(favorite);
                SaveFavorites();
            }
        }
        
        public bool IsFavorite(ApiDocItem item)
        {
            return Favorites.Any(f => f.Id == item.Id);
        }
        
        public List<ApiDocItem> GetItemsByCategory(string categoryName)
        {
            if (_documentationCache.TryGetValue(categoryName, out var items))
            {
                return items.ToList();
            }
            
            return new List<ApiDocItem>();
        }
        
        public List<string> GetSuggestions(string partialQuery)
        {
            var suggestions = new List<string>();
            
            foreach (var categoryItems in _documentationCache.Values)
            {
                var matches = categoryItems
                    .Where(item => item.Name.StartsWith(partialQuery, StringComparison.OrdinalIgnoreCase))
                    .Select(item => item.Name)
                    .Take(10);
                
                suggestions.AddRange(matches);
            }
            
            return suggestions.Distinct().OrderBy(s => s).ToList();
        }
        
        public async Task RefreshDocumentationAsync()
        {
            // Clear cache
            _documentationCache.Clear();
            Categories.Clear();
            
            // Reload all documentation
            await InitializeAsync();
        }
        
        public async Task ExportDocumentationAsync(string filePath, List<ApiDocItem> items = null)
        {
            var itemsToExport = items ?? GetAllDocumentationItems();
            
            var exportData = new
            {
                ExportedAt = DateTime.UtcNow,
                TotalItems = itemsToExport.Count,
                Categories = Categories.Select(c => new { c.Name, c.Description, ItemCount = c.Items.Count }),
                Items = itemsToExport
            };
            
            var json = JsonSerializer.Serialize(exportData, new JsonSerializerOptions
            {
                WriteIndented = true
            });
            
            await File.WriteAllTextAsync(filePath, json);
        }
        
        #endregion
        
        #region Private Methods
        
        private async Task LoadRobloxApiDocumentationAsync()
        {
            try
            {
                var cacheFile = Path.Combine(_cacheDirectory, "roblox_api.json");
                var shouldDownload = !File.Exists(cacheFile) || 
                    (DateTime.Now - File.GetLastWriteTime(cacheFile)).TotalDays > 7;
                
                string apiData;
                
                if (shouldDownload)
                {
                    apiData = await _httpClient.GetStringAsync(ROBLOX_API_URL);
                    await File.WriteAllTextAsync(cacheFile, apiData);
                }
                else
                {
                    apiData = await File.ReadAllTextAsync(cacheFile);
                }
                
                await ParseRobloxApiDataAsync(apiData);
            }
            catch (Exception ex)
            {
                // Log error: Debug statement removed
                await LoadFallbackRobloxDocumentation();
            }
        }
        
        private async Task ParseRobloxApiDataAsync(string apiData)
        {
            try
            {
                using var document = JsonDocument.Parse(apiData);
                var root = document.RootElement;
                
                // Parse classes
                if (root.TryGetProperty("Classes", out var classes))
                {
                    var classItems = new List<ApiDocItem>();
                    
                    foreach (var classElement in classes.EnumerateArray())
                    {
                        var classItem = ParseClassElement(classElement);
                        if (classItem != null)
                        {
                            classItems.Add(classItem);
                        }
                    }
                    
                    _documentationCache["Classes"] = classItems;
                    Categories.Add(new ApiCategory
                    {
                        Name = "Classes",
                        Description = "Roblox API Classes",
                        Icon = "📦",
                        Items = new ObservableCollection<ApiDocItem>(classItems)
                    });
                }
                
                // Parse enums
                if (root.TryGetProperty("Enums", out var enums))
                {
                    var enumItems = new List<ApiDocItem>();
                    
                    foreach (var enumElement in enums.EnumerateArray())
                    {
                        var enumItem = ParseEnumElement(enumElement);
                        if (enumItem != null)
                        {
                            enumItems.Add(enumItem);
                        }
                    }
                    
                    _documentationCache["Enums"] = enumItems;
                    Categories.Add(new ApiCategory
                    {
                        Name = "Enums",
                        Description = "Roblox API Enumerations",
                        Icon = "📋",
                        Items = new ObservableCollection<ApiDocItem>(enumItems)
                    });
                }
            }
            catch (Exception ex)
            {
                // Log error: Debug statement removed
            }
        }
        
        private ApiDocItem ParseClassElement(JsonElement classElement)
        {
            try
            {
                var name = classElement.GetProperty("Name").GetString();
                var item = new ApiDocItem
                {
                    Id = $"class_{name}",
                    Name = name,
                    Type = ApiItemType.Class,
                    Category = "Classes",
                    Description = GetClassDescription(name),
                    Syntax = $"local {name.ToLower()} = {name}.new()",
                    Tags = new List<string> { "class", "roblox" }
                };
                
                // Parse members
                if (classElement.TryGetProperty("Members", out var members))
                {
                    var memberItems = new List<ApiDocItem>();
                    
                    foreach (var member in members.EnumerateArray())
                    {
                        var memberItem = ParseMemberElement(member, name);
                        if (memberItem != null)
                        {
                            memberItems.Add(memberItem);
                        }
                    }
                    
                    item.Children = memberItems;
                }
                
                return item;
            }
            catch
            {
                return null;
            }
        }
        
        private ApiDocItem ParseMemberElement(JsonElement memberElement, string className)
        {
            try
            {
                var memberName = memberElement.GetProperty("Name").GetString();
                var memberType = memberElement.GetProperty("MemberType").GetString();
                
                var item = new ApiDocItem
                {
                    Id = $"{className}_{memberName}",
                    Name = $"{className}.{memberName}",
                    Type = GetApiItemType(memberType),
                    Category = "Members",
                    Description = GetMemberDescription(className, memberName, memberType),
                    Tags = new List<string> { "member", memberType.ToLower(), className.ToLower() }
                };
                
                // Get return type or value type
                if (memberElement.TryGetProperty("ValueType", out var valueType))
                {
                    var returnType = ParseValueType(valueType);
                    item.ReturnType = returnType;
                    item.Syntax = GenerateMemberSyntax(className, memberName, memberType, returnType);
                }
                
                // Get parameters for functions
                if (memberType == "Function" && memberElement.TryGetProperty("Parameters", out var parameters))
                {
                    item.Parameters = ParseParameters(parameters);
                    item.Syntax = GenerateFunctionSyntax(className, memberName, item.Parameters, item.ReturnType);
                }
                
                return item;
            }
            catch
            {
                return null;
            }
        }
        
        private ApiDocItem ParseEnumElement(JsonElement enumElement)
        {
            try
            {
                var name = enumElement.GetProperty("Name").GetString();
                var item = new ApiDocItem
                {
                    Id = $"enum_{name}",
                    Name = name,
                    Type = ApiItemType.Enum,
                    Category = "Enums",
                    Description = GetEnumDescription(name),
                    Syntax = $"Enum.{name}",
                    Tags = new List<string> { "enum", "roblox" }
                };
                
                // Parse enum items
                if (enumElement.TryGetProperty("Items", out var items))
                {
                    var enumItems = new List<ApiDocItem>();
                    
                    foreach (var enumItem in items.EnumerateArray())
                    {
                        if (enumItem.TryGetProperty("Name", out var itemName))
                        {
                            var enumItemDoc = new ApiDocItem
                            {
                                Id = $"enum_{name}_{itemName.GetString()}",
                                Name = $"Enum.{name}.{itemName.GetString()}",
                                Type = ApiItemType.EnumItem,
                                Category = "Enum Items",
                                Description = $"Enum value of {name}",
                                Syntax = $"Enum.{name}.{itemName.GetString()}",
                                Tags = new List<string> { "enum-item", name.ToLower() }
                            };
                            
                            enumItems.Add(enumItemDoc);
                        }
                    }
                    
                    item.Children = enumItems;
                }
                
                return item;
            }
            catch
            {
                return null;
            }
        }
        
        private async Task LoadCustomDocumentationAsync()
        {
            var customDocsPath = Path.Combine(_cacheDirectory, "custom_docs.json");
            
            if (File.Exists(customDocsPath))
            {
                try
                {
                    var json = await File.ReadAllTextAsync(customDocsPath);
                    var customDocs = JsonSerializer.Deserialize<List<ApiDocItem>>(json);
                    
                    if (customDocs != null)
                    {
                        _documentationCache["Custom"] = customDocs;
                        Categories.Add(new ApiCategory
                        {
                            Name = "Custom",
                            Description = "Custom API Documentation",
                            Icon = "📝",
                            Items = new ObservableCollection<ApiDocItem>(customDocs)
                        });
                    }
                }
                catch (Exception ex)
                {
                    // Log error: Debug statement removed
                }
            }
            
            // Add Luau built-in documentation
            await LoadLuauBuiltinsAsync();
        }
        
        private async Task LoadLuauBuiltinsAsync()
        {
            var builtins = new List<ApiDocItem>
            {
                new ApiDocItem
                {
                    Id = "print",
                    Name = "print",
                    Type = ApiItemType.Function,
                    Category = "Luau",
                    Description = "Prints the given arguments to the output",
                    Syntax = "print(...)",
                    Parameters = new List<ApiParameter>
                    {
                        new ApiParameter { Name = "...", Type = "any", Description = "Values to print" }
                    },
                    ReturnType = "void",
                    Tags = new List<string> { "luau", "builtin", "debug" },
                    Examples = new List<ApiExample>
                    {
                        new ApiExample
                        {
                            Title = "Basic Usage",
                            Code = "print(\"Hello, World!\")\nprint(42, true, nil)",
                            Description = "Print different types of values"
                        }
                    }
                },
                
                new ApiDocItem
                {
                    Id = "warn",
                    Name = "warn",
                    Type = ApiItemType.Function,
                    Category = "Luau",
                    Description = "Prints a warning message to the output",
                    Syntax = "warn(...)",
                    Parameters = new List<ApiParameter>
                    {
                        new ApiParameter { Name = "...", Type = "any", Description = "Warning message" }
                    },
                    ReturnType = "void",
                    Tags = new List<string> { "luau", "builtin", "debug" }
                },
                
                new ApiDocItem
                {
                    Id = "error",
                    Name = "error",
                    Type = ApiItemType.Function,
                    Category = "Luau",
                    Description = "Raises an error with the given message",
                    Syntax = "error(message, level)",
                    Parameters = new List<ApiParameter>
                    {
                        new ApiParameter { Name = "message", Type = "string", Description = "Error message" },
                        new ApiParameter { Name = "level", Type = "number", Description = "Stack level (optional)", IsOptional = true }
                    },
                    ReturnType = "never",
                    Tags = new List<string> { "luau", "builtin", "error" }
                }
            };
            
            _documentationCache["Luau"] = builtins;
            Categories.Add(new ApiCategory
            {
                Name = "Luau",
                Description = "Luau Built-in Functions",
                Icon = "⚡",
                Items = new ObservableCollection<ApiDocItem>(builtins)
            });
        }
        
        private async Task LoadFallbackRobloxDocumentation()
        {
            // Load basic Roblox API items as fallback
            var fallbackItems = new List<ApiDocItem>
            {
                new ApiDocItem
                {
                    Id = "game",
                    Name = "game",
                    Type = ApiItemType.Service,
                    Category = "Services",
                    Description = "The top-level DataModel object",
                    Syntax = "game",
                    Tags = new List<string> { "roblox", "service", "global" }
                },
                
                new ApiDocItem
                {
                    Id = "workspace",
                    Name = "workspace",
                    Type = ApiItemType.Service,
                    Category = "Services",
                    Description = "Alias for game.Workspace",
                    Syntax = "workspace",
                    Tags = new List<string> { "roblox", "service", "global" }
                }
            };
            
            _documentationCache["Services"] = fallbackItems;
            Categories.Add(new ApiCategory
            {
                Name = "Services",
                Description = "Roblox Services",
                Icon = "🔧",
                Items = new ObservableCollection<ApiDocItem>(fallbackItems)
            });
        }
        
        private void LoadCachedData()
        {
            // Load any cached documentation that doesn't require network
        }
        
        private void LoadFavorites()
        {
            var favoritesFile = Path.Combine(_cacheDirectory, "favorites.json");
            
            if (File.Exists(favoritesFile))
            {
                try
                {
                    var json = File.ReadAllText(favoritesFile);
                    var favoriteIds = JsonSerializer.Deserialize<List<string>>(json);
                    
                    if (favoriteIds != null)
                    {
                        foreach (var id in favoriteIds)
                        {
                            var item = FindItemById(id);
                            if (item != null)
                            {
                                Favorites.Add(item);
                            }
                        }
                    }
                }
                catch (Exception ex)
                {
                    // Log error: Debug statement removed
                }
            }
        }
        
        private void LoadRecentlyViewed()
        {
            var recentFile = Path.Combine(_cacheDirectory, "recent.json");
            
            if (File.Exists(recentFile))
            {
                try
                {
                    var json = File.ReadAllText(recentFile);
                    var recentIds = JsonSerializer.Deserialize<List<string>>(json);
                    
                    if (recentIds != null)
                    {
                        foreach (var id in recentIds.Take(20)) // Limit to 20 recent items
                        {
                            var item = FindItemById(id);
                            if (item != null)
                            {
                                RecentlyViewed.Add(item);
                            }
                        }
                    }
                }
                catch (Exception ex)
                {
                    // Log error: Debug statement removed
                }
            }
        }
        
        private void SaveFavorites()
        {
            try
            {
                var favoriteIds = Favorites.Select(f => f.Id).ToList();
                var json = JsonSerializer.Serialize(favoriteIds, new JsonSerializerOptions
                {
                    WriteIndented = true
                });
                
                var favoritesFile = Path.Combine(_cacheDirectory, "favorites.json");
                File.WriteAllText(favoritesFile, json);
            }
            catch (Exception ex)
            {
                // Log error: Debug statement removed
            }
        }
        
        private void AddToRecentlyViewed(ApiDocItem item)
        {
            // Remove if already exists
            var existing = RecentlyViewed.FirstOrDefault(r => r.Id == item.Id);
            if (existing != null)
            {
                RecentlyViewed.Remove(existing);
            }
            
            // Add to beginning
            RecentlyViewed.Insert(0, item);
            
            // Limit to 20 items
            while (RecentlyViewed.Count > 20)
            {
                RecentlyViewed.RemoveAt(RecentlyViewed.Count - 1);
            }
            
            // Save to file
            SaveRecentlyViewed();
        }
        
        private void SaveRecentlyViewed()
        {
            try
            {
                var recentIds = RecentlyViewed.Select(r => r.Id).ToList();
                var json = JsonSerializer.Serialize(recentIds, new JsonSerializerOptions
                {
                    WriteIndented = true
                });
                
                var recentFile = Path.Combine(_cacheDirectory, "recent.json");
                File.WriteAllText(recentFile, json);
            }
            catch (Exception ex)
            {
                // Log error: Debug statement removed
            }
        }
        
        private ApiDocItem FindItemById(string id)
        {
            foreach (var categoryItems in _documentationCache.Values)
            {
                var item = categoryItems.FirstOrDefault(i => i.Id == id);
                if (item != null)
                {
                    return item;
                }
                
                // Search in children
                foreach (var parent in categoryItems.Where(i => i.Children != null))
                {
                    var child = parent.Children.FirstOrDefault(c => c.Id == id);
                    if (child != null)
                    {
                        return child;
                    }
                }
            }
            
            return null;
        }
        
        private List<ApiDocItem> GetAllDocumentationItems()
        {
            var allItems = new List<ApiDocItem>();
            
            foreach (var categoryItems in _documentationCache.Values)
            {
                allItems.AddRange(categoryItems);
                
                foreach (var item in categoryItems.Where(i => i.Children != null))
                {
                    allItems.AddRange(item.Children);
                }
            }
            
            return allItems;
        }
        
        private bool MatchesSearchQuery(ApiDocItem item, string query)
        {
            var lowerQuery = query.ToLowerInvariant();
            
            return item.Name.ToLowerInvariant().Contains(lowerQuery) ||
                   item.Description.ToLowerInvariant().Contains(lowerQuery) ||
                   item.Tags.Any(tag => tag.ToLowerInvariant().Contains(lowerQuery));
        }
        
        private bool MatchesFilter(ApiDocItem item, ApiSearchFilter filter)
        {
            if (filter == null) return true;
            
            if (filter.Types != null && filter.Types.Any() && !filter.Types.Contains(item.Type))
                return false;
            
            if (filter.Categories != null && filter.Categories.Any() && !filter.Categories.Contains(item.Category))
                return false;
            
            if (filter.Tags != null && filter.Tags.Any() && !filter.Tags.Any(tag => item.Tags.Contains(tag)))
                return false;
            
            return true;
        }
        
        private double CalculateRelevanceScore(ApiDocItem item, string query)
        {
            var score = 0.0;
            var lowerQuery = query.ToLowerInvariant();
            var lowerName = item.Name.ToLowerInvariant();
            
            // Exact name match gets highest score
            if (lowerName == lowerQuery)
                score += 100;
            else if (lowerName.StartsWith(lowerQuery))
                score += 80;
            else if (lowerName.Contains(lowerQuery))
                score += 60;
            
            // Description match
            if (item.Description.ToLowerInvariant().Contains(lowerQuery))
                score += 20;
            
            // Tag match
            foreach (var tag in item.Tags)
            {
                if (tag.ToLowerInvariant().Contains(lowerQuery))
                    score += 10;
            }
            
            return score;
        }
        
        // Helper methods for parsing API data
        private string GetClassDescription(string className) => $"Roblox API class: {className}";
        private string GetMemberDescription(string className, string memberName, string memberType) => 
            $"{memberType} member of {className}";
        private string GetEnumDescription(string enumName) => $"Roblox enumeration: {enumName}";
        
        private ApiItemType GetApiItemType(string memberType)
        {
            return memberType switch
            {
                "Property" => ApiItemType.Property,
                "Function" => ApiItemType.Function,
                "Event" => ApiItemType.Event,
                "Callback" => ApiItemType.Callback,
                _ => ApiItemType.Property
            };
        }
        
        private string ParseValueType(JsonElement valueType)
        {
            // Simplified value type parsing
            return valueType.GetString() ?? "any";
        }
        
        private List<ApiParameter> ParseParameters(JsonElement parameters)
        {
            var paramList = new List<ApiParameter>();
            
            foreach (var param in parameters.EnumerateArray())
            {
                if (param.TryGetProperty("Name", out var name) && 
                    param.TryGetProperty("Type", out var type))
                {
                    paramList.Add(new ApiParameter
                    {
                        Name = name.GetString(),
                        Type = ParseValueType(type),
                        Description = "",
                        IsOptional = param.TryGetProperty("Default", out _)
                    });
                }
            }
            
            return paramList;
        }
        
        private string GenerateMemberSyntax(string className, string memberName, string memberType, string returnType)
        {
            return memberType switch
            {
                "Property" => $"{className}.{memberName}: {returnType}",
                "Function" => $"{className}:{memberName}(): {returnType}",
                "Event" => $"{className}.{memberName}: RBXScriptSignal",
                _ => $"{className}.{memberName}"
            };
        }
        
        private string GenerateFunctionSyntax(string className, string memberName, List<ApiParameter> parameters, string returnType)
        {
            var paramString = string.Join(", ", parameters.Select(p => 
                p.IsOptional ? $"{p.Name}?: {p.Type}" : $"{p.Name}: {p.Type}"));
            
            return $"{className}:{memberName}({paramString}): {returnType}";
        }
        
        #endregion
        
        public void Dispose()
        {
            _httpClient?.Dispose();
        }
    }
    
    #region Supporting Classes
    
    public class ApiSearchFilter
    {
        public List<ApiItemType> Types { get; set; }
        public List<string> Categories { get; set; }
        public List<string> Tags { get; set; }
    }
    
    #endregion
}