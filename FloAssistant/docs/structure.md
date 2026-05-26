# 项目结构说明

本文档用于记录当前 Flutter 最小记事本版本的项目结构，以及每个目录和文件的作用。

```text
FloAssistant/
  pubspec.yaml
  lib/
    main.dart
  docs/
    project-requirements.md
    structure.md
```

## 根目录

### `pubspec.yaml`

Flutter 项目的核心配置文件，作用包括：

- 定义项目名称、版本号和描述。
- 声明 Dart SDK 版本范围。
- 声明 Flutter SDK 依赖。
- 声明第三方依赖。
- 打开 Material Design 图标和组件支持。

当前使用的主要依赖：

- `flutter`：Flutter 应用基础 SDK。
- `shared_preferences`：用于在 Android 和 Windows 本地保存简单笔记数据。
- `flutter_lints`：提供 Flutter 推荐的代码规范检查。

## `lib/`

Flutter 应用源码目录。当前最小版本只有一个入口文件，后续功能变复杂后，可以再拆分为 `pages/`、`models/`、`controllers/` 等目录。

### `lib/main.dart`

应用主入口文件，包含当前最小记事本的全部业务代码。

主要结构如下：

- `main()`：应用启动入口，初始化 Flutter，并创建记事本控制器。
- `FloNotepadApp`：应用根组件，配置中文应用标题、主题和首页。
- `Note`：笔记数据模型，负责描述一条笔记的数据结构，并提供 JSON 转换方法。
- `NotepadController`：记事本控制器，负责读取笔记、保存笔记、新建笔记、更新笔记和删除笔记。
- `NotesHomePage`：首页页面，负责展示笔记列表、空状态和新建按钮。
- `_NoteListItem`：单条笔记列表项，展示标题、正文预览、更新时间和删除按钮。
- `NoteEditorPage`：笔记编辑页面，负责标题和正文输入，以及保存操作。
- `_formatTime()`：时间格式化方法，用于在列表里展示更新时间。

## `docs/`

项目文档目录，用于保存需求、设计、结构说明和后续接口文档。

### `docs/project-requirements.md`

项目需求文档，记录完整版本的产品目标、核心功能、AI 接入、同步设计、技术选型和版本规划。

### `docs/structure.md`

当前文档，记录项目结构和各文件职责。后续每次新增目录或核心模块时，都应同步更新此文件。

## 平台目录说明

当前环境没有安装 Flutter 命令行工具，因此没有自动生成 `android/` 和 `windows/` 平台目录。

安装 Flutter 后，可在项目根目录执行：

```bash
flutter create --platforms=android,windows .
flutter pub get
flutter run
```

执行后会生成以下平台目录：

- `android/`：Android 平台工程，用于构建 APK 或安装到 Android 设备。
- `windows/`：Windows 平台工程，用于构建 Windows 桌面应用。

这些目录通常由 Flutter 工具生成和维护，业务代码仍主要放在 `lib/` 目录中。
