# Button 绘制流程检查（RenderSkia vs RenderGDI）

## 1) Button 的绘制调用链（以 RenderSkia 为终点）

> 说明：`Button` 本身不重写绘制逻辑，主要继承 `LabelTemplate<Control>`，真正绘制仍走 `Control` 基类流程。

伪代码：

```cpp
Button::Paint(...)          // 继承链，不单独实现
  -> Control::Paint(...)
      PaintBkColor()
      PaintStateColors()    // normal/hot/pushed/disabled 状态色
      PaintBkImage()
      PaintStateImages()    // 状态图片（按钮最常用）
      PaintText()           // LabelImpl 文本
      PaintBorder()
      PaintFocusRect()
```

状态图片阶段（按钮常见）：

```cpp
Control::PaintStateImages()
  -> StateImageMap::PaintStateImage(kStateImageBk, state)
       -> StateImage::PaintStateImage(...)
          // 处理 hot fade、状态回退(normal/hot/pushed/disabled)
          -> Control::PaintImage(...)
             -> IRender::DrawImage(...) / DrawImageRect(...)
                 (Skia 实现: Render_Skia::DrawImage* )
```

RenderSkia 中，图片绘制的关键入口：

```cpp
Render_Skia::DrawImage(... 9宫格/平铺/source-dest/corner/fade ...)
  -> DrawSkiaImage::DrawImage(...)
     -> SkCanvas::drawImageRect(...)
```

## 2) RenderGDI 对 Button 绘制能力的缺口（相对 RenderSkia）

以下均为“与按钮视觉相关”的差异，非编译缺失：

1. **9 宫格/角区参数未实现**
   - `Render_GDI::DrawImage(...)` 的多参数版本里，`rcDestCorners/rcSourceCorners/TiledDrawParam` 没有被真正使用，直接退化为 `DrawImageRect`。
   - 影响：按钮使用 9 宫格资源（边角保持不拉伸）时，GDI 下会整体拉伸，边角易变形。

2. **平铺绘制参数未实现**
   - `TiledDrawParam` 传入后被忽略。
   - 影响：按钮纹理背景在 GDI 下不会按预期平铺，表现与 Skia 不一致。

3. **圆角裁剪退化为普通矩形裁剪**
   - `SetRoundClip(...)` 仅调用了 `SetClip(...)`。
   - 影响：按钮圆角 + 图片/背景组合时，GDI 下可能出现直角泄露。

4. **圆角描边/填充退化为普通矩形**
   - `DrawRoundRect/FillRoundRect` 在 GDI 中都转成 `DrawRect/FillRect`。
   - 影响：按钮设置 `border_round`、圆角状态底图时，视觉会“方角化”。

5. **渐变填充退化为单色填充**
   - `FillRect(dwColor, dwColor2, direction)` 在 GDI 中忽略第二颜色和方向。
   - 影响：按钮 `bkcolor + bkcolor2` 的渐变样式在 GDI 下丢失。

6. **路径绘制能力简化**
   - `DrawPath/FillPath` 仅按路径边界矩形处理。
   - 影响：若按钮样式依赖矢量路径（特殊角标/异形），GDI 结果会不一致。

7. **富文本缓存相关接口未实现**
   - `CreateDrawRichTextCache/IsValid.../Update.../Equal...` 返回 `false` 或空实现。
   - 影响：普通按钮文本通常不受影响；富文本按钮性能与一致性可能弱于 Skia。

## 3) 结论（针对“按钮”）

- **主流程是完整可走通的**：按钮在 GDI 下可以绘制出“基础文本 + 基础位图 + 基础边框”。
- **主要缺的是“高级视觉能力”**：9宫格、平铺、圆角裁剪、圆角几何、渐变等，这些正是现代按钮皮肤最常用的效果。
- 如果要做到与 RenderSkia 观感接近，优先补齐顺序建议：
  1) DrawImage 的 corner/tile 语义；
  2) 圆角 clip + RoundRect 真实现；
  3) 渐变填充；
  4) 路径绘制（按需求）。
