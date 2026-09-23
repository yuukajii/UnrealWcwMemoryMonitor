# 📊 WCW Memory Monitor Plugin ![Beta](https://img.shields.io/badge/Status-Beta-orange) ![C++](https://img.shields.io/badge/Code-C++-00599C?logo=c%2B%2B)

> **⚠️ Notice (Beta Version)**  
> **[English]**  
> This plugin is currently in **Beta**.   
> API specifications may change, and unexpected behavior could occur in certain environments. Please ensure thorough testing in a staging or test environment before deploying to production.  
> *Tested on Unreal Engine 5.6, 5.7, and 5.8.*
> ## Download
>
>Pre-built Win64 binaries for UE 5.6 / 5.7 / 5.8 are on the Releases page.
> - Source build: **Windows, PlayStation 5, Xbox Series X/S, Nintendo Switch, Nintendo Switch 2**  
> - Not loaded in **Shipping** (`TargetConfigurationDenyList`). Use Development / Test.  
> - Some OS-level counters are Windows-only. Console HUD shows the Unreal-side metrics (LLM, TextureGroup, RHI where available).
>Use the zip that matches your engine version.
>
>- Releases (binaries): https://github.com/yuukajii/UnrealWcwMemoryMonitor/releases
>- Current build: [v0.1.0 Beta](https://github.com/yuukajii/UnrealWcwMemoryMonitor/releases/tag/v0.1.0)
>
>This plugin is **not** included in Shipping builds. Tested in Editor.
>
>Manuals: see the `Doc/` folder.
>
> Note: The RHI Resource Monitoring feature incurs a relatively high CPU overhead because it internally uses RHIGetTrackedResourceStats to lock and iterate through all GPU resources. We recommend enabling it specifically when you need to conduct memory investigations.
> **[Japanese]**  
> 現在このプラグインは **ベータ版（Beta）** です。  
> APIの仕様変更や、特定の環境における予期せぬ挙動が発生する可能性があります。導入の際はテスト環境での十分な検証をお願いいたします。  
> ## Download
>
> *※UE5.6、UE5.7、UE5.8 にてテスト済みです。*  
> - バイナリ配布: https://github.com/yuukajii/UnrealWcwMemoryMonitor/releases
> - 現行ビルド: [v0.1.0 Beta](https://github.com/yuukajii/UnrealWcwMemoryMonitor/releases/tag/v0.1.0)
> - ソースからビルドすれば **Windows / PS5 / Series X|S / Switch / Switch 2** で利用可能  
> - **Shipping には入りません。** Development / Test 向けです  
> - OS 由来のカウンタは Windows のみのことがあります。コンソールでは LLM や TextureGroup などエンジン側の指標が中心です
> 
> Shipping ビルドには入りません。Editor でテスト済みです。
> 
> 操作マニュアルは `Doc/` フォルダにあります。
>
> 注意: RHIリソース監視 機能は、内部で RHIGetTrackedResourceStats を使用して全GPUリソースのロックと走査を行うため、CPUオーバーヘッドが比較的重くなっています。メモリ調査を行うタイミングに絞って有効化してご使用いただくことを推奨します。
> 
> ## Feedback & Issues  
> If you encounter any bugs, have feature requests, or want to share feedback, please feel free to open an [Issue](https://github.com/yuukajii/UnrealWcwMemoryMonitor/issues). Contributions are also welcome!  
