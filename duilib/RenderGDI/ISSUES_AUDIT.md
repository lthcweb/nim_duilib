# RenderGDI 问题修复进展

> 说明：`DrawGDIImage` 已弃用且不参与工程构建，本清单不再将其作为修复项。

## 已修复

1. `Render_GDI::GetPixelBits()` 不再返回失效指针：改为返回 `nullptr`，并明确应使用 `ReadPixels()/WritePixels()`。
2. 裁剪状态栈不一致：`SetClip()/SetRoundClip()` 现在会保存并入栈状态，与 `ClearClip()/RestoreClip()` 成对。
3. `SetRoundClip()` 忽略 `ry`：圆角路径改为分别使用 `rx/ry`。
4. `ReadPixels()/WritePixels()` Stride 风险：改为按行拷贝，避免 `stride != width*4` 时错位。
5. `Bitmap_GDI::Init()` 外部像素生命周期风险：改为始终创建内部位图并执行像素拷贝。

## 待完善

1. 富文本测量/绘制/缓存接口已补齐可用实现（`Render_GDI_Text.cpp`），后续可继续做性能优化与与 Skia 的逐像素对齐。
2. 文档完成度描述与真实实现仍有差距（`README.md`、`IMPLEMENTATION_COMPLETE.md`）。
