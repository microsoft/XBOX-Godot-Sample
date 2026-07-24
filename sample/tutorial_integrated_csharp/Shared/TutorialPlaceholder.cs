using Godot;

public partial class TutorialPlaceholder : Control
{
    [Export] public string TutorialTitle { get; set; } = "Tutorial — placeholder";
    [Export] public string TutorialSubtitle { get; set; } = "Real content lands in a subsequent commit.";
    public override void _Ready()
    {
        GetNode<Label>("Root/Title").Text = TutorialTitle;
        GetNode<Label>("Root/Subtitle").Text = TutorialSubtitle;
        GetNode<Button>("Root/Back").Pressed += () => GetTree().ChangeSceneToFile("res://Shared/tutorial_picker.tscn");
    }
}

