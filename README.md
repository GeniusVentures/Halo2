# Halo2
C++ library for Zero Knowledge Proofs and Rollups with Accumulation

# Build on Windows
Build Halo2 using following commands in Release configuration:

    ```
    cd Halo2/build/Windows 
    md Release
    cd Release
    cmake ../ -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release 
    cmake --build . --config Release -DCMAKE_BUILD_PARALLEL_LEVEL=8
    ```

if you are going to build and test , then use following commands

    cmake .. -G "Visual Studio 17 2022" -A x64 -DTESTING=ON -DCMAKE_BUILD_TYPE=Release
    cmake --build . --config Release -DCMAKE_BUILD_PARALLEL_LEVEL=8
    cd Halo2/build/Windows/Release
    ctest -C Release

To run sepecific test with detailed log, you can use following commands.

    ctest -C Release -R <test_name> --verbose

To run all tests and display log for failed tests, you can use following commands.

    ctest -C Release --output-on-failure

You can use Debug configuration to debug in Visual Studio.

example build commands

    cd Halo2/build/Windows
    mkdir Debug
    cd Debug

    cmake .. -G "Visual Studio 17 2022" -A x64 -DTESTING=ON  -DCMAKE_BUILD_TYPE=Debug
    cmake --build . --config Debug -DCMAKE_BUILD_PARALLEL_LEVEL=8
    cd build/Windows/Debug
    ctest -C Debug

# Build on Linux

    cd Halo2/build/Linux 
    mkdir Release
    cd Release
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j8

# Build on Linux for Android cross compile
## Preinstall
- CMake
- Android NDK Latest LTS Version (r25e) [(link)](https://developer.android.com/ndk/downloads#lts-downloads)
- ([Build thirdparty project](../README.md))
## Building
	○ export ANDROID_NDK=/path/to/android-ndk-r21e
	○ export ANDROID_TOOLCHAIN="$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin"
	○ export PATH="$ANDROID_TOOLCHAIN":"$PATH" 
* armeabi-v7a
```
○ mkdir -p build/Android/Release/Android.armeabi-v7a
○ cd build/Android/Release/Android.armeabi-v7a
○ cmake ../.. -DANDROID_ABI="armeabi-v7a" -DCMAKE_ANDROID_NDK=$ANDROID_NDK -DANDROID_TOOLCHAIN=clang
○ make -j8
```
* arm64-v8a
```
○ mkdir -p build/Android/Release/Android.arm64-v8a
○ cd build/Android/Release/Android.arm64-v8a
○ cmake ../.. -DANDROID_ABI="arm64-v8a" -DCMAKE_ANDROID_NDK=$ANDROID_NDK -DANDROID_TOOLCHAIN=clang
○ make -j8
```
* x86
```
○ mkdir -p build/Android/Release/Android.x86
○ cd build/Android/Release/Android.x86
○ cmake ../.. -DANDROID_ABI="x86" -DCMAKE_ANDROID_NDK=$ANDROID_NDK -DANDROID_TOOLCHAIN=clang
○ make -j8
```
* x86_64
```
○ mkdir build/Android/Release/Android.x86_64
○ cd build/Android/Release/Android.x86_64
○ cmake ../.. -DANDROID_ABI="x86_64" -DCMAKE_ANDROID_NDK=$ANDROID_NDK -DANDROID_TOOLCHAIN=clang
○ make -j8
```
# Build on OSX 
```
○ cd Halo2
○ mkdir -p build/OSX/Release
○ cd build/OSX/Release
○ cmake .. -DCMAKE_BUILD_TYPE=Release
○ make -j8
```
# Build on OSX for iOS cross compile 

```
○ cd Halo2
○ mkdir -p build/iOS/Release
○ cd build/iOS/Release
○ cmake .. -DCMAKE_BUILD_TYPE=Release -DTHIRDPARTY_DIR=[ABSOLUTE_PATH_TO_THIRDPARTY_BUILD_RELEASE] -DCMAKE_TOOLCHAIN_FILE=[/ABSOLUTE/PATH/TO/GeniusTokens/thirdparty/build/iOS/iOS.cmake] -DiOS_ABI=arm64-v8a -DIOS_ARCH="arm64" -DENABLE_ARC=0 -DENABLE_BITCODE=0 -DENABLE_VISIBILITY=1
○ make -j8
```
