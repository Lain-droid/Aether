using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Text.Json;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using AetherVisor.Frontend.Models;

namespace AetherVisor.Frontend.Services
{
    public class TutorialService
    {
        private readonly string _tutorialDirectory;
        private Tutorial _currentTutorial;
        private int _currentStepIndex;
        
        public ObservableCollection<Tutorial> AvailableTutorials { get; }
        public ObservableCollection<TutorialProgress> UserProgress { get; }
        
        public event EventHandler<TutorialStepEventArgs> StepChanged;
        public event EventHandler<TutorialEventArgs> TutorialCompleted;
        public event EventHandler<TutorialEventArgs> TutorialStarted;
        
        public Tutorial CurrentTutorial => _currentTutorial;
        public TutorialStep CurrentStep => _currentTutorial?.Steps?[_currentStepIndex];
        public bool IsActive => _currentTutorial != null;
        public int CurrentStepIndex => _currentStepIndex;
        public int TotalSteps => _currentTutorial?.Steps?.Count ?? 0;
        public double Progress => TotalSteps > 0 ? (double)_currentStepIndex / TotalSteps * 100 : 0;
        
        public TutorialService()
        {
            _tutorialDirectory = Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
                "Aether", "Tutorials");
            
            Directory.CreateDirectory(_tutorialDirectory);
            
            AvailableTutorials = new ObservableCollection<Tutorial>();
            UserProgress = new ObservableCollection<TutorialProgress>();
            
            LoadTutorials();
            LoadUserProgress();
        }
        
        #region Public Methods
        
        public async Task InitializeAsync()
        {
            await LoadBuiltInTutorialsAsync();
            await LoadCustomTutorialsAsync();
        }
        
        public bool StartTutorial(string tutorialId)
        {
            var tutorial = AvailableTutorials.FirstOrDefault(t => t.Id == tutorialId);
            if (tutorial == null) return false;
            
            _currentTutorial = tutorial;
            _currentStepIndex = 0;
            
            // Mark tutorial as started
            var progress = GetOrCreateProgress(tutorialId);
            progress.IsStarted = true;
            progress.StartedAt = DateTime.UtcNow;
            progress.LastAccessedAt = DateTime.UtcNow;
            
            TutorialStarted?.Invoke(this, new TutorialEventArgs(tutorial));
            StepChanged?.Invoke(this, new TutorialStepEventArgs(CurrentStep, _currentStepIndex, TotalSteps));
            
            SaveUserProgress();
            return true;
        }
        
        public bool NextStep()
        {
            if (_currentTutorial == null) return false;
            
            if (_currentStepIndex < _currentTutorial.Steps.Count - 1)
            {
                _currentStepIndex++;
                
                // Update progress
                var progress = GetOrCreateProgress(_currentTutorial.Id);
                progress.CurrentStepIndex = _currentStepIndex;
                progress.LastAccessedAt = DateTime.UtcNow;
                
                StepChanged?.Invoke(this, new TutorialStepEventArgs(CurrentStep, _currentStepIndex, TotalSteps));
                SaveUserProgress();
                return true;
            }
            else
            {
                // Tutorial completed
                CompleteTutorial();
                return false;
            }
        }
        
        public bool PreviousStep()
        {
            if (_currentTutorial == null) return false;
            
            if (_currentStepIndex > 0)
            {
                _currentStepIndex--;
                
                // Update progress
                var progress = GetOrCreateProgress(_currentTutorial.Id);
                progress.CurrentStepIndex = _currentStepIndex;
                progress.LastAccessedAt = DateTime.UtcNow;
                
                StepChanged?.Invoke(this, new TutorialStepEventArgs(CurrentStep, _currentStepIndex, TotalSteps));
                SaveUserProgress();
                return true;
            }
            
            return false;
        }
        
        public void GoToStep(int stepIndex)
        {
            if (_currentTutorial == null) return;
            if (stepIndex < 0 || stepIndex >= _currentTutorial.Steps.Count) return;
            
            _currentStepIndex = stepIndex;
            
            // Update progress
            var progress = GetOrCreateProgress(_currentTutorial.Id);
            progress.CurrentStepIndex = _currentStepIndex;
            progress.LastAccessedAt = DateTime.UtcNow;
            
            StepChanged?.Invoke(this, new TutorialStepEventArgs(CurrentStep, _currentStepIndex, TotalSteps));
            SaveUserProgress();
        }
        
        public void CompleteTutorial()
        {
            if (_currentTutorial == null) return;
            
            var progress = GetOrCreateProgress(_currentTutorial.Id);
            progress.IsCompleted = true;
            progress.CompletedAt = DateTime.UtcNow;
            progress.CompletionPercentage = 100;
            
            TutorialCompleted?.Invoke(this, new TutorialEventArgs(_currentTutorial));
            
            _currentTutorial = null;
            _currentStepIndex = 0;
            
            SaveUserProgress();
        }
        
        public void ExitTutorial()
        {
            if (_currentTutorial == null) return;
            
            var progress = GetOrCreateProgress(_currentTutorial.Id);
            progress.LastAccessedAt = DateTime.UtcNow;
            progress.CompletionPercentage = (double)_currentStepIndex / TotalSteps * 100;
            
            _currentTutorial = null;
            _currentStepIndex = 0;
            
            SaveUserProgress();
        }
        
        public List<Tutorial> GetRecommendedTutorials()
        {
            var completed = UserProgress.Where(p => p.IsCompleted).Select(p => p.TutorialId).ToList();
            var available = AvailableTutorials.Where(t => !completed.Contains(t.Id)).ToList();
            
            // Sort by difficulty and category
            return available.OrderBy(t => t.Difficulty).ThenBy(t => t.Category).Take(5).ToList();
        }
        
        public List<Tutorial> GetTutorialsByCategory(string category)
        {
            return AvailableTutorials.Where(t => t.Category == category).ToList();
        }
        
        public List<Tutorial> GetTutorialsByDifficulty(TutorialDifficulty difficulty)
        {
            return AvailableTutorials.Where(t => t.Difficulty == difficulty).ToList();
        }
        
        public Tutorial CreateCustomTutorial(string name, string description, string category)
        {
            var tutorial = new Tutorial
            {
                Id = Guid.NewGuid().ToString(),
                Name = name,
                Description = description,
                Category = category,
                Difficulty = TutorialDifficulty.Beginner,
                EstimatedDuration = TimeSpan.FromMinutes(10),
                CreatedAt = DateTime.UtcNow,
                IsCustom = true,
                Steps = new List<TutorialStep>()
            };
            
            AvailableTutorials.Add(tutorial);
            SaveCustomTutorial(tutorial);
            
            return tutorial;
        }
        
        public void AddStepToTutorial(Tutorial tutorial, TutorialStep step)
        {
            if (tutorial.Steps == null)
                tutorial.Steps = new List<TutorialStep>();
            
            step.Id = Guid.NewGuid().ToString();
            step.StepNumber = tutorial.Steps.Count + 1;
            tutorial.Steps.Add(step);
            
            if (tutorial.IsCustom)
            {
                SaveCustomTutorial(tutorial);
            }
        }
        
        public bool DeleteTutorial(string tutorialId)
        {
            var tutorial = AvailableTutorials.FirstOrDefault(t => t.Id == tutorialId);
            if (tutorial == null || !tutorial.IsCustom) return false;
            
            AvailableTutorials.Remove(tutorial);
            
            var tutorialFile = Path.Combine(_tutorialDirectory, $"{tutorialId}.json");
            if (File.Exists(tutorialFile))
            {
                File.Delete(tutorialFile);
            }
            
            return true;
        }
        
        public TutorialStats GetUserStats()
        {
            var completedTutorials = UserProgress.Count(p => p.IsCompleted);
            var totalTime = UserProgress.Where(p => p.CompletedAt.HasValue && p.StartedAt.HasValue)
                .Sum(p => (p.CompletedAt.Value - p.StartedAt.Value).TotalMinutes);
            
            return new TutorialStats
            {
                TotalTutorials = AvailableTutorials.Count,
                CompletedTutorials = completedTutorials,
                InProgressTutorials = UserProgress.Count(p => p.IsStarted && !p.IsCompleted),
                TotalTimeSpent = TimeSpan.FromMinutes(totalTime),
                CompletionRate = AvailableTutorials.Count > 0 ? 
                    (double)completedTutorials / AvailableTutorials.Count * 100 : 0,
                FavoriteCategory = GetFavoriteCategory(),
                AverageDifficulty = GetAverageDifficulty()
            };
        }
        
        public async Task ExportProgressAsync(string filePath)
        {
            var exportData = new
            {
                ExportedAt = DateTime.UtcNow,
                UserProgress = UserProgress,
                CompletedTutorials = UserProgress.Where(p => p.IsCompleted).Count(),
                Stats = GetUserStats()
            };
            
            var json = JsonSerializer.Serialize(exportData, new JsonSerializerOptions
            {
                WriteIndented = true
            });
            
            await File.WriteAllTextAsync(filePath, json);
        }
        
        #endregion
        
        #region Private Methods
        
        private void LoadTutorials()
        {
            // This will be populated by InitializeAsync
        }
        
        private async Task LoadBuiltInTutorialsAsync()
        {
            var builtInTutorials = CreateBuiltInTutorials();
            
            foreach (var tutorial in builtInTutorials)
            {
                AvailableTutorials.Add(tutorial);
            }
        }
        
        private async Task LoadCustomTutorialsAsync()
        {
            var tutorialFiles = Directory.GetFiles(_tutorialDirectory, "*.json");
            
            foreach (var file in tutorialFiles)
            {
                try
                {
                    var json = await File.ReadAllTextAsync(file);
                    var tutorial = JsonSerializer.Deserialize<Tutorial>(json);
                    
                    if (tutorial != null)
                    {
                        tutorial.IsCustom = true;
                        AvailableTutorials.Add(tutorial);
                    }
                }
                catch (Exception ex)
                {
                    System.Diagnostics.Debug.WriteLine($"Failed to load tutorial from {file}: {ex.Message}");
                }
            }
        }
        
        private List<Tutorial> CreateBuiltInTutorials()
        {
            return new List<Tutorial>
            {
                CreateGettingStartedTutorial(),
                CreateScriptingBasicsTutorial(),
                CreateAdvancedFeaturesTutorial(),
                CreateDebuggingTutorial(),
                CreatePluginDevelopmentTutorial()
            };
        }
        
        private Tutorial CreateGettingStartedTutorial()
        {
            return new Tutorial
            {
                Id = "getting-started",
                Name = "Getting Started with Aether",
                Description = "Learn the basics of using Aether for Luau scripting",
                Category = "Basics",
                Difficulty = TutorialDifficulty.Beginner,
                EstimatedDuration = TimeSpan.FromMinutes(15),
                CreatedAt = DateTime.UtcNow,
                IsCustom = false,
                Steps = new List<TutorialStep>
                {
                    new TutorialStep
                    {
                        Id = "step1",
                        StepNumber = 1,
                        Title = "Welcome to Aether",
                        Content = "Welcome to Aether! This tutorial will guide you through the basic features of the application.",
                        Type = TutorialStepType.Information,
                        ExpectedDuration = TimeSpan.FromMinutes(2)
                    },
                    new TutorialStep
                    {
                        Id = "step2",
                        StepNumber = 2,
                        Title = "Understanding the Interface",
                        Content = "Let's explore the main interface. You can see the editor in the center, file explorer on the left, and console at the bottom.",
                        Type = TutorialStepType.Guided,
                        ExpectedDuration = TimeSpan.FromMinutes(3),
                        TargetElement = "MainEditor",
                        HighlightArea = new System.Windows.Rect(100, 100, 400, 300)
                    },
                    new TutorialStep
                    {
                        Id = "step3",
                        StepNumber = 3,
                        Title = "Writing Your First Script",
                        Content = "Let's write a simple 'Hello World' script. Type the following code in the editor:",
                        Type = TutorialStepType.Interactive,
                        ExpectedDuration = TimeSpan.FromMinutes(5),
                        Code = "print(\"Hello, World!\")",
                        ExpectedAction = "Type the hello world code"
                    },
                    new TutorialStep
                    {
                        Id = "step4",
                        StepNumber = 4,
                        Title = "Executing Your Script",
                        Content = "Now let's run the script by clicking the Execute button or pressing F5.",
                        Type = TutorialStepType.Interactive,
                        ExpectedDuration = TimeSpan.FromMinutes(2),
                        TargetElement = "ExecuteButton",
                        ExpectedAction = "Click execute button"
                    },
                    new TutorialStep
                    {
                        Id = "step5",
                        StepNumber = 5,
                        Title = "Congratulations!",
                        Content = "Great job! You've successfully written and executed your first Luau script in Aether. Check the console to see the output.",
                        Type = TutorialStepType.Information,
                        ExpectedDuration = TimeSpan.FromMinutes(3)
                    }
                }
            };
        }
        
        private Tutorial CreateScriptingBasicsTutorial()
        {
            return new Tutorial
            {
                Id = "scripting-basics",
                Name = "Luau Scripting Basics",
                Description = "Learn fundamental Luau programming concepts",
                Category = "Scripting",
                Difficulty = TutorialDifficulty.Beginner,
                EstimatedDuration = TimeSpan.FromMinutes(25),
                CreatedAt = DateTime.UtcNow,
                IsCustom = false,
                Steps = new List<TutorialStep>
                {
                    new TutorialStep
                    {
                        Id = "vars1",
                        StepNumber = 1,
                        Title = "Variables and Data Types",
                        Content = "Let's start with variables. In Luau, you can create variables using the 'local' keyword:",
                        Type = TutorialStepType.Interactive,
                        Code = "local myNumber = 42\nlocal myString = \"Hello\"\nlocal myBoolean = true\n\nprint(myNumber)\nprint(myString)\nprint(myBoolean)",
                        ExpectedAction = "Create variables of different types"
                    },
                    new TutorialStep
                    {
                        Id = "funcs1",
                        StepNumber = 2,
                        Title = "Functions",
                        Content = "Functions are reusable blocks of code. Let's create a simple function:",
                        Type = TutorialStepType.Interactive,
                        Code = "local function greet(name)\n    return \"Hello, \" .. name .. \"!\"\nend\n\nprint(greet(\"World\"))",
                        ExpectedAction = "Create and call a function"
                    },
                    new TutorialStep
                    {
                        Id = "conditions1",
                        StepNumber = 3,
                        Title = "Conditional Statements",
                        Content = "Use if statements to make decisions in your code:",
                        Type = TutorialStepType.Interactive,
                        Code = "local age = 18\n\nif age >= 18 then\n    print(\"Adult\")\nelse\n    print(\"Minor\")\nend",
                        ExpectedAction = "Write conditional logic"
                    }
                }
            };
        }
        
        private Tutorial CreateAdvancedFeaturesTutorial()
        {
            return new Tutorial
            {
                Id = "advanced-features",
                Name = "Advanced Aether Features",
                Description = "Explore advanced features like plugins, themes, and debugging",
                Category = "Advanced",
                Difficulty = TutorialDifficulty.Advanced,
                EstimatedDuration = TimeSpan.FromMinutes(30),
                CreatedAt = DateTime.UtcNow,
                IsCustom = false,
                Steps = new List<TutorialStep>
                {
                    new TutorialStep
                    {
                        Id = "plugins1",
                        StepNumber = 1,
                        Title = "Installing Plugins",
                        Content = "Learn how to extend Aether with plugins from the marketplace.",
                        Type = TutorialStepType.Guided,
                        TargetElement = "PluginMenu"
                    },
                    new TutorialStep
                    {
                        Id = "debugging1",
                        StepNumber = 2,
                        Title = "Debugging Your Scripts",
                        Content = "Use the built-in debugging tools to find and fix issues in your code.",
                        Type = TutorialStepType.Interactive,
                        Code = "local function divide(a, b)\n    if b == 0 then\n        error(\"Division by zero!\")\n    end\n    return a / b\nend\n\nprint(divide(10, 2))\n-- Try: print(divide(10, 0))"
                    }
                }
            };
        }
        
        private Tutorial CreateDebuggingTutorial()
        {
            return new Tutorial
            {
                Id = "debugging-guide",
                Name = "Debugging and Error Handling",
                Description = "Master debugging techniques and error handling in Luau",
                Category = "Debugging",
                Difficulty = TutorialDifficulty.Intermediate,
                EstimatedDuration = TimeSpan.FromMinutes(20),
                CreatedAt = DateTime.UtcNow,
                IsCustom = false,
                Steps = new List<TutorialStep>
                {
                    new TutorialStep
                    {
                        Id = "debug1",
                        StepNumber = 1,
                        Title = "Understanding Error Messages",
                        Content = "Learn how to read and understand error messages in Aether.",
                        Type = TutorialStepType.Information
                    },
                    new TutorialStep
                    {
                        Id = "debug2",
                        StepNumber = 2,
                        Title = "Using Print Statements",
                        Content = "The simplest debugging technique is using print statements to track your code execution:",
                        Type = TutorialStepType.Interactive,
                        Code = "local function fibonacci(n)\n    print(\"Calculating fibonacci for:\", n)\n    if n <= 1 then\n        print(\"Base case reached\")\n        return n\n    end\n    local result = fibonacci(n-1) + fibonacci(n-2)\n    print(\"Result for\", n, \"is\", result)\n    return result\nend\n\nprint(fibonacci(5))"
                    }
                }
            };
        }
        
        private Tutorial CreatePluginDevelopmentTutorial()
        {
            return new Tutorial
            {
                Id = "plugin-development",
                Name = "Creating Aether Plugins",
                Description = "Learn how to develop custom plugins for Aether",
                Category = "Development",
                Difficulty = TutorialDifficulty.Expert,
                EstimatedDuration = TimeSpan.FromMinutes(45),
                CreatedAt = DateTime.UtcNow,
                IsCustom = false,
                Steps = new List<TutorialStep>
                {
                    new TutorialStep
                    {
                        Id = "plugin1",
                        StepNumber = 1,
                        Title = "Plugin Architecture",
                        Content = "Understand how Aether plugins work and the plugin architecture.",
                        Type = TutorialStepType.Information
                    },
                    new TutorialStep
                    {
                        Id = "plugin2",
                        StepNumber = 2,
                        Title = "Creating Your First Plugin",
                        Content = "Let's create a simple plugin that adds a new menu item.",
                        Type = TutorialStepType.Interactive,
                        Code = "-- Plugin template will be provided here"
                    }
                }
            };
        }
        
        private void LoadUserProgress()
        {
            var progressFile = Path.Combine(_tutorialDirectory, "progress.json");
            
            if (File.Exists(progressFile))
            {
                try
                {
                    var json = File.ReadAllText(progressFile);
                    var progressList = JsonSerializer.Deserialize<List<TutorialProgress>>(json);
                    
                    if (progressList != null)
                    {
                        UserProgress.Clear();
                        foreach (var progress in progressList)
                        {
                            UserProgress.Add(progress);
                        }
                    }
                }
                catch (Exception ex)
                {
                    System.Diagnostics.Debug.WriteLine($"Failed to load user progress: {ex.Message}");
                }
            }
        }
        
        private void SaveUserProgress()
        {
            try
            {
                var json = JsonSerializer.Serialize(UserProgress.ToList(), new JsonSerializerOptions
                {
                    WriteIndented = true
                });
                
                var progressFile = Path.Combine(_tutorialDirectory, "progress.json");
                File.WriteAllText(progressFile, json);
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Failed to save user progress: {ex.Message}");
            }
        }
        
        private void SaveCustomTutorial(Tutorial tutorial)
        {
            try
            {
                var json = JsonSerializer.Serialize(tutorial, new JsonSerializerOptions
                {
                    WriteIndented = true
                });
                
                var tutorialFile = Path.Combine(_tutorialDirectory, $"{tutorial.Id}.json");
                File.WriteAllText(tutorialFile, json);
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Failed to save custom tutorial: {ex.Message}");
            }
        }
        
        private TutorialProgress GetOrCreateProgress(string tutorialId)
        {
            var progress = UserProgress.FirstOrDefault(p => p.TutorialId == tutorialId);
            if (progress == null)
            {
                progress = new TutorialProgress
                {
                    TutorialId = tutorialId,
                    CurrentStepIndex = 0,
                    IsStarted = false,
                    IsCompleted = false,
                    CompletionPercentage = 0
                };
                UserProgress.Add(progress);
            }
            return progress;
        }
        
        private string GetFavoriteCategory()
        {
            var completedByCategory = UserProgress
                .Where(p => p.IsCompleted)
                .Join(AvailableTutorials, p => p.TutorialId, t => t.Id, (p, t) => t.Category)
                .GroupBy(c => c)
                .OrderByDescending(g => g.Count())
                .FirstOrDefault();
            
            return completedByCategory?.Key ?? "None";
        }
        
        private TutorialDifficulty GetAverageDifficulty()
        {
            var completedTutorials = UserProgress
                .Where(p => p.IsCompleted)
                .Join(AvailableTutorials, p => p.TutorialId, t => t.Id, (p, t) => t.Difficulty)
                .ToList();
            
            if (!completedTutorials.Any())
                return TutorialDifficulty.Beginner;
            
            var averageDifficulty = completedTutorials.Average(d => (int)d);
            return (TutorialDifficulty)Math.Round(averageDifficulty);
        }
        
        #endregion
    }
    
    #region Event Args
    
    public class TutorialEventArgs : EventArgs
    {
        public Tutorial Tutorial { get; }
        
        public TutorialEventArgs(Tutorial tutorial)
        {
            Tutorial = tutorial;
        }
    }
    
    public class TutorialStepEventArgs : EventArgs
    {
        public TutorialStep Step { get; }
        public int StepIndex { get; }
        public int TotalSteps { get; }
        
        public TutorialStepEventArgs(TutorialStep step, int stepIndex, int totalSteps)
        {
            Step = step;
            StepIndex = stepIndex;
            TotalSteps = totalSteps;
        }
    }
    
    #endregion
}