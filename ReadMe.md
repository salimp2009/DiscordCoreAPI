# DiscordCoreAPI
[![Codacy Badge](https://img.shields.io/codacy/grade/1e5ae970aed34d0b96249cdfd02099cf?color=lightblue&label=Code%20Quality&style=plastic)](https://www.codacy.com/gh/RealTimeChris/DiscordCoreAPI/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=RealTimeChris/DiscordCoreAPI&amp;utm_campaign=Badge_Grade)
[![Discord](https://img.shields.io/discord/931640556814237706?color=red&label=Discord%20Server&style=plastic)](https://discord.gg/adgMqeBuhP)
[![Build DiscordCoreAPI](https://img.shields.io/github/actions/workflow/status/RealTimeChris/DiscordCoreAPI/Construct-Vcpkg-Info.yml?color=green&style=plastic)](https://img.shields.io/github/actions/workflow/status/RealTimeChris/DiscordCoreAPI/Construct-Vcpkg-Info.yml)
![Commit Activity](https://img.shields.io/github/commit-activity/m/realtimechris/discordcoreapi?color=green&label=Commits&style=plastic)
![Lines of code](https://img.shields.io/tokei/lines/github/realtimechris/discordcoreapi?&style=plastic&label=Lines%20of%20Code)

<p align="center"><a href="http://discordcoreapi.com" target="_blank"><img src="https://github.com/RealTimeChris/DiscordCoreAPI/blob/main/Documentation/Images/TheLogo.png?raw=true" 
alt="DiscordCoreAPI WebSite" width="500"/></p>

Hello, and welcome to DiscordCoreAPI! This is a Discord bot library, written in C++, that leverages custom asynchronous [CoRoutines](https://github.com/RealTimeChris/DiscordCoreAPI/blob/main/Include/discordcoreapi/CoRoutine.hpp), as well as a home-brew set of [Https](https://github.com/RealTimeChris/DiscordCoreAPI/blob/main/Source/Https.cpp#L407),
[WebSocket](https://github.com/RealTimeChris/DiscordCoreAPI/blob/main/Source/WebSocketEntities.cpp#L379), and [Datagram](https://github.com/RealTimeChris/DiscordCoreAPI/blob/main/Source/VoiceConnection.cpp#L249) socket clients - all to deliver the utmost performance and efficiency for your bot. It uses roughly 0.1% of an Intel i7 9750h CPU to stream audio in high quality (Opus @ 48Khz, 16-bit) to a single server.

# Documentation/Examples
[Documentation/Examples](https://discordcoreapi.org/d1/d0c/a07495.html)

# Discord Server
[This is a link to the Discord server!](https://discord.gg/adgMqeBuhP)
# Bot Template
[A template for utilizing this library.](https://github.com/RealTimeChris/Bot-Template-for-DiscordCoreAPI)

# Features   

## Performant
- Thanks to utilizing [Erlang Text Format](https://github.com/RealTimeChris/DiscordCoreAPI/blob/main/Source/ErlParser.cpp) for websocket transfer, and a pool of [kept-alive HTTPS connections](https://github.com/RealTimeChris/DiscordCoreAPI/blob/main/Source/Https.cpp#L386-L399) - this library offers the snappiest responses to your interactions and user input.

## Audio-Bridge
- Connect multiple voice-channels to one-another using the [`DiscordCoreAPI::StreamInfo`](https://github.com/RealTimeChris/DiscordCoreAPI/blob/7749c58ed70dfc8e2efb1839e8a06e1bb9d75dda/Include/discordcoreapi/Utilities.hpp#L648) member of the [`DiscordCoreAPI::VoiceConnectInitData`](https://github.com/RealTimeChris/DiscordCoreAPI/blob/7749c58ed70dfc8e2efb1839e8a06e1bb9d75dda/Include/discordcoreapi/Utilities.hpp#L1316) structure, with the [`DiscordCoreAPI::VoiceConnection`](https://github.com/RealTimeChris/DiscordCoreAPI/blob/7749c58ed70dfc8e2efb1839e8a06e1bb9d75dda/Include/discordcoreapi/VoiceConnection.hpp#L135) class.

## CPU Efficient   
- It only uses about 0.1% of an Intel i7 9750h to stream audio in high quality (Opus 48Khz 16-bit Stereo) to a single server.   

## Entire Discord API Covered   
- All of the Discord API endpoints are covered in this library, including voice communication.

## Concurrent Discord API Access   
- As a result of using [custom asynchronous coroutines](https://github.com/RealTimeChris/DiscordCoreAPI/blob/main/Include/discordcoreapi/CoRoutine.hpp) along with a [thread pool](https://github.com/RealTimeChris/DiscordCoreAPI/blob/main/Source/ThreadPool.cpp#L77), this library has the ability to make fully    asynchronous/concurrent requests to the Discord API.   

## Advanced Rate-Limiting System
- Guarantees that the order in which HTTPS requests are executed is the same that they were submitted in - despite being launched across different threads, while never infracting on any of the Discord API's rate-limits and while running concurrently across all of the endpoints.
<p align="left">
	<img src="https://github.com/RealTimeChris/DiscordCoreAPI/blob/main/Documentation/Images/Rate-Limit.png?raw=true" width="700">
</p>

## Slash Commands and Buttons
<p align="left">
 	<img src="https://github.com/RealTimeChris/DiscordCoreAPI/blob/main/Documentation/Images/Slash-Commands and Buttons.png?raw=true"  width="700">   
</p>

## Select Menus
<p align="left">
 	<img src="https://github.com/RealTimeChris/DiscordCoreAPI/blob/main/Documentation/Images/Select-Menu-01.png?raw=true"  width="700">   
</p>
<p align="left">
 	<img src="https://github.com/RealTimeChris/DiscordCoreAPI/blob/main/Documentation/Images/Select-Menu-02.png?raw=true"  width="700">   
</p>
   
## User Commands   
<p align="left">
 	<img src="https://github.com/RealTimeChris/DiscordCoreAPI/blob/main/Documentation/Images/User-Command.png?raw=true"  width="700">   
</p>
   
## Message Commands   
<p align="left">
 	<img src="https://github.com/RealTimeChris/DiscordCoreAPI/blob/main/Documentation/Images/Message-Command.png?raw=true"  width="700">   
</p>
   
## Modal Text Inputs
<p align="left">
 	<img src="https://github.com/RealTimeChris/DiscordCoreAPI/blob/main/Documentation/Images/Modal-01.png?raw=true"  width="700">   
	<img src="https://github.com/RealTimeChris/DiscordCoreAPI/blob/main/Documentation/Images/Modal-02.png?raw=true"  width="700">   
</p>
   
## A Unified "Input-Event" System
- User interactions (Application Commands, Message Commands, User Commands) are accepted via the `DiscordCoreAPI::EventManager::onInputEventCreation` event.
- They can all be responded to using the `DiscordCoreAPI::InputEvents::respondToInputEventAsync()` function.
- Alternatively you can implement your own input-event handling by using the raw `DiscordCoreAPI::EventManager::onInteractionCreation` or `DiscordCoreAPI::EventManager::onMessageCreation` events.
```cpp
DiscordCoreAPI::EmbedData newEmbed{};
newEmbed.setAuthor(args.eventData.getUserName(), args.eventData.getAvatarURL());
newEmbed.setDescription("------\\n__**Sorry, but there's already something play
ing!**__\\n------");
newEmbed.setTimeStamp(getTimeAndDate());
newEmbed.setTitle("__**Playing Issue:**__");
newEmbed.setColor(discordGuild.data.borderColor);
DiscordCoreAPI::RespondToInputEventData dataPackage{ args.eventData };
dataPackage.addMessageEmbed(newEmbed);
dataPackage.setResponseType(DiscordCoreAPI::InputEventResponseType::Ephemeral_Interaction_Response);
DiscordCoreAPI::InputEventData newEvent = DiscordCoreAPI::InputEvents::respondToInputEvent(dataPackage);
DiscordCoreAPI::InputEvents::deleteInputEventResponseAsync(newEvent, 20000).get();
```

# Supported Platforms
- This library currently supports being built within Visual Studio 2022/MSVC on the Windows platform as well as g++ 11 on the Linux platform.

# Build Instructions (Full-Vcpkg)
- [Install vcpkg](https://vcpkg.io/en/getting-started.html), if need be.
- Make sure to run `vcpkg integrate install`.
- Enter within a terminal `vcpkg install discordcoreapi:x64-windows_OR_linux`.
- Set up a console project in your IDE and make sure to set the C++ standard to C++23 or later - and include `discordcoreapi/Index.hpp`.
- Build and run!

# Dependencies
- [CMake](https://cmake.org/) (Version 3.20 or greater)
- NOTE: I installed these using the [vcpkg](https://github.com/microsoft/vcpkg) installer.
- [FFMPEG](https://github.com/FFmpeg/FFmpeg) (.\\vcpkg install ffmpeg:x64-windows_OR_linux)
- [SimdJson](https://github.com/simdjson/simdjson) (.\\vcpkg install simdjson:x64-windows_OR_linux)
- [OpenSSL](https://github.com/openssl/openssl) (.\\vcpkg install openssl:x64-windows_OR_linux)
- [Opus](https://github.com/xiph/opus) (.\\vcpkg install opus:x64-windows_OR_linux)
- [Sodium](https://github.com/jedisct1/libsodium) (.\\vcpkg install libsodium:x64-windows_OR_linux)

# Build Instructions (Non-Vcpkg) - The Library   
- Install the [dependencies](https://github.com/RealTimeChris/DiscordCoreAPI/blob/main/ReadMe.md#dependencies).   
- Clone [this](https://github.com/RealTimeChris/DiscordCoreAPI) git repository into a folder.   
- Set, in CMakeLists.txt, the `_VCPKG_ROOT_DIR`, or the `FFMPEG_DIR`, `Opus_DIR`, `simdjson_DIR`, and `unofficial-sodium_DIR` paths to wherever each of the respective dependency files are located and they are as follows:   
	- FFMPEG_DIR # Set this one to the folder location of the file "FindFFMPEG.cmake".   
	- Opus_DIR # Set this one to the folder location of the file "OpusConfig.cmake".   
	- simdjson_DIR # Set this one to the folder location of the file "simdjson-config.cmake".   
	- unofficial-sodium_DIR # Set this one to the folder location of the file "unofficial-sodiumConfig.cmake".   
	- OPENSSL_ROOT_DIR # Set this one to the folder location of the include folder and library folders of OpenSSL.   
- Open a terminal inside the git repo's folder.   
- Run `cmake -S . --preset Linux_OR_Windows-Debug_OR_Release`.
- Then run `cmake --build --preset Linux_OR_Windows-Debug_OR_Release`.   
- Run within the same terminal and folder `cmake --install ./Build/Debug_OR_Release`.
- The default installation paths are: Windows = "ROOT_DRIVE:/Users/USERNAME/CMake/DiscordCoreAPI", Linux = "/home/USERNAME/CMake/DiscordCoreAPI"

# The CMAKE Package
- By running `cmake --install ./Build/Debug_OR_Release`, you will be given a cmake package, which can be used to build from this library, using other cmake projects.
- It is used by setting `DiscordCoreAPI_DIR` to wherever the DiscordCoreAPIConfig.cmake file would have been installed on your system by having run the `cmake --install` command, and then using `find_package()` on `DiscordCoreAPI`.
- When found, you will be granted the following cmake "variables"; `DiscordCoreAPI::DiscordCoreAPI` - this is the library target which can be linked to from other targets in cmake, and on Windows; `$<TARGET_RUNTIME_DLLS:DiscordCoreAPI-Bot>` - which is a list of dll files to be copied into your executable's final location after building. As well as `RELEASE_PDB_FILE_PATH`, `DEBUG_PDB_FILE_PATH`, `RELEASE_PDB_FILE_NAME`, and `DEBUG_PDB_FILE_NAME`, which are full file/directory paths/filenames to the library's PDB files.
- [Here](https://github.com/RealTimeChris/Bot-Template-for-DiscordCoreAPI/blob/main/CMakeLists.txt) and [here](https://github.com/RealTimeChris/Bot-Template-for-DiscordCoreAPI/blob/main/Executable/CMakeLists.txt) is an example of building an executable from this library with this method.

# Build Instructions (Non-Vcpkg) - The Executable
- Download the [bot template](https://github.com/RealTimeChris/Bot-Template-for-DiscordCoreAPI) or create your own with the same [imports](https://github.com/RealTimeChris/Bot-Template-For-DiscordCoreAPI/blob/main/CMakeLists.txt#L49), and set within it either the `VCPKG_ROOT_DIR`, or the `CMAKE_CONFIG_FILE_DIR`, `FFMPEG_DIR`, `Opus_DIR`, `simdjson_DIR`, and `unofficial-sodium_DIR` paths to wherever each of the respective dependency files are located and they are as follows:   
	- CMAKE_CONFIG_FILE_DIR # Set this one to the folder location of the DiscordCoreAPIConfig.cmake generated while running CMake --install.
	- FFMPEG_DIR # Set this one to the folder location of the file "FindFFMPEG.cmake".   
	- Opus_DIR # Set this one to the folder location of the file "OpusConfig.cmake".   
	- simdjson_DIR # Set this one to the folder location of the file "simdjson-config.cmake".   
	- unofficial-sodium_DIR # Set this one to the folder location of the file "unofficial-sodiumConfig.cmake".   
	- OPENSSL_ROOT_DIR # Set this one to the folder location of the include folder and library folders of OpenSSL.   
- Set up a main.cpp like [this one](https://github.com/RealTimeChris/Bot-Template-for-DiscordCoreAPI/blob/main/main.cpp), including the header `discordcoreapi/Index.hpp`.
- Run in a terminal from within the same folder as the top-level CMakeLists.txt, `cmake -S . --preset Linux_OR_Windows-Debug_OR_Release`.
- Then run `cmake --build --preset Linux_OR_Windows-Debug_OR_Release`.
- Run within the same terminal and folder `cmake --install ./Build/Debug_OR_Release`.
- The default installation paths are: Windows = "ROOT_DRIVE:/Users/USERNAME/CMake/Bot-Template-For-DiscordCoreAPI", Linux = "/home/USERNAME/CMake/Bot-Template-For-DiscordCoreAPI"
	
# Roadmap
I am currently working on getting this thing to be used by people like you! So, if you have any suggestions for the library that would make it more usable - don't hesitate to let me know! I can be
easily found on the Discord server that is linked to above! Cheers and thanks for your time.
