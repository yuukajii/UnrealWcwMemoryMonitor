# 📊 WCW Memory Monitor Plugin ![Beta](https://img.shields.io/badge/Status-Beta-orange) ![C++](https://img.shields.io/badge/Code-C++-00599C?logo=c%2B%2B) ![Version](https://img.shields.io/badge/Version-v0.2.0-blue)

WinterCrownWORKS real-time memory & VRAM overlay for Unreal Engine. Slate HUD in the viewport: physical RAM, PC VRAM, texture streaming pool, TextureGroup resident size, selected LLM tags, optional RHI buckets, and a build stamp for QA captures.

> **⚠️ Notice (Beta)**
>
> **[English]**
> This plugin is **Beta**. APIs may change. Test in a staging / Development build before production use.
> Tested on Unreal Engine **5.6 / 5.7 / 5.8**.
>
> **[Japanese]**
> 現在このプラグインは **ベータ版** です。API の変更や環境依存の挙動があり得ます。本番前に Development / テスト環境で検証してください。
> **UE 5.6 / 5.7 / 5.8** でテスト済みです。

## Download

Pre-built Win64 binaries for UE 5.6 / 5.7 / 5.8 are on the Releases page. Use the zip that matches your engine version.

- Releases: https://github.com/yuukajii/UnrealWcwMemoryMonitor/releases
- Current: [v0.2.0](https://github.com/yuukajii/UnrealWcwMemoryMonitor/releases/tag/v0.2.0)
- Source build: **Windows, PlayStation 5, Xbox Series X/S, Nintendo Switch, Nintendo Switch 2**
- Not loaded in **Shipping** (`TargetConfigurationDenyList`). Use Development / Test.
- Some OS-level counters are Windows-only. On console the HUD shows Unreal-side metrics (LLM, TextureGroup, RHI where available).
- Manuals: `Doc/` (`WCW Memory Monitor - User Manual_EN.pdf` / `_jp.pdf`)

Shipping には入りません。Editor / Development でテスト済みです。操作マニュアルは `Doc/` にあります。

## v0.2.0

What landed in this drop (on top of the 0.1 HUD):

- Yellow **build stamp** on the HUD: `Build: 0.0.1 | CL 12345 | UE 5.8.0`
- **Quality Presets** panel on the overlay (AntiAliasing, ViewDistance, Shadow, GI, Reflection, PostProcess, Texture, Effects, Foliage, Shading) so a QA video shows which scalability level produced the numbers
- Reads `<Project>/Config/Wcw.ini` `[Wcw.Build]` first (`TitleVersion`, then `ProjectVersion`, plus `Changelist`)
- If that file is missing: `DefaultGame.ini` `[/Script/EngineSettings.GeneralProjectSettings]` `ProjectVersion`, then `1.0.0.0`
- Engine patch always from `FEngineVersion::Current()`
- `Wcw.MemoryMonitor.EnableRHI 0|1` documented as a session switch (the walk is expensive)
- Texture / LLM **peak hold** documented as a QA-video ↔ log clock (`Peak: … MB @ HH:MM:SS`)

Not in 0.2.0: Lumen / Nanite budget lines, RHI peak-hold.

## Console

| Command | |
| --- | --- |
| `Wcw.MemoryMonitor.ToggleUI` | Show / hide the overlay |
| `Wcw.MemoryMonitor.FontScale 1.2` | Multiply Project Settings font size |
| `Wcw.MemoryMonitor.EnableRHI 0\|1` | `1` walks tracked RHI resources (Lumen / Nanite / Shadow / VT / …). `0` skips the walk; those rows print `Use: --- MB (Disabled)` |

Launch with `-llm` if you need real LLM group numbers. PIE heaps are not device heaps — sign off budgets on Standalone or a packaged Development build.

### RHI cost

`EnableRHI` gates `RHIGetTrackedResourceStats`. That call locks and walks tracked GPU resources. On a dense scene (CitySample-class) frame time can jump from ~45 ms to 100–200 ms on the walk frames. Turn it on when you are investigating memory; turn it off when you only need Texture / LLM / the build stamp. Needs `RHI_ENABLE_RESOURCE_INFO` (Development / Test). Shipping often has nothing to walk.

### Peak hold (Texture / LLM)

When a TextureGroup or LLM row crosses its `BudgetMB`, the HUD keeps `Peak: 1753.1 MB @ 21:41:06` for about five seconds. Spikes last a frame; without the hold a QA video looks “fine”. Pause the video:

1. Yellow stamp → which TitleVersion / CL
2. Peak clock → `FDateTime::Now()`, same clock as log / memreport lines

RHI rows do not use this hold in 0.2.0 (no signed-off budget on Lumen / Nanite).

## Wcw.ini (project file, not inside the plugin)

Put this on the **game** project: `Config/Wcw.ini`. CI / a PreBuild step should write it **before** cook. The plugin only reads it.

```ini
; Path: <Project>/Config/Wcw.ini
[Wcw.Build]
TitleVersion=0.9.3
Changelist=88421
Stamp=2026-09-28T16:00:00+09:00
Dirty=0
```

Resolution for the version token:

1. `Wcw.ini` `[Wcw.Build]` `TitleVersion`
2. Same section, `ProjectVersion` (alias)
3. `DefaultGame.ini` `[/Script/EngineSettings.GeneralProjectSettings]` `ProjectVersion`
4. `1.0.0.0`

Example PreBuild / CI:

```bash
CL=$(git rev-list --count HEAD)   # or P4 changelist
VER=${TITLE_VERSION:-0.9.3}
cat > "$PROJECT_DIR/Config/Wcw.ini" <<EOF
[Wcw.Build]
TitleVersion=$VER
Changelist=$CL
Stamp=$(date -Iseconds)
Dirty=0
EOF
```

A custom ini that is not `Default*.ini` can be dropped by cook filters. If the packaged game has no `Config/Wcw.ini`, the HUD silently uses ProjectVersion / `1.0.0.0`. That is fallback, not a widget bug.

## Install

```
YourProject/
  YourProject.uproject
  Config/
    Wcw.ini                 ← optional stamp (CI)
  Plugins/
    WcwMemoryMonitorModule/
      WcwMemoryMonitorModule.uplugin
      Source/
      Resources/
```

Do not rename the plugin folder. Enable **WCW Memory Monitor** / `WcwMemoryMonitorModule` under Edit → Plugins. Settings: Edit → Project Settings → Plugins → WCW Memory Monitor (persisted to `DefaultEngine.ini`).

Settings walkthrough: https://www.youtube.com/watch?v=o5P6D3sfGsk

## 日本語メモ

- 黄文字は `MakeBuildLabel()`。`Config/Wcw.ini` があれば優先、無ければ `ProjectVersion`。
- Quality Presets（AA / ViewDistance / Shadow など）を同じ HUD に出す。動画だけで品質設定とメモリを突き合わせられる。
- `Wcw.ini` はゲームプロジェクト側。プラグイン同梱ではない。クック前に CI が書く。
- `EnableRHI 0|1` は有効 / 無効のスイッチ。既定オフ、という意味ではない。
- ピークホールドは Texture / LLM のみ。QA の動画とログを同じ時計で結ぶため。
- Lumen / Nanite に Texture と同じ BudgetMB は 0.2.0 では持たない（シーン依存が大きすぎる）。

## License

MIT. Use, modify, redistribute.

Repository: https://github.com/yuukajii/UnrealWcwMemoryMonitor

## Feedback & Issues

Bugs, requests, or notes: open an [Issue](https://github.com/yuukajii/UnrealWcwMemoryMonitor/issues). Contributions welcome.

© 2026 WinterCrownWORKS
