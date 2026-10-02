# VCV-Rack-Tutorial

目前是準備對接新 DSP 的 **Process Bypass** 版本。

- 保留原本六個旋鈕、三個輸入、六個輸出、指示燈與面板。
- `INPUT1` 音訊直接傳到 `OUTPUT1`，不做濾波、增益或調變。
- UI 參數與其餘輸入不參與音訊處理；`OUTPUT2`–`OUTPUT6` 與指示燈維持為零。
- 已移除舊 FilterDesigner、濾波／LFO 狀態與未使用檔案；尚未加入 TPTSVF。

使用 VCV Rack 2 SDK 編譯：

```sh
make -C MyPlugin RACK_DIR=/path/to/Rack-SDK
```
