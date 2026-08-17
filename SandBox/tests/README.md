# SandBox 模块回归测试

SandBox 中的每个待测模块使用独立的 TestLayer。当前 System/World 测试位于 `SandBox/src/Tests/SystemTestLayer.*`，以后其他模块应新增自己的 `XxxTestLayer`，不要继续把测试堆入 SystemTestLayer。

## 环境变量

- `AYIN_SANDBOX_TEST_LAYER`：选择本次进程运行的测试 Layer。当前支持 `system`；未设置或设为 `none` 时不加载测试 Layer。
- `AYIN_SANDBOX_TEST_AUTO_EXIT`：非空且不为 `0` 时，测试完成后自动关闭 SandBox。

推荐一个进程只运行一个测试 Layer，使模块之间保持隔离。将来新增测试模块时，只需要：

1. 新增独立的 `XxxTestLayer`。
2. 在 `SandBox/src/SandBoxApp.cpp` 的注册表中添加名称和工厂函数。
3. 新增对应运行脚本，设置上述两个环境变量并检查该模块的 PASS 标记。

## 运行 System 测试

```powershell
pwsh -File SandBox/tests/Run-SystemTests.ps1
```

已完成构建时可以跳过编译：

```powershell
pwsh -File SandBox/tests/Run-SystemTests.ps1 -SkipBuild
```

日志写入 `SandBox/tests/results/`。脚本在编译失败、超时、程序异常退出或缺少 `SYSTEM_WORLD_TEST: PASS` 时返回非零退出码，适合接入 CI。

## SystemTestLayer 当前覆盖规则

- Pipeline 注册与默认/显式 Order
- 四个 Update Phase 的固定顺序与 Context 转发
- SceneMode 过滤、非法状态拒绝和模式切换
- OnAttach 正序、OnBegin 正序、OnEnd 逆序、OnDetach 逆序
- 重复注册、移除幂等和单次 Detach
- World 析构时兜底 End，Schedule 析构时兜底 Detach
- SystemSchedule 移动构造不丢失或重复生命周期回调
- 空 Scene 拒绝建立执行会话
- 真实 Application 帧中的 World 完整执行路径
