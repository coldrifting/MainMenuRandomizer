# Main Menu Randomizer
This SKSE plugin randomizes the main menu background by
replacing the mesh the main menu loads at runtime with a random mesh
located in the 'Data/Meshes/Interface/MainMenu' folder.
If no meshes are found there, the default main menu icon will display instead.


Since many main menu replacers use the same texture file, I've created a utility to automatically
rename the texture paths inside the mesh to use a unique path, as well as format the mesh correclty
so that they can be seen by this plugin. Simply drag a zip file containing the main menu file logo.nif
onto the exe and it will automagically create a new properly formatted zip for easy install into Mod Organizer.

# Issues
Files in archives are currently unsupported.
The golden icon on the AE Upgrade version of the game might not be properly replaced.
I don't own it, so I can't test that the patch I made for it actually works. Let me know if it does.

# Credits
This CommonLib SSE plugin example was the best I found for getting the enviornment started.
At this time though it does required chaning some vcpkg settings. See the issues in that repo for more.
https://gitlab.com/colorglass/commonlibsse-sample-plugin

@Nobody on Discord - Was very helpful for figuring out how to actually hook functions

hyralux on Nexus Mods - Inspred me to make this plugin thanks to their awesome backgrounds