# Todo List Demo

一个基于 Qt 6 与 QML 的待办事项示例项目。

当前版本已经完成了从 JSON 文件存储迁移到 SQLite 数据库，并围绕日期查看、主题切换和任务管理做了界面优化。

## 界面预览

![主界面预览 1](./README.assets/87a286619507df9903b5a8a01315e4d1.png)

![主界面预览 2](./README.assets/225ddef77d6a9a6296d009d3295b2568.png)

![主界面预览 3](./README.assets/2327d6ff1cc4eaafd11020e576cb4014.png)

## 功能概览

- 使用 SQLite 持久化保存任务数据
- 基于 `QAbstractListModel` 向 QML 提供列表数据
- 支持按日期查看对应任务
- 支持任务新建、删除、编辑、完成状态切换
- 支持周视图切换日期
- 支持月历弹窗选择日期
- 支持多套界面主题切换与记忆

## 技术栈

- Qt 6
- QML / Qt Quick Controls
- SQLite
- `QSqlDatabase`
- `QSqlQuery`
- `QAbstractListModel`

## 项目结构

- `main.cpp`
  - 应用入口
  - 初始化数据库连接和数据表
- `todomodel.h`
  - 模型接口定义
- `todomodel.cpp`
  - 任务数据加载、数据库读写、过滤逻辑
- `todoitem.h`
  - 任务结构体
- `Main.qml`
  - 主界面
- `PROJECT_NOTES.md`
  - 项目实现说明和迭代记录

## 数据存储

任务数据使用 SQLite 存储，数据库文件位于：

`data/todo.db`

主要数据表：

- `tasks`
- `app_meta`

## 当前功能

### 任务管理

- 新建任务
- 编辑任务标题和日期
- 删除任务
- 勾选完成状态

### 日期相关

- 选择当前日期
- 上一周 / 下一周切换
- 快速定位到今天
- 月历弹层选择日期
- 当前日期对应任务显示

### 界面

- 多主题切换
- 主题记忆
- 顶部周视图日期导航
- 有任务日期显示提示点

## 模型接口

QML 主要通过 `todoModel` 调用以下方法：

- `addItem(title, dueDate)`
- `removeItem(id)`
- `setCompleted(id, completed)`
- `setCurrentDate(date)`
- `updateTask(id, newTitle, newDueDate)`
- `hasTasksForDate(date)`

## 过滤逻辑

当前界面里的筛选模式和所选日期联动：

- `AllTasks`
  - 显示所选日期的全部任务
- `TodayTasks`
  - 显示所选日期的未完成任务

## 构建

项目使用 CMake。

依赖模块：

- `Qt6::Core`
- `Qt6::Quick`
- `Qt6::QuickControls2`
- `Qt6::Sql`

## 说明

- 数据库存储已经替代原来的 JSON 主存储
- 保留了旧 JSON 数据导入兼容逻辑
- 运行产生的 `data/todo.db` 属于本地数据文件，不建议直接提交到仓库

## 后续可继续优化

- 清理界面文案和样式细节
- 拆分 QML 组件
- 增加测试与构建验证
- 增加任务统计与更完整的日历体验
