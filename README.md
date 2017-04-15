# VRDemoHelper
A simple tool to make VR demonstration better and easier

## Features
1. Maximize VR game window for demonstration, both win32(not supported now) and x64 games  

2. Minimize SteamVR window, configurate SteamVR(Arcade mode, block future warning notifications)  

3. Configurable

4. Minimize performance loss

5. Hotkey support 

6. Show FPS overlay  
low priority, for steam already supports it
UPDATE: SteamVR failed to work with Arizona Sunshine, check the cause later.

7. Show warning overlap when steam menu is toogled(you can't easily tell by game screen)  
The issue can be easily solved by turning on Steam Arcade mode, system menu will be blocked.  
Offer a shortcut in VRDemoHelper should be a resonable solution for non-professional steam users.
### TODO 
1. make "pause" hotkey configuable(F8 by default)

2. localization for existing strings

3. add default implementation or create one if settings.ini doesn't exist to make configuration files optional  
   
4. hook D3D to show FPS like fraps

5. support win32 games and applications

## Files
### settings.ini  
Used to define windows(games and steam vr) to be processed

## FAQ
none for now, if you have one, email me at sunzhuoshi#gmail.com