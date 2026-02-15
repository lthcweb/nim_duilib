# GDI+ 渲染引擎实现

本目录包含了 nim_duilib 的 GDI+ 渲染引擎实现，严格对齐 Skia 渲染引擎的接口和实现结构。

## 文件结构

### 核心渲染类
- `Render_GDI.h/cpp` - 主渲染类，对应 `Render_Skia`
- `RenderFactory_GDI.h/cpp` - 渲染工厂类，对应 `RenderFactory_Skia`

### 辅助类（严格对齐 Skia 实现）
- `Font_GDI.h/cpp` - 字体类，对应 `Font_Skia`
- `Pen_GDI.h/cpp` - 画笔类，对应 `Pen_Skia`
- `Brush_GDI.h/cpp` - 画刷类，对应 `Brush_Skia`
- `Path_GDI.h/cpp` - 路径类，对应 `Path_Skia`
- `Matrix_GDI.h/cpp` - 矩阵类，对应 `Matrix_Skia`
- `Bitmap_GDI.h/cpp` - 位图类，对应 `Bitmap_Skia`
- `FontMgr_GDI.h/cpp` - 字体管理器，对应 `FontMgr_Skia`

### 文本绘制辅助类（需要实现）
- `HorizontalDrawText.h/cpp` - 横向文本绘制
- `VerticalDrawText.h/cpp` - 纵向文本绘制  
- `DrawRichText.h/cpp` - 富文本绘制
- `DrawSkiaImage.h/cpp` - 图像绘制辅助（对应 Skia 实现）

## 接口对齐说明

所有类和接口都严格对齐 Skia 实现：

### 1. 渲染器类 (Render_GDI)
```cpp
// 对齐 Render_Skia 的所有公共接口
- GetRenderType() -> RenderType::kRenderType_GDI
- GetRenderBackendType()
- GetWidth() / GetHeight()
- Resize(width, height)
- OffsetWindowOrg() / SetWindowOrg() / GetWindowOrg()
- SaveClip() / RestoreClip() / SetClip() / ClearClip()
- BitBlt() / StretchBlt() / AlphaBlend()
- DrawImage() / DrawImageRect()
- DrawLine() / DrawRect() / FillRect()
- DrawRoundRect() / FillRoundRect()
- DrawCircle() / FillCircle()
- DrawArc() / DrawPath() / FillPath()
- MeasureString() / DrawString()
- MeasureRichText() / DrawRichText()
- DrawBoxShadow()
- MakeImageSnapshot()
- ClearAlpha() / RestoreAlpha()
```

### 2. 字体类 (Font_GDI)
```cpp
// 对齐 Font_Skia
- InitFont(const UiFont& fontInfo)
- FontName() / FontSize()
- IsBold() / IsUnderline() / IsItalic() / IsStrikeOut()
- GetFont() // 返回 Gdiplus::Font* (对应 Skia 的 SkFont*)
```

### 3. 画笔类 (Pen_GDI)
```cpp
// 对齐 Pen_Skia
- SetWidth() / GetWidth()
- SetColor() / GetColor()
- SetStartCap() / GetStartCap()
- SetEndCap() / GetEndCap()
- SetDashCap() / GetDashCap()
- SetLineJoin() / GetLineJoin()
- SetDashStyle() / GetDashStyle()
- Clone()
- GetPen() // 返回 Gdiplus::Pen* (对应 Skia 的 SkPaint*)
```

### 4. 路径类 (Path_GDI)
```cpp
// 对齐 Path_Skia
- SetFillType() / GetFillType()
- AddLine() / AddLines()
- AddBezier() / AddBeziers()
- AddRect() / AddEllipse() / AddArc()
- AddPolygon()
- Transform(IMatrix*)
- GetBounds()
- Close() / Reset() / Clone()
- GetPath() // 返回 Gdiplus::GraphicsPath* (对应 Skia 的 SkPath*)
```

### 5. 位图类 (Bitmap_GDI)
```cpp
// 对齐 Bitmap_Skia
- Init(width, height, pixelBits, scale, alphaType)
- GetWidth() / GetHeight() / GetSize()
- LockPixelBits() / UnLockPixelBits()
- Clone()
- GetBitmap() // 返回 Gdiplus::Bitmap* (对应 Skia 的 SkBitmap*)
```

## 实现要点

### 1. 坐标系统
- GDI+ 和 Skia 都使用左上角为原点的坐标系
- 通过 `m_pPointOrg` 实现坐标偏移，对齐 Skia 的 `m_pSkPointOrg`

### 2. 状态管理
- 使用 `Graphics::Save()` 和 `Graphics::Restore()` 管理绘图状态
- `m_stateStack` 保存状态栈，对齐 Skia 实现

### 3. 裁剪区域
- 使用 `Graphics::SetClip()` 实现矩形和圆角矩形裁剪
- 支持 Intersect 和 Exclude 模式

### 4. 透明度混合
- 使用 `ColorMatrix` 实现透明度调整
- 使用 `CompositingMode` 实现不同的混合模式

### 5. 文本渲染
- 支持横向和纵向文本绘制
- 支持富文本（多种字体、颜色、背景色）
- 支持文本对齐、省略号、自动换行等

### 6. 图像绘制
- 支持九宫格绘制（对齐 Skia 实现）
- 支持平铺绘制（横向、纵向）
- 支持图像变换（旋转、缩放）

## 编译配置

### 宏定义
```cpp
#define DUILIB_BUILD_FOR_WIN  // Windows 平台编译
```

### 依赖库
```cpp
#pragma comment(lib, "gdiplus.lib")
```

### 初始化 GDI+
在 `RenderFactory_GDI` 构造函数中初始化：
```cpp
Gdiplus::GdiplusStartupInput gdiplusStartupInput;
Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, nullptr);
```

在析构函数中清理：
```cpp
Gdiplus::GdiplusShutdown(m_gdiplusToken);
```

## 使用示例

### 创建渲染器
```cpp
// 创建工厂
RenderFactory_GDI* pFactory = new RenderFactory_GDI();

// 创建 DPI 接口
IRenderDpiPtr spDpi = std::make_shared<RenderDpi>();

// 创建渲染器
IRender* pRender = pFactory->CreateRender(spDpi, hWnd);

// 设置大小
pRender->Resize(800, 600);
```

### 绘制图形
```cpp
// 填充矩形
UiRect rc(10, 10, 100, 100);
pRender->FillRect(rc, UiColor(255, 0, 0, 0)); // 黑色

// 绘制圆角矩形
pRender->DrawRoundRect(rc, 5, 5, UiColor(255, 255, 0, 0), 2); // 红色边框

// 绘制文本
DrawStringParam param;
param.textRect = UiRect(10, 110, 200, 130);
param.dwTextColor = UiColor(255, 0, 0, 255); // 蓝色文本
param.pFont = pFactory->CreateIFont();
param.uFormat = TEXT_LEFT | TEXT_VCENTER | TEXT_SINGLELINE;
pRender->DrawString(_T("Hello World!"), param);
```

### 绘制图片（九宫格）
```cpp
// 加载位图
IBitmap* pBitmap = pFactory->CreateBitmap();
pBitmap->Init(imageWidth, imageHeight, pPixelData);

// 九宫格绘制
UiRect rcDest(0, 0, 300, 200);
UiRect rcDestCorners(10, 10, 10, 10);  // 边角大小
UiRect rcSource(0, 0, imageWidth, imageHeight);
UiRect rcSourceCorners(10, 10, 10, 10);

pRender->DrawImage(rcPaint, pBitmap, 
                  rcDest, rcDestCorners,
                  rcSource, rcSourceCorners);
```

## 性能优化

### 1. 渲染质量设置
```cpp
// 高质量模式（默认）
m_pGraphics->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
m_pGraphics->SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);
m_pGraphics->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

// 高性能模式
m_pGraphics->SetSmoothingMode(Gdiplus::SmoothingModeHighSpeed);
m_pGraphics->SetTextRenderingHint(Gdiplus::TextRenderingHintSystemDefault);
m_pGraphics->SetInterpolationMode(Gdiplus::InterpolationModeLowQuality);
```

### 2. 批量绘制
- 尽量减少状态切换
- 使用裁剪区域避免不必要的绘制
- 缓存字体、画笔等对象

### 3. 内存管理
- 及时释放临时 Bitmap
- 使用 Lock/Unlock 访问像素数据
- 避免频繁创建销毁 Graphics 对象

## 与 Skia 实现的差异

### 1. 文本渲染
- Skia 使用 SkTextBox 处理文本布局
- GDI+ 使用 StringFormat 和 MeasureString
- 需要自行实现文本换行、省略号等逻辑

### 2. 阴影效果
- Skia 使用 ImageFilter 实现阴影
- GDI+ 需要手动实现高斯模糊或使用近似算法

### 3. 路径填充模式
- Skia: kEvenOdd, kWinding, kInverseEvenOdd, kInverseWinding
- GDI+: FillModeAlternate, FillModeWinding
- 需要转换映射

### 4. 性能
- Skia 支持 GPU 加速（OpenGL/Vulkan）
- GDI+ 仅支持 CPU 渲染
- 大规模图形操作 Skia 性能更优

## 测试建议

### 单元测试
1. 测试基本图形绘制
2. 测试文本渲染（各种对齐方式）
3. 测试图片绘制（九宫格、平铺）
4. 测试路径绘制
5. 测试坐标变换
6. 测试裁剪区域

### 性能测试
1. 大量图形绘制
2. 复杂路径填充
3. 文本密集渲染
4. 图片缩放性能

### 兼容性测试
1. Windows 7/8/10/11
2. 不同 DPI 设置
3. 不同显示器配置

## 待完成的文件

以下文件需要继续实现（参考 Skia 对应文件）：

1. `Font_GDI.cpp` - 字体类实现
2. `Pen_GDI.cpp` - 画笔类实现
3. `Brush_GDI.cpp` - 画刷类实现
4. `Path_GDI.cpp` - 路径类实现
5. `Matrix_GDI.cpp` - 矩阵类实现
6. `Bitmap_GDI.cpp` - 位图类实现
7. `FontMgr_GDI.cpp` - 字体管理器实现
8. `RenderFactory_GDI.cpp` - 工厂类实现
9. `Render_GDI.cpp` - 完整实现（当前只有部分）
10. `HorizontalDrawText.cpp` - 横向文本绘制
11. `VerticalDrawText.cpp` - 纵向文本绘制
12. `DrawRichText.cpp` - 富文本绘制

## 参考资料

### GDI+ 文档
- [GDI+ Reference](https://docs.microsoft.com/en-us/windows/win32/gdiplus/-gdiplus-gdi-start)
- [Graphics Class](https://docs.microsoft.com/en-us/windows/win32/api/gdiplusgraphics/nl-gdiplusgraphics-graphics)
- [Pen Class](https://docs.microsoft.com/en-us/windows/win32/api/gdipluspen/nl-gdipluspen-pen)
- [Bitmap Class](https://docs.microsoft.com/en-us/windows/win32/api/gdiplusheaders/nl-gdiplusheaders-bitmap)

### Skia 参考实现
- `duilib/RenderSkia/Render_Skia.h`
- `duilib/RenderSkia/Render_Skia.cpp`
- `duilib/RenderSkia/Font_Skia.cpp`
- `duilib/RenderSkia/DrawRichText.cpp`

## 许可证

遵循 nim_duilib 项目的许可证（MIT License）

## 贡献

欢迎贡献代码和改进建议！请确保：
1. 代码风格与项目一致
2. 接口严格对齐 Skia 实现
3. 添加必要的注释和文档
4. 通过相关测试

## 联系方式

如有问题请在 GitHub Issues 中提出。
