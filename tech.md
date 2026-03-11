# 基于 OpenGL 的实时渲染系统 - 技术梳理（当前实现状态）

## 1. 技术栈

### 1.1 语言与构建

- 语言：C++17
- 构建系统：CMake（单可执行目标 `LearnOpenGL`）
- 图形 API：OpenGL 3.3 Core

### 1.2 图形与数学基础库

- GLFW 3.4：窗口、输入、OpenGL 上下文
- GLAD：OpenGL 函数加载
- GLM：数学库（矩阵/向量/变换）
- stb_image：纹理加载

### 1.3 UI 与编辑器相关库

- Dear ImGui：编辑器 UI
- ImGuizmo：场景 Gizmo（TRS 操作器）
- imgui-node-editor：Shader Graph 节点编辑 UI
- Native File Dialog (NFD)：本地文件选择对话框

### 1.4 资产导入

- Assimp 6.0.2：模型导入（`obj/fbx/gltf`）

## 2. 构建配置要点

- 编译宏：
  - `IMGUI_DEFINE_MATH_OPERATORS`
  - `SHADERGRAPH_ENABLE_NODE_EDITOR`
- 三方子目录：
  - `Dev/glfw-3.4`
  - `Dev/imgui-1.92.5`
  - `Dev/assimp-6.0.2`
- 链接库：
  - `glfw`
  - `opengl32`
  - `imgui`
  - `assimp`

## 3. 目录结构（核心）

```text
/mnt/d/Renderer
├── CMakeLists.txt
├── include/                 # GLAD/KHR 头文件
├── src/
│   ├── main.cpp             # 程序入口 + 主循环 + 渲染流程调度
│   ├── build.h              # 场景对象定义与创建系统（BuildSystem）
│   ├── ui.h                 # ImGui 菜单、场景列表、Inspector、ShaderGraph入口
│   ├── shadergraph.h        # 节点编辑器 UI 逻辑
│   ├── raycaster.h          # 鼠标拾取（屏幕射线 + AABB）
│   ├── camera.h             # 相机控制（WASD/鼠标/滚轮）
│   ├── grid.h               # 地面网格绘制
│   ├── shader.h             # Shader 编译/链接/uniform封装
│   ├── model.h              # Assimp 模型加载
│   ├── mesh.h               # 网格与材质纹理绘制
│   ├── shader/              # 主渲染/阴影/后处理/天空盒 Shader
│   └── gridshader/          # 网格专用 Shader
└── Dev/                     # 第三方依赖源码
```

## 4. 系统分层（按当前代码实际）

### 4.1 表现层（UI Layer）

- 文件：`src/ui.h`、`src/shadergraph.h`
- 职责：菜单、对象创建入口、场景树、Inspector、ShaderGraph 面板。
- 现状：UI 直接操作 `BuildSystem.objects`，耦合较高。

### 4.2 业务层（Scene Domain）

- 文件：`src/build.h`、`src/raycaster.h`
- 职责：场景对象模型、对象生命周期管理、选中状态管理、拾取逻辑。
- 现状：`BuildSystem` 为轻量容器，缺少独立服务/命令层。

### 4.3 渲染层（Render Pipeline）

- 文件：`src/main.cpp` + `src/shader/*` + `src/gridshader/*`
- 职责：
  - 阴影 Pass（2D 深度图 + 点光源深度立方体）
  - 主场景 Pass（不透明/透明）
  - 轮廓高亮（Stencil）
  - 后处理 Pass（FBO 屏幕合成 + Gamma）
  - Skybox / Grid 辅助渲染
- 现状：Pass 编排在 `main.cpp` 中，状态切换分散。

### 4.4 资源层（Asset & Resource）

- 文件：`src/model.h`、`src/mesh.h`、`src/stb_image.cpp`
- 职责：模型导入、纹理加载、网格缓冲创建与绘制。
- 现状：纹理缓存使用全局 `textures_loaded`，生命周期管理较粗粒度。

## 5. 关键运行流程

1. 初始化：GLFW/GLAD、UI、基础网格与几何体、FBO/阴影贴图、Shader。
2. 输入处理：右键进入相机控制，WASD 移动，左键拾取对象。
3. 每帧渲染：

- 更新相机与 uniform
- 生成方向光深度图
- 生成点光源深度立方体
- 渲染不透明物体
- 排序并渲染透明 `Image`
- 绘制选中轮廓、Skybox、Grid
- FBO 输出到屏幕并叠加 UI

1. 交互编辑：ImGuizmo 操作选中对象矩阵。

## 6. 当前技术债与架构风险

- `main.cpp` 职责过载：初始化、输入、渲染、编辑逻辑混在一起。
- 全局状态偏多：相机、UI 开关、FBO 资源、渲染开关分散在全局变量。
- 模块边界不清：UI 直接操作业务数据结构，缺少中间服务层。
- 复用不足：相似渲染/状态设置代码重复，后续新增 Pass 成本高。

## 7. 推荐的分层重构方向

1. `SceneManager`：封装对象 CRUD、选中、查询与事件通知。
2. `RenderPipeline`：按 Pass 拆分为独立类（ShadowPass、OpaquePass、TransparentPass、PostProcessPass）。
3. `ResourceManager`：统一纹理/模型/Shader 缓存与释放。
4. `EditorController`：隔离 UI 事件与业务命令，降低 `ui.h` 对场景容器的直接依赖。

