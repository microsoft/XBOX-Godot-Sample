To use the demo build the CMAKE file with ```cmake -S ./ -B ./out/build``` in ```godot-public-gdk-ext\CMakeLists.txt```

go to ```godot-public-gdk-ext/out/build/godot_public_gdk_ext.sln``` and build the solution

go to ```\godot-public-gdk-ext\sample\addons\godot_playfab\godot_playfab.gdextension```

replace line 8 and 9 with 
```
windows.debug.x86.64 = "res://bin/godot_playfab.windows.debug.x86_64.dll"
windows.release.x86.64 = "res://bin/godot_playfab.windows.release.x86_64.dll"
```

Open Godot and open ```godot-public-gdk-ext\addons\godot_playfab\demo\project.godot```