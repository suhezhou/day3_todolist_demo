<img width="754" height="947" alt=“b73ae2a516ebffd506192a780418cddd” src="https://github.com/user-attachments/assets/4e191fbe-5810-48a7-9778-da6c9961d437" /><img width="754" height="947" alt="86066ab56ae71fba04ca10f4aa921496" src="https://github.com/user-attachments/assets/0100dc9b-2a7c-4e55-bb24-7354971ac9d0" /><img width="754" height="947" alt="f6cc30056b9f648e52ad5b52d7f55d45" src="https://github.com/user-attachments/assets/52b22080-b120-4176-ab9c-950396193547" /><img width="754" height="947" alt="07e081511729961c0581209137c025b6" src="https://github.com/user-attachments/assets/f51b4e4a-0870-4b72-8c20-2caa6967cd77" /><img width="754" height="947" alt="494ae65476ed62950009d047f7a50474" src="https://github.com/user-attachments/assets/e97f3480-a4b9-443f-8113-3165649da65a" />
# 待办事项列表演示

一个基于 **Qt 6** 和 **QML** 的跨平台待办事项应用，支持任务管理、日期筛选、自动迁移过期任务及自动保存。

---

## ✨ 功能特性

- ✅ 添加、编辑、删除任务  
- ✅ 标记任务完成状态  
- ✅ 为任务指定截止日期  
- ✅ 按日期筛选显示任务（通过顶部日期输入框）  
- ✅ 两种视图模式：**全部任务** / **今日待办**（仅显示今日未完成的任务）  
- ✅ 自动迁移过期任务：未完成且截止日期早于上次运行日期的任务会自动顺延至当前日期  
- ✅ 数据持久化：任务列表保存为本地 JSON 文件，支持应用关闭后重新加载  
- ✅ 自动保存：每 30 秒自动保存一次，退出时也保存  

---

## 🛠️ 技术栈

- **C++17**：核心业务逻辑  
- **Qt 6**：框架与元对象系统  
- **QML / Qt Quick**：动态界面  
- **CMake**：构建系统  

---

## 📦 构建与运行

### 环境要求
- Qt 6.2 或更高版本（需包含 Qt Quick 模块）  
- CMake 3.16+  
- C++17 兼容的编译器（MSVC、GCC、Clang）  

> 💡 如果使用 **Qt Creator**，直接打开项目文件 `CMakeLists.txt`，配置好 Qt 版本后即可编译运行。

---

## 📝 使用说明

- **添加任务**：点击“＋ 添加任务”按钮，输入标题和截止日期（格式 `yyyy-MM-dd`）。  
- **编辑任务**：点击任务右侧的“编辑”按钮，可修改标题和截止日期。  
- **删除任务**：点击任务右侧的“删除”按钮。  
- **切换完成状态**：勾选任务前的复选框。  
- **按日期查看**：修改顶部的日期输入框，列表将只显示该日期下的任务。  
- **视图切换**：通过底部的 `TabBar` 可在“全部任务”和“今日待办”之间切换（今日待办仅显示未完成且截止日期为今天的任务）。

![正在上传 07e081511729961c0581209137c025b6.png…]()
![正在上传 494ae65476ed62950009d047f7a50474.png…]()
![正在上传 f6cc30056b9f648e52ad5b52d7f55d45.png…]()
![正在上传 86066ab56ae71fba04ca10f4aa921496.png…]()
![正在上传 b73ae2a516ebffd506192a780418cddd.png…]()


> 任务数据保存在系统应用数据目录下的 `tasks.json` 文件中（例如 Windows 下为 `%APPDATA%\MyCompany\TodoListDemo\tasks.json`）。

---

**尽情管理您的任务吧！**
