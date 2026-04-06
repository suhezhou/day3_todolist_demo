# Todo List Demo Notes

## 项目概述

这是一个基于 Qt Quick / QML 的待办事项示例项目。

当前项目已经完成两类主要改造：

1. 数据持久化从 JSON 文件迁移到了 SQLite
2. QML 界面完成了一轮较完整的视觉和交互改造

## 技术栈

- Qt 6
- QML / Qt Quick Controls
- `QAbstractListModel`
- SQLite
- `QSqlDatabase`
- `QSqlQuery`

## 当前目录中的关键文件

- `CMakeLists.txt`
  - 配置 Qt 模块
  - 已加入 `Qt6::Sql`

- `main.cpp`
  - 应用启动入口
  - 初始化 SQLite 数据库连接
  - 创建数据表

- `todomodel.h`
  - `TodoModel` 对外接口定义
  - 供 QML 调用的增删改查方法

- `todomodel.cpp`
  - 数据加载、数据库读写、模型过滤逻辑

- `Main.qml`
  - 主界面
  - 日期切换、周视图、主题切换、任务列表、弹窗编辑

- `todoitem.h`
  - 任务结构体定义

## 数据存储说明

### 当前存储方式

任务数据现在使用 SQLite 存储，不再写入 JSON 作为主存储。

数据库相关初始化在 `main.cpp` 中完成：

- 创建 SQLite 连接
- 创建 `tasks` 表
- 创建 `app_meta` 表

### 数据表

#### `tasks`

字段：

- `id INTEGER PRIMARY KEY AUTOINCREMENT`
- `title TEXT`
- `completed INTEGER`
- `dueDate TEXT`
- `createdDate TEXT`

#### `app_meta`

用于保存一些附加状态信息，例如：

- `lastDate`

### 数据库文件位置

数据库文件位于项目目录下的：

`data/todo.db`

## 模型说明

`TodoModel` 仍然保持 `QAbstractListModel` 形式，对 QML 的接口没有大改。

### 主要角色

- `id`
- `title`
- `dueDate`
- `createdDate`
- `completed`

### QML 可调用方法

- `addItem(title, dueDate)`
- `removeItem(id)`
- `setCompleted(id, completed)`
- `setCurrentDate(date)`
- `updateTask(id, newTitle, newDueDate)`
- `hasTasksForDate(date)`

### 过滤逻辑

当前模型中的筛选已经和所选日期联动：

- `AllTasks`
  - 显示当前选中日期的全部任务

- `TodayTasks`
  - 显示当前选中日期的未完成任务

## 界面功能说明

### 顶部区域

当前顶部绿色区域承载以下信息和操作：

- 当前选中日期
- 主题切换
- 周视图日期切换
- 上一周 / 下一周
- 定位今天
- 每个日期的小圆点提示
  - 当该日期存在任务时显示

### 月历弹层

点击月历入口后可进行按月选择日期：

- 上一月 / 下一月
- 点击具体日期切换当前日期
- 当前选中日期高亮
- 有任务的日期会显示提示点

### 任务区域

任务区域包含：

- 当前日期的任务列表
- 新建按钮
- 编辑按钮
- 删除按钮
- 完成状态切换

### 弹窗

当前有两个弹窗：

- 新建任务弹窗
- 编辑任务弹窗

二者都已经和主界面保持统一风格：

- 顶部说明卡片
- 统一输入框样式
- 统一的低高度圆角按钮

## 已实现的改造点

### 数据层

- 使用 SQLite 替代 JSON 作为主存储
- 启动时自动初始化数据库
- 保留旧 JSON 数据导入逻辑
- 模型增删改查同步更新数据库

### 界面层

- 多主题切换
- 主题持久化
- 周视图日期导航
- 月历选择日期
- 选中日期与任务列表联动
- 有任务日期提示点

## 目前需要注意的点

- `Main.qml` 经过多轮迭代，建议后续保持小步修改，避免再次引入结构性混乱
- 如果继续扩展功能，建议把 QML 中的可复用按钮和卡片样式抽成独立组件
- 如果后续要做更完整的日历体验，可以继续加入：
  - 周滑动动画
  - 月历“今天”更强标识
  - 任务数量统计
  - 日期范围筛选

## 建议的下一步

如果继续迭代，比较推荐的方向：

1. 清理 `Main.qml` 中所有残留的乱码和历史样式差异
2. 将按钮、日期卡片、任务卡片提取为复用组件
3. 补充构建验证和界面回归检查
