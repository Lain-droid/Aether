using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Text.Json;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using AetherVisor.Frontend.Models;

namespace AetherVisor.Frontend.Services
{
    public class LuauIntelliSense
    {
        private List<IntelliSenseItem> _apiItems = new();
        private List<IntelliSenseItem> _keywords = new();
        private List<IntelliSenseItem> _snippets = new();
        private readonly HttpClient _httpClient = new();

        public LuauIntelliSense()
        {
            InitializeKeywords();
            InitializeSnippets();
        }

        public async Task LoadApiDefinitionsAsync()
        {
            try
            {
                // Load Roblox API from local file or web
                var apiData = await LoadRobloxApiAsync();
                _apiItems = ParseApiData(apiData);
            }
            catch (Exception ex)
            {
                // Fallback to basic API
                LoadBasicApi();
                System.Diagnostics.Debug.WriteLine($"Failed to load API: {ex.Message}");
            }
        }

        public async Task<List<IntelliSenseItem>> GetSuggestionsAsync(string prefix, string context)
        {
            var suggestions = new List<IntelliSenseItem>();

            // Add keywords
            suggestions.AddRange(_keywords.Where(k => 
                k.Name.StartsWith(prefix, StringComparison.OrdinalIgnoreCase)));

            // Add API items
            suggestions.AddRange(_apiItems.Where(api => 
                api.Name.StartsWith(prefix, StringComparison.OrdinalIgnoreCase)));

            // Add context-specific suggestions
            var contextSuggestions = await GetContextualSuggestionsAsync(prefix, context);
            suggestions.AddRange(contextSuggestions);

            // Add snippets
            suggestions.AddRange(_snippets.Where(s => 
                s.Name.StartsWith(prefix, StringComparison.OrdinalIgnoreCase)));

            // Remove duplicates and sort by priority
            return suggestions
                .GroupBy(s => s.Name)
                .Select(g => g.OrderByDescending(s => s.Priority).First())
                .OrderByDescending(s => s.Priority)
                .ThenBy(s => s.Name)
                .Take(20)
                .ToList();
        }

        private void InitializeKeywords()
        {
            var luauKeywords = new[]
            {
                "and", "break", "continue", "do", "else", "elseif", "end", "false", "for",
                "function", "if", "in", "local", "nil", "not", "or", "repeat", "return",
                "then", "true", "until", "while", "export", "type"
            };

            _keywords = luauKeywords.Select(keyword => new IntelliSenseItem
            {
                Name = keyword,
                ItemType = IntelliSenseItemType.Keyword,
                Description = $"Luau keyword: {keyword}",
                Documentation = GetKeywordDocumentation(keyword),
                Priority = 100
            }).ToList();
        }

        private void InitializeSnippets()
        {
            _snippets = new List<IntelliSenseItem>
            {
                new()
                {
                    Name = "for",
                    CompletionText = "for i = 1, 10 do\n    \nend",
                    ItemType = IntelliSenseItemType.Snippet,
                    Description = "For loop with numeric range",
                    Documentation = "Creates a numeric for loop",
                    Priority = 90
                },
                new()
                {
                    Name = "forp",
                    CompletionText = "for key, value in pairs() do\n    \nend",
                    ItemType = IntelliSenseItemType.Snippet,
                    Description = "For loop with pairs",
                    Documentation = "Iterates over key-value pairs",
                    Priority = 90
                },
                new()
                {
                    Name = "fori",
                    CompletionText = "for index, value in ipairs() do\n    \nend",
                    ItemType = IntelliSenseItemType.Snippet,
                    Description = "For loop with ipairs",
                    Documentation = "Iterates over indexed values",
                    Priority = 90
                },
                new()
                {
                    Name = "func",
                    CompletionText = "local function name()\n    \nend",
                    ItemType = IntelliSenseItemType.Snippet,
                    Description = "Local function declaration",
                    Documentation = "Creates a local function",
                    Priority = 95
                },
                new()
                {
                    Name = "if",
                    CompletionText = "if condition then\n    \nend",
                    ItemType = IntelliSenseItemType.Snippet,
                    Description = "If statement",
                    Documentation = "Conditional statement",
                    Priority = 95
                },
                new()
                {
                    Name = "while",
                    CompletionText = "while condition do\n    \nend",
                    ItemType = IntelliSenseItemType.Snippet,
                    Description = "While loop",
                    Documentation = "Creates a while loop",
                    Priority = 90
                }
            };
        }

        private async Task<string> LoadRobloxApiAsync()
        {
            // Try to load from local cache first
            var cacheFile = Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
                "Aether", "roblox-api.json");

            if (File.Exists(cacheFile))
            {
                var cacheInfo = new FileInfo(cacheFile);
                if (DateTime.Now - cacheInfo.LastWriteTime < TimeSpan.FromDays(7))
                {
                    return await File.ReadAllTextAsync(cacheFile);
                }
            }

            // Download latest API
            try
            {
                var response = await _httpClient.GetStringAsync(
                    "https://raw.githubusercontent.com/CloneTrooper1019/Roblox-Client-Tracker/roblox/API-Dump.json");

                // Cache the response
                Directory.CreateDirectory(Path.GetDirectoryName(cacheFile));
                await File.WriteAllTextAsync(cacheFile, response);

                return response;
            }
            catch
            {
                // If download fails, return cached version or empty
                return File.Exists(cacheFile) ? await File.ReadAllTextAsync(cacheFile) : "{}";
            }
        }

        private List<IntelliSenseItem> ParseApiData(string jsonData)
        {
            var items = new List<IntelliSenseItem>();

            try
            {
                using var document = JsonDocument.Parse(jsonData);
                var root = document.RootElement;

                if (root.TryGetProperty("Classes", out var classes))
                {
                    foreach (var classElement in classes.EnumerateArray())
                    {
                        ParseClass(classElement, items);
                    }
                }

                if (root.TryGetProperty("Enums", out var enums))
                {
                    foreach (var enumElement in enums.EnumerateArray())
                    {
                        ParseEnum(enumElement, items);
                    }
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Failed to parse API data: {ex.Message}");
            }

            return items;
        }

        private void ParseClass(JsonElement classElement, List<IntelliSenseItem> items)
        {
            if (!classElement.TryGetProperty("Name", out var nameElement)) return;
            var className = nameElement.GetString();

            // Add the class itself
            items.Add(new IntelliSenseItem
            {
                Name = className,
                ItemType = IntelliSenseItemType.Class,
                Description = $"Roblox class: {className}",
                Documentation = GetClassDocumentation(className),
                Category = "Classes",
                Priority = 80
            });

            // Parse members
            if (classElement.TryGetProperty("Members", out var members))
            {
                foreach (var member in members.EnumerateArray())
                {
                    ParseClassMember(member, className, items);
                }
            }
        }

        private void ParseClassMember(JsonElement member, string className, List<IntelliSenseItem> items)
        {
            if (!member.TryGetProperty("Name", out var nameElement)) return;
            var memberName = nameElement.GetString();

            if (!member.TryGetProperty("MemberType", out var typeElement)) return;
            var memberType = typeElement.GetString();

            var itemType = memberType switch
            {
                "Property" => IntelliSenseItemType.Property,
                "Function" => IntelliSenseItemType.Function,
                "Event" => IntelliSenseItemType.Event,
                _ => IntelliSenseItemType.Property
            };

            var description = $"{className}.{memberName}";
            if (member.TryGetProperty("ValueType", out var valueType))
            {
                description += $" : {valueType.GetString()}";
            }

            items.Add(new IntelliSenseItem
            {
                Name = $"{className}.{memberName}",
                ItemType = itemType,
                Description = description,
                Documentation = GetMemberDocumentation(className, memberName),
                Category = className,
                Priority = 70
            });

            // Also add just the member name for context-based completion
            items.Add(new IntelliSenseItem
            {
                Name = memberName,
                ItemType = itemType,
                Description = description,
                Documentation = GetMemberDocumentation(className, memberName),
                Category = className,
                Priority = 60
            });
        }

        private void ParseEnum(JsonElement enumElement, List<IntelliSenseItem> items)
        {
            if (!enumElement.TryGetProperty("Name", out var nameElement)) return;
            var enumName = nameElement.GetString();

            items.Add(new IntelliSenseItem
            {
                Name = enumName,
                ItemType = IntelliSenseItemType.Enum,
                Description = $"Roblox enum: {enumName}",
                Documentation = GetEnumDocumentation(enumName),
                Category = "Enums",
                Priority = 75
            });

            if (enumElement.TryGetProperty("Items", out var items_prop))
            {
                foreach (var item in items_prop.EnumerateArray())
                {
                    if (item.TryGetProperty("Name", out var itemName))
                    {
                        items.Add(new IntelliSenseItem
                        {
                            Name = $"Enum.{enumName}.{itemName.GetString()}",
                            ItemType = IntelliSenseItemType.Property,
                            Description = $"Enum value: {enumName}.{itemName.GetString()}",
                            Documentation = $"Enum value of {enumName}",
                            Category = enumName,
                            Priority = 65
                        });
                    }
                }
            }
        }

        private async Task<List<IntelliSenseItem>> GetContextualSuggestionsAsync(string prefix, string context)
        {
            var suggestions = new List<IntelliSenseItem>();

            // Analyze context for smart suggestions
            if (context.Contains("game:GetService("))
            {
                suggestions.AddRange(GetServiceSuggestions(prefix));
            }
            else if (context.Contains("workspace.") || context.Contains("game.Workspace."))
            {
                suggestions.AddRange(GetWorkspaceSuggestions(prefix));
            }
            else if (Regex.IsMatch(context, @"\w+\."))
            {
                var objectName = Regex.Match(context, @"(\w+)\.").Groups[1].Value;
                suggestions.AddRange(GetObjectMemberSuggestions(objectName, prefix));
            }

            return suggestions;
        }

        private List<IntelliSenseItem> GetServiceSuggestions(string prefix)
        {
            var services = new[]
            {
                "Players", "Workspace", "Lighting", "ReplicatedStorage", "ServerStorage",
                "SoundService", "TweenService", "UserInputService", "RunService",
                "ContextActionService", "GuiService", "MarketplaceService", "DataStoreService",
                "HttpService", "PathfindingService", "TextService", "LocalizationService"
            };

            return services
                .Where(s => s.StartsWith(prefix, StringComparison.OrdinalIgnoreCase))
                .Select(service => new IntelliSenseItem
                {
                    Name = $'"{service}"',
                    CompletionText = $'"{service}"',
                    ItemType = IntelliSenseItemType.Service,
                    Description = $"Roblox service: {service}",
                    Documentation = GetServiceDocumentation(service),
                    Priority = 85
                })
                .ToList();
        }

        private List<IntelliSenseItem> GetWorkspaceSuggestions(string prefix)
        {
            // Common workspace members
            var members = new[]
            {
                "CurrentCamera", "Gravity", "FallenPartsDestroyHeight",
                "FindFirstChild", "WaitForChild", "GetChildren", "GetDescendants"
            };

            return members
                .Where(m => m.StartsWith(prefix, StringComparison.OrdinalIgnoreCase))
                .Select(member => new IntelliSenseItem
                {
                    Name = member,
                    ItemType = member.EndsWith("Child") || member.StartsWith("Get") ? 
                        IntelliSenseItemType.Function : IntelliSenseItemType.Property,
                    Description = $"Workspace.{member}",
                    Documentation = GetMemberDocumentation("Workspace", member),
                    Priority = 75
                })
                .ToList();
        }

        private List<IntelliSenseItem> GetObjectMemberSuggestions(string objectName, string prefix)
        {
            // Return members relevant to the object type
            return _apiItems
                .Where(item => item.Category == objectName && 
                              item.Name.StartsWith(prefix, StringComparison.OrdinalIgnoreCase))
                .ToList();
        }

        private void LoadBasicApi()
        {
            _apiItems = new List<IntelliSenseItem>
            {
                new()
                {
                    Name = "game",
                    ItemType = IntelliSenseItemType.Property,
                    Description = "The top-level DataModel",
                    Documentation = "The root object that contains all other objects in the game",
                    Priority = 100
                },
                new()
                {
                    Name = "workspace",
                    ItemType = IntelliSenseItemType.Property,
                    Description = "Alias for game.Workspace",
                    Documentation = "The physical environment of the game",
                    Priority = 100
                },
                new()
                {
                    Name = "script",
                    ItemType = IntelliSenseItemType.Property,
                    Description = "Reference to the current script",
                    Documentation = "The script object that contains this code",
                    Priority = 95
                }
            };
        }

        private string GetKeywordDocumentation(string keyword)
        {
            return keyword switch
            {
                "local" => "Declares a variable with local scope",
                "function" => "Declares a function",
                "if" => "Conditional statement",
                "for" => "Loop statement",
                "while" => "Loop statement with condition",
                "return" => "Returns a value from function",
                "break" => "Exits from a loop",
                "continue" => "Skips to next iteration",
                _ => $"Luau keyword: {keyword}"
            };
        }

        private string GetClassDocumentation(string className)
        {
            return className switch
            {
                "Part" => "A physical object in 3D space",
                "Player" => "Represents a player in the game",
                "Script" => "Contains server-side Lua code",
                "LocalScript" => "Contains client-side Lua code",
                _ => $"Roblox class: {className}"
            };
        }

        private string GetMemberDocumentation(string className, string memberName)
        {
            return $"{className}.{memberName} - Documentation not available";
        }

        private string GetEnumDocumentation(string enumName)
        {
            return $"Roblox enumeration: {enumName}";
        }

        private string GetServiceDocumentation(string serviceName)
        {
            return serviceName switch
            {
                "Players" => "Manages player objects and connections",
                "Workspace" => "Contains all physical objects in the game",
                "Lighting" => "Controls lighting and atmosphere",
                "ReplicatedStorage" => "Storage replicated to all clients",
                "SoundService" => "Manages audio and sound effects",
                "TweenService" => "Creates smooth animations",
                "UserInputService" => "Handles user input (client-side)",
                "RunService" => "Provides frame-based events",
                _ => $"Roblox service: {serviceName}"
            };
        }

        public void Dispose()
        {
            _httpClient?.Dispose();
        }
    }
}