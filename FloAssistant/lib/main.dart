import 'dart:convert';

import 'package:flutter/material.dart';
import 'package:shared_preferences/shared_preferences.dart';

void main() {
  WidgetsFlutterBinding.ensureInitialized();

  // 应用启动时创建控制器，并开始读取本地保存的笔记。
  final controller = NotepadController()..loadNotes();

  runApp(FloNotepadApp(controller: controller));
}

/// 应用根组件：负责设置主题、中文标题和首页入口。
class FloNotepadApp extends StatelessWidget {
  const FloNotepadApp({
    super.key,
    required this.controller,
  });

  final NotepadController controller;

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      title: '简单记事本',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.teal),
        useMaterial3: true,
      ),
      home: NotesHomePage(controller: controller),
    );
  }
}

/// 笔记数据模型：只保留最基础的标题、正文和更新时间。
class Note {
  const Note({
    required this.id,
    required this.title,
    required this.content,
    required this.updatedAt,
  });

  final String id;
  final String title;
  final String content;
  final DateTime updatedAt;

  /// 将 JSON 数据转换成 Note 对象，用于从本地存储恢复笔记。
  factory Note.fromJson(Map<String, dynamic> json) {
    return Note(
      id: json['id'] as String,
      title: json['title'] as String,
      content: json['content'] as String,
      updatedAt: DateTime.parse(json['updatedAt'] as String),
    );
  }

  /// 将 Note 对象转换为 JSON，用于写入本地存储。
  Map<String, dynamic> toJson() {
    return {
      'id': id,
      'title': title,
      'content': content,
      'updatedAt': updatedAt.toIso8601String(),
    };
  }

  /// 生成一份更新后的笔记，避免直接修改原对象。
  Note copyWith({
    String? title,
    String? content,
    DateTime? updatedAt,
  }) {
    return Note(
      id: id,
      title: title ?? this.title,
      content: content ?? this.content,
      updatedAt: updatedAt ?? this.updatedAt,
    );
  }
}

/// 记事本控制器：集中管理笔记列表、增删改查和本地保存。
class NotepadController extends ChangeNotifier {
  static const String _storageKey = 'flo_assistant_notes';

  final SharedPreferencesAsync _preferences = SharedPreferencesAsync();

  final List<Note> _notes = [];
  bool _isLoading = true;

  List<Note> get notes => List.unmodifiable(_notes);
  bool get isLoading => _isLoading;

  /// 从本地存储读取笔记，并按更新时间倒序排列。
  Future<void> loadNotes() async {
    final savedNotes = await _preferences.getString(_storageKey);

    _notes.clear();
    if (savedNotes != null && savedNotes.isNotEmpty) {
      final rawList = jsonDecode(savedNotes) as List<dynamic>;
      _notes.addAll(
        rawList.map(
          (item) => Note.fromJson(Map<String, dynamic>.from(item as Map)),
        ),
      );
      _sortNotes();
    }

    _isLoading = false;
    notifyListeners();
  }

  /// 新建笔记，并立即保存到本地。
  Future<void> addNote({
    required String title,
    required String content,
  }) async {
    final now = DateTime.now();
    _notes.add(
      Note(
        id: now.microsecondsSinceEpoch.toString(),
        title: _cleanTitle(title, content),
        content: content.trim(),
        updatedAt: now,
      ),
    );

    await _saveNotes();
  }

  /// 更新已有笔记，并刷新更新时间。
  Future<void> updateNote({
    required String id,
    required String title,
    required String content,
  }) async {
    final index = _notes.indexWhere((note) => note.id == id);
    if (index == -1) {
      return;
    }

    _notes[index] = _notes[index].copyWith(
      title: _cleanTitle(title, content),
      content: content.trim(),
      updatedAt: DateTime.now(),
    );

    await _saveNotes();
  }

  /// 删除指定笔记。
  Future<void> deleteNote(String id) async {
    _notes.removeWhere((note) => note.id == id);
    await _saveNotes();
  }

  /// 将内存中的笔记列表写入本地存储。
  Future<void> _saveNotes() async {
    _sortNotes();

    final encodedNotes = jsonEncode(
      _notes.map((note) => note.toJson()).toList(),
    );

    await _preferences.setString(_storageKey, encodedNotes);
    notifyListeners();
  }

  /// 保证最新编辑的笔记显示在列表最上方。
  void _sortNotes() {
    _notes.sort((a, b) => b.updatedAt.compareTo(a.updatedAt));
  }

  /// 如果用户没有填写标题，就从正文第一行生成一个标题。
  String _cleanTitle(String title, String content) {
    final trimmedTitle = title.trim();
    if (trimmedTitle.isNotEmpty) {
      return trimmedTitle;
    }

    final firstLine = content.trim().split('\n').first.trim();
    if (firstLine.isNotEmpty) {
      return firstLine.length > 20 ? '${firstLine.substring(0, 20)}...' : firstLine;
    }

    return '未命名笔记';
  }
}

/// 首页：展示笔记列表，并提供新建入口。
class NotesHomePage extends StatelessWidget {
  const NotesHomePage({
    super.key,
    required this.controller,
  });

  final NotepadController controller;

  @override
  Widget build(BuildContext context) {
    return AnimatedBuilder(
      animation: controller,
      builder: (context, _) {
        return Scaffold(
          appBar: AppBar(
            title: const Text('简单记事本'),
            centerTitle: false,
          ),
          body: _buildBody(context),
          floatingActionButton: FloatingActionButton.extended(
            onPressed: () => _openEditor(context),
            icon: const Icon(Icons.add),
            label: const Text('新建'),
          ),
        );
      },
    );
  }

  /// 根据当前状态展示加载中、空列表或笔记列表。
  Widget _buildBody(BuildContext context) {
    if (controller.isLoading) {
      return const Center(child: CircularProgressIndicator());
    }

    if (controller.notes.isEmpty) {
      return const Center(
        child: Text(
          '还没有笔记\n点击右下角“新建”开始记录',
          textAlign: TextAlign.center,
        ),
      );
    }

    return ListView.separated(
      padding: const EdgeInsets.fromLTRB(16, 12, 16, 96),
      itemCount: controller.notes.length,
      separatorBuilder: (context, index) => const SizedBox(height: 8),
      itemBuilder: (context, index) {
        final note = controller.notes[index];
        return _NoteListItem(
          note: note,
          onTap: () => _openEditor(context, note: note),
          onDelete: () => _confirmDelete(context, note),
        );
      },
    );
  }

  /// 打开编辑页；传入 note 时为编辑，不传时为新建。
  Future<void> _openEditor(BuildContext context, {Note? note}) async {
    await Navigator.of(context).push(
      MaterialPageRoute<void>(
        builder: (_) => NoteEditorPage(
          note: note,
          onSave: (title, content) async {
            if (note == null) {
              await controller.addNote(title: title, content: content);
            } else {
              await controller.updateNote(
                id: note.id,
                title: title,
                content: content,
              );
            }
          },
        ),
      ),
    );
  }

  /// 删除前弹出确认框，避免误删。
  Future<void> _confirmDelete(BuildContext context, Note note) async {
    final shouldDelete = await showDialog<bool>(
      context: context,
      builder: (context) {
        return AlertDialog(
          title: const Text('删除笔记'),
          content: Text('确定要删除“${note.title}”吗？'),
          actions: [
            TextButton(
              onPressed: () => Navigator.of(context).pop(false),
              child: const Text('取消'),
            ),
            FilledButton(
              onPressed: () => Navigator.of(context).pop(true),
              child: const Text('删除'),
            ),
          ],
        );
      },
    );

    if (shouldDelete == true) {
      await controller.deleteNote(note.id);
    }
  }
}

/// 笔记列表项：展示标题、正文摘要、更新时间和删除按钮。
class _NoteListItem extends StatelessWidget {
  const _NoteListItem({
    required this.note,
    required this.onTap,
    required this.onDelete,
  });

  final Note note;
  final VoidCallback onTap;
  final VoidCallback onDelete;

  @override
  Widget build(BuildContext context) {
    return Card(
      margin: EdgeInsets.zero,
      child: ListTile(
        onTap: onTap,
        title: Text(
          note.title,
          maxLines: 1,
          overflow: TextOverflow.ellipsis,
        ),
        subtitle: Padding(
          padding: const EdgeInsets.only(top: 6),
          child: Text(
            _buildSubtitle(),
            maxLines: 2,
            overflow: TextOverflow.ellipsis,
          ),
        ),
        trailing: IconButton(
          tooltip: '删除',
          onPressed: onDelete,
          icon: const Icon(Icons.delete_outline),
        ),
      ),
    );
  }

  /// 组合正文预览和更新时间，让列表更容易浏览。
  String _buildSubtitle() {
    final preview = note.content.isEmpty ? '无正文内容' : note.content;
    return '$preview\n${_formatTime(note.updatedAt)}';
  }
}

/// 编辑页：负责输入标题和正文，并保存笔记。
class NoteEditorPage extends StatefulWidget {
  const NoteEditorPage({
    super.key,
    required this.onSave,
    this.note,
  });

  final Note? note;
  final Future<void> Function(String title, String content) onSave;

  @override
  State<NoteEditorPage> createState() => _NoteEditorPageState();
}

class _NoteEditorPageState extends State<NoteEditorPage> {
  late final TextEditingController _titleController;
  late final TextEditingController _contentController;
  bool _isSaving = false;

  bool get _isEditing => widget.note != null;

  @override
  void initState() {
    super.initState();

    // 使用已有笔记填充输入框；新建笔记时输入框为空。
    _titleController = TextEditingController(text: widget.note?.title ?? '');
    _contentController = TextEditingController(text: widget.note?.content ?? '');
  }

  @override
  void dispose() {
    // 页面销毁时释放输入控制器，避免资源泄漏。
    _titleController.dispose();
    _contentController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(_isEditing ? '编辑笔记' : '新建笔记'),
        actions: [
          TextButton.icon(
            onPressed: _isSaving ? null : _save,
            icon: _isSaving
                ? const SizedBox(
                    width: 16,
                    height: 16,
                    child: CircularProgressIndicator(strokeWidth: 2),
                  )
                : const Icon(Icons.check),
            label: const Text('保存'),
          ),
        ],
      ),
      body: SafeArea(
        child: Padding(
          padding: const EdgeInsets.all(16),
          child: Column(
            children: [
              _buildTitleField(),
              const SizedBox(height: 12),
              Expanded(child: _buildContentField()),
            ],
          ),
        ),
      ),
    );
  }

  /// 标题输入框。
  Widget _buildTitleField() {
    return TextField(
      controller: _titleController,
      textInputAction: TextInputAction.next,
      decoration: const InputDecoration(
        labelText: '标题',
        hintText: '请输入标题',
        border: OutlineInputBorder(),
      ),
    );
  }

  /// 正文输入框，支持多行输入。
  Widget _buildContentField() {
    return TextField(
      controller: _contentController,
      expands: true,
      maxLines: null,
      minLines: null,
      textAlignVertical: TextAlignVertical.top,
      decoration: const InputDecoration(
        labelText: '正文',
        hintText: '开始记录...',
        alignLabelWithHint: true,
        border: OutlineInputBorder(),
      ),
    );
  }

  /// 校验并保存笔记。
  Future<void> _save() async {
    final title = _titleController.text;
    final content = _contentController.text;

    if (title.trim().isEmpty && content.trim().isEmpty) {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(content: Text('请先输入标题或正文')),
      );
      return;
    }

    setState(() => _isSaving = true);
    await widget.onSave(title, content);

    if (!mounted) {
      return;
    }

    setState(() => _isSaving = false);
    Navigator.of(context).pop();
  }
}

/// 将 DateTime 格式化为简单中文时间文本。
String _formatTime(DateTime time) {
  String twoDigits(int value) => value.toString().padLeft(2, '0');

  return '${time.year}-${twoDigits(time.month)}-${twoDigits(time.day)} '
      '${twoDigits(time.hour)}:${twoDigits(time.minute)}';
}
