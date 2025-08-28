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
    public class MarketplaceService
    {
        private readonly HttpClient _httpClient;
        private readonly string _cacheDirectory;
        private readonly string _marketplaceUrl;
        
        public ObservableCollection<MarketplaceCategory> Categories { get; }
        public ObservableCollection<MarketplaceItem> FeaturedItems { get; }
        public ObservableCollection<MarketplaceItem> PopularItems { get; }
        public ObservableCollection<MarketplaceItem> RecentItems { get; }
        public ObservableCollection<MarketplaceItem> SearchResults { get; }
        public ObservableCollection<MarketplaceItem> InstalledItems { get; }
        
        private readonly Dictionary<string, List<MarketplaceItem>> _categoryCache;
        
        public MarketplaceService()
        {
            _httpClient = new HttpClient();
            _cacheDirectory = Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
                "Aether", "Marketplace");
            
            Directory.CreateDirectory(_cacheDirectory);
            
            _marketplaceUrl = "https://api.aether-marketplace.com"; // Hypothetical marketplace API
            
            Categories = new ObservableCollection<MarketplaceCategory>();
            FeaturedItems = new ObservableCollection<MarketplaceItem>();
            PopularItems = new ObservableCollection<MarketplaceItem>();
            RecentItems = new ObservableCollection<MarketplaceItem>();
            SearchResults = new ObservableCollection<MarketplaceItem>();
            InstalledItems = new ObservableCollection<MarketplaceItem>();
            
            _categoryCache = new Dictionary<string, List<MarketplaceItem>>();
            
            LoadOfflineData();
        }
        
        #region Public Methods
        
        public async Task InitializeAsync()
        {
            try
            {
                await LoadCategoriesAsync();
                await LoadFeaturedItemsAsync();
                await LoadPopularItemsAsync();
                await LoadRecentItemsAsync();
                LoadInstalledItems();
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Failed to initialize marketplace: {ex.Message}");
                LoadOfflineData();
            }
        }
        
        public async Task<List<MarketplaceItem>> SearchAsync(string query, MarketplaceSearchFilter filter = null)
        {
            try
            {
                // In a real implementation, this would call the marketplace API
                var results = await SearchOfflineAsync(query, filter);
                
                SearchResults.Clear();
                foreach (var item in results.Take(50)) // Limit results
                {
                    SearchResults.Add(item);
                }
                
                return results;
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Search failed: {ex.Message}");
                return new List<MarketplaceItem>();
            }
        }
        
        public async Task<MarketplaceItem> GetItemDetailsAsync(string itemId)
        {
            try
            {
                // Check cache first
                var cachedItem = FindItemInCache(itemId);
                if (cachedItem != null)
                {
                    return cachedItem;
                }
                
                // In a real implementation, fetch from API
                var response = await _httpClient.GetStringAsync($"{_marketplaceUrl}/items/{itemId}");
                var item = JsonSerializer.Deserialize<MarketplaceItem>(response);
                
                return item;
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Failed to get item details: {ex.Message}");
                return null;
            }
        }
        
        public async Task<bool> InstallItemAsync(MarketplaceItem item)
        {
            try
            {
                // Download the item
                var downloadPath = await DownloadItemAsync(item);
                if (downloadPath == null)
                {
                    return false;
                }
                
                // Install based on item type
                var installSuccess = await InstallItemByTypeAsync(item, downloadPath);
                
                if (installSuccess)
                {
                    item.IsInstalled = true;
                    
                    if (!InstalledItems.Any(i => i.Id == item.Id))
                    {
                        InstalledItems.Add(item);
                    }
                    
                    await SaveInstalledItemsAsync();
                    await TrackInstallationAsync(item);
                }
                
                return installSuccess;
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Installation failed: {ex.Message}");
                return false;
            }
        }
        
        public async Task<bool> UninstallItemAsync(MarketplaceItem item)
        {
            try
            {
                var uninstallSuccess = await UninstallItemByTypeAsync(item);
                
                if (uninstallSuccess)
                {
                    item.IsInstalled = false;
                    
                    var installedItem = InstalledItems.FirstOrDefault(i => i.Id == item.Id);
                    if (installedItem != null)
                    {
                        InstalledItems.Remove(installedItem);
                    }
                    
                    await SaveInstalledItemsAsync();
                }
                
                return uninstallSuccess;
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Uninstallation failed: {ex.Message}");
                return false;
            }
        }
        
        public async Task<List<UserReview>> GetItemReviewsAsync(string itemId, int page = 1, int pageSize = 20)
        {
            try
            {
                // In a real implementation, this would call the marketplace API
                return GenerateSampleReviews(itemId, page, pageSize);
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Failed to get reviews: {ex.Message}");
                return new List<UserReview>();
            }
        }
        
        public async Task<bool> SubmitReviewAsync(string itemId, float rating, string reviewText)
        {
            try
            {
                var review = new
                {
                    ItemId = itemId,
                    Rating = rating,
                    ReviewText = reviewText,
                    UserId = Environment.UserName,
                    CreatedAt = DateTime.UtcNow
                };
                
                var json = JsonSerializer.Serialize(review);
                var content = new StringContent(json, System.Text.Encoding.UTF8, "application/json");
                
                var response = await _httpClient.PostAsync($"{_marketplaceUrl}/items/{itemId}/reviews", content);
                return response.IsSuccessStatusCode;
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Failed to submit review: {ex.Message}");
                return false;
            }
        }
        
        public async Task<List<MarketplaceItem>> GetItemsByCategoryAsync(string categoryName)
        {
            if (_categoryCache.TryGetValue(categoryName, out var cachedItems))
            {
                return cachedItems;
            }
            
            try
            {
                // In a real implementation, fetch from API
                var items = GenerateSampleItemsForCategory(categoryName);
                _categoryCache[categoryName] = items;
                return items;
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Failed to get category items: {ex.Message}");
                return new List<MarketplaceItem>();
            }
        }
        
        public async Task<bool> PublishItemAsync(MarketplaceItem item, string filePath)
        {
            try
            {
                // In a real implementation, this would upload the item to the marketplace
                // For now, just simulate the process
                
                // Validate item
                if (!ValidateItemForPublishing(item))
                {
                    return false;
                }
                
                // Upload file
                var uploadSuccess = await UploadItemFileAsync(filePath, item.Id);
                if (!uploadSuccess)
                {
                    return false;
                }
                
                // Submit metadata
                var metadata = JsonSerializer.Serialize(item);
                var content = new StringContent(metadata, System.Text.Encoding.UTF8, "application/json");
                
                var response = await _httpClient.PostAsync($"{_marketplaceUrl}/items", content);
                return response.IsSuccessStatusCode;
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Failed to publish item: {ex.Message}");
                return false;
            }
        }
        
        public List<MarketplaceItem> GetRecommendationsFor(MarketplaceItem item)
        {
            // Simple recommendation algorithm based on tags and category
            var allItems = GetAllCachedItems();
            
            var recommendations = allItems
                .Where(i => i.Id != item.Id)
                .Where(i => i.Tags.Any(tag => item.Tags.Contains(tag)) || i.Type == item.Type)
                .OrderByDescending(i => CalculateSimilarityScore(item, i))
                .Take(5)
                .ToList();
            
            return recommendations;
        }
        
        public async Task<Dictionary<string, object>> GetMarketplaceStatsAsync()
        {
            try
            {
                var stats = new Dictionary<string, object>
                {
                    ["TotalItems"] = GetAllCachedItems().Count,
                    ["TotalCategories"] = Categories.Count,
                    ["TotalDownloads"] = GetAllCachedItems().Sum(i => i.Downloads),
                    ["AverageRating"] = GetAllCachedItems().Where(i => i.RatingCount > 0).Average(i => i.Rating),
                    ["PopularCategory"] = Categories.OrderByDescending(c => c.ItemCount).FirstOrDefault()?.Name,
                    ["NewItemsThisWeek"] = GetAllCachedItems().Count(i => (DateTime.UtcNow - i.CreatedAt).TotalDays <= 7),
                    ["FreeItemsPercentage"] = GetAllCachedItems().Count(i => i.IsFree) * 100.0 / GetAllCachedItems().Count
                };
                
                return stats;
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Failed to get marketplace stats: {ex.Message}");
                return new Dictionary<string, object>();
            }
        }
        
        #endregion
        
        #region Private Methods
        
        private void LoadOfflineData()
        {
            // Load sample data for offline mode
            LoadSampleCategories();
            LoadSampleItems();
        }
        
        private void LoadSampleCategories()
        {
            var sampleCategories = new List<MarketplaceCategory>
            {
                new MarketplaceCategory { Name = "Plugins", Description = "Editor plugins and extensions", Icon = "🔌", ItemCount = 25, IsPopular = true },
                new MarketplaceCategory { Name = "Themes", Description = "Editor themes and color schemes", Icon = "🎨", ItemCount = 15, IsPopular = true },
                new MarketplaceCategory { Name = "Script Templates", Description = "Ready-to-use script templates", Icon = "📝", ItemCount = 50, IsPopular = true },
                new MarketplaceCategory { Name = "Tools", Description = "Development tools and utilities", Icon = "🔧", ItemCount = 20, IsPopular = false },
                new MarketplaceCategory { Name = "Libraries", Description = "Code libraries and frameworks", Icon = "📚", ItemCount = 30, IsPopular = false },
                new MarketplaceCategory { Name = "Educational", Description = "Learning resources and tutorials", Icon = "🎓", ItemCount = 40, IsPopular = false }
            };
            
            Categories.Clear();
            foreach (var category in sampleCategories)
            {
                Categories.Add(category);
            }
        }
        
        private void LoadSampleItems()
        {
            var sampleItems = new List<MarketplaceItem>
            {
                new MarketplaceItem
                {
                    Id = "1",
                    Name = "Advanced Code Formatter",
                    Description = "Automatically format your Luau code with customizable style options",
                    Type = MarketplaceItemType.Plugin,
                    Author = "DevTools Team",
                    Version = "2.1.0",
                    Price = 0,
                    Tags = new List<string> { "formatting", "code-style", "productivity" },
                    Rating = 4.8f,
                    RatingCount = 156,
                    Downloads = 2340,
                    CreatedAt = DateTime.UtcNow.AddDays(-30),
                    UpdatedAt = DateTime.UtcNow.AddDays(-5),
                    ThumbnailUrl = "https://example.com/formatter-thumb.png"
                },
                
                new MarketplaceItem
                {
                    Id = "2", 
                    Name = "Dark Pro Theme",
                    Description = "Professional dark theme with syntax highlighting optimized for Luau",
                    Type = MarketplaceItemType.Theme,
                    Author = "ThemeStudio",
                    Version = "1.5.2",
                    Price = 2.99m,
                    Tags = new List<string> { "dark", "professional", "syntax-highlighting" },
                    Rating = 4.9f,
                    RatingCount = 89,
                    Downloads = 1890,
                    CreatedAt = DateTime.UtcNow.AddDays(-60),
                    UpdatedAt = DateTime.UtcNow.AddDays(-10)
                },
                
                new MarketplaceItem
                {
                    Id = "3",
                    Name = "Game Framework Template",
                    Description = "Complete game framework with player management, data persistence, and more",
                    Type = MarketplaceItemType.ScriptTemplate,
                    Author = "GameDev Pro",
                    Version = "3.0.1",
                    Price = 9.99m,
                    Tags = new List<string> { "framework", "game", "template", "advanced" },
                    Rating = 4.7f,
                    RatingCount = 234,
                    Downloads = 3450,
                    CreatedAt = DateTime.UtcNow.AddDays(-90),
                    UpdatedAt = DateTime.UtcNow.AddDays(-2)
                }
            };
            
            FeaturedItems.Clear();
            PopularItems.Clear();
            RecentItems.Clear();
            
            foreach (var item in sampleItems)
            {
                FeaturedItems.Add(item);
                if (item.Downloads > 2000) PopularItems.Add(item);
                if ((DateTime.UtcNow - item.CreatedAt).TotalDays <= 30) RecentItems.Add(item);
            }
        }
        
        private async Task LoadCategoriesAsync()
        {
            try
            {
                // In a real implementation, fetch from API
                LoadSampleCategories();
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Failed to load categories: {ex.Message}");
            }
        }
        
        private async Task LoadFeaturedItemsAsync()
        {
            try
            {
                // In a real implementation, fetch from API
                LoadSampleItems();
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Failed to load featured items: {ex.Message}");
            }
        }
        
        private async Task LoadPopularItemsAsync()
        {
            // Already loaded in LoadSampleItems()
            await Task.CompletedTask;
        }
        
        private async Task LoadRecentItemsAsync()
        {
            // Already loaded in LoadSampleItems()
            await Task.CompletedTask;
        }
        
        private void LoadInstalledItems()
        {
            var installedFile = Path.Combine(_cacheDirectory, "installed.json");
            
            if (File.Exists(installedFile))
            {
                try
                {
                    var json = File.ReadAllText(installedFile);
                    var installedIds = JsonSerializer.Deserialize<List<string>>(json);
                    
                    if (installedIds != null)
                    {
                        foreach (var id in installedIds)
                        {
                            var item = FindItemInCache(id);
                            if (item != null)
                            {
                                item.IsInstalled = true;
                                InstalledItems.Add(item);
                            }
                        }
                    }
                }
                catch (Exception ex)
                {
                    System.Diagnostics.Debug.WriteLine($"Failed to load installed items: {ex.Message}");
                }
            }
        }
        
        private async Task<List<MarketplaceItem>> SearchOfflineAsync(string query, MarketplaceSearchFilter filter)
        {
            var allItems = GetAllCachedItems();
            
            var results = allItems.Where(item =>
                MatchesSearchQuery(item, query) &&
                MatchesFilter(item, filter)
            ).ToList();
            
            // Apply sorting
            if (filter?.SortBy != null)
            {
                results = ApplySorting(results, filter.SortBy, filter.SortDescending);
            }
            
            return results;
        }
        
        private bool MatchesSearchQuery(MarketplaceItem item, string query)
        {
            if (string.IsNullOrWhiteSpace(query))
                return true;
            
            var lowerQuery = query.ToLowerInvariant();
            
            return item.Name.ToLowerInvariant().Contains(lowerQuery) ||
                   item.Description.ToLowerInvariant().Contains(lowerQuery) ||
                   item.Author.ToLowerInvariant().Contains(lowerQuery) ||
                   item.Tags.Any(tag => tag.ToLowerInvariant().Contains(lowerQuery));
        }
        
        private bool MatchesFilter(MarketplaceItem item, MarketplaceSearchFilter filter)
        {
            if (filter == null) return true;
            
            if (filter.Types.Any() && !filter.Types.Contains(item.Type))
                return false;
            
            if (filter.MinPrice.HasValue && item.Price < filter.MinPrice)
                return false;
            
            if (filter.MaxPrice.HasValue && item.Price > filter.MaxPrice)
                return false;
            
            if (filter.MinRating.HasValue && item.Rating < filter.MinRating)
                return false;
            
            if (filter.IsFree.HasValue && item.IsFree != filter.IsFree)
                return false;
            
            if (filter.IsInstalled.HasValue && item.IsInstalled != filter.IsInstalled)
                return false;
            
            if (filter.CreatedAfter.HasValue && item.CreatedAt < filter.CreatedAfter)
                return false;
            
            if (filter.UpdatedAfter.HasValue && item.UpdatedAt < filter.UpdatedAfter)
                return false;
            
            return true;
        }
        
        private List<MarketplaceItem> ApplySorting(List<MarketplaceItem> items, MarketplaceSortBy sortBy, bool descending)
        {
            var sorted = sortBy switch
            {
                MarketplaceSortBy.Name => items.OrderBy(i => i.Name),
                MarketplaceSortBy.Price => items.OrderBy(i => i.Price),
                MarketplaceSortBy.Rating => items.OrderBy(i => i.Rating),
                MarketplaceSortBy.Downloads => items.OrderBy(i => i.Downloads),
                MarketplaceSortBy.CreatedDate => items.OrderBy(i => i.CreatedAt),
                MarketplaceSortBy.UpdatedDate => items.OrderBy(i => i.UpdatedAt),
                _ => items.OrderBy(i => i.Name)
            };
            
            return descending ? sorted.Reverse().ToList() : sorted.ToList();
        }
        
        private List<MarketplaceItem> GetAllCachedItems()
        {
            var allItems = new List<MarketplaceItem>();
            allItems.AddRange(FeaturedItems);
            allItems.AddRange(PopularItems);
            allItems.AddRange(RecentItems);
            
            foreach (var categoryItems in _categoryCache.Values)
            {
                allItems.AddRange(categoryItems);
            }
            
            return allItems.GroupBy(i => i.Id).Select(g => g.First()).ToList();
        }
        
        private MarketplaceItem FindItemInCache(string itemId)
        {
            var allItems = GetAllCachedItems();
            return allItems.FirstOrDefault(i => i.Id == itemId);
        }
        
        private async Task<string> DownloadItemAsync(MarketplaceItem item)
        {
            try
            {
                if (string.IsNullOrEmpty(item.DownloadUrl))
                {
                    // Generate download URL
                    item.DownloadUrl = $"{_marketplaceUrl}/download/{item.Id}";
                }
                
                var downloadPath = Path.Combine(_cacheDirectory, "downloads", $"{item.Id}.zip");
                Directory.CreateDirectory(Path.GetDirectoryName(downloadPath));
                
                // In a real implementation, download the actual file
                // For now, create a placeholder file
                await File.WriteAllTextAsync(downloadPath, $"Placeholder download for {item.Name}");
                
                return downloadPath;
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Download failed: {ex.Message}");
                return null;
            }
        }
        
        private async Task<bool> InstallItemByTypeAsync(MarketplaceItem item, string downloadPath)
        {
            try
            {
                var installPath = GetInstallPathForItem(item);
                Directory.CreateDirectory(installPath);
                
                switch (item.Type)
                {
                    case MarketplaceItemType.Plugin:
                        return await InstallPluginAsync(item, downloadPath, installPath);
                    
                    case MarketplaceItemType.Theme:
                        return await InstallThemeAsync(item, downloadPath, installPath);
                    
                    case MarketplaceItemType.ScriptTemplate:
                        return await InstallTemplateAsync(item, downloadPath, installPath);
                    
                    default:
                        return await InstallGenericItemAsync(item, downloadPath, installPath);
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Installation failed: {ex.Message}");
                return false;
            }
        }
        
        private async Task<bool> UninstallItemByTypeAsync(MarketplaceItem item)
        {
            try
            {
                var installPath = GetInstallPathForItem(item);
                
                if (Directory.Exists(installPath))
                {
                    Directory.Delete(installPath, true);
                }
                
                return true;
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Uninstallation failed: {ex.Message}");
                return false;
            }
        }
        
        private string GetInstallPathForItem(MarketplaceItem item)
        {
            var basePath = Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
                "Aether");
            
            return item.Type switch
            {
                MarketplaceItemType.Plugin => Path.Combine(basePath, "Plugins", item.Id),
                MarketplaceItemType.Theme => Path.Combine(basePath, "Themes", item.Id),
                MarketplaceItemType.ScriptTemplate => Path.Combine(basePath, "Templates", item.Id),
                _ => Path.Combine(basePath, "Extensions", item.Id)
            };
        }
        
        private async Task<bool> InstallPluginAsync(MarketplaceItem item, string downloadPath, string installPath)
        {
            // Extract and install plugin
            // In a real implementation, extract ZIP and copy files
            await Task.Delay(100); // Simulate installation time
            return true;
        }
        
        private async Task<bool> InstallThemeAsync(MarketplaceItem item, string downloadPath, string installPath)
        {
            // Extract and install theme
            await Task.Delay(50);
            return true;
        }
        
        private async Task<bool> InstallTemplateAsync(MarketplaceItem item, string downloadPath, string installPath)
        {
            // Extract and install template
            await Task.Delay(30);
            return true;
        }
        
        private async Task<bool> InstallGenericItemAsync(MarketplaceItem item, string downloadPath, string installPath)
        {
            // Generic installation
            await Task.Delay(20);
            return true;
        }
        
        private async Task SaveInstalledItemsAsync()
        {
            try
            {
                var installedIds = InstalledItems.Select(i => i.Id).ToList();
                var json = JsonSerializer.Serialize(installedIds, new JsonSerializerOptions
                {
                    WriteIndented = true
                });
                
                var installedFile = Path.Combine(_cacheDirectory, "installed.json");
                await File.WriteAllTextAsync(installedFile, json);
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Failed to save installed items: {ex.Message}");
            }
        }
        
        private async Task TrackInstallationAsync(MarketplaceItem item)
        {
            try
            {
                // Track installation analytics
                var installData = new
                {
                    ItemId = item.Id,
                    ItemName = item.Name,
                    ItemType = item.Type.ToString(),
                    InstallDate = DateTime.UtcNow,
                    UserId = Environment.UserName
                };
                
                // In a real implementation, send to analytics service
                System.Diagnostics.Debug.WriteLine($"Tracked installation: {item.Name}");
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Failed to track installation: {ex.Message}");
            }
        }
        
        private bool ValidateItemForPublishing(MarketplaceItem item)
        {
            if (string.IsNullOrWhiteSpace(item.Name)) return false;
            if (string.IsNullOrWhiteSpace(item.Description)) return false;
            if (string.IsNullOrWhiteSpace(item.Author)) return false;
            if (string.IsNullOrWhiteSpace(item.Version)) return false;
            
            return true;
        }
        
        private async Task<bool> UploadItemFileAsync(string filePath, string itemId)
        {
            try
            {
                // In a real implementation, upload file to marketplace
                await Task.Delay(1000); // Simulate upload time
                return true;
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Upload failed: {ex.Message}");
                return false;
            }
        }
        
        private List<MarketplaceItem> GenerateSampleItemsForCategory(string categoryName)
        {
            // Generate sample items for the category
            return new List<MarketplaceItem>();
        }
        
        private List<UserReview> GenerateSampleReviews(string itemId, int page, int pageSize)
        {
            // Generate sample reviews for testing
            var reviews = new List<UserReview>
            {
                new UserReview
                {
                    Id = "1",
                    UserName = "Developer123",
                    Rating = 5.0f,
                    ReviewText = "Excellent plugin! Works perfectly and saves me tons of time.",
                    CreatedAt = DateTime.UtcNow.AddDays(-10),
                    IsVerifiedPurchase = true,
                    HelpfulVotes = 15,
                    TotalVotes = 18
                },
                
                new UserReview
                {
                    Id = "2",
                    UserName = "CodeMaster",
                    Rating = 4.0f,
                    ReviewText = "Good functionality, but could use better documentation.",
                    CreatedAt = DateTime.UtcNow.AddDays(-5),
                    IsVerifiedPurchase = true,
                    HelpfulVotes = 8,
                    TotalVotes = 10
                }
            };
            
            return reviews.Skip((page - 1) * pageSize).Take(pageSize).ToList();
        }
        
        private double CalculateSimilarityScore(MarketplaceItem item1, MarketplaceItem item2)
        {
            var score = 0.0;
            
            // Same type
            if (item1.Type == item2.Type) score += 3.0;
            
            // Common tags
            var commonTags = item1.Tags.Intersect(item2.Tags).Count();
            score += commonTags * 2.0;
            
            // Similar rating
            var ratingDiff = Math.Abs(item1.Rating - item2.Rating);
            score += Math.Max(0, 2.0 - ratingDiff);
            
            return score;
        }
        
        #endregion
        
        public void Dispose()
        {
            _httpClient?.Dispose();
        }
    }
}