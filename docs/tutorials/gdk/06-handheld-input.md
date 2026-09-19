# GDK 6 — Windows handheld text input

This scene demonstrates two baseline Windows capabilities exposed through the
GDK addon. Neither call requires `GDK.initialize()` or a signed-in Xbox user:

- `GDK.system.is_handheld()` reports whether Windows identifies the device as
  a gaming handheld.
- `GDK.game_ui.show_virtual_keyboard()` and `hide_virtual_keyboard()` request
  the Windows input pane for the currently focused Godot text control.

## Try it

Open `sample/tutorial_gdk/g06_handheld_input.tscn`, focus the `LineEdit`, and
select **Show virtual keyboard**. The scene restores focus to the `LineEdit`
before making the request so keyboard input continues through Godot's normal
text-input path.

Both keyboard methods return an `XboxResult`. When the call succeeds,
`result.data` is the Boolean returned by Windows: `true` means the request was
accepted. A `false` value is not a native failure; the Windows API is
best-effort and can decline when the title is not foreground or Windows prefers
an attached hardware keyboard.

Use `show_text_entry_async()` instead when the game wants a separate modal GDK
text-entry UI that returns the complete submitted string.
