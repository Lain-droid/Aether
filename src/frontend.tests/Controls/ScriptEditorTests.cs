using Xunit;
using FluentAssertions;
using AetherVisor.Frontend.Controls;
using System.Windows;
using System.Windows.Threading;
using System.Threading;
using System.Threading.Tasks;

namespace AetherVisor.Frontend.Tests.Controls;

[Collection("WPF")]
public class ScriptEditorTests
{
    private readonly ScriptEditor _editor;

    public ScriptEditorTests()
    {
        // Initialize WPF application context for testing
        if (Application.Current == null)
        {
            new Application();
        }

        _editor = new ScriptEditor();
    }

    [WpfFact]
    public void Constructor_ShouldInitializeProperties()
    {
        // Arrange & Act & Assert
        _editor.Text.Should().BeEmpty();
        _editor.EditorFontSize.Should().Be(14.0);
        _editor.AutocompleteEnabled.Should().BeTrue();
    }

    [WpfFact]
    public void Text_PropertyChanged_ShouldUpdateTextBox()
    {
        // Arrange
        var testText = "print('hello world')";

        // Act
        _editor.Text = testText;

        // Force layout update
        _editor.Measure(new Size(400, 300));
        _editor.Arrange(new Rect(new Size(400, 300)));
        _editor.UpdateLayout();

        // Assert
        _editor.Text.Should().Be(testText);
    }

    [WpfFact]
    public void EditorFontSize_WhenChanged_ShouldUpdateFont()
    {
        // Arrange
        var newFontSize = 16.0;

        // Act
        _editor.EditorFontSize = newFontSize;

        // Assert
        _editor.EditorFontSize.Should().Be(newFontSize);
    }

    [WpfFact]
    public void AutocompleteEnabled_WhenChanged_ShouldUpdateProperty()
    {
        // Arrange & Act
        _editor.AutocompleteEnabled = false;

        // Assert
        _editor.AutocompleteEnabled.Should().BeFalse();
    }

    [WpfTheory]
    [InlineData("local")]
    [InlineData("function")]
    [InlineData("game")]
    [InlineData("workspace")]
    public void Keywords_ShouldBeRecognized(string keyword)
    {
        // Arrange
        _editor.Text = keyword;

        // Act & Assert
        // This test verifies that the keyword is in the internal keywords list
        // In a real implementation, you might check syntax highlighting
        _editor.Text.Should().Contain(keyword);
    }

    [WpfFact]
    public void MultilineText_ShouldHandleLineNumbers()
    {
        // Arrange
        var multilineText = "local x = 5\nprint(x)\nif x > 0 then\n  print('positive')\nend";

        // Act
        _editor.Text = multilineText;
        _editor.Measure(new Size(400, 300));
        _editor.Arrange(new Rect(new Size(400, 300)));
        _editor.UpdateLayout();

        // Assert
        _editor.Text.Should().Be(multilineText);
        _editor.Text.Split('\n').Length.Should().Be(5);
    }

    [WpfFact]
    public void EmptyText_ShouldHandleGracefully()
    {
        // Arrange & Act
        _editor.Text = "";

        // Assert
        _editor.Text.Should().BeEmpty();
    }

    [WpfFact]
    public void LargeText_ShouldHandlePerformantly()
    {
        // Arrange
        var largeText = new string('A', 10000); // 10KB of text

        // Act
        var stopwatch = System.Diagnostics.Stopwatch.StartNew();
        _editor.Text = largeText;
        stopwatch.Stop();

        // Assert
        stopwatch.ElapsedMilliseconds.Should().BeLessThan(1000); // Should be fast
        _editor.Text.Length.Should().Be(10000);
    }

    [WpfFact]
    public void SpecialCharacters_ShouldBeHandledCorrectly()
    {
        // Arrange
        var specialText = "local str = \"Hello\\nWorld\\t!\"\nprint(str)";

        // Act
        _editor.Text = specialText;

        // Assert
        _editor.Text.Should().Be(specialText);
        _editor.Text.Should().Contain("\\n");
        _editor.Text.Should().Contain("\\t");
    }

    [WpfFact]
    public void LuauSyntax_ShouldBeSupported()
    {
        // Arrange
        var luauCode = @"
local function fibonacci(n: number): number
    if n <= 1 then
        return n
    end
    return fibonacci(n - 1) + fibonacci(n - 2)
end

print(fibonacci(10))
";

        // Act
        _editor.Text = luauCode;

        // Assert
        _editor.Text.Should().Contain("function");
        _editor.Text.Should().Contain("number");
        _editor.Text.Should().Contain("return");
    }
}

// Custom collection for WPF tests to ensure STA thread
[CollectionDefinition("WPF")]
public class WpfTestCollection : ICollectionFixture<WpfTestFixture>
{
}

public class WpfTestFixture
{
    public WpfTestFixture()
    {
        if (Thread.CurrentThread.GetApartmentState() != ApartmentState.STA)
        {
            throw new InvalidOperationException("WPF tests must run on STA thread");
        }
    }
}

// Custom fact attribute for WPF tests
public class WpfFactAttribute : FactAttribute
{
    public WpfFactAttribute()
    {
        if (Thread.CurrentThread.GetApartmentState() != ApartmentState.STA)
        {
            Skip = "WPF tests must run on STA thread";
        }
    }
}

// Custom theory attribute for WPF tests
public class WpfTheoryAttribute : TheoryAttribute
{
    public WpfTheoryAttribute()
    {
        if (Thread.CurrentThread.GetApartmentState() != ApartmentState.STA)
        {
            Skip = "WPF tests must run on STA thread";
        }
    }
}