using Xunit;
using FluentAssertions;
using Moq;
using AetherVisor.Frontend.ViewModels;
using AetherVisor.Frontend.Services;
using AetherVisor.Frontend.Models;
using System.Threading.Tasks;
using System.IO;
using System.Linq;

namespace AetherVisor.Frontend.Tests.ViewModels;

public class MainViewModelTests : IDisposable
{
    private readonly Mock<IIpcClientService> _mockIpcService;
    private readonly MainViewModel _viewModel;
    private readonly string _testDirectory;

    public MainViewModelTests()
    {
        _mockIpcService = new Mock<IIpcClientService>();
        _testDirectory = Path.Combine(Path.GetTempPath(), "AetherTests", Guid.NewGuid().ToString());
        Directory.CreateDirectory(_testDirectory);
        
        // Create test files
        File.WriteAllText(Path.Combine(_testDirectory, "test1.lua"), "print('test1')");
        File.WriteAllText(Path.Combine(_testDirectory, "test2.luau"), "print('test2')");
        
        _viewModel = new MainViewModel(_mockIpcService.Object, _testDirectory);
    }

    public void Dispose()
    {
        if (Directory.Exists(_testDirectory))
        {
            Directory.Delete(_testDirectory, true);
        }
    }

    [Fact]
    public void Constructor_ShouldInitializeProperties()
    {
        // Arrange & Act & Assert
        _viewModel.ScriptText.Should().NotBeNullOrEmpty();
        _viewModel.ConsoleOutput.Should().NotBeEmpty();
        _viewModel.Files.Should().NotBeEmpty();
        _viewModel.Tabs.Should().BeEmpty();
        _viewModel.ExecuteScriptCommand.Should().NotBeNull();
        _viewModel.InjectCommand.Should().NotBeNull();
    }

    [Fact]
    public void ScriptText_PropertyChanged_ShouldRaiseEvent()
    {
        // Arrange
        var propertyChangedRaised = false;
        _viewModel.PropertyChanged += (s, e) =>
        {
            if (e.PropertyName == nameof(MainViewModel.ScriptText))
                propertyChangedRaised = true;
        };

        // Act
        _viewModel.ScriptText = "test script";

        // Assert
        propertyChangedRaised.Should().BeTrue();
        _viewModel.ScriptText.Should().Be("test script");
    }

    [Fact]
    public void CanExecuteScript_WithEmptyScript_ShouldReturnFalse()
    {
        // Arrange
        _viewModel.ScriptText = "";

        // Act
        var canExecute = _viewModel.ExecuteScriptCommand.CanExecute(null);

        // Assert
        canExecute.Should().BeFalse();
    }

    [Fact]
    public void CanExecuteScript_WithValidScript_ShouldReturnTrue()
    {
        // Arrange
        _viewModel.ScriptText = "print('hello')";

        // Act
        var canExecute = _viewModel.ExecuteScriptCommand.CanExecute(null);

        // Assert
        canExecute.Should().BeTrue();
    }

    [Fact]
    public async Task ExecuteScript_ShouldCallIpcService()
    {
        // Arrange
        _viewModel.ScriptText = "print('test')";
        _mockIpcService.Setup(x => x.ConnectAsync()).Returns(Task.CompletedTask);
        _mockIpcService.Setup(x => x.SendScriptAsync(It.IsAny<string>())).Returns(Task.CompletedTask);

        // Act
        _viewModel.ExecuteScriptCommand.Execute(null);
        await Task.Delay(100); // Wait for async operation

        // Assert
        _mockIpcService.Verify(x => x.ConnectAsync(), Times.Once);
        _mockIpcService.Verify(x => x.SendScriptAsync("print('test')"), Times.Once);
    }

    [Fact]
    public async Task ExecuteScript_WhenIpcFails_ShouldAddErrorToConsole()
    {
        // Arrange
        _viewModel.ScriptText = "print('test')";
        _mockIpcService.Setup(x => x.ConnectAsync()).ThrowsAsync(new Exception("Connection failed"));

        var initialConsoleCount = _viewModel.ConsoleOutput.Count;

        // Act
        _viewModel.ExecuteScriptCommand.Execute(null);
        await Task.Delay(100); // Wait for async operation

        // Assert
        _viewModel.ConsoleOutput.Count.Should().BeGreaterThan(initialConsoleCount);
        _viewModel.ConsoleOutput.Should().Contain(x => x.Contains("IPC error"));
    }

    [Fact]
    public void OpenFile_WithValidFile_ShouldCreateTab()
    {
        // Arrange
        var testFile = Path.Combine(_testDirectory, "test1.lua");
        var fileNode = new FileNode 
        { 
            Name = "test1.lua", 
            FullPath = testFile, 
            IsDirectory = false 
        };

        // Act
        _viewModel.OpenFile(fileNode);

        // Assert
        _viewModel.Tabs.Should().HaveCount(1);
        _viewModel.ActiveTab.Should().NotBeNull();
        _viewModel.ActiveTab.FileName.Should().Be("test1.lua");
        _viewModel.ActiveTab.Content.Should().Be("print('test1')");
        _viewModel.ScriptText.Should().Be("print('test1')");
    }

    [Fact]
    public void OpenFile_WithDirectory_ShouldNotCreateTab()
    {
        // Arrange
        var fileNode = new FileNode 
        { 
            Name = "TestDir", 
            FullPath = _testDirectory, 
            IsDirectory = true 
        };

        // Act
        _viewModel.OpenFile(fileNode);

        // Assert
        _viewModel.Tabs.Should().BeEmpty();
        _viewModel.ActiveTab.Should().BeNull();
    }

    [Fact]
    public void OpenFile_WithNonExistentFile_ShouldAddErrorToConsole()
    {
        // Arrange
        var fileNode = new FileNode 
        { 
            Name = "nonexistent.lua", 
            FullPath = Path.Combine(_testDirectory, "nonexistent.lua"), 
            IsDirectory = false 
        };

        var initialConsoleCount = _viewModel.ConsoleOutput.Count;

        // Act
        _viewModel.OpenFile(fileNode);

        // Assert
        _viewModel.ConsoleOutput.Count.Should().BeGreaterThan(initialConsoleCount);
        _viewModel.ConsoleOutput.Should().Contain(x => x.Contains("Open error"));
    }

    [Fact]
    public void ActiveTab_WhenChanged_ShouldUpdateScriptText()
    {
        // Arrange
        var tab1 = new OpenDocument 
        { 
            FileName = "test1.lua", 
            Content = "print('test1')", 
            FilePath = "test1.lua" 
        };
        var tab2 = new OpenDocument 
        { 
            FileName = "test2.lua", 
            Content = "print('test2')", 
            FilePath = "test2.lua" 
        };

        _viewModel.Tabs.Add(tab1);
        _viewModel.Tabs.Add(tab2);

        // Act
        _viewModel.ActiveTab = tab2;

        // Assert
        _viewModel.ScriptText.Should().Be("print('test2')");
    }

    [Fact]
    public void Files_ShouldLoadFromDirectory()
    {
        // Arrange & Act (constructor already loads files)

        // Assert
        _viewModel.Files.Should().HaveCount(1); // Root directory
        var rootNode = _viewModel.Files.First();
        rootNode.Children.Should().HaveCountGreaterOrEqualTo(2); // test1.lua and test2.luau
        
        var luaFiles = rootNode.Children.Where(c => c.Name.EndsWith(".lua") || c.Name.EndsWith(".luau"));
        luaFiles.Should().HaveCountGreaterOrEqualTo(2);
    }

    [Theory]
    [InlineData("")]
    [InlineData("   ")]
    [InlineData("\t\n")]
    public void CanExecuteScript_WithWhitespaceOnly_ShouldReturnFalse(string scriptText)
    {
        // Arrange
        _viewModel.ScriptText = scriptText;

        // Act
        var canExecute = _viewModel.ExecuteScriptCommand.CanExecute(null);

        // Assert
        canExecute.Should().BeFalse();
    }

    [Theory]
    [InlineData("print('hello')")]
    [InlineData("local x = 5")]
    [InlineData("game:GetService('Players')")]
    public void CanExecuteScript_WithValidScript_ShouldReturnTrue(string scriptText)
    {
        // Arrange
        _viewModel.ScriptText = scriptText;

        // Act
        var canExecute = _viewModel.ExecuteScriptCommand.CanExecute(null);

        // Assert
        canExecute.Should().BeTrue();
    }
}